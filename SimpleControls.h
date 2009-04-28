#ifndef SIMPLECONTROLS_H_
#define SIMPLECONTROLS_H_

#include <Fl/Fl_Group.H>
#include <Fl/Fl_Button.H>
#include <Fl/Fl_Slider.H>

#include "debug.h"
#include "globals.h"
#include "pulsefuncs.h"

class SimpleControls : Fl_Group
{
public:
	SimpleControls(int x, int y, int w, int h, char* title = 0);
	virtual ~SimpleControls();
  
private:
  Fl_Button* m_Prev;
  Fl_Button* m_Play;
  Fl_Button* m_Stop;
  Fl_Button* m_Next;
  Fl_Slider* m_Slider;
};

void cb_prev(Fl_Widget* o, void* userdata);
void cb_play(Fl_Widget* o, void* userdata);
void cb_stop(Fl_Widget* o, void* userdata);
void cb_next(Fl_Widget* o, void* userdata);

#endif /*SIMPLECONTROLS_H_*/
