#include "PlaylistBrowser.h"

PlaylistBrowser::PlaylistBrowser(int x, int y, int w, int h, cols_t ColumnData)
  : Fl_Group(x, y, w, h)
{
  int i;
  int runningtotal = 0;
  
  m_NumCols = ColumnData.NumCols;
  
  /* Allocate widths and titles lists */
  m_Widths = new int[m_NumCols + 1];
  m_Titles = new char*[m_NumCols];
  m_Types = new enum COL_DATA[m_NumCols];
  for (i = 0; i < m_NumCols; i++)
  {
    m_Widths[i] = ColumnData.widths[i];
    runningtotal += ColumnData.widths[i];
    m_Types[i] = ColumnData.types[i];
    
    switch (ColumnData.types[i])
    {
      case COL_DATA_TITLE:
        m_Titles[i] = (char*)"Title";
        break;
      
      case COL_DATA_ARTIST:
        m_Titles[i] = (char*)"Artist";
        break;
        
      case COL_DATA_ALBUM:
        m_Titles[i] = (char*)"Album";
        break;
      
      case COL_DATA_YEAR:
        m_Titles[i] = (char*)"Year";
        break;
        
      case COL_DATA_TRACKNUMBER:
        m_Titles[i] = (char*)"#";
        break;
      
      case COL_DATA_GENRE:
        m_Titles[i] = (char*)"Genre";
        break;
      
      case COL_DATA_COMMENT:
        m_Titles[i] = (char*)"Comment";
        break;
      
      case COL_DATA_LENGTH:
        m_Titles[i] = (char*)"Length";
        break;
      
      default:
        m_Titles[i] = (char*)"???";
        break;
    }
  }
  m_Widths[m_NumCols - 1] = w - runningtotal;
  m_Widths[m_NumCols] = 0;
  
  /* Create the browser and the title buttons */
  m_Browser = new ColumnBrowser(x, y + 20, w, h - 20);
  m_Buttons = new Fl_Button*[m_NumCols];
  
  runningtotal = 0;  
  for (i = 0; i < ColumnData.NumCols; i++)
  {
    m_Buttons[i] = new Fl_Button(x + runningtotal,
                                 y,
                                 m_Widths[i],
                                 20,
                                 m_Titles[i]);
    runningtotal += m_Widths[i];
  }
  
  m_Browser->column_widths(m_Widths);
  
  resize(x, y, w, h);
  
  end();
}

PlaylistBrowser::~PlaylistBrowser()
{
  delete m_Browser;
  delete[] m_Widths;
  delete[] m_Titles;
}

void PlaylistBrowser::draw()
{
  int x, y, w, h, i;
  int colx;
  
  Fl_Group::draw();
  
  colx = m_Browser->x();
  m_Browser->bbox(x, y, w, h);
  
  fl_color(FL_BLACK);
  for (i=0; m_Widths[i+1]; i++)
  {
    colx += m_Widths[i];
    if ((colx > x) && (colx < x + w))
    {
      fl_line(colx, y - 1, colx, y+h);
    }
  }
}

