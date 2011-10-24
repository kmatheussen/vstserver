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

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>

#include <dirent.h>

#include <vstserver.h>

#include <k_locks.h>

#include <k_cache.h>



static void stringtolower(char *s){
  int lokke;
  for(lokke=0;lokke<strlen(s)-1;lokke++){
    s[lokke]=tolower(s[lokke]);
  }
}



/* Gets msdos name of a plugin. */
/* "name" can be in any format. */
static char *CACHE_getMSDosNameForPlugin(const char *name){
  char *ret=malloc(strlen(name)+1);

  sprintf(ret,"%s",name);
  stringtolower(ret);
  if(ret[strlen(ret)-4]=='.'){
    if(!strcmp(ret+strlen(ret)-4,".dll")){
      ret[strlen(ret)-4]=0;
    }
  }

  return ret;
}


/* Gets the real unix name of a plugin. */
/* "name" can be in any format. */
static char *CACHE_getUnixNameForPlugin(const char *name){
  char temp[500];
  char *vstpath=getenv("VST_PATH");
  char *ret=NULL;
  struct stat vststat;

  if(vstpath[strlen(vstpath)-1]=='/'){
    sprintf(temp,"%s%s",vstpath,name);
  }else{
    sprintf(temp,"%s/%s",vstpath,name);
  }

  if(stat(temp,&vststat)==-1){
    DIR *dir;
    char *msdosname1;

    dir=opendir(vstpath);
    if(dir==NULL){
      fprintf(stderr,"Could not open vst path directory.\n");
      return NULL;
    }

    msdosname1=CACHE_getMSDosNameForPlugin(name);

    for(;;){

      char *msdosname2;
      struct dirent *direntry;

      direntry=readdir(dir);
      if(direntry==NULL) break;

      msdosname2=CACHE_getMSDosNameForPlugin(direntry->d_name);
      if(!strcmp(msdosname1,msdosname2)){
	ret=malloc(strlen(direntry->d_name)+1);
	sprintf(ret,"%s",direntry->d_name);
	free(msdosname2);
	break;
      }

      free(msdosname2);
    }

    free(msdosname1);
    closedir(dir);

  }else{
    ret=malloc(strlen(name)+1);
    sprintf(ret,"%s",name);
  }

  return ret;
}


/* Returns -1 if plugin is not a valid plugin, but cache is updated. */
/* Returns 0 if plugin is a valid plugin, and cache is updated. */
/* Returns 1 if cache is not updated. */

int CACHE_checkPluginUpdate(const char *name){
  char *homepath=getenv("HOME");
  char *vstpath=getenv("VST_PATH");
  char *unixname2;
  char unixname[500];
  char *msdosname2;
  char msdosname[500];
  char temp[500];
  struct stat homestat,vststat;
  struct Locks *locks;

  char lockfilename[500];
  sprintf(lockfilename,"%s/.vstserver/locks/l_",getenv("HOME"));

  //if(!strcmp("vstservant.so",name)) return 1;
  //if(!strcmp("vstserver",name)) return 1;

  if(homepath==NULL || vstpath==NULL) return -1;

  msdosname2=CACHE_getMSDosNameForPlugin(name);
  if(msdosname2==NULL) return -1;
  sprintf(msdosname,"%s",msdosname2);
  free(msdosname2);

  unixname2=CACHE_getUnixNameForPlugin(name);
  if(unixname2==NULL) return -1;
  sprintf(unixname,"%s",unixname2);
  free(unixname2);


  if(vstpath[strlen(vstpath)-1]=='/'){
    sprintf(temp,"%s%s",vstpath,unixname);
  }else{
    sprintf(temp,"%s/%s",vstpath,unixname);
  }

  locks=LOCKS_new(lockfilename,0);

  if(stat(temp,&vststat)==-1){
    LOCKS_delete(locks);
    return -1;
  }

  if(S_ISDIR(vststat.st_mode)){
    LOCKS_delete(locks);
    return -1;
  }

  sprintf(temp,"%s/.vstserver/cache/%s",homepath,msdosname);
  if(stat(temp,&homestat)==-1){
    LOCKS_delete(locks);
    return 1;
  }

  if(vststat.st_mtime>=homestat.st_mtime){
    LOCKS_delete(locks);
    return 1;
  }

  if(homestat.st_size==sizeof(int)){
    LOCKS_delete(locks);
    return -1;
  }

  LOCKS_delete(locks);
  return 0;
}


