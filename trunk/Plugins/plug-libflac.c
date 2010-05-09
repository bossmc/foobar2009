#include "plugin_api.h"
#include "FLAC/stream_decoder.h"
#include "FLAC/metadata.h"

FLAC__StreamDecoder* flac_decoder = NULL;
FLAC__StreamMetadata* flac_metadata_streaminfo = NULL;
FLAC__StreamMetadata* flac_metadata_vorbis_comment = NULL;
const FLAC__Frame* flac_frame = NULL;
const FLAC__int32** flac_buffer = NULL;

FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder *decoder,
                                              const FLAC__Frame *frame,
                                              const FLAC__int32 *const buffer[],
                                              void *client_data)
{
  flac_frame = frame;
  flac_buffer = (const FLAC__int32**)buffer;
  
  return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void metadata_callback(const FLAC__StreamDecoder *decoder,
                       const FLAC__StreamMetadata *metadata,
                       void *client_data)
{
  printf("metadata_callback\n");
  if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO)
  {
    if (flac_metadata_streaminfo)
    {
      FLAC__metadata_object_delete(flac_metadata_streaminfo);
    }
    
    flac_metadata_streaminfo = FLAC__metadata_object_clone(metadata);
  }
  else if (metadata->type == FLAC__METADATA_TYPE_VORBIS_COMMENT)
  {
    if (flac_metadata_vorbis_comment)
    {
      FLAC__metadata_object_delete(flac_metadata_vorbis_comment);
    }
    
    flac_metadata_vorbis_comment = FLAC__metadata_object_clone(metadata);
  }
}

void error_callback(const FLAC__StreamDecoder *decoder,
                    FLAC__StreamDecoderErrorStatus status,
                    void *client_data)
{
  fprintf(stderr,
          "FLAC ERROR: Got error callback: %s\n",
          FLAC__StreamDecoderErrorStatusString[status]);
}

void Initialise(char*** Extension)
{
  *Extension = (char**)malloc(sizeof(char*) * 2);
  (*Extension)[0] = (char*)"flac";
  (*Extension)[1] = 0;
}

enum GET_FILE_INFO_RETURN GetFileInfo(const char* filename, FileInfo_t* fi)
{
  FLAC__StreamDecoderInitStatus init_status;
  FLAC__StreamDecoder* decoder = FLAC__stream_decoder_new();
  int i = 0;
  
  if (!decoder)
  {
    return GET_FILE_INFO_ERROR;
  }
  
  FLAC__stream_decoder_set_metadata_respond(decoder,
                                            FLAC__METADATA_TYPE_VORBIS_COMMENT);
  
  if ((init_status = FLAC__stream_decoder_init_file(decoder,
                                                    filename,
                                                    write_callback,
                                                    metadata_callback,
                                                    error_callback,
                                                    NULL)) != FLAC__STREAM_DECODER_INIT_STATUS_OK)
  {
    fprintf(stderr,
            "FLAC ERROR: initializing decoder: %s\n",
            FLAC__StreamDecoderInitStatusString[init_status]);
    FLAC__stream_decoder_delete(flac_decoder);
    return GET_FILE_INFO_ERROR;
  }
  
  FLAC__stream_decoder_process_until_end_of_metadata(decoder);
   
  /* extract track info from metadata in flac_metadata */
  for (i = 0; i < flac_metadata_vorbis_comment->data.vorbis_comment.num_comments; i++)
  {
    if (!(strncmp((const char*)flac_metadata_vorbis_comment->data.vorbis_comment.comments[i].entry, "TITLE", 5)))
    {
      fi->Title = malloc(flac_metadata_vorbis_comment->data.vorbis_comment.comments[i].length - 5);
      strcpy(fi->Title, (const char*)flac_metadata_vorbis_comment->data.vorbis_comment.comments[i].entry + 6);
      fi->Title[flac_metadata_vorbis_comment->data.vorbis_comment.comments[i].length - 6] = '\0';
    }
    
    if (!(strncmp((const char*)flac_metadata_vorbis_comment->data.vorbis_comment.comments[i].entry, "ARTIST", 6)))
    {
      fi->Artist = malloc(flac_metadata_vorbis_comment->data.vorbis_comment.comments[i].length - 6);
      strcpy(fi->Artist, (const char*)flac_metadata_vorbis_comment->data.vorbis_comment.comments[i].entry + 7);
      fi->Artist[flac_metadata_vorbis_comment->data.vorbis_comment.comments[i].length - 7] = '\0';
    }
    
    if (!(strncmp((const char*)flac_metadata_vorbis_comment->data.vorbis_comment.comments[i].entry, "ALBUM", 5)))
    {
      fi->Album = malloc(flac_metadata_vorbis_comment->data.vorbis_comment.comments[i].length - 5);
      strcpy(fi->Album, (const char*)flac_metadata_vorbis_comment->data.vorbis_comment.comments[i].entry + 6);
      fi->Album[flac_metadata_vorbis_comment->data.vorbis_comment.comments[i].length - 6] = '\0';
    }
    
    if (!(strncmp((const char*)flac_metadata_vorbis_comment->data.vorbis_comment.comments[i].entry, "GENRE", 5)))
    {
      fi->Genre = malloc(flac_metadata_vorbis_comment->data.vorbis_comment.comments[i].length - 5);
      strcpy(fi->Genre, (const char*)flac_metadata_vorbis_comment->data.vorbis_comment.comments[i].entry + 6);
      fi->Genre[flac_metadata_vorbis_comment->data.vorbis_comment.comments[i].length - 6] = '\0';
    }
    
    if (!(strncmp((const char*)flac_metadata_vorbis_comment->data.vorbis_comment.comments[i].entry, "DESCRIPTION", 11)))
    {
      fi->Comment = malloc(flac_metadata_vorbis_comment->data.vorbis_comment.comments[i].length - 11);
      strcpy(fi->Comment, (const char*)flac_metadata_vorbis_comment->data.vorbis_comment.comments[i].entry + 12);
      fi->Comment[flac_metadata_vorbis_comment->data.vorbis_comment.comments[i].length - 12] = '\0';
    }
    
    if (!(strncmp((const char*)flac_metadata_vorbis_comment->data.vorbis_comment.comments[i].entry, "YEAR", 4)))
    {
      fi->Year = atoi((const char*)flac_metadata_vorbis_comment->data.vorbis_comment.comments[i].entry + 5);
    }
    
    if (!(strncmp((const char*)flac_metadata_vorbis_comment->data.vorbis_comment.comments[i].entry, "TRACKNUMBER", 11)))
    {
      fi->Track = atoi((const char*)flac_metadata_vorbis_comment->data.vorbis_comment.comments[i].entry + 12);
    }

    fi->Length = (int)(flac_metadata_streaminfo->data.stream_info.total_samples / flac_metadata_streaminfo->data.stream_info.sample_rate) + 1;
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
  
  fi->Seekable = 1;
  
  FLAC__stream_decoder_delete(decoder);
  return GET_FILE_INFO_OK;
}

enum OPEN_FILE_RETURN OpenFile(const char* filename, pa_sample_spec* ss)
{
  FLAC__StreamDecoderInitStatus init_status;

