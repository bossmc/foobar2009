#include "callbacks.h"

FILE* tempfilp;

void cb_context_state(pa_context* c, void* userdata)
{
  switch (pa_context_get_state(c))
  {  
    case PA_CONTEXT_CONNECTING:
      debug("Context message CONNECTING received");
      break;
    
    case PA_CONTEXT_AUTHORIZING:
      debug("Context message AUTHORIZING received");
      break;
    
    case PA_CONTEXT_SETTING_NAME:
      debug("Context message SETTING_NAME received");
      break;

    case PA_CONTEXT_READY:
      debug("Context message READY received");
      PulsePlayNextSong();
      break;

    case PA_CONTEXT_TERMINATED:
      debug("Context message TERMINATED received");
      PAAPI->quit(PAAPI, 0);
      break;

    case PA_CONTEXT_FAILED:
    default:
      fprintf(stderr, "Context failure: %s", pa_strerror(pa_context_errno(c)));
  }
}

void cb_context_drain(pa_context* c, void* userdata)
{
  debug("Context drain message received");
  pa_context_disconnect(c);
}

void cb_stream_state(pa_stream* s, void* userdata)
{
  switch (pa_stream_get_state(s))
  {
    case PA_STREAM_CREATING:
      debug("Stream state CREATING message received");
      tempfilp = fopen("plug-libmad-output.raw", "wb");
      break;
    
    case PA_STREAM_TERMINATED:
      debug("Stream state TERMINATED message received");
      pa_stream_unref(s);
      PulsePlayNextSong();
      break;

    case PA_STREAM_READY:
      debug("Stream state READY message received");
      break;

    case PA_STREAM_FAILED:
    default:
      fprintf(stderr,
              "Stream failure: %s",
              pa_strerror(pa_context_errno(pa_stream_get_context(s))));
      break;
  }
}

void cb_stream_write(pa_stream* s, size_t length, void* userdata)
{
  unsigned char* data = pa_xmalloc(length);
  
  if (PACurrentFuncs)
  {
    switch(PACurrentFuncs->FillBuffer(data, &length))
    {
      case FILL_BUFFER_OK:
        fwrite(data, 1, length, tempfilp);
        pa_stream_write(s,
                        data,
                        length,
                        pa_xfree,
                        0,
                        PA_SEEK_RELATIVE);
        break;
  
      case FILL_BUFFER_EOFF:
        fwrite(data, 1, length, tempfilp);
        pa_stream_write(s,
                        data,
                        length,
                        pa_xfree,
                        0,
                        PA_SEEK_RELATIVE);
        PACurrentFuncs->Cleanup();
        PACurrentFuncs = NULL;
        pa_operation_unref(pa_stream_drain(PAStream,
                                           cb_stream_drain,
                                           NULL));
        break;
  
      case FILL_BUFFER_ERROR:
        debug("FILE_ERROR");
        PACurrentFuncs->Cleanup();
        PACurrentFuncs = NULL;
        pa_xfree(data);
        pa_operation_unref(pa_stream_drain(PAStream,
                                           cb_stream_drain,
                                           NULL));
        break;
    }
  }
}

void cb_stream_drain(pa_stream* s, int success, void* userdata)
{
  debug("Stream drain message received");
  fclose(tempfilp);
  pa_stream_disconnect(s);
  pa_threaded_mainloop_signal(PALoop, 0);
}

void cb_stream_suspended(pa_stream *s, void *userdata)
{
  assert(s);

  if (pa_stream_is_suspended(s))
  {
    debug("Stream device suspended.");
  }
  else
  {
    debug("Stream device resumed.");
  }
}

void cb_stream_underflow(pa_stream *s, void *userdata)
{
  assert(s);

  debug("Stream underrun.");
}

void cb_stream_overflow(pa_stream *s, void *userdata)
{
  assert(s);

  debug("Stream overrun.");
}

void cb_stream_moved(pa_stream *s, void *userdata)
{
  assert(s);

  fprintf(stderr, "Stream moved to device %s (%u, %ssuspended)\n", pa_stream_get_device_name(s), pa_stream_get_device_index(s), pa_stream_is_suspended(s) ? "" : "not ");
}