static void CACHE_updatePart(char *name,char *postfix,void *stuff,size_t size){
  char *homepath=getenv("HOME");
  char temp[500];
  FILE *file;
  int dummy=0;

  sprintf(temp,"%s/.vstserver/cache/%s%s",homepath,name,postfix);

  file=fopen(temp,"w");
  if(file==NULL) return;

  /* If the plugin isnt working, or its not a plugin. */
  if(stuff==NULL){
    stuff=&dummy;
    size=sizeof(int);
  }

  if(fwrite(stuff,size,1,file)==0){
    fclose(file);
    unlink(temp);
    return;
  }

  fclose(file);
}

bool CACHE_getPart(const char *name,char *postfix,void *stuff,size_t size){
  char *homepath=getenv("HOME");
  char temp[500];
  FILE *file;

  char *msdosname2;
  char msdosname[500];

  msdosname2=CACHE_getMSDosNameForPlugin(name);
  if(msdosname2==NULL) return false;
  sprintf(msdosname,"%s",msdosname2);
  free(msdosname2);


  sprintf(temp,"%s/.vstserver/cache/%s%s",homepath,msdosname,postfix);

  file=fopen(temp,"r");
  if(file==NULL) return false;

  if(fread(stuff,size,1,file)==0){
    fclose(file);
    return false;
  }

  fclose(file);

  return true;
}


void CACHE_update(const char *name,struct AEffect *effect){
  int lokke;
  int confresult=CACHE_checkPluginUpdate(name);
  char *msdosname2;
  char msdosname[500];

  char lockfilename[500];
  sprintf(lockfilename,"%s/.vstserver/locks/l_",getenv("HOME"));

  if(confresult==0 || confresult==-1) return;

  msdosname2=CACHE_getMSDosNameForPlugin(name);
  if(msdosname2==NULL) return;
  sprintf(msdosname,"%s",msdosname2);
  free(msdosname2);

  if(effect==NULL){
    struct Locks *locks=LOCKS_new(lockfilename,0);
    CACHE_updatePart(msdosname,"",NULL,0);
    LOCKS_delete(locks);
  }else{
    char paramnames[effect->numParams][9];
    char paramdisplays[effect->numParams][9];
    char paramlabels[effect->numParams][9];
    float vals[effect->numParams];

    struct Locks *locks;

    for(lokke=0;lokke<effect->numParams;lokke++){
      vals[lokke]=effect->getParameter(effect,lokke);
      memset(paramnames[lokke],0,9);
      memset(paramdisplays[lokke],0,25);
      memset(paramlabels[lokke],0,9);

      effect->dispatcher(
			 effect,
			 effGetParamName,
			 lokke, 0, paramnames[lokke], 0
			 );
      effect->dispatcher(
			 effect,
			 effGetParamDisplay,
			 lokke, 0, paramdisplays[lokke], 0
			 );
      effect->dispatcher(
			 effect,
			 effGetParamLabel,
			 lokke, 0, paramlabels[lokke], 0
			 );
    }


    locks=LOCKS_new(lockfilename,0);
    
    CACHE_updatePart(msdosname,"",effect,sizeof(struct AEffect));
    for(lokke=0;lokke<effect->numParams;lokke++){
      char temp[500];
      sprintf(temp,"_paramval_%d",lokke);
      CACHE_updatePart(msdosname,temp,&vals[lokke],sizeof(float));
      sprintf(temp,"_paramname_%d",lokke);
      CACHE_updatePart(msdosname,temp,paramnames[lokke],9);
      sprintf(temp,"_paramdisplay_%d",lokke);
      CACHE_updatePart(msdosname,temp,paramdisplays[lokke],25);
      sprintf(temp,"_paramlabel_%d",lokke);
      CACHE_updatePart(msdosname,temp,paramlabels[lokke],9);
    }

    LOCKS_delete(locks);
  }

}