  if ((flac_decoder = FLAC__stream_decoder_new()) == NULL)
  {
    fprintf(stderr,
            "FLAC ERROR: allocating decoder\n");
    return OPEN_FILE_ERROR;
  }
  
  if ((init_status = FLAC__stream_decoder_init_file(flac_decoder,
                                                    filename,
                                                    write_callback,
                                                    metadata_callback,
                                                    error_callback,
                                                    NULL)) != FLAC__STREAM_DECODER_INIT_STATUS_OK)
  {
    fprintf(stderr,
            "FLAC ERROR: initializing decoder: %s\n",
            FLAC__StreamDecoderInitStatusString[init_status]);
    FLAC__stream_decoder_delete(flac_decoder);
    return OPEN_FILE_ERROR;
  }
  
  FLAC__stream_decoder_process_until_end_of_metadata(flac_decoder);
  
  ss->rate = flac_metadata_streaminfo->data.stream_info.sample_rate;
  ss->channels = flac_metadata_streaminfo->data.stream_info.channels;
  
  switch (flac_metadata_streaminfo->data.stream_info.bits_per_sample)
  {
    case 32:
      ss->format = PA_SAMPLE_S32LE;
      break;
    case 16:
      ss->format = PA_SAMPLE_S16LE;
      break;
    case 8:
      ss->format = PA_SAMPLE_U8;
      break;
    default:
      ss->format = PA_SAMPLE_INVALID;
      break;
  }
  
  if (ss->format == PA_SAMPLE_INVALID)
  {
    FLAC__stream_decoder_delete(flac_decoder);
    return OPEN_FILE_ERROR;
  }
  
  return OPEN_FILE_OK;
}

enum FILL_BUFFER_RETURN FillBuffer(unsigned char* pBuf, size_t* length)
{
  static int samples_remaining = 0;
  static int current_sample = 0;
  size_t temp_length = *length;
  FLAC__int16 sample_value = 0;
  int i = 0;
  enum FILL_BUFFER_RETURN rc = FILL_BUFFER_OK;
  
  /* copy the samples out of flac_buffer and into pBuf (in the right order!) */
  while (temp_length)
  {
    if (!samples_remaining)
    {
      if (rc == FILL_BUFFER_EOFF)
      {
        *length = temp_length;
        return rc;
      }
      
      FLAC__stream_decoder_process_single(flac_decoder);
      if (FLAC__stream_decoder_get_state(flac_decoder) == FLAC__STREAM_DECODER_END_OF_STREAM)
      {
        rc = FILL_BUFFER_EOFF;
      }
      
      samples_remaining = flac_frame->header.blocksize;
      current_sample = 0;
    }
    
    for (i = 0; i < flac_frame->header.channels; i++)
    {
      sample_value = (FLAC__int16)flac_buffer[i][current_sample];
      *(pBuf++) = (flac_buffer[i][current_sample] >> 0) & 0xff;
      *(pBuf++) = (flac_buffer[i][current_sample] >> 8) & 0xff;
    }
    
    temp_length -= flac_frame->header.channels * 2;
    samples_remaining--;
    current_sample++;
  }
  
  return rc;
}

enum SEEK_RETURN Seek(int* seconds)
{
  debug("Seek");
  return SEEK_OK;
}

void Cleanup()
{
  if (flac_metadata_streaminfo)
  {
    FLAC__metadata_object_delete(flac_metadata_streaminfo);
  }
  
  if (flac_metadata_vorbis_comment)
  {
    FLAC__metadata_object_delete(flac_metadata_vorbis_comment);
  }
  
  if (flac_decoder)
  {
    FLAC__stream_decoder_delete(flac_decoder);
  }
}
