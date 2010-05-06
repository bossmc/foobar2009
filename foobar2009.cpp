/****************************************************************************
 * Includes                                                                 *
 ****************************************************************************/
#include <pulse/pulseaudio.h>
#include <pthread.h>
#include <stdio.h>

#define GLOBALISE__
#include "globals.h"
#undef GLOBALISE__

#include <Fl/Fl.H>
#include "pulsefuncs.h"
#include "MainWindow.h"
#include "c_wrapper.h"

MainWindow* win;

FileInfo_t* GetNextSong()
{
  return win->GetNextSong();
}

/****************************************************************************
 * Main app entry point.                                                    *
 ****************************************************************************/
int main(int argc, char *argv[])
{
  win = new MainWindow(500, 300);
  win->show(argc, argv);
  
  if (PulseInit() < 0)
  {
    PulseFini();
    return -1;
  }
  
  win->AddFile("/home/andy/Music/Vivaldi/The Four Seasons/Vivaldi - The Four Seasons - 01 - Spring - Allegro.flac");
  
  pa_threaded_mainloop_start(PALoop);
  
  Fl::scheme("gtk+");
  Fl::run();

  PulseFini();
}