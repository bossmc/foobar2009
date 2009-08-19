#ifndef MAINWINDOW_H__
#define MAINWINDOW_H__

#include <Fl/Fl_Double_Window.H>
#include <Fl/Fl_Button.H>
#include <Fl/Fl_Menu_Bar.H>
#include <Fl/Fl_File_Chooser.H>

#include "pulsefuncs.h"
#include "PlaylistBrowser.h"
#include "SimpleControls.h"

class MainWindow : public Fl_Double_Window
{
public:
  MainWindow(int w, int h);
  ~MainWindow();
  
  FileInfo_t* GetNextSong();
  void AddFile(const char* filename);
  void AddDirectory(const char* path);
  
private:
  Fl_Menu_Bar*     m_Menu;
  SimpleControls*  m_Controls;
  PlaylistBrowser* m_Playlist;
  
  void LoadConfig(cols_t* ColData);
  void SaveConfig();
};

void cb_file_add_file  (Fl_Widget* o, void* userdata);
void cb_file_add_folder(Fl_Widget* o, void* userdata);
void cb_file_exit      (Fl_Widget* o, void* userdata);
void cb_playback_play  (Fl_Widget* o, void* userdata);
void cb_playback_next  (Fl_Widget* o, void* userdata);

#endif /*MAINWINDOW_H__*/
