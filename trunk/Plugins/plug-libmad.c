#include "plug-libmad.h"

bstdfile_t*       bfilp = NULL;
FILE*             filp = NULL;
struct mad_stream MADStream;
struct mad_frame  MADFrame;
struct mad_synth  MADSynth;

unsigned char*    GuardPtr = NULL;

unsigned char  PreBuffer[INPUT_BUFFER_SIZE + MAD_BUFFER_GUARD];
unsigned char* ResidualProcessed = NULL;
size_t         ResidueSize = 0;

signed short MadFixedToShort(mad_fixed_t Fixed)
{
  if (Fixed >= MAD_F_ONE)
    return SHRT_MAX;
  if (Fixed <= -MAD_F_ONE)
    return -SHRT_MAX;

  Fixed = Fixed >> (MAD_F_FRACBITS - 15);
  return ((signed short) Fixed);
}

void Initialise(char*** Extension)
{
  *Extension = (char**)malloc(sizeof(char*) * 4);
  (*Extension)[0] = (char*)"mp3";
  (*Extension)[1] = (char*)"mp2";
  (*Extension)[2] = (char*)"mp1";
  (*Extension)[3] = 0;
}

enum GET_FILE_INFO_RETURN GetFileInfo(const char* filename, FileInfo_t* fi)
{
  ID3_Tag tag;
  char* field = NULL;
  
  if(!tag.Link(filename))
  {
    return GET_FILE_INFO_ERROR;
  }
  
  field = ID3_GetTitle(&tag);
  if (field)
  {
    fi->Title = (char*)malloc(strlen(field) + 1);
    strcpy(fi->Title, field);
  }
  else
  {
    fi->Title = (char*)malloc(4);
    strcpy(fi->Title, "???");
  }
  
  field = ID3_GetArtist(&tag);
  if (field)
  {
    fi->Artist = (char*)malloc(strlen(field) + 1);
    strcpy(fi->Artist, field);
  }
  else
  {
    fi->Artist = (char*)malloc(4);
    strcpy(fi->Artist, "???");
  }
  
  field = ID3_GetAlbum(&tag);
  if (field)
  {
    fi->Album = (char*)malloc(strlen(field) + 1);
    strcpy(fi->Album, field);
  }
  else
  {
    fi->Album = (char*)malloc(4);
    strcpy(fi->Album, "???");
  }
  
  field = ID3_GetGenre(&tag);
  if (field)
  {
    fi->Genre = (char*)malloc(strlen(field) + 1);
    strcpy(fi->Genre, field);
  }
  else
  {
    fi->Genre = (char*)malloc(4);
    strcpy(fi->Genre, "???");
  }
  
  field = ID3_GetComment(&tag);
  if (field)
  {
    fi->Comment = (char*)malloc(strlen(field) + 1);
    strcpy(fi->Comment, field);
  }
  else
  {
    fi->Comment = (char*)malloc(4);
    strcpy(fi->Comment, "???");
  }
  
  field = ID3_GetYear(&tag);
  if (field)
  {
    fi->Year = atoi(field);
  }
  else
  {
    fi->Year = 0;
  }
  
  field = ID3_GetTrack(&tag);
  if (field)
  {
    fi->Track = atoi(field);
  }
  else
  {
    fi->Track = 0;
  }
  
  fi->Seekable = 0;
  
  tag.Clear();
  
  FetchMP3Header(filename);
  fi->Length = GetLengthInSeconds();
  
  return GET_FILE_INFO_OK;
}

enum FILL_BUFFER_RETURN FillBuffer(unsigned char* buffer, size_t* length)
{
  enum FILL_BUFFER_RETURN rc = FILL_BUFFER_OK;
  signed short         TempSample;
  unsigned char*       ResPtr;
  size_t               ReadSize;
  size_t               mlength = *(length);
  unsigned int         i;

  if (ResidueSize)
  {
    /* Take left over data and give it to the stream */
    if (ResidueSize > mlength)
    {
      /* Stream needs more than the residue, so just pass it over, relocate the
       * residue data and update numbers
       */
      memcpy(buffer, ResidualProcessed, mlength);
      memmove(ResidualProcessed,
              ResidualProcessed + mlength,
              ResidueSize - mlength);
      ResidueSize -= mlength;
      mlength = 0;
    }
    else
    {
      /* Stream needs less than the residue, give what we can and then
       * calculate the rest
       */
      memcpy(buffer, ResidualProcessed, ResidueSize);
      free(ResidualProcessed);
      ResidualProcessed = NULL;
      mlength -= ResidueSize;
      buffer += ResidueSize;
      ResidueSize = 0;
    }
  }

