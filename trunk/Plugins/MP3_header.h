#ifndef MP3_HEADER_H__
#define MP3_HEADER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

int FetchMP3Header(const char* filename);
int IsValidHeader();

int GetFrameSync();
int GetVersionIndex();
int GetLayerIndex();
int GetProtectionBit();
int GetBitrateIndex();
int GetFrequencyIndex();
int GetPaddingBit();
int GetPrivateBit();
int GetModeIndex();
int GetModeExtIndex();
int GetCopyrightBit();
int GetOriginalBit();
int GetEmphasisIndex();
int GetVersion();
int GetLayer();
int GetBitrate();
int GetFrequency();
char* GetMode();
int GetLengthInSeconds();
int GetNumberOfFrames();

#ifdef __cplusplus
}
#endif

#endif /*MP3_HEADER_H__*/
