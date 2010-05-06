#include "plug-loader.h"

funcsl_t* FuncList = NULL;

void LoadPlugins(char* path)
{
  DIR* PluginDir = NULL;
  struct dirent* FileEntity = NULL;
  char* FullPluginName = NULL;
  void* PluginSOHandle = NULL;

  funcs_t* TempFunc = NULL;
  funcsl_t** EndOfList = &FuncList;
  
  PluginDir = opendir(path);
  if (PluginDir == NULL)
  {
    perror("Plugin directory does not exist...");
  }
  else
  {
    while ((FileEntity = readdir(PluginDir)) != NULL)
    {
      if (FileEntity->d_type != DT_REG)
      {
        continue;
      }
      if (strncmp(FileEntity->d_name, "plug-", 5))
      {
        continue;
      }
      
      FullPluginName = (char*)malloc(strlen(path) + strlen(FileEntity->d_name) + 2);
      strcpy(FullPluginName, path);
      strcat(FullPluginName, "/");
      strcat(FullPluginName, FileEntity->d_name);
      
      PluginSOHandle = dlopen(FullPluginName, RTLD_LAZY);
      if (PluginSOHandle == NULL)
      {
        free(FullPluginName);
        continue;
      }
      else
      {
        TempFunc = (funcs_t*)malloc(sizeof(funcs_t));
        TempFunc->Cleanup = (clean_t*)dlsym(PluginSOHandle, "Cleanup");
        TempFunc->GetFileInfo = (info_t*)dlsym(PluginSOHandle, "GetFileInfo");
        TempFunc->FillBuffer = (fill_t*)dlsym(PluginSOHandle, "FillBuffer");
        TempFunc->OpenFile = (open_t*)dlsym(PluginSOHandle, "OpenFile");
        TempFunc->Seek = (seek_t*)dlsym(PluginSOHandle, "Seek");
        TempFunc->Initialise = (init_t*)dlsym(PluginSOHandle, "Initialise");
        TempFunc->PluginSOHandle = PluginSOHandle;
        
        if (TempFunc->Cleanup
         && TempFunc->GetFileInfo
         && TempFunc->FillBuffer 
         && TempFunc->OpenFile
         && TempFunc->Seek
         && TempFunc->Initialise)
        {
          TempFunc->Initialise(&(TempFunc->Extensions));
          *(EndOfList) = (funcsl_t*)malloc(sizeof(funcsl_t));
          (*EndOfList)->funcs = TempFunc;
          (*EndOfList)->next = NULL;
          EndOfList = &((*EndOfList)->next);
        }
        else
        {
          printf("Failed to load %s\n", FullPluginName);
          dlclose(PluginSOHandle);
          free(TempFunc);
        }
        free(FullPluginName);
      }
    }
    closedir(PluginDir);
  }
}

funcs_t* GetFuncs(const char* extension)
{
  funcsl_t* ListItem = FuncList;
  funcs_t*  rc = NULL;
  int       i = 0;

  while (ListItem != NULL)
  {
    while (ListItem->funcs->Extensions[i] != 0)
    {
      if (!strcmp(ListItem->funcs->Extensions[i], extension))
      {
        return ListItem->funcs;
      }
      else
      {
        i++;
      }
    }
    
    ListItem = ListItem->next;
    i = 0;
  }
  
  return rc;
}

void UnloadPlugins()
{
  funcsl_t* ListItem = FuncList;
  funcsl_t* NextItem = NULL;
  
  while (ListItem != NULL)
  {
    dlclose(ListItem->funcs->PluginSOHandle);
    free(ListItem->funcs);
    NextItem = ListItem->next;
    free(ListItem);
    ListItem = NextItem;
  }
}
