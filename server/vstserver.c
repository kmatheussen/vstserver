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


#include <signal.h>
#include <stdio.h>

#include <stdlib.h>

#include <dirent.h>

#include <sys/types.h>
#include <unistd.h>

#include <vstserver.h>
#include <k_communicate.h>
#include <vstlib.h>
#include <k_cache.h>

static struct Communicate *input;

void finish(int sig){
  char *homedir=getenv("HOME");
  char temp[500];

  C_delete(input);  

  sprintf(temp,"rm -f %s/.vstserver/sockets/s_*",homedir);
  system(temp);

  sprintf(temp,"rm -f %s/.vstserver/locks/l_*",homedir);
  system(temp);

  system("killall -9 wine-pthread vstserver >/dev/null 2>/dev/null");
  exit(0);
}


void VSTSERVER_deleteLocks(void){
  char *homedir=getenv("HOME");
  char temp[500];
  sprintf(temp,"rm -f %s/.vstserver/locks/l_*",homedir);
  system(temp);
}


bool VSTSERVER_makeDirectories(void){
  DIR *dir,*dir2,*dir3;
  char *homedir=getenv("HOME");
  char temp[500];

  sprintf(temp,"%s/.vstserver",homedir);
  dir=opendir(temp);

  if(dir==NULL){
    sprintf(temp,"mkdir %s/.vstserver",homedir);
    system(temp);
  }else{
    closedir(dir);
  }

  sprintf(temp,"%s/.vstserver/sockets",homedir);
  dir=opendir(temp);

  if(dir==NULL){
    sprintf(temp,"mkdir %s/.vstserver/sockets",homedir);
    system(temp);
  }else{
    closedir(dir);
  }

  sprintf(temp,"%s/.vstserver/locks",homedir);
  dir=opendir(temp);

  if(dir==NULL){
    sprintf(temp,"mkdir %s/.vstserver/locks",homedir);
    system(temp);
  }else{
    closedir(dir);
  }

  sprintf(temp,"%s/.vstserver/cache",homedir);
  dir=opendir(temp);

  if(dir==NULL){
    sprintf(temp,"mkdir %s/.vstserver/cache",homedir);
    system(temp);
  }else{
    closedir(dir);
  }

  sprintf(temp,"%s/.vstserver/cache",homedir);
  dir=opendir(temp);
  sprintf(temp,"%s/.vstserver/locks",homedir);
  dir2=opendir(temp);
  sprintf(temp,"%s/.vstserver/sockets",homedir);
  dir3=opendir(temp);

  if(dir==NULL || dir2==NULL || dir3==NULL){
    return false;
  }

  closedir(dir);closedir(dir2);closedir(dir3);

  return true;
}


static int myselect(const struct dirent *dasdirent){
  if(!strcmp("..",dasdirent->d_name)) return 0;
  if(!strcmp(".",dasdirent->d_name)) return 0;
  if(!strcmp("vstservant.so",dasdirent->d_name)) return 0;
  if(!strcmp("vstserver",dasdirent->d_name)) return 0;
  return 1;
}

static void updatecache(int generation){
  char *vstpath=getenv("VST_PATH");
  struct dirent **namelist;
  int numdirentries;
  int lokke;
  int havecalledupdate=0;

  if(vstpath==NULL){
    goto end;
  }

  numdirentries=scandir(vstpath, &namelist, myselect, alphasort);
  if(numdirentries<=0){
    goto end;
  }


  for(lokke=0;lokke<numdirentries;lokke++){
    if(havecalledupdate==0 && CACHE_checkPluginUpdate(namelist[lokke]->d_name)==1){
      int numplugins;
      havecalledupdate=1;
      if(!fork()){
	if(generation>5) sleep(40);
	else sleep(generation*3+10);
	updatecache(generation+1);
	goto end;
      }
      if(generation==0){
	fprintf(stderr,"\n\n----> VSTSERVER/updatecache: Please wait. Updating cache.\n");
	fprintf(stderr,"         If nothing happens for 40 seconds, or the server is trying to start the same plugin over and over again;\n");
	fprintf(stderr,"         press ctrl-c, and start vstserver once more.\n\n");
      }
      VSTLIB_deleteCacheList(VSTLIB_newCacheList(&numplugins));
    }
    free(namelist[lokke]);
  }

  free(namelist);

 end:
  if(havecalledupdate==0 && generation>0){
    sleep(10);
    system("killall -9 wine-pthread");
    printf("\n\n----> VSTSERVER/updatecache: Cache updated.\n\n");
  }
  exit(0);
}


