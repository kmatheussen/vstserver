/*
    Copyright (C) 2002 Kjetil S. Matheussen / Notam.
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.
    
    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software 
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
    
*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>

#include <vstlib.h>

#include <vstserver.h>

#include "vstfuncs.h"

#include <k_locks.h>
#include <k_cache.h>

#include "newdelete.h"



float VSTLIB_cache_getParameter(
		      AEffect *effect, 
		      long index
		      )
{
  struct MyAEffect *myeffect=(struct MyAEffect*)effect;
  struct Locks *locks;
  char temp[500];
  float ret;
  char lockfilename[500];
  sprintf(lockfilename,"%s/.vstserver/locks/l_",getenv("HOME"));

  sprintf(temp,"_paramval_%d",(int)index);
  locks=LOCKS_new(lockfilename,0);
  CACHE_getPart(myeffect->name,temp,&ret,sizeof(float));
  LOCKS_delete(locks);

  return ret;
	       
}



long VSTLIB_cache_dispatcher(
		    AEffect *effect,
		    long opCode,
		    long index,
		    long value,
		    void *ptr,
		    float opt
		    )
{
  struct MyAEffect *myeffect=(struct MyAEffect*)effect;
  struct Locks *locks;
  char temp[500];
  char lockfilename[500];
  sprintf(lockfilename,"%s/.vstserver/locks/l_",getenv("HOME"));

  switch(opCode){
  case effGetParamName:
    sprintf(temp,"_paramname_%d",(int)index);
    locks=LOCKS_new(lockfilename,0);
    CACHE_getPart(myeffect->name,temp,ptr,9);
    LOCKS_delete(locks);
    return 0;
  case effGetParamDisplay:
    sprintf(temp,"_paramdisplay_%d",(int)index);
    locks=LOCKS_new(lockfilename,0);
    CACHE_getPart(myeffect->name,temp,ptr,25);
    LOCKS_delete(locks);
    return 0;
  case effGetParamLabel:
    sprintf(temp,"_paramlabel_%d",(int)index);
    locks=LOCKS_new(lockfilename,0);
    CACHE_getPart(myeffect->name,temp,ptr,9);
    LOCKS_delete(locks);
    return 0;
  default:
    break;
  }

  return -1;
}





struct AEffect *VSTLIB_newCache(const char *name){
  struct MyAEffect *myeffect;
  struct AEffect *effect;
  struct VSTLib *vstlib;
  struct Locks *locks;
  int confresult;
  char lockfilename[500];
  sprintf(lockfilename,"%s/.vstserver/locks/l_",getenv("HOME"));

  if(name==NULL) return NULL;

  confresult=CACHE_checkPluginUpdate(name);

  if(confresult==-1) return NULL;
  if(confresult==1){
    effect=VSTLIB_newReal(name,false);
    VSTLIB_delete(effect);
    confresult=CACHE_checkPluginUpdate(name);
    if(confresult==-1 || confresult==1) return NULL;
  }

  vstlib=calloc(1,sizeof(struct VSTLib));
  vstlib->connectedtoserver=false;
  myeffect=calloc(1,sizeof(struct MyAEffect));
  myeffect->vstlib=vstlib;
  effect=&myeffect->effect;

  myeffect->name=malloc(strlen(name)+1);
  sprintf(myeffect->name,"%s",name);

  locks=LOCKS_new(lockfilename,0);

  if(CACHE_getPart(name,"",effect,sizeof(struct AEffect))==false){
    fprintf(stderr,"VSTLIB/newCache: Something very strange has happened. Please report this message. (%s)\n",name);
    free(vstlib);
    free(myeffect);
    LOCKS_delete(locks);
    return NULL;
  }
  LOCKS_delete(locks);

  effect->flags |=  effFlagsCanReplacing ;

  effect->dispatcher=VSTLIB_cache_dispatcher;
  effect->getParameter=VSTLIB_cache_getParameter;

  return effect;
}

void VSTLIB_deleteCacheList(struct AEffect **aeffect){
  int lokke=0;
  if(aeffect==NULL) return;

  while(aeffect[lokke]!=NULL){
    VSTLIB_delete(aeffect[lokke]);
    lokke++;
  }

  free(aeffect);
}


static int myselect(const struct dirent *dasdirent){
  if(!strcmp("..",dasdirent->d_name)) return 0;
  if(!strcmp(".",dasdirent->d_name)) return 0;
  if(!strcmp("vstservant.so",dasdirent->d_name)) return 0;
  if(!strcmp("vstserver",dasdirent->d_name)) return 0;
  return 1;
}

struct AEffect **VSTLIB_newCacheList(int *numberofplugins){
  char *vstpath=getenv("VST_PATH");
  struct dirent **namelist;
  int lokke;
  struct AEffect **ret;
  int numdirentries;

  *numberofplugins=0;

  if(vstpath==NULL){
    fprintf(stderr,"VSTLIB/newCacheList: VST_PATH environment variable not set.\n");
    return NULL;
  }

  numdirentries=scandir(vstpath, &namelist, myselect, alphasort);

  if(numdirentries<=0){
    if(numdirentries<0){      
      fprintf(stderr,"VSTLIB/newCacheList: Could not open vst path directory.\n");
    }
    return NULL;
  }

  ret=calloc(sizeof(struct AEffect *),numdirentries+1);

  for(lokke=0;lokke<numdirentries;lokke++){
    ret[*numberofplugins]=VSTLIB_newCache(namelist[lokke]->d_name);
    if(ret[*numberofplugins]!=NULL){
      (*numberofplugins)++;
    }
    free(namelist[lokke]);
  }

  free(namelist);

  return ret;

}


const char *VSTLIB_getName(AEffect *effect){
  struct MyAEffect *myeffect=(struct MyAEffect*)effect;
  return (const char*) myeffect->name;
}

