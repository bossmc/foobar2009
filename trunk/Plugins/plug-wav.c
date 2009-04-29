#include "plug-wav.h"

FILE* filp = NULL;
int g_channels     = 0;
int g_rate         = 0;
int g_format       = 0;
int g_headerlength = 0;

void Initialise(char*** Extensions)
{
  *Extensions = (char**)malloc(sizeof(char**) * 2);
  (*Extensions)[0] = "wav";
  (*Extensions)[1] = 0;
}

enum GET_FILE_INFO_RETURN GetFileInfo(const char* filename, FileInfo_t* fi)
{
  FILE* tempfilp = NULL;
  char* title;
  int BigEnd = BigEndianTest();
  unsigned char chunk[4] = { '\0', '\0', '\0', '\0' };
  int channelcount = 0;
  int samplerate = 0;
  int datalength = 0;
  int bitspersample = 0;
  
  tempfilp = fopen(filename, "rb");
  if (!tempfilp)
  {
    return GET_FILE_INFO_ERROR;
  }
  
  /* Ensure this is a RIFF file */
  fread(chunk, 1, 4, tempfilp);
  if (strncmp((char*)chunk, "RIFF", 4))
    goto fail;
  
  /* Get size and ignore... */
  fread(chunk, 1, 4, tempfilp);
  
  /* Ensure the format is recorded as WAVE */
  fread(chunk, 1, 4, tempfilp);
  if (strncmp((char*)chunk, "WAVE", 4))
    goto fail;
  
  /* Next section should be the 'fmt ' section */
  fread(chunk, 1, 4, tempfilp);
  if (strncmp((char*)chunk, "fmt ", 4))
    goto fail;
  
  /* The length of the 'fmt ' section, should be 0x10 */
  fread(chunk, 1, 4, tempfilp);
  if (!((chunk[0] == 0x10)
        && (chunk[1] == 0x00)
        && (chunk[2] == 0x00)
        && (chunk[3] == 0x00)))
    goto fail;
  
  /* Now the format (should be 0x0001) and channels */
  fread (chunk, 1, 4, tempfilp);
  if (!((chunk[0] == 0x01)
        && (chunk[1] == 0x00)))
    goto fail;
  
  channelcount = chunk[2];
  
  /* Sample rate, fun fun fun endian fun */
  fread(chunk, 1, 4, tempfilp);
  if (BigEnd)
  {
    samplerate = 0;
    samplerate |= chunk[0] << 24;
    samplerate |= chunk[1] << 16;
    samplerate |= chunk[2] << 8;
    samplerate |= chunk[3] << 0;
  }
  else
  {
    samplerate = 0;
    samplerate |= chunk[3] << 24;
    samplerate |= chunk[2] << 16;
    samplerate |= chunk[1] << 8;
    samplerate |= chunk[0] << 0;
  }
  
  /* Ignore the byte rate */
  fread(chunk, 1, 4, tempfilp);
  
  /* Look up the bits per sample (and block align but we ignore that) */
  fread(chunk, 1, 4, tempfilp);
  if (BigEnd)
  {
    bitspersample = 0;
    bitspersample |= chunk[2] << 8;
    bitspersample |= chunk[3] << 0;
  }
  else
  {
    bitspersample = 0;
    bitspersample |= chunk[2] << 0;
    bitspersample |= chunk[3] << 8;
  }
  
  /* Next section should be 'data' */
  fread(chunk, 1, 4, tempfilp);
  if (strncmp((char*)chunk, "data", 4))
    goto fail;
  
  /* Fetch the data length */
  fread(chunk, 1, 4, tempfilp);
  if (BigEnd)
  {
    datalength = 0;
    datalength |= chunk[0] << 24;
    datalength |= chunk[1] << 16;
    datalength |= chunk[2] << 8;
    datalength |= chunk[3] << 0;
  }
  else
  {
    datalength = 0;
    datalength |= chunk[3] << 24;
    datalength |= chunk[2] << 16;
    datalength |= chunk[1] << 8;
    datalength |= chunk[0] << 0;
  }
  
  title = strrchr(filename, '/');
  if (title)
  {
    fi->Title = (char*)malloc(strlen(title));
    strcpy(fi->Title, title + 1);
  }
  else
  {
    fi->Title = (char*)malloc(strlen(filename) + 1);
    strcpy(fi->Title, filename);
  }
  
  fi->Artist = malloc(4);
  strcpy(fi->Artist, "???");
  
  fi->Album = malloc(4);
  strcpy(fi->Album, "???");
  
  fi->Genre = malloc(4);
  strcpy(fi->Genre, "???");
  
  fi->Comment = malloc(4);
  strcpy(fi->Comment, "???");
  
  fi->Year = 0;
  fi->Track = 0;
  fi->Seekable = 1;
  
