#ifndef PLUG_LIBMAD_H__
#define PLUG_LIBMAD_H__

#include <mad.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <id3/misc_support.h>
#include <pulse/pulseaudio.h>

#include "bstdfile.h"
#include "MP3_header.h"
#include "plugin_api.h"

#define INPUT_BUFFER_SIZE (5*8192)
#define OUTPUT_BUFFER_SIZE 8192 /* must be multiple of 4 */

/* Internal functions and enums*/
enum FILL_S_BUFF_RETURN
{
  OK,
  EOFF,
  EOFF_JUST,
  ERROR
};

typedef struct FrameInfo
{
  size_t Index;
  struct FrameInfo* next;
} FrameInfo_t;

enum FILL_S_BUFF_RETURN FillStreamBuffer(struct mad_stream* s,
                                         unsigned char* buf,
                                         size_t* length,
                                         bstdfile_t* bfilp);
int GetLength(const char* filename);

#endif /* PLUG_LIBMAD_H__ */