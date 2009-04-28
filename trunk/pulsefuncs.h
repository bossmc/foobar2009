#ifndef PULSE_THREAD_H__
#define PULSE_THREAD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <pulse/pulseaudio.h>
#include "globals.h"
#include "callbacks.h"
#include "plug-loader.h"
#include "c_wrapper.h"

int PulseInit();

void PulsePlayNextSong();
void PulsePlayPause();
void PulseStop();
void PulsePlay();

void PulseFini();

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /*PULSE_THREAD_H__*/
