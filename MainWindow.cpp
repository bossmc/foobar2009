#include "MainWindow.h"

MainWindow::MainWindow(int w, int h) : Fl_Double_Window(w, h)
{
  cols_t ColumnData;
  
  LoadConfig(&ColumnData);
  
  m_Menu = new Fl_Menu_Bar(0, 0, w, 25);
  m_Menu->add("File/Add Files...", 0, cb_file_add_file, this);
  m_Menu->add("File/Add Folder...", 0, cb_file_add_folder, this);
  m_Menu->add("File/Exit", 0, cb_file_exit, this);
  
  m_Controls = new SimpleControls(5, 30, w-10, 30);
  m_Playlist = new PlaylistBrowser(5, 65, w - 10, h - 70, ColumnData);
  
  resizable(m_Playlist);
  
  delete[] ColumnData.types;
  delete[] ColumnData.widths;
  
  this->end();
}

MainWindow::~MainWindow()
{
  delete m_Controls;
  delete m_Playlist;
}

FileInfo_t* MainWindow::GetNextSong()
{
  return m_Playlist->GetNextSong();
}

void MainWindow::AddFile(const char* filename)
{
  m_Playlist->AddFile(filename);
}

void MainWindow::AddDirectory(const char* path)
{
  m_Playlist->AddDirectory(path);
}

void MainWindow::LoadConfig(cols_t* ColData)
{
  FILE* filp = NULL;
  char* chunk = NULL;
  int   i;
  
  filp = fopen("~/.foobar2009/foobar2009.cfg", "rb");
  
  if (!filp)
  {
    ColData->NumCols = 5;
    ColData->types = new enum COL_DATA[5];
    ColData->widths = new int[5];
    
    ColData->types[0] = COL_DATA_TRACKNUMBER;
    ColData->types[1] = COL_DATA_TITLE;
    ColData->types[2] = COL_DATA_LENGTH;
    ColData->types[3] = COL_DATA_ARTIST;
    ColData->types[4] = COL_DATA_ALBUM;
    
    ColData->widths[0] = 20;
    ColData->widths[1] = 100;
    ColData->widths[2] = 50;
    ColData->widths[3] = 100;
    ColData->widths[4] = 100;
  }
  else
  {
    chunk = new char[2];
    fread(chunk, 1, 1, filp);
    ColData->NumCols = (int)(chunk[0]);

    ColData->types = new enum COL_DATA[ColData->NumCols];
    ColData->widths = new int[ColData->NumCols];

    for (i = 0; i < ColData->NumCols; i++)
    {
      /* Get the column type */
      fread(chunk, 1, 1, filp);
      ColData->types[i] = (enum COL_DATA)((int)chunk[0]);
      
      /* Get the width of the column */
      fread(chunk, 2, 1, filp);
      ColData->widths[i] = (int)chunk[0];
      ColData->widths[i] += ((int)chunk[1]) << 8;
    }
    
    delete[] chunk;
    
    fclose(filp);
  }
}

void cb_file_add_file(Fl_Widget* o, void* userdata)
{
  Fl_File_Chooser chooser(".",                      // directory
                          "Audio Files (*.{mp3,mp2,mp1,ogg,wav})", // filter
                          Fl_File_Chooser::MULTI,                  // chooser type
                          "Select Audio files to opem");           // title
  chooser.preview(0);
  chooser.show();

  while(chooser.shown())
  {
    Fl::wait();
  }

  if ( chooser.value() != NULL )
  {
    for ( int t=1; t<=chooser.count(); t++ )
    {
      ((MainWindow*)userdata)->AddFile(chooser.value(t));
    }
  }
}

void cb_file_add_folder(Fl_Widget* o, void* userdata)
{
  Fl_File_Chooser chooser("/home/andy/Music",
                          "*",
                          Fl_File_Chooser::DIRECTORY,
                          "Select folder to import");
  chooser.preview(0);
  chooser.show();
  
  while (chooser.shown())
  {
    Fl::wait();
  }
  
  if (chooser.value() != NULL)
  {
    ((MainWindow*)userdata)->AddDirectory(chooser.value());
  }
}

void cb_file_exit(Fl_Widget* o, void* userdata)
{
  ((MainWindow*)userdata)->hide();
}