  while (mlength > 0)
  {
    /* If the buffer is empty or the MAD decoder requested more data to
     * process then we fill up the buffer by another chunk, in the 
     * event of EOF, we note this happening (and if a guard was used).
     */
    if (MADStream.buffer == NULL || MADStream.error == MAD_ERROR_BUFLEN)
    {
      ReadSize = INPUT_BUFFER_SIZE;
      switch (FillStreamBuffer(&MADStream, PreBuffer, &ReadSize, bfilp))
      {
      case ERROR:
        /* Error reading from the file give up */
        return FILL_BUFFER_ERROR;

      case EOFF:
        /* File was already at the end when FillStreamBuffer was called */
        *(length) -= mlength;
        return FILL_BUFFER_EOFF;
      
      case EOFF_JUST:
        /* File is now at the end, after FillStreamBuffer was called, there
         * is now a guard buffer appended to the PreBuffer and another call 
         * to FillStreamBuffer will certainly return EOFF */
        break;      

      case OK:
        /* Standard behaviour */
        break;
      }

      /* Pass the data to libmad's decoding stream */
      mad_stream_buffer(&MADStream, PreBuffer, ReadSize);
      MADStream.error = (mad_error)0;
    }

    /* Process a frame of audio */
    if (mad_frame_decode(&MADFrame, &MADStream))
    {
      if (MAD_RECOVERABLE(MADStream.error))
      {
        if ((MADStream.error == MAD_ERROR_LOSTSYNC)
         && (MADStream.this_frame == GuardPtr))
        {
          /* File came to an end during this stream, the next frame to be
           * decoded would start at the start of the guard buffer so a loss
           * of syncronisation error is thrown, we need to know where the guard
           * buffer starts in case there is a genuine sync loss in the last 
           * section of the mp3 stream. */
           continue;
        }
        else
        {
          /* Recoverable error, report it and loop round, the next call to 
           * mad_frame_decode will skip the dodgy bit of data */ 
          fprintf(stderr,
                  "plug-libmad.c: Recoverable error (%s)\n",
                  mad_stream_errorstr(&MADStream));
          continue;
        }
      }
      else
      {
        if (MADStream.error == MAD_ERROR_BUFLEN)
        {
          /* The PreBuffer has run out, don't synth this frame, loop around to
           * pick up more data from the file */
          continue;
        }
        else
        {
          /* Unrecoverable error (e.g. out of memory) report and fail */
          fprintf(stderr,
                  "plug-libmad.c: Unrecoverable error (%s)\n",
                  mad_stream_errorstr(&MADStream));
          return FILL_BUFFER_ERROR;
        }
      }
    }

    mad_synth_frame(&MADSynth, &MADFrame);
    
    if (((size_t)MADSynth.pcm.length * 4) > mlength)
    {
      for (i = 0; i < mlength / 4; i++)
      {
        TempSample = MadFixedToShort(MADSynth.pcm.samples[0][i]);
        *(buffer++) = ((TempSample >> 0) & 0xff);
        *(buffer++) = ((TempSample >> 8) & 0xff);

        if(MAD_NCHANNELS(&MADFrame.header)==2)
        {
          TempSample = MadFixedToShort(MADSynth.pcm.samples[1][i]);
        }
        *(buffer++) = ((TempSample >> 0) & 0xff);
        *(buffer++) = ((TempSample >> 8) & 0xff);
      }

      ResidueSize = (MADSynth.pcm.length - (mlength / 4)) * 4;
      ResidualProcessed = (unsigned char*)malloc(ResidueSize);
      ResPtr = ResidualProcessed;
      mlength -= (mlength / 4) * 4;

      TempSample = MadFixedToShort(MADSynth.pcm.samples[0][i]);
      if (mlength)
      {
        *(buffer++) = ((TempSample >> 0) & 0xff);
        mlength--;
      }
      else
      {
        *(ResPtr++) = ((TempSample >> 0) & 0xff);
      }
      
      if (mlength)
      {
        *(buffer++) = ((TempSample >> 8) & 0xff);
        mlength--;
      }
      else
      {
        *(ResPtr++) = ((TempSample >> 8) & 0xff);
      }

      if(MAD_NCHANNELS(&MADFrame.header)==2)
      {
        TempSample = MadFixedToShort(MADSynth.pcm.samples[1][i]);
      }
      if (mlength)
      {
        *(buffer++) = ((TempSample >> 0) & 0xff);
        mlength--;
      }
      else
      {
        *(ResPtr++) = ((TempSample >> 0) & 0xff);
      }
      *(ResPtr++) = ((TempSample >> 8)& 0xff);

      /* Move to next sample */
      i++;

      for (; i < MADSynth.pcm.length; i++)
      {
        TempSample = MadFixedToShort(MADSynth.pcm.samples[0][i]);
        *(ResPtr++) = ((TempSample >> 0) & 0xff);
        *(ResPtr++) = ((TempSample >> 8) & 0xff);

        if(MAD_NCHANNELS(&MADFrame.header)==2)
        {
          TempSample = MadFixedToShort(MADSynth.pcm.samples[1][i]);
        }
        *(ResPtr++) = ((TempSample >> 0) & 0xff);
        *(ResPtr++) = ((TempSample >> 8) & 0xff);
      }
    }
    else
    {
      for (i = 0; i < MADSynth.pcm.length; i++)
      {
        TempSample = MadFixedToShort(MADSynth.pcm.samples[0][i]);
        *(buffer++) = ((TempSample >> 0) & 0xff);
        *(buffer++) = ((TempSample >> 8) & 0xff);

        if (MAD_NCHANNELS(&MADFrame.header) == 2)
        {
          TempSample = MadFixedToShort(MADSynth.pcm.samples[1][i]);
        }
        *(buffer++) = ((TempSample >> 0) & 0xff);
        *(buffer++) = ((TempSample >> 8) & 0xff);
      }

      mlength -= (MADSynth.pcm.length * 4);
    }
  }

