#include "ColumnBrowser.h"

ColumnBrowser::ColumnBrowser(int x, int y, int w, int h) : Fl_Multi_Browser(x, y, w, h)
{
  has_scrollbar(VERTICAL);
}

ColumnBrowser::~ColumnBrowser()
{
}

void ColumnBrowser::bbox(int& x, int& y, int& w, int& h)
{
  Fl_Multi_Browser::bbox(x, y, w, h);
}
  