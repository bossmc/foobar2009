#ifndef PLUGIN_API_H__
#define PLUGIN_API_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>
#include <pulse/pulseaudio.h>

#include "../debug.h"

enum GET_FILE_INFO_RETURN
{
  GET_FILE_INFO_OK,
  GET_FILE_INFO_ERROR
};

enum FILL_BUFFER_RETURN
{
  FILL_BUFFER_OK,
  FILL_BUFFER_EOFF,
  FILL_BUFFER_ERROR
};

enum SEEK_RETURN
{
  SEEK_OK,
  SEEK_EOFF,
  SEEK_ERROR
};

enum OPEN_FILE_RETURN
{
  OPEN_FILE_OK,
  OPEN_FILE_ERROR
};

struct funcs;

typedef struct FileInfo
{
  char* Title;
  char* Artist;
  char* Album;
  int   Year;
  int   Track;
  char* Genre;
  char* Comment;
  int   Length;
  int   Seekable;
  char* Path;
  struct funcs* Funcs;
} FileInfo_t;   

/******************************************************************************/
/* Plugin entry point function signatures.                                    */
/******************************************************************************/
typedef void (init_t)(char*** Extensions);
typedef enum GET_FILE_INFO_RETURN (info_t)(const char* filename, FileInfo_t* fi);
typedef enum FILL_BUFFER_RETURN (fill_t)(unsigned char* pBuf, size_t* length);
typedef enum OPEN_FILE_RETURN   (open_t)(const char* filename, pa_sample_spec* ss);
typedef enum SEEK_RETURN        (seek_t)(int* seconds);
typedef void (clean_t)();

/******************************************************************************/
/* Plugin entry point functions:                                              */
/******************************************************************************/
init_t Initialise;
info_t GetFileInfo;
fill_t  FillBuffer;
seek_t Seek;
open_t OpenFile;
clean_t Cleanup;

#ifdef __cplusplus
}
#endif

#endif /*PLUGIN_API_H__*/
