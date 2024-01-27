/*
 * i2sAudio.c
 *
 *  Created on: Mar 21, 2021
 *      Author: David
 */
#include "i2sAudio.h"
#include "files.h"
#include "mp3Decoder.h"
#include "wavDecoder.h"

system_t systemStatus;
static I2S_HandleTypeDef *i2sHandle;
extern FIL USBHFile;
extern FATFS USBHFatFS;
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream0;

void initAudio(I2S_HandleTypeDef *hi2s){
  i2sHandle = hi2s;
  systemStatus.file = &USBHFile;
  systemStatus.fat = &USBHFatFS;
}


void handleAudio(void){
  if(systemStatus.updateBuffer==1){
    systemStatus.updateBuffer=0;
    handleBuffer(0);                          // Refill first half
  }
  else if(systemStatus.updateBuffer==2){
    systemStatus.updateBuffer=0;
    handleBuffer(systemStatus.PCMSamples/2);  // Refill last half
  }
  if(systemStatus.underflow){
    systemStatus.underflow=0;
    iprintf("AUDIO: Buffer underflow!\n");
  }
	if(systemStatus.audioStatus != audio_play && systemStatus.driveStatus == drive_ready && systemStatus.fileStatus == file_opened){
		AudioStart();
	}
	if(systemStatus.audioStatus == audio_play){
		static uint32_t btnTimer=0;
		static char btnPrev=1;
		char btnNow = HAL_GPIO_ReadPin(btn_GPIO_Port, btn_Pin);
		if(!btnNow && btnPrev && HAL_GetTick()-btnTimer > 200){     // If button pressed for more than 200ms, force end of file to skip it and play next one
		  AudioStop();
			systemStatus.fileStatus = file_end;
		}
		if(btnNow != btnPrev){                                      // If button reading different than last reading, reset the timer and store value
			btnNow = btnPrev;
			btnTimer = HAL_GetTick();
		}
	}
}



void AudioStart(void){
  if( systemStatus.PCMbuffer || systemStatus.decoder ){          // If not empty, something went out of control
    AudioStop();                                                  // Stop and empty everything
  }

  switch((uint8_t)systemStatus.filetype){

    case file_mp3:
      systemStatus.startDecoder = mp3Start;
      systemStatus.stopDecoder = mp3Stop;
      break;

    case file_wav:
      systemStatus.startDecoder = wavStart;
      systemStatus.stopDecoder = wavStop;
      break;

    default:
      iprintf("AUDIO: Skipping unknown filetype!\n");
      AudioStop();
      return;
  }
  if(!systemStatus.startDecoder()){
    AudioStop();
    return;
  }
  systemStatus.remainingSamples = systemStatus.fillBuffer(systemStatus.PCMbuffer, systemStatus.PCMSamples);   // Fill the entire buffer with data if no error

	if(systemStatus.remainingSamples == 0 ){							        // If zero bytes transferred or error
    AudioStop();                                                // Stop
    return;
	}
	else{
		if(systemStatus.remainingSamples < systemStatus.PCMSamples){					          // If less than full buffer size transferred
			systemStatus.fileStatus = file_end;							                              // Reached end of file
			padBuffer(&systemStatus.PCMbuffer[systemStatus.remainingSamples], 0, systemStatus.PCMSamples-systemStatus.remainingSamples);		            // Fill the remaining data with silence
		}
		HAL_I2S_Transmit_DMA(i2sHandle, (uint16_t*)systemStatus.PCMbuffer, systemStatus.PCMSamples);   // Start I2S DMA
		systemStatus.audioStatus=audio_play;							                // Status = playing
	}
	iprintf("AUDIO: Playback started\n");
}


void AudioStop(void){

  systemStatus.fileStatus = file_end;

  if(systemStatus.audioStatus==audio_play){
    HAL_I2S_DMAStop(i2sHandle);
    systemStatus.audioStatus = audio_stop;                // Play finished
  }
  if(systemStatus.startDecoder){
    systemStatus.stopDecoder();
  }

  if(systemStatus.decoder){
    free(systemStatus.decoder);
    systemStatus.decoder = NULL;
  }
  if(systemStatus.PCMbuffer){
    free(systemStatus.PCMbuffer);
    systemStatus.PCMbuffer = NULL;
  }
	iprintf("AUDIO: Playback stopped\n\n");
}


void padBuffer(audio_t* dest, audio_t data, uint16_t count){
  static int32_t val;
  val = data;
  HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream0, (uint32_t)&val, (uint32_t)dest, count); // Start DMA transfer to fill the buffer, this will be very fast, we don't need further actions, HAL will clear/handle the DMA when the interrupt happens.
}


void handleBuffer(uint16_t offset){
	if(systemStatus.fileStatus==file_opened){							// File opened?

	  uint32_t count = systemStatus.fillBuffer(&systemStatus.PCMbuffer[offset], systemStatus.PCMSamples/2);

		if(count<systemStatus.PCMSamples/2){											            // If less data than expected
			systemStatus.fileStatus=file_end;							                      // File end reached
			systemStatus.remainingSamples -= systemStatus.PCMSamples/2;					// Subtract 1/2 buffer count
			systemStatus.remainingSamples += count;						                  // Add the remaining bytes
			padBuffer(&systemStatus.PCMbuffer[offset+count], 0, (systemStatus.PCMSamples/2)-count);	// Fill the remaining data with silence
		}
	}
	else{																                                    // File already reached end, so no more data to transfer
		if(systemStatus.remainingSamples <= systemStatus.PCMSamples/2){				// Remaining bytes less than 1/2 buffer?
			AudioStop();												                                // Done, stop audio
		}
		else{
			systemStatus.remainingSamples -= systemStatus.PCMSamples/2;					// Buffer not done yet
		}
	}

}


void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef* hi2s){
  if(systemStatus.updateBuffer){
    systemStatus.underflow=1;
  }
  systemStatus.updateBuffer=1;
}


void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef* hi2s){
  if(systemStatus.updateBuffer){
    systemStatus.underflow=1;
  }
  systemStatus.updateBuffer=2;
}