  fi->Length = ((datalength / channelcount) / samplerate) / (bitspersample / 8);
  
  /* Rest of the file is data! woo! */
  return GET_FILE_INFO_OK;
  
fail:
  fclose(tempfilp);
  return GET_FILE_INFO_ERROR;
}
  

enum FILL_BUFFER_RETURN FillBuffer(unsigned char* pBuf, size_t* length)
{
  enum FILL_BUFFER_RETURN rc = FILL_BUFFER_OK;
  size_t ReadSize = 0;
  
  ReadSize = fread(pBuf, 1, *length, filp);
  if (ReadSize == 0)
  {
    *length = ReadSize;
    rc = FILL_BUFFER_EOFF;
  }
  else if (ReadSize < 0)
  {
    rc = FILL_BUFFER_ERROR;
  }
  
  return rc;
}

enum SEEK_RETURN Seek(int* seconds)
{
  size_t SeekPoint = 0;
  
  SeekPoint =  (*seconds) * g_rate;
  SeekPoint *= g_channels;
  SeekPoint *= g_format / 0x8;
  SeekPoint += g_headerlength;
  
  fseek(filp, SeekPoint, SEEK_SET);
  if (feof(filp))
  {
    return SEEK_EOFF;
  }
  else
  {
    return SEEK_OK;
  }
}

enum OPEN_FILE_RETURN OpenFile(const char* filename, pa_sample_spec* ss)
{
  unsigned char chunk[4];
  int BigEnd = BigEndianTest();
  
  filp = fopen(filename, "rb");
  if (!filp)
    goto fail;
  
  /* Ensure this is a RIFF file */
  fread(chunk, 1, 4, filp);
  if (strncmp((char*)chunk, "RIFF", 4))
    goto fail;
  
  /* Get size and ignore... */
  fread(chunk, 1, 4, filp);
  
  /* Ensure the format is recorded as WAVE */
  fread(chunk, 1, 4, filp);
  if (strncmp((char*)chunk, "WAVE", 4))
    goto fail;
  
  /* Next section should be the 'fmt ' section */
  fread(chunk, 1, 4, filp);
  if (strncmp((char*)chunk, "fmt ", 4))
    goto fail;
  
  /* The length of the 'fmt ' section, should be 0x10 */
  fread(chunk, 1, 4, filp);
  if (!((chunk[0] == 0x10)
        && (chunk[1] == 0x00)
        && (chunk[2] == 0x00)
        && (chunk[3] == 0x00)))
    goto fail;
  
  /* Now the format (should be 0x0001) and channels */
  fread (chunk, 1, 4, filp);
  if (!((chunk[0] == 0x01)
        && (chunk[1] == 0x00)))
    goto fail;
  
  ss->channels = (int)chunk[2];
  g_channels   = (int)chunk[2];
  
  /* Sample rate, fun fun fun endian fun */
  fread(chunk, 1, 4, filp);
  if (BigEnd)
  {
    ss->rate = 0;
    ss->rate |= chunk[0] << 24;
    ss->rate |= chunk[1] << 16;
    ss->rate |= chunk[2] << 8;
    ss->rate |= chunk[3] << 0;
  }
  else
  {
    ss->rate = 0;
    ss->rate |= chunk[3] << 24;
    ss->rate |= chunk[2] << 16;
    ss->rate |= chunk[1] << 8;
    ss->rate |= chunk[0] << 0;
  }
  g_rate = ss->rate;
  
  /* Ignore the sample rate */
  fread(chunk, 1, 4, filp);
  
  /* Look up the bits per sample (and block align but we ignore that */
  fread(chunk, 1, 4, filp);
  switch(chunk[2])
  {
    case 0x10:
      ss->format = PA_SAMPLE_S16LE;
      break;
    
    case 0x08:
      ss->format = PA_SAMPLE_U8;
      break;
  }
  g_format = (int)chunk[2];
  
  /* Next section should be 'data' */
  fread(chunk, 1, 4, filp);
  if (strncmp((char*)chunk, "data", 4))
    goto fail;
  
  /* Ignore the data length */
  fread(chunk, 1, 4, filp);
  
  /* Rest of the file is data! woo! */
  g_headerlength = ftell(filp);
  return OPEN_FILE_OK;
  
fail:
  fclose(filp);
  return OPEN_FILE_ERROR;
}

void Cleanup()
{
  if (filp)
  {
    fclose(filp);
    filp = NULL;
  }
  
  g_headerlength = 0;
  g_channels     = 0;
  g_format       = 0;
  g_rate         = 0;
}

/* returns true if on big endian, else false */
int BigEndianTest()
{
  unsigned char EndianTest[2] = {1,0};
  short x = *(short *)EndianTest;

  if( x == 1 )
    return 0;
  else
    return 1;
}