  *(length) -= mlength;
  
  return rc;
}

enum OPEN_FILE_RETURN OpenFile(const char* filename, pa_sample_spec* ss)
{
  filp = fopen(filename, "rb");
  if ((bfilp = NewBstdFile(filp)) == NULL)
  {
    fprintf(stderr, "Couldn't create bstdfile_t (%s).\n", strerror(errno));
    if (filp)
    {
      fclose(filp);
    }
    return OPEN_FILE_ERROR;
  }
  else
  {
    /* File is available so activate and prepare MAD structs */
    mad_stream_init(&MADStream);
    mad_frame_init(&MADFrame);
    mad_synth_init(&MADSynth);
    
    ss->rate = 44100;
    ss->channels = 2;
    ss->format = PA_SAMPLE_S16LE;
    
    return OPEN_FILE_OK;
  }
}

void Cleanup()
{
  if (ResidualProcessed)
  {
    free(ResidualProcessed);
  }
  ResidueSize = 0;
  if (bfilp)
  {
    fclose(filp);
    filp = NULL;
    BstdFileDestroy(bfilp);
    bfilp = NULL;

    mad_synth_finish(&MADSynth);
  	mad_frame_finish(&MADFrame);
  	mad_stream_finish(&MADStream);
  }
  else if (filp)
  {
    fclose(filp);
    filp = NULL;
  }
  
  GuardPtr = NULL;
}

/*****************************************************************************/
/* Internal functions                                                        */
/*****************************************************************************/
enum FILL_S_BUFF_RETURN FillStreamBuffer(struct mad_stream* s,
                                         unsigned char* buf,
                                         size_t* length,
                                         bstdfile_t* bfilp)
{
  size_t ReadSize, Remaining;
  unsigned char* ReadStart;
  enum FILL_S_BUFF_RETURN rc = OK;
  
  if (MADStream.next_frame != NULL)
  {
    Remaining = MADStream.bufend - MADStream.next_frame;
    memmove(buf, MADStream.next_frame, Remaining);
    ReadStart = buf + Remaining;
    ReadSize = *(length) - Remaining;
  }
  else
  {
    ReadSize = *(length);
    ReadStart = buf;
    Remaining = 0;
  }
  
  ReadSize = BstdRead(ReadStart, 1, ReadSize, bfilp);
  if (ReadSize <= 0)
  {
    if (ferror(bfilp->fp))
    {
      fprintf(stderr, "Read error on input file (%s).\n", strerror(errno));
      rc = ERROR;
    }
    if (feof(bfilp->fp))
    {
      rc = EOFF;
    }
  }
  else if (BstdFileEofP(bfilp))
  {
    /* We're exactly at the end of the file, create a buffer to prevent
     * next_frame pointer wandering off into unallocated memory */
    GuardPtr = ReadStart + ReadSize;
    memset(GuardPtr, 0, MAD_BUFFER_GUARD);
    ReadSize += MAD_BUFFER_GUARD;
    rc = EOFF_JUST;
  }
  
  *(length) = Remaining + ReadSize;
  
  return rc;
}
