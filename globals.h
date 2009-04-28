#ifndef GLOBALS_H__
#define GLOBALS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <pulse/pulseaudio.h>
#include "plug-loader.h"

enum PULSE_STATE
{
  PULSE_PAUSED = 0x1,
  PULSE_STOPPED = 0x2
};

#ifdef GLOBALISE__
#define GLOBALVAR(X, Y) X Y
#else
#define GLOBALVAR(X, Y) extern X Y
#endif

GLOBALVAR(pa_threaded_mainloop*, PALoop);
GLOBALVAR(pa_context*, PAContext);
GLOBALVAR(pa_stream*, PAStream);
GLOBALVAR(pa_mainloop_api*, PAAPI);
GLOBALVAR(funcs_t*, PACurrentFuncs);
GLOBALVAR(int, PulseState);

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /*GLOBALS_H__*/
