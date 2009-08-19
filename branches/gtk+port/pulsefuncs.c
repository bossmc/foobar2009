#include "pulsefuncs.h"

int PulseInit()
{
  LoadPlugins("./Plugins");
  
  if (!(PALoop = pa_threaded_mainloop_new()))
  {
    fprintf(stderr, "pa_mainloop_new() failed\n");
    return -1;
  }

  PAAPI = pa_threaded_mainloop_get_api(PALoop);
  
  if (!(PAContext = pa_context_new(PAAPI, "foobar2009")))
  {
    fprintf(stderr, "pa_context_new() failed");
    return -1;
  }

  pa_context_set_state_callback(PAContext, cb_context_state, NULL);

  if(pa_context_connect(PAContext, NULL, 0, NULL) < 0)
  {
    fprintf(stderr,
            "pa_context_connect() failed with error: %s",
            pa_strerror(pa_context_errno(PAContext)));
    return -1;
  }
  
  return 0;
}

void PulsePlayNextSong()
{
  FileInfo_t* NextSong = NULL;
  
  pa_cvolume PACVol;
  pa_sample_spec PASampleSpec;
  
  debug("Entered PulsePlayNextSong");
  
  if (PulseState & PULSE_STOPPED)
  {
    debug("Pulse is stopped");
    return;
  }
  else
  {
    debug("Pulse not stopped");
  }

  NextSong = GetNextSong();
  if (NextSong)
  {
    PACurrentFuncs = NextSong->Funcs;
    if(PACurrentFuncs->OpenFile(NextSong->Path, &PASampleSpec) == OPEN_FILE_ERROR)
    {
      debug("Error opening file, maybe insufficient plugins?");
      PulsePlayNextSong();
    }
    else
    {
      PAStream = pa_stream_new(PAContext,
                               NextSong->Title,
                               &PASampleSpec,
                               NULL);
      assert(PAStream);
  
      pa_stream_set_write_callback(PAStream, cb_stream_write, NULL);
      pa_stream_set_state_callback(PAStream, cb_stream_state, NULL);
  
      pa_stream_set_suspended_callback(PAStream, cb_stream_suspended, NULL);
      pa_stream_set_moved_callback(PAStream, cb_stream_moved, NULL);
      pa_stream_set_underflow_callback(PAStream, cb_stream_underflow, NULL);
      pa_stream_set_overflow_callback(PAStream, cb_stream_overflow, NULL);
  
      pa_stream_connect_playback(PAStream,
                                 NULL,
                                 NULL,
                                 0,
                                 pa_cvolume_set(&PACVol,
                                                PASampleSpec.channels,
                                                PA_VOLUME_NORM),
                                 NULL);
    }
  }
  else
  {
    /* This should be wait for a signal from GUI that a new song is ready to
     * play */
  }
}

void PulsePlayPause()
{
  pa_threaded_mainloop_lock(PALoop);
  
  PulseState ^= PULSE_PAUSED;
  pa_stream_cork(PAStream, PulseState & PULSE_PAUSED, NULL, NULL);
  
  pa_threaded_mainloop_unlock(PALoop);
}

void PulseStop()
{
  pa_operation* o;
    
  if (!(PulseState & PULSE_STOPPED))
  {
    /* Stop pulse before PulsePlayNextSong() gets called again */
    PulseState ^= PULSE_STOPPED;
    
    /* Drain the stream and wait for the operation to finish which prevents 
     * songs overlapping */
    pa_stream_flush(PAStream, NULL, NULL);
    o = pa_stream_drain(PAStream, cb_stream_drain, NULL);
    
    /* We might end up here after EOFF but before the stream has been reset
     * in which case PACurrentFuncs have already been cleared up so don't do 
     * it again */
    if (PACurrentFuncs)
    {
      PACurrentFuncs->Cleanup();
      PACurrentFuncs = NULL;
    }
    
    if (o)
    {
      while (pa_operation_get_state(o) == PA_OPERATION_RUNNING)
      {
        debug("Waiting for drain signal");
        pa_threaded_mainloop_wait(PALoop);
      }
      pa_operation_unref(o);
    }
  }
}

void PulsePlay()
{
  pa_threaded_mainloop_lock(PALoop);
  
  if (!(PulseState & PULSE_STOPPED))
  {
    PulsePlayPause();
  }
  else
  {
    PulseState ^= PULSE_STOPPED;
    PulsePlayNextSong();
  }
  
  pa_threaded_mainloop_unlock(PALoop);
}

void PulseSeek(int* seconds)
{
  pa_operation* o;
  
  o = pa_stream_cork(PAStream, 1, cb_stream_cork, NULL);
  while (pa_operation_get_state(o) == PA_OPERATION_RUNNING)
    pa_threaded_mainloop_wait(PALoop);
  pa_operation_unref(o);
  
  o = pa_stream_flush(PAStream, cb_stream_flush, NULL);
  while (pa_operation_get_state(o) == PA_OPERATION_RUNNING)
    pa_threaded_mainloop_wait(PALoop);
  pa_operation_unref(o);
  
  PACurrentFuncs->Seek(seconds);
  
  o = pa_stream_cork(PAStream, 0, cb_stream_cork, NULL);
  while (pa_operation_get_state(o) == PA_OPERATION_RUNNING)
    pa_threaded_mainloop_wait(PALoop);
  pa_operation_unref(o);
}

void PulseFini()
{
  if (PALoop)
  {
    pa_threaded_mainloop_free(PALoop);
  }
  
  UnloadPlugins();
}