int main(int argc,char **argv){
  bool realtime=true;
  char socketfilename[500];
  sprintf(socketfilename,"%s/.vstserver/sockets/s_",getenv("HOME"));


  if(argc>1){
    if(strcmp(argv[1],"-NR") && strcmp(argv[1],"--nonrealtime")){
      printf("Usage: %s [-NR or --nonrealtime]\n",argv[0]);
      return 0;
    }
    realtime=false;
  }

  if(VSTSERVER_makeDirectories()==false){
    fprintf(stderr,"\n\nVSTSERVER/main: Could not create directories in your homedirectory.\n");
    fprintf(stderr,"Check diskspace and filepermissions and try again.\n\n");
    return 1;
  }

  input=C_newInput(socketfilename,0);

  if(input==NULL){
    fprintf(stderr,"\n\nVSTSERVER/main: Could not create main socket.\n\n");
    fprintf(stderr,"Server is allready running or there are left\n");
    fprintf(stderr,"files in .vstserver/sockets/. Just delete them manually.\n");
    fprintf(stderr,"Or, there is something seriously wrong. (hope not)\n\n");
    return 2;
  }

  signal(SIGINT,finish);

  /* Nah. This operation can _theoretically_ screw things up. But thats hopefully not very likely. */
  VSTSERVER_deleteLocks();

  printf("\n\nVSTSERVER/main: Vstserver %d.%d.%d started. Waiting for requests.\n",VERSION_MAJOR,VERSION_MINOR,VERSION_MINOR_MINOR);


  if(!fork()){
    updatecache(0);
  }


  for(;;){
    struct VSTS_serverRequest sr;
    char temp[1024];

    for(;;){
      int size;
      while(C_acceptIncomingConnection(input)==false);

      if(
	 (size=C_receive(input,&sr,sizeof(struct VSTS_serverRequest)))
	 != sizeof(struct VSTS_serverRequest)
	 )
	{
	  if(size!=-1){
	    fprintf(stderr,"VSTSERVER/main: Wrong type of request to vstserver. Size: %d\n",size);
	  }else{
	    fprintf(stderr,"VSTSERVER/main: Request failed.\n");
	  }
	}else{
	  break;
	}
    }

    switch(sr.reqtype){
    case VSTP_new:
      if(
	 sr.version_major!=VERSION_MAJOR
	 || sr.version_minor!=VERSION_MINOR
	 )
	{
	  fprintf(
		  stderr,
		  "Cant start \"%s\". Client was compiled V%d.%d.%d of vstlib, which wont work with this version of the vstserver.\n",
		  sr.pluginname,
		  sr.version_major,
		  sr.version_minor,
		  sr.version_minor_minor
		  );
	}else{
	  sprintf(
		  temp,
		  "wine vstservant.so %d %d %d %s &",
		  sr.client_control_id,
		  sr.client_process_id,
		  (realtime==true && sr.realtime==true)?1:0,
		  sr.pluginname
		  );
	  printf("VSTSERVER/main: Going to try to start vst plugin \"%s\".\n",sr.pluginname);
	  system(temp);
	}
      break;

    default:
      fprintf(stderr,"VSTSERVER/main: Illegal request %d.\n",sr.reqtype);
      break;
    }
  }


  return 0;
}

