/*
 * i2sAudio.h
 *
 *  Created on: Mar 21, 2021
 *      Author: David
 */

#ifndef INC_I2SAUDIO_H_
#define INC_I2SAUDIO_H_
#include "main.h"
#include "fatfs.h"
#include "spiritMP3Dec.h"

//#define FLAC_PCMSamples       (uint16_t)16384
typedef struct{
  uint16_t data[16384];
}data_test;

typedef enum{
	drive_nodrive = 0,			// No drive on system
	drive_inserted,					// Drive was inserted
	drive_mounted,					// Drive was successfully mounted
	drive_ready,						// Drive is mounted and has files
	drive_nofiles,					// Drive mounted but no files
	drive_removed,					// Drive removed while mounted
	drive_error,						// Drive error
	drive_unmounted,		    // Drive unmounted
}driveStatus_t;

typedef enum{
	file_none = 0,					// No file opened
	file_opened,						// File opened
	file_end,							  // File reached end
}fileStatus_t;

typedef enum{
  file_mp3,               // .MP3
  file_wav                // .WAV
}filetype_t;

typedef enum{
	audio_idle = 0,					// Audio idle after boot
	audio_play,							// Audio is playing
	audio_stop,							// Audio was stopped after playing

	audio_mono,
	audio_stereo,

	audio_8bit,							// Standard = 8 bit unsigned
	audio_16bit,						// Standard = 16 bit signed

	audio_8KHz,
	audio_16KHz,
	audio_22KHz,
	audio_32KHz,
	audio_44KHz,
	audio_48KHz,
	audio_96KHz

}audioStatus_t;

typedef int16_t audio_t;

typedef struct{
	driveStatus_t	 	driveStatus; 		    // 0 = no drive, 1=mounted, 2=scanned
	fileStatus_t		fileStatus; 		    // 0 = No file, 1 = File opened, 2 = File end reached
	filetype_t      filetype;           // File status
	FATFS           *fat;               // Pointer to FAT
	FIL             *file;              // Pointer to file
	char            *currentFilename;   // Pointer to current filename

  uint32_t        (*fillBuffer)(audio_t* dest, uint32_t samples);
	void            *decoder;           // Pointer to decoder
	uint8_t         (*startDecoder)(void);// Pointer to stop decoder function
	void            (*stopDecoder)(void);// Pointer to stop decoder function
	audio_t         *PCMbuffer;         // Pointer to PCM buffer
	uint16_t        PCMSamples;         // Buffer capacity
	volatile uint8_t updateBuffer;      // To tell the handler to add new data to the buffer
	volatile uint8_t underflow;
	uint16_t			  remainingSamples;	  // Remaining samples in the PCM buffer
	audioStatus_t		audioBits;			    // 8Bits, 16Bits
	audioStatus_t		audioChannels;		  // Mono, Stereo
	audioStatus_t		audioRate;			    // 16KHz, 32KHz, 48KHz
	audioStatus_t		audioStatus;		    // Idle, Playing, Stopped(after playing)
}system_t;


extern system_t systemStatus;

void initAudio(I2S_HandleTypeDef *hi2s);
void handleAudio(void);
void AudioStop(void);
void AudioStart(void);
void padBuffer(audio_t* dest, audio_t data, uint16_t count);
void handleBuffer(uint16_t offset);


#endif /* INC_PWMAUDIO_H_ */
