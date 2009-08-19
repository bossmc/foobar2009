#ifndef COLUMNBROWSER_H__
#define COLUMNBROWSER_H__

#include <Fl/Fl.H>
#include <Fl/Fl_Multi_Browser.H>
#include <Fl/fl_draw.H>

#include "debug.h"

#define MIN_COL_SIZE 20

class ColumnBrowser : public Fl_Multi_Browser
{
public:
	ColumnBrowser(int x, int y, int w, int h);
	virtual ~ColumnBrowser();
  
  void bbox(int& x, int& y, int& w, int& h);
};

#endif /*COLUMNBROWSER_H__*/
