#include "SimpleControls.h"

SimpleControls::SimpleControls(int x, int y, int w, int h, char* title)
 : Fl_Group(x, y, w, h, title)
{
  m_Prev = new Fl_Button(x, y, h, h, "@|<");
  m_Play = new Fl_Button(x + h + 5, y, h, h, "@>");
  m_Stop = new Fl_Button(x + h + h + 10, y, h, h, "@-2square");
  m_Next = new Fl_Button(x + h + h + h + 15, y, h, h, "@>|");
  m_Slider = new Fl_Slider(x + h + h + h + h + 20, y, w - (h + h + h + h + 20), h);
  
  m_Slider->bounds(0, 1);
  m_Slider->value(0);
  m_Slider->type(FL_HOR_FILL_SLIDER);
  
  m_Prev->callback(cb_prev, NULL);
  m_Play->callback(cb_play, NULL);
  m_Stop->callback(cb_stop, NULL);
  m_Next->callback(cb_next, NULL);
  
  resizable(m_Slider);
  
  end();
}

SimpleControls::~SimpleControls()
{
  delete m_Play;
  delete m_Stop;
  delete m_Next;
  delete m_Prev;
  delete m_Slider;
}

void cb_prev(Fl_Widget* o, void* userdata)
{
  int seconds = 53;

  pa_threaded_mainloop_lock(PALoop);
  PulseSeek(&seconds);
  pa_threaded_mainloop_unlock(PALoop);
}

void cb_play(Fl_Widget* o, void* userdata)
{
  PulsePlay();
}

void cb_stop(Fl_Widget* o, void* userdata)
{
  pa_threaded_mainloop_lock(PALoop);
  PulseStop();
  pa_threaded_mainloop_unlock(PALoop);
}

void cb_next(Fl_Widget* o, void* userdata)
{
  pa_threaded_mainloop_lock(PALoop);
  
  if (!(PulseState & PULSE_STOPPED))
  {
    PulseStop();
    
    /* Pulse state is stopped after pulse stop but stream has not terminated
     * which leads to PulsePlayNextSong being called.  Hence if we just set
     * state back to non-stopped then PlayNextSong will be called correctly */
    PulseState ^= PULSE_STOPPED;
  }
  else
  {
    PulseState ^= PULSE_STOPPED;
    PulsePlayNextSong();
  }
  
  pa_threaded_mainloop_unlock(PALoop);
}
