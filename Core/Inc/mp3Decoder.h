/*
 * mp3.h
 *
 *  Created on: 22 mar. 2021
 *      Author: David
 */

#ifndef INC_MP3DECODER_H_
#define INC_MP3DECODER_H_
#include "main.h"


typedef struct{
  uint8_t id[3];
  uint8_t version; // $04
  uint8_t revision; // $00
  uint8_t flags; // %abcd0000
  uint8_t size_[4];
  uint32_t size;
}ID3TAG_t;

// Optimal size of output buffer is a multiple of MP3 granule size (576 samples)
#define MP3_GRANULE_SIZE      576
#define MP3Samples            MP3_GRANULE_SIZE*4        // 4 frames provide 52mS audio buffer (44100Hz)
#define MP3_PCMSamples        MP3Samples*2              // *2 (Stereo)


int unsynchsafe(uint32_t in);
uint8_t readID3(void);
uint8_t mp3Start(void);
void mp3Stop(void);
unsigned int RetrieveMP3Data(void *pMP3CompressedData, unsigned int nMP3DataSizeInChars, void *token);
uint32_t mp3FillBuffer(int16_t* dest, uint16_t samples);

#endif /* INC_MP3DECODER_H_ */
