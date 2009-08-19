#ifndef CALLBACKS_H__
#define CALLBACKS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <pulse/pulseaudio.h>
#include <stdio.h>

#include "globals.h"
#include "pulsefuncs.h"
#include "plug-loader.h" 
#include "debug.h"

void cb_context_state(pa_context* c, void* userdata);
void cb_context_drain(pa_context* c, void* userdata);

void cb_stream_state(pa_stream* s, void* userdata);
void cb_stream_write(pa_stream* s, size_t length, void* userdata);
void cb_stream_cork(pa_stream* s, int success, void* userdata);
void cb_stream_flush(pa_stream* s, int success, void* userdata);
void cb_stream_drain(pa_stream* s, int success, void* userdata);
void cb_stream_suspended(pa_stream *s, void *userdata);
void cb_stream_underflow(pa_stream *s, void *userdata);
void cb_stream_overflow(pa_stream *s, void *userdata);
void cb_stream_moved(pa_stream *s, void *userdata);

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* CALLBACKS_H__ */
