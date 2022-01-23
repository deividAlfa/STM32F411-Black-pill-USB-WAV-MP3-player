## STM32F411 "Black Pill" USB WAV/MP3 Player

<!-- MarkdownTOC -->

* [Project description](#description)
* [How to use](#use)
* [Firmware](#firmware)

<!-- /MarkdownTOC -->

<a id="description"></a>
## Project description<br>

This is just a training project that reads, parses and plays WAV/MP3 files using PCM5102A I2S DAC.<br>
Uses a STM32F411CE "Black Pill" board running at 96MHz.<br>
MP3 decoding is done with the library "Spirit MP3 decoder", freely available from ST.<br>

For simplicity, only 44.1KHz, 16-bit stereo files are supported.<br>
MP3 format is not checked! It assumes your MP3 is on the correct format.<br>
Any bitrate should work, either VBR or CBR.<br>
WAV files are checked and skipped if not valid.<br>

PCM5102A connections are as follows:<br>
- SCK: Unconnected, uses self-generated PLL clock. Pull this pin to gnd in the DAC.<br>
- BCK: PB13<br>
- DIN: PB15<br>
- LRCK: PB12<br>

Few waveforms:<br>

<img src="/Readme_files/wave1.png?raw=true"><br>
<img src="/Readme_files/wave2.png?raw=true"><br>
<img src="/Readme_files/wave3.png?raw=true"><br><br>

The SWO output is enabled on PB3, you can see the debug messages using SWO console.<br>
You can see them without debugging, just connect the ST-Link utility, open "Prinf via SWO viewer", setting clock to 96000000Hz.<br>
<img src="/Readme_files/swo.png?raw=true"><br>

For more details, check:

      /Inc/files.h
      /Inc/i2sAudio.h
      /Src/files.c
      /Src/i2sAudio.c
  
<a id="use"></a>
## How to use

Connect a FAT32-formatted USB drive with wav/mp3 files in the root folder (/), it will automatically start playing all files found in a loop .<br>
Press "Key" button (PA0) to skip current song.<br>
<a id="firmware"></a>
## Firmware

You can download already compiled binaries in the Release folder (mp3 player.bin file)<br>
If you want to compile your own:<br>
- Download STM32 Cube IDE<br>
- Clone or download the code<br>
- Open STM32 Cube IDE, import existing project and select the folder where the code is.<br>
- It should recognize it and be ready for compiling or modifying for your own needs.<br>

 The code is stripped from most ST libraries, so you need to regenerate them.<br>
 Inside the IDE, open cubeMX (Double click the .ioc file) file and click on the code generation button.<br>
 <img src="/Readme_files/gen.png?raw=true"><br>
 CubeMx will make new code, and now it will compile correctly.<br>
