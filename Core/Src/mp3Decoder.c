/*
 * mp3Decoder.c
 *
 *  Created on: Mar 21, 2021
 *      Author: David
 */
#include "i2sAudio.h"
#include "fatfs.h"
#include "mp3Decoder.h"
#include "spiritMP3Dec.h"


ID3TAG_t ID3TAG;
extern system_t systemStatus;


int unsynchsafe(uint32_t in){
  int out = 0, mask = 0x7F000000;
  in = ((in >> 24) | ((in >> 8) & 0xFF00ul) | ((in << 8) & 0xFF0000ul) | (in << 24));
  while (mask) {
    out >>= 1;
    out |= in & mask;
    mask >>= 8;
  }
  return out;
}

uint8_t readID3(void){
  UINT count = 0;
  if( f_read( systemStatus.file, &ID3TAG,10, &count ) != FR_OK ){
    return 0;
  }
  if((ID3TAG.id[0] != 'I') || (ID3TAG.id[1] != 'D') || (ID3TAG.id[2] != '3')){
    iprintf("MP3: ID3Tag not found\n" );
    f_lseek(systemStatus.file,0);                     // Seek to start of file
    return 0;
  }
  ID3TAG.size = (uint32_t)ID3TAG.size_[3]<<24 | (uint32_t)ID3TAG.size_[2]<<16 | (uint32_t)ID3TAG.size_[1]<<8 | (uint32_t)ID3TAG.size_[0];
  ID3TAG.size = unsynchsafe(ID3TAG.size);
  f_lseek(systemStatus.file, ID3TAG.size+10);          // We skip ID3 for now.
  return 1;
}


// Starts mp3 decoder
uint8_t mp3Start(void){
  readID3();
  systemStatus.decoder = calloc(1,sizeof(TSpiritMP3Decoder));
  systemStatus.PCMbuffer = malloc(MP3_PCMSamples*2);                      // *2 because 1sample = 16bits
  systemStatus.fillBuffer = mp3FillBuffer;
  if( !systemStatus.decoder){
    iprintf("MP3: Error allocating decoder!\n");
    return 0;
  }
  if(!systemStatus.PCMbuffer ){
    iprintf("MP3: Error allocating Buffer!\n");
    return 0;
  }
  systemStatus.PCMSamples = MP3_PCMSamples;
  SpiritMP3DecoderInit(systemStatus.decoder, RetrieveMP3Data, NULL, NULL);
  return 1;
}

// Stops mp3 decoder
void mp3Stop(void){

}

// Callback function, reads MP3 data from the file.
unsigned int RetrieveMP3Data(void *pMP3CompressedData, unsigned int nMP3DataSizeInChars, void *token){
  UINT count=0;
  if( f_read( systemStatus.file, (uint8_t *)pMP3CompressedData, nMP3DataSizeInChars, &count ) != FR_OK){
    return -1;
  }
  return count;
}

// Callback function, transfers PCM samples to the audio buffer
uint32_t mp3FillBuffer(int16_t* dest, uint16_t samples){
  return (SpiritMP3Decode( systemStatus.decoder, (int16_t *)dest, samples/2, NULL )*2); // 1 MP3 sample outputs 2 PCM samples (stereo)
}