int PlaylistBrowser::handle(int e)
{
  static int dragging = 0;
  static int draggedcol = -1;
  int newwidth = 0;
  int i;
  
  switch (e)
  {
    case FL_ENTER:
      Fl_Group::handle(e);
      return 1;
      
    case FL_MOVE:
      if (NearColumn() >= 0)
      {
        ChangeCursor(FL_CURSOR_WE);
      }
      else
      {
        ChangeCursor(FL_CURSOR_DEFAULT);
      }
      Fl_Group::handle(e);
      return 1;
      
    case FL_PUSH:
      if ((draggedcol = NearColumn()) >= 0)
      {
        dragging = 1;
      }
      else
      {
        Fl_Group::handle(e);
      }
      return 1;
      
    case FL_DRAG:
      if (dragging)
      {
        newwidth = Fl::event_x() - x();
        for (i = 0; i < draggedcol; i++)
        {
          newwidth -= m_Widths[i];
        }
        if (newwidth > 0)
        {
          /* Columns cannot be smaller than MIN_COL_SIZE */
          if (newwidth < MIN_COL_SIZE)
          {
            newwidth = MIN_COL_SIZE;
          }
          if (m_Widths[draggedcol] + m_Widths[draggedcol + 1] - newwidth < MIN_COL_SIZE)
          {
            newwidth = m_Widths[draggedcol] + m_Widths[draggedcol + 1] - MIN_COL_SIZE;
          }
          
          /* Update button sizes */
          m_Buttons[draggedcol]->resize(m_Buttons[draggedcol]->x(),
                                        m_Buttons[draggedcol]->y(),
                                        newwidth,
                                        20);
          m_Buttons[draggedcol + 1]->resize(m_Buttons[draggedcol]->x() + newwidth,
                                            m_Buttons[draggedcol + 1]->y(),
                                            m_Widths[draggedcol]
                                             + m_Widths[draggedcol + 1]
                                             - newwidth,
                                            20);
          
          /* Update column widths */
          m_Widths[draggedcol + 1] = m_Widths[draggedcol]
                                     + m_Widths[draggedcol + 1]
                                     - newwidth;
          m_Widths[draggedcol] = newwidth;
          redraw();
        }
      }
      Fl_Group::handle(e);
      return 1;
      
    case FL_LEAVE:
      dragging = 0;
      ChangeCursor(FL_CURSOR_DEFAULT);
      Fl_Group::handle(e);
      return 1;
      
    case FL_RELEASE:
      dragging = 0;
      ChangeCursor(FL_CURSOR_DEFAULT);
      Fl_Group::handle(e);
      return 1;
    
    default:
      return Fl_Group::handle(e);
      break;
  }
}

void PlaylistBrowser::resize(int x, int y, int w, int h)
{
  int wo = w;
  int totalcolwidth = 0;
  int excess = 0;
  int i, j;
  
  Fl_Group::resize(x, y, w, h);
  
  for (i = 0; m_Widths[i]; i++)
  {
    totalcolwidth += m_Widths[i];
  }
  
  /* Starting at the end shrink columns till total width is w, no column is
   * allowed to be less than MIN_COL_SIZE wide */
  for (j = i - 1; j >= 0; j--)
  {
    if (totalcolwidth > wo)
    {
      excess = totalcolwidth - wo;
      if(m_Widths[j] - excess < MIN_COL_SIZE)
      {
        totalcolwidth -= m_Widths[j] - MIN_COL_SIZE;
        m_Widths[j] = MIN_COL_SIZE;
      }
      else
      {
        totalcolwidth -= excess;
        m_Widths[j] -= excess;
      }
    }
    else
    {
      break;
    }
  }
  
  /* The last column width should fill up the rest of the width */
  for (i = 0; m_Widths[i+1]; i++)
  {
    wo -= m_Widths[i];
  }
  
  m_Widths[i] = wo;
  
  totalcolwidth = 0;
  for (i = 0; m_Widths[i]; i++)
  {
    m_Buttons[i]->resize(totalcolwidth + x,
                         y,
                         m_Widths[i],
                         20);
    totalcolwidth += m_Widths[i];
  }
  
  m_Browser->resize(x, y + 20, w, h - 20);
}

void PlaylistBrowser::add(char* text, void* userdata)
{
  m_Browser->add(text, userdata);
}

FileInfo_t* PlaylistBrowser::GetNextSong()
{
  if (m_Browser->size())
  {
    m_CurrentlyPlaying++;
    
    if (m_CurrentlyPlaying > m_Browser->size())
    {
      m_CurrentlyPlaying = 0;
      PulseState |= PULSE_STOPPED;
      return NULL;
    }
    else
    {
      return (FileInfo_t*)m_Browser->data(m_CurrentlyPlaying);
    }
  }
  else
  {
    PulseState |= PULSE_STOPPED;
    return NULL;
  }
}

