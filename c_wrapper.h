#ifndef C_WRAPPER_H__
#define C_WRAPPER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plug-loader.h"

FileInfo_t* GetNextSong();

#ifdef __cplusplus
}
#endif

#endif /*C_WRAPPER_H__*/
