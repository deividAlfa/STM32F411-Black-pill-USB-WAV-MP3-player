/*
 * files.c
 *
 *  Created on: Dec 26, 2020
 *      Author: David
 */


#include "files.h"
#include "fatfs.h"
#include "spiritMP3Dec.h"
#include "i2sAudio.h"
#include <time.h>

#define MAXFILES 100

DWORD clmt[32];
DIR dir;         /* Directory object */
FILINFO fil;    /* File information */

extern system_t systemStatus;
uint16_t fileCount;
uint16_t filePos;
char fileList[MAXFILES][13];
#define FILETYPES 2
const char* filetypes[6] = {"*.MP3", "*.WAV" };   // Files we are interested in

void testFS(void){
	FRESULT res;
	UINT count;
	volatile uint32_t total=0, testTim;
	#define bs	512
	uint8_t testData[bs];
	if( f_open(systemStatus.file, "1.rar", FA_READ) != FR_OK){				// Open file
		iprintf("Error opening test file\n");
		systemStatus.driveStatus=drive_error;					// No files
		return;
	}
	iprintf("Starting transfer speed test\n");
	testTim = HAL_GetTick();
	do{// Read whole file
		res=f_read(systemStatus.file, testData, bs, &count);
		if(res!=FR_OK){
			iprintf("Error reading test file\n");
			f_close(systemStatus.file);
			return;
		}
		if(count){
			total+=count;
			if(count<bs){
				break;
			}
			//iprintf(data);
		}
	}while(count);
	f_close(systemStatus.file);
	testTim = HAL_GetTick()-testTim;
	iprintf("Done. bs=%u %luKB in %lumS, %luKB/s\n", bs, total/1024, testTim, total/(testTim*1024/1000));
	asm("nop");
}

void printFiles(void){
  printf("File list:\n");
  for(uint8_t t=0; t<fileCount;t++){
    printf("\t%s\n",fileList[t]);
  }
  printf("\n\n");
}

void sortFS(void){
  char temp[13];
  uint32_t Min=0;

  for(uint16_t j=0; j<fileCount; j++){         // Sort alphabetically
    Min = j;
    for(uint16_t i=j+1; i<fileCount; i++){
      if(strcmp(fileList[Min], fileList[i]) > 0){  // List[Min] > List[i]
        Min = i;
      }
    }
    if(Min!=j){
      strcpy(temp, fileList[j]);
      strcpy(fileList[j], fileList[Min]);
      strcpy(fileList[Min], temp);
    }
  }
}


void scanFS(void){
	fileCount = 0;
	filePos = 0;
	for(uint8_t t=0;t<FILETYPES;t++){
	  if( f_findfirst(&dir, &fil, "/", filetypes[t]) != FR_OK){       // Find first file of the current type
	    continue;                                                     // If not OK, continue with next filetype
	  }
	  while(fil.fname[0] && fileCount<MAXFILES){                      // Stop when no file found, last file or exceeded max file count
	    strcpy(fileList[fileCount++],fil.fname);                      // Copy file name
	    f_findnext(&dir, &fil);                                       // Find next file
	  }
	}
	sortFS();                                                         // Sort alphabetically
	printFiles();                                                     // Print file list
}

void handleFS(void){
	static driveStatus_t driveStatus = drive_nodrive;

	if(systemStatus.driveStatus==drive_inserted){						// Drive present
		if( f_mount(systemStatus.fat, "", 1) != FR_OK ){
			  iprintf("SYSTEM: Failed to mount volume\n");
			  systemStatus.driveStatus=drive_error;						//Failure on mount
		}
		else{
			if( f_opendir(&dir, "/") != FR_OK ){
				 iprintf("SYSTEM: Failed to open root dir\n");
			}
			else{
				iprintf("SYSTEM: Drive mounted\n");
				driveStatus=drive_ready;
			}
		}
	}
	if(driveStatus==drive_ready && systemStatus.driveStatus==drive_inserted){
		//testFS();
		systemStatus.driveStatus 	= drive_ready;
		systemStatus.fileStatus		= file_none;
		scanFS();
		if( fileCount==0 || fileList[0][0] == 0){
		  iprintf("SYSTEM: No files found\n");
			systemStatus.driveStatus=drive_error;					// No files
			return;
		}
	}
	if(systemStatus.driveStatus==drive_ready && systemStatus.fileStatus != file_opened	&& systemStatus.audioStatus != audio_play){
		if(systemStatus.fileStatus == file_end){
			f_close(systemStatus.file);
			if(++filePos > fileCount-1){
				filePos=0;
			}
		}
    char currentType[6] = "*.\0\0\0\0";                         // Empty extension string
    char* namePtr = (char*)fileList[filePos];                   // Point to current filename
    while (*(namePtr+3)){ namePtr++; }                          // Find last char
    currentType[2] = *namePtr++;                                // Copy extension (Last 3 chars, ex. "MP3" from "FILE.MP3")
    currentType[3] = *namePtr++;
    currentType[4] = *namePtr;

    for(uint8_t t=0;t<FILETYPES;t++){                           // Detect file extension
      if(strcmp(currentType,  filetypes[t])==0){
        systemStatus.filetype = t;
        break;
      }
    }

		if( f_open(systemStatus.file, (char *)fileList[filePos], FA_READ) != FR_OK){				// Open file
		  iprintf("SYSTEM: Error opening file\n");
			systemStatus.driveStatus=drive_error;					                            // No files
			return;
		}
		clmt[0] = 32;													                                      // Set table size
		systemStatus.file->cltbl = clmt;                       					                      // Enable fast seek feature (cltbl != NULL)
		if( f_lseek(systemStatus.file, CREATE_LINKMAP) != FR_OK ){				                  // Create CLMT
			f_close( systemStatus.file );
			iprintf("SYSTEM: FatFS Seek error\n");
		}
    iprintf("SYSTEM: Opened file: %s\n",fileList[filePos] );
    systemStatus.currentFilename=fileList[filePos];
		systemStatus.audioChannels=audio_stereo;
		systemStatus.audioRate = audio_44KHz;
		systemStatus.audioBits = audio_16bit;
		systemStatus.fileStatus = file_opened;						                          // Valid file
	}

	if((systemStatus.driveStatus==drive_error) || (systemStatus.driveStatus==drive_removed)){	// if drive removed or error
	  iprintf("SYSTEM: Removing mounting point\n");
		f_mount(0, "", 1);															// remove mount point
		if(systemStatus.driveStatus==drive_removed){
			systemStatus.driveStatus=drive_nodrive;
		}
		else{
			systemStatus.driveStatus=drive_unmounted;
		}
		AudioStop();
    systemStatus.fileStatus=file_none;
	}
}

