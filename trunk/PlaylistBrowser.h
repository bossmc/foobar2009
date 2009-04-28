#ifndef PLAYLISTBROWSER_H__
#define PLAYLISTBROWSER_H__

#include <string.h>
#include <Fl/Fl_Button.H>
#include <Fl/Fl_Group.H>
#include "ColumnBrowser.h"
#include "plug-loader.h"
#include "globals.h"

enum COL_DATA
{
  COL_DATA_TITLE,
  COL_DATA_ARTIST,
  COL_DATA_ALBUM,
  COL_DATA_YEAR,
  COL_DATA_TRACKNUMBER,
  COL_DATA_GENRE,
  COL_DATA_COMMENT,
  COL_DATA_LENGTH
};

typedef struct cols
{
  enum COL_DATA* types;
  int*   widths;
  int    NumCols;
} cols_t;

class PlaylistBrowser : public Fl_Group
{
public:
  PlaylistBrowser(int x, int y, int w, int h, cols_t ColumnData);
  ~PlaylistBrowser();
  
  void draw();
  int handle(int e);
  void resize(int x, int y, int w, int h);
  
  void add(char* text, void* userdata);
  FileInfo_t* GetNextSong();
  void AddFile(const char* filename);
  void AddDirectory(const char* path);

private:
  void ChangeCursor(Fl_Cursor c);
  int NearColumn();

  ColumnBrowser* m_Browser;
  Fl_Button** m_Buttons;
  
  int    m_CurrentlyPlaying;
  
  char** m_Titles;
  int*   m_Widths;
  int    m_NumCols;
  
  enum COL_DATA* m_Types;
};

#endif /*PLAYLISTBROWSER_H__*/