void PlaylistBrowser::AddFile(const char* filename)
{
  funcs_t* Funcs = NULL;
  FileInfo_t* fi = new FileInfo_t;
  int i;
  char FileEntry[1024];
  char NumberString[32];
  
  fi->Title = 0;
  fi->Artist = 0;
  fi->Album = 0;
  fi->Year = 0;
  fi->Track = 0;
  fi->Genre = 0;
  fi->Comment = 0;
  fi->Length = 0;
  fi->Path = 0;
  
  memset(FileEntry, '\0', sizeof(FileEntry));
  
  Funcs = GetFuncs(filename + strlen(filename) - 3);
  
  if (!Funcs)
  {
    /* No plugin can handle this file */
    debug("No handler for this file");
    delete fi;
    return;
  }
  
  fi->Funcs = Funcs;
  fi->Path = (char*)malloc(strlen(filename) + 1);
  strcpy(fi->Path, filename);
  
  if (Funcs->GetFileInfo(filename, fi) == GET_FILE_INFO_OK)
  {
    for (i = 0; i < m_NumCols; i++)
    {
      switch (m_Types[i])
      {
        case COL_DATA_TITLE:
          strcat(FileEntry, fi->Title);
          strcat(FileEntry, "\t");
          break;
          
        case COL_DATA_ARTIST:
          strcat(FileEntry, fi->Artist);
          strcat(FileEntry, "\t");
          break;
          
        case COL_DATA_ALBUM:
          strcat(FileEntry, fi->Album);
          strcat(FileEntry, "\t");
          break;
          
        case COL_DATA_YEAR:
          sprintf(NumberString, "%04d", fi->Year);
          strcat(FileEntry, NumberString);
          strcat(FileEntry, "\t");
          break;
          
        case COL_DATA_TRACKNUMBER:
          sprintf(NumberString, "%d", fi->Track);
          strcat(FileEntry, NumberString);
          strcat(FileEntry, "\t");
          break;
          
        case COL_DATA_GENRE:
          strcat(FileEntry, fi->Genre);
          strcat(FileEntry, "\t");
          break;
          
        case COL_DATA_COMMENT:
          strcat(FileEntry, fi->Comment);
          strcat(FileEntry, "\t");
          break;
        
        case COL_DATA_LENGTH:
          sprintf(NumberString, "%d:%02d", (fi->Length) / 60, (fi->Length) % 60);
          strcat(FileEntry, NumberString);
          strcat(FileEntry, "\t");
          break;
      }
    }
  }
  else
  {
    for (i = 0; i < m_NumCols; i++)
    {
      strcat(FileEntry, "???\t");
    }
  }
  
  FileEntry[strlen(FileEntry)-1] = '\0';
  m_Browser->add(FileEntry, fi);
  
  Fl::check();
}

void PlaylistBrowser::AddDirectory(const char* path)
{
  DIR* dirp = NULL;
  struct dirent* direntp = NULL;
  char* NextFile = NULL;
  
  dirp = opendir(path);
  direntp = readdir(dirp);
  
  do
  {
    switch (direntp->d_type)
    {
      case DT_DIR:
        if ((!strcmp(direntp->d_name, ".")) || (!strcmp(direntp->d_name, "..")))
        {
          direntp = readdir(dirp);
          continue;
        }
        
        NextFile = new char[strlen(path) + strlen(direntp->d_name) + 2];
        strcpy(NextFile, path);
        strcat(NextFile, "/");
        strcat(NextFile, direntp->d_name);
        AddDirectory(NextFile);
        delete[] NextFile;
        break;
        
      case DT_REG:
        NextFile = new char[strlen(path) + strlen(direntp->d_name) + 2];
        strcpy(NextFile, path);
        strcat(NextFile, "/");
        strcat(NextFile, direntp->d_name);
        AddFile(NextFile);
        delete[] NextFile;
        break;
      
      default:
        break;
    }
    
    direntp = readdir(dirp);
  }
  while (direntp != NULL);
  
  closedir(dirp);  
}

void PlaylistBrowser::ChangeCursor(Fl_Cursor c)
{
  static Fl_Cursor temp = FL_CURSOR_DEFAULT;
  
  if (c != temp)
  {
    fl_cursor(c, FL_BLACK, FL_WHITE);
    temp = c;
  }
}

int PlaylistBrowser::NearColumn()
{
  int x, y, w, h;
  int mousex, colx, t;
  
  m_Browser->bbox(x, y, w, h);
  if (!Fl::event_inside(x, y - 20, w, h + 20))
  {
    return(-1);
  }
  
  mousex = Fl::event_x();
  colx = this->x();
  for (t=0; m_Widths[t]; t++)
  {
    colx += m_Widths[t];
 
    if ((mousex - colx >= -MIN_COL_SIZE/4) && (mousex - colx <= MIN_COL_SIZE/4) && m_Widths[t+1])
    {
      return (t);
      if ((mousex - colx <= mousex - colx - m_Widths[t+1]) || (!m_Widths[t+1]))
      {
        return(t);
      }
      else
      {
        return(t+1);
      }
    }
  }
  return(-1);
}