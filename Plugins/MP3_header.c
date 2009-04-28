#include "MP3_header.h"

unsigned long Header;
size_t FileLength;

int VBR;
int VFrameCount;

int FetchMP3Header(const char* filename)
{
  int rc = -1;
  FILE* filp = NULL;
  char chunk[4];
  char vchunk[12];
  size_t ReadPoint = 0;
  
  filp = fopen(filename, "rb");
  fseek(filp, 0, SEEK_END);
  FileLength = ftell(filp);
  rewind(filp);
  
  do
  {
    fseek(filp, ReadPoint, SEEK_SET);
    fread(chunk, 1, 4, filp);
    ReadPoint++;
    Header = (((chunk[0] & 255) << 24)
            | ((chunk[1] & 255) << 16)
            | ((chunk[2] & 255) <<  8)
            | ((chunk[3] & 255) <<  0)); 
  }
  while (!IsValidHeader() && !feof(filp));
  
  if (!feof(filp))
  {
    rc = 1;
    ReadPoint += 3;
    
    if (GetVersionIndex() == 3)
    {
      if (GetModeIndex() == 3)
      {
        ReadPoint += 17;
      }
      else
      {
        ReadPoint += 32;
      }
    }
    else
    {
      if (GetModeIndex() == 3)
      {
        ReadPoint += 9;
      }
      else
      {
        ReadPoint += 17;
      }
    }
    
    fseek(filp, ReadPoint, SEEK_SET);
    fread(vchunk, 1, 12, filp);
    
    if (vchunk[0] == 'X'
     && vchunk[1] == 'i'
     && vchunk[2] == 'n'
     && vchunk[3] == 'g')
    {
      int vflags = (int)(((vchunk[4] & 255) << 24)
                       | ((vchunk[5] & 255) << 16)
                       | ((vchunk[6] & 255) <<  8)
                       | ((vchunk[7] & 255) <<  0));
      if (vflags & 0x0001)
      {
        VFrameCount = (int)(((vchunk[8]  & 255) << 24)
                          | ((vchunk[9]  & 255) << 16)
                          | ((vchunk[10] & 255) <<  8)
                          | ((vchunk[11] & 255) <<  0));
      }
      else
      {
        VFrameCount = -1;
      }
      
      VBR = 1;
    }
    else
    {
      VBR = 0;
    }
  }
  
  fclose(filp);
  
  return -1;
}
    
int IsValidHeader()
{
  return (((GetFrameSync()      & 2047)==2047) &&
          ((GetVersionIndex()   &    3)!=   1) &&
          ((GetLayerIndex()     &    3)!=   0) && 
          ((GetBitrateIndex()   &   15)!=   0) &&
          ((GetBitrateIndex()   &   15)!=  15) &&
          ((GetFrequencyIndex() &    3)!=   3) &&
          ((GetEmphasisIndex()  &    3)!=   2));
}

int GetFrameSync()
{
  return (int)((Header >> 21) & 2047);
}

int GetVersionIndex()
{
  return (int)((Header >> 19) & 3);
}

int GetLayerIndex()
{
  return (int)((Header >> 17) & 3);
}

int GetProtectionBit()
{
  return (int)((Header >> 16) & 1);
}

int GetBitrateIndex()
{
  return (int)((Header >> 12) & 15);
}

int GetFrequencyIndex()
{
  return (int)((Header >> 10) & 3);
}

int GetPaddingBit()
{
  return (int)((Header >> 9) & 1);
}

int GetPrivateBit()
{
  return (int)((Header >> 8) & 1);
}

int GetModeIndex()
{
  return (int)((Header >> 6) & 3);
}

int GetModeExtIndex()
{
  return (int)((Header >> 4) & 3);
}

int GetCopyrightBit()
{
  return (int)((Header >> 3) & 1);
}

int GetOriginalBit()
{
  return (int)((Header >> 2) & 1);
}

int GetEmphasisIndex()
{
  return (int)((Header >> 0) & 3);
}

int GetVersion()
{
  double table[] = {2.5, 0.0, 2.0, 1.0};
  return table[GetVersionIndex()];
}

int GetLayer()
{
  return (int)(4 - GetLayerIndex());
}

int GetBitrate()
{
  if (VBR)
  {
    double AvgFrameSize = (double)FileLength / (double)GetNumberOfFrames();
    return (int)((AvgFrameSize * (double)GetFrequency()) / (1000.0 * ((GetLayerIndex() == 3) ? 12.0 : 144.0)));
  }
  else
  {
    int table[2][3][16] = {
                         {
                         {0,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160,0},
                         {0,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160,0},
                         {0, 32, 48, 56, 64, 80, 96,112,128,144,160,176,192,224,256,0}
                         },
                         {
                         {0, 32, 40, 48, 56, 64, 80, 96,112,128,160,192,224,256,320,0},
                         {0, 32, 48, 56, 64, 80, 96,112,128,160,192,224,256,320,384,0},
                         {0, 32, 64, 96,128,160,192,224,256,288,320,352,384,416,448,0}
                         }
                         };
    return table[GetVersionIndex() & 1][GetLayerIndex() - 1][GetBitrateIndex()];
  }
}

int GetFrequency()
{
  int table[4][3] = {
                      {32000, 16000,  8000},
                      {    0,     0,     0},
                      {22050, 24000, 16000},
                      {44100, 48000, 32000}
                    };
  return table[GetVersionIndex()][GetFrequencyIndex()];
}

char* GetMode()
{
  switch (GetModeIndex())
  {
    case 1:
      return (char*)"Joint Stereo";
    
    case 2:
      return (char*)"Dual Stereo";
      
    case 3:
      return (char*)"Single Channel";
    
    default:
      return (char*)"Stereo";
  }
}

int GetLengthInSeconds()
{
  int FileSizeInKilobits = (int)((8 * FileLength) / 1000);
  return (int)(FileSizeInKilobits/GetBitrate());
}

int GetNumberOfFrames()
{
  if (VBR)
  {
    return VFrameCount;
  }
  else
  {
    double AvgFrameSize = (double)(((GetLayerIndex()==3) ? 12 : 144) * ((1000.0 * (float)GetBitrate())/(float)GetFrequency()));
    return (int)(FileLength/AvgFrameSize);
  }
}