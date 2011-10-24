/*
    Copyright (C) 2002 Kjetil S. Matheussen / Notam.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include <stdbool.h>
#include <stdio.h>
#include <windows.h>

#include "windowsstuff.h"

#include "../s_audioMaster.h"


void WINDOWS_delete(struct WindowsStuff *ws){
  FreeLibrary((HINSTANCE)ws->dll);
  free(ws);
}

struct WindowsStuff *WINDOWS_new(char *name,void *winwinstuff){
  typedef AEffect* (*vst_main_func)(audioMasterCallback);
  vst_main_func vstmain;

  struct WindowsStuff *ws=calloc(1,sizeof(struct WindowsStuff));

  ws->winwinstuff=winwinstuff;
  ws->dll=LoadLibrary(name);

  if(ws->dll==NULL){
    char temp[500];
    char *vstpath=getenv("VST_PATH");
    if(vstpath==NULL){
      fprintf(stderr,"VSTSERVANT/WINDOWS_new: Environment variable VST_PATH not set.\n");
    }else{
      if(vstpath[strlen(vstpath)-1]=='/'){
	sprintf(temp,"%s%s",vstpath,name);
      }else{
	sprintf(temp,"%s/%s",vstpath,name);
      }
      printf("trying -%s-\n",temp);
      ws->dll=LoadLibrary(temp);
    }
  }

  if(ws->dll==NULL){
    fprintf(stderr,"Plugin %s not found\n",name);
    free(ws);
    return NULL;
  }
  vstmain = (vst_main_func)GetProcAddress (ws->dll, "main");
  if(vstmain==NULL){
    fprintf(stderr,"Main function not found in plugin %s.\n",name);
    FreeLibrary((HINSTANCE)ws->dll);
    free(ws);
    return NULL;
  }
  ws->effect=vstmain(VSTS_audioMaster);
  if(ws->effect==NULL){
    fprintf(stderr,"Problem starting plugin %s.\n",name); 
    FreeLibrary((HINSTANCE)ws->dll);
    free(ws);
    return NULL;
  }

  if(ws->effect->magic!=kEffectMagic){
    fprintf(stderr,"File %s is probably not a vst plugin.\n",name);
    FreeLibrary((HINSTANCE)ws->dll);
    free(ws);
    return NULL;
  }

  return ws;

}


