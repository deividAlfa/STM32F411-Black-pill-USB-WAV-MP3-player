/*
 * wavDecoder.c
 *
 *  Created on: 29 abr. 2021
 *      Author: David
 */


/*
 * mp3Decoder.c
 *
 *  Created on: Mar 21, 2021
 *      Author: David
 */
#include "i2sAudio.h"
#include "fatfs.h"
#include "wavDecoder.h"
#include <malloc.h>

extern system_t systemStatus;


uint8_t checkWav(void){
  uint8_t wav_OK = 1;
  //char WavInfo[36] = "WAV Info: ";
  wav_header_t wavHeader;
  UINT count = 0;

  if( (f_read(systemStatus.file, &wavHeader, 44, &count) != FR_OK) || count<44){           // Read wav header
    return 0;
  }
  if(wavHeader.ChunkID != 0x46464952 ){   // "RIFF"
    wav_OK = 0;
  }
  if(wavHeader.Format != 0x45564157 ){    // "WAVE"
    wav_OK = 0;
  }
  if(wavHeader.Subchunk1ID != 0x20746d66 ){ // "fmt "
    wav_OK = 0;
  }
  if(wavHeader.Subchunk1Size != 16 ){     // Should be 16
    wav_OK = 0;
  }
  if(wavHeader.AudioFormat != 1 ){      // Should be 1
    wav_OK = 0;
  }

  if((wavHeader.NumChannels != 2) || (wavHeader.SampleRate!=44100) || (wavHeader.BitsPerSample != 16 ) || (wavHeader.Subchunk2ID != 0x61746164 ) ){
    wav_OK = 0;
  }
  else{
    systemStatus.audioChannels=audio_stereo;
    systemStatus.audioRate = audio_44KHz;
    systemStatus.audioBits = audio_16bit;
  }
  // Removed support for other audio settings. MP3 assumes 44.1KHz 16bit stereo.
  /*
  if(wavHeader.NumChannels == 1){
    systemStatus.audioChannels=audio_mono;
    strcat(WavInfo, "Mono, ");
  }
  else if(wavHeader.NumChannels == 2 ){
    systemStatus.audioChannels=audio_stereo;
    strcat(WavInfo, "Stereo, ");
  }
  else{ wav_OK = 0; }

  if(wavHeader.SampleRate == 8000){
    systemStatus.audioRate = audio_8KHz;
    strcat(WavInfo, "8KHz, ");

  }
  if(wavHeader.SampleRate == 16000){
    systemStatus.audioRate = audio_16KHz;
    strcat(WavInfo, "16KHz, ");
  }
  else if(wavHeader.SampleRate == 32000){
    systemStatus.audioRate = audio_32KHz;
    strcat(WavInfo, "32KHz, ");
  }
  else if(wavHeader.SampleRate == 22050){
    systemStatus.audioRate = audio_22KHz;
    strcat(WavInfo, "22.05KHz, ");
  }
  else if(wavHeader.SampleRate == 44100){
    systemStatus.audioRate = audio_44KHz;
    strcat(WavInfo, "44.1KHz, ");
  }
  else if(wavHeader.SampleRate == 48000){
    systemStatus.audioRate = audio_48KHz;
    strcat(WavInfo, "48KHz, ");
  }
  else if(wavHeader.SampleRate == 96000){
    systemStatus.audioRate = audio_96KHz;
    strcat(WavInfo, "96KHz, ");
  }
  else{ wav_OK = 0; }

  if(wavHeader.BitsPerSample == 16 ){
    systemStatus.audioBits = audio_16bit;
    strcat(WavInfo, "16bit\n");
  }
  else if(wavHeader.BitsPerSample == 8 ){
    systemStatus.audioBits = audio_8bit;
    strcat(WavInfo, "8bit\n");
  }
  else{ wav_OK = 0; }
  if(wavHeader.Subchunk2ID != 0x61746164 ){ // "data"
    wav_OK = 0;
  }
  */
  //if(wav_OK){ iprintf(WavInfo); }

  return wav_OK;
}

// Starts wav decoder
uint8_t wavStart(void){
  if(!checkWav()){
    iprintf("WAV: Bad file!\n");
    return 0;
  }
  systemStatus.PCMbuffer = malloc(WAV_PCMSamples*2);                      // *2 because 1sample = 16bits
  if(!systemStatus.PCMbuffer ){
    iprintf("WAV: Error allocating Buffer!\n");
    return 0;
  }
  systemStatus.PCMSamples = WAV_PCMSamples;
  systemStatus.fillBuffer = wavFillBuffer;
  return 1;
}

// Stops wav decoder
void wavStop(void){

}

// Callback function, transfers PCM samples to the audio buffer
uint32_t wavFillBuffer(int16_t* dest, uint16_t samples){
  UINT count;
  if( f_read( systemStatus.file, (uint8_t*)dest,samples*2, &count ) != FR_OK ){
    return 0;
  }
  return count/2; // count is bytes, samples are 16 bit
}
