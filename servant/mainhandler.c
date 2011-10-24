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

#include <stdio.h>
#include <stdlib.h>

#include <vstserver.h>

#include "clienthandler.h"
#include "mainhandler.h"

#include <k_cache.h>


int MAINHANDLER_control(void *wws,char *argline_org){
  struct VSTClient *vstclient;
  int client_control_id;
  int client_process_id;

  char argline[1024];

  char *client_libname=NULL;

  int client_realtime;
  int lokke;



  /**************************************************************************************/
  /************ Gather client control id, process id, and library name. *****************/
  /**************************************************************************************/

  sprintf(argline,"%s",argline_org);

  sscanf(argline,"%d %d %d %n",&client_control_id,&client_process_id,&client_realtime,&lokke);
  client_libname=&argline[lokke];


  /**************************************************************************************/
  /************************** Trying to serve new client. *******************************/
  /**************************************************************************************/

  if(strcmp("vstservant.so",client_libname)){
    vstclient=CH_new(
		     wws,
		     client_control_id,
		     client_process_id,
		     client_realtime,
		     client_libname
		     );
  }else{
    vstclient=NULL;
  }

  if(vstclient!=(void*)-1){
    CACHE_update(client_libname,vstclient==NULL?NULL:vstclient->ws->effect);

    if(vstclient!=NULL){
      int rec;
      
      printf("VSTSERVANT/MAINHANDLER_control: %s started.\n",client_libname);
      
      if(C_receive(vstclient->newdelete_input,&rec,sizeof(int))==0){
	fprintf(stderr,"VSTSERVANT/MAINHANDLER_control: Seems like client has died. Exiting.\n");
      }
      
      CH_delete(vstclient);
      printf("VSTSERVANT/MAINHANDLER_control: %s stopped.\n",client_libname);
    }
  }

  return 0;
}


