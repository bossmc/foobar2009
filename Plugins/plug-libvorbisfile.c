#include "plug-libvorbisfile.h"

OggVorbis_File ov;

void Initialise(char*** Extension)
{
  *Extension = malloc(sizeof(char*) * 2);
  (*Extension)[0] = "ogg";
  (*Extension)[1] = 0;
}

enum GET_FILE_INFO_RETURN GetFileInfo(const char* filename, FileInfo_t* fi)
{
  int i;
  OggVorbis_File tempov;
  vorbis_comment* vc = NULL;
  
  if (ov_fopen(filename, &tempov) < 0)
  {
    return GET_FILE_INFO_ERROR;
  }
  
  vc = ov_comment(&tempov, -1);
  
  for (i = 0; i < vc->comments; i++)
  {
    if (!(strncmp(vc->user_comments[i], "TITLE", 5)))
    {
      fi->Title = malloc(vc->comment_lengths[i] - 5);
      strcpy(fi->Title, vc->user_comments[i] + 6);
      fi->Title[vc->comment_lengths[i] - 6] = '\0';
    }
    
    if (!(strncmp(vc->user_comments[i], "ARTIST", 6)))
    {
      fi->Artist = malloc(vc->comment_lengths[i] - 6);
      strcpy(fi->Artist, vc->user_comments[i] + 7);
      fi->Artist[vc->comment_lengths[i] - 7] = '\0';
    }
    
    if (!(strncmp(vc->user_comments[i], "ALBUM", 5)))
    {
      fi->Album = malloc(vc->comment_lengths[i] - 5);
      strcpy(fi->Album, vc->user_comments[i] + 6);
      fi->Album[vc->comment_lengths[i] - 6] = '\0';
    }
    
    if (!(strncmp(vc->user_comments[i], "GENRE", 5)))
    {
      fi->Genre = malloc(vc->comment_lengths[i] - 5);
      strcpy(fi->Genre, vc->user_comments[i] + 6);
      fi->Genre[vc->comment_lengths[i] - 6] = '\0';
    }
    
    if (!(strncmp(vc->user_comments[i], "DESCRIPTION", 11)))
    {
      fi->Comment = malloc(vc->comment_lengths[i] - 11);
      strcpy(fi->Comment, vc->user_comments[i] + 12);
      fi->Comment[vc->comment_lengths[i] - 12] = '\0';
    }
    
    if (!(strncmp(vc->user_comments[i], "YEAR", 4)))
    {
      fi->Year = atoi(vc->user_comments[i] + 5);
    }
    
    if (!(strncmp(vc->user_comments[i], "TRACKNUMBER", 11)))
    {
      fi->Track = atoi(vc->user_comments[i] + 12);
    }
  }
  
  if (!fi->Title)
  {
    fi->Title = malloc(4);
    strcpy(fi->Title, "???");
  }
  
  if (!fi->Artist)
  {
    fi->Artist = malloc(4);
    strcpy(fi->Artist, "???");
  }
  
  if (!fi->Album)
  {
    fi->Album = malloc(4);
    strcpy(fi->Album, "???");
  }
  
  if (!fi->Genre)
  {
    fi->Genre = malloc(4);
    strcpy(fi->Genre, "???");
  }
  
  if (!fi->Comment)
  {
    fi->Comment = malloc(4);
    strcpy(fi->Comment, "???");
  }
  
  fi->Seekable = ov_seekable(&tempov);
  
  fi->Length = fi->Seekable ? ov_time_total(&tempov, -1) : 0;
  
  ov_clear(&tempov);
  
  return GET_FILE_INFO_OK;
}

enum FILL_BUFFER_RETURN FillBuffer(unsigned char* buffer, size_t* length)
{
  size_t                mlength = *(length);
  int                   bitstream;
  long                  ReadSize;
  
  enum FILL_BUFFER_RETURN rc = FILL_BUFFER_OK;
  
  while (mlength > 0 && rc == FILL_BUFFER_OK)
  {
    ReadSize = ov_read(&ov,
                       (char*)buffer,
                       mlength,
                       0,
                       2,
                       1,
                       &bitstream);
    switch (ReadSize)
    {
      case 0:
        rc = FILL_BUFFER_EOFF;
        break;

      case OV_EBADLINK:
      case OV_EINVAL:
        rc = FILL_BUFFER_ERROR;
        break;
      
      case OV_HOLE:
      default:
        /* ReadSize contains actual bytes read */
        mlength -= ReadSize;
        buffer += ReadSize;
        break;
    }
  }
  
  return rc;
}

enum OPEN_FILE_RETURN OpenFile(const char* filename, pa_sample_spec* ss)
{
  if (ov_fopen(filename, &ov) < 0)
  {
    return OPEN_FILE_ERROR;
  }
  else
  {
    ss->rate = 44100;
    ss->channels = 2;
    ss->format = PA_SAMPLE_S16LE;
    
    return OPEN_FILE_OK;
  }
}

void Cleanup()
{
  ov_clear(&ov);
}