#ifndef PLUGLOADER_H__
#define PLUGLOADER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <dlfcn.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <pulse/pulseaudio.h>

#include "Plugins/plugin_api.h"
#include "debug.h"

typedef struct funcs
{
  init_t* Initialise;
  info_t* GetFileInfo;
  fill_t* FillBuffer;
  open_t* OpenFile;
  seek_t* Seek;
  clean_t* Cleanup;
  
  void* PluginSOHandle;
  
  char** Extensions;
} funcs_t;

typedef struct funcsl
{
  funcs_t* funcs;
  struct funcsl* next;
} funcsl_t;

void LoadPlugins(char* path);
funcs_t* GetFuncs(const char* extension);
void UnloadPlugins();

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /*PLUGLOADER_H__*/
