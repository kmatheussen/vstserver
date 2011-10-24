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
#include <unistd.h>

#include <vstserver.h>

#include "s_.h"
#include "win/winwin_proc.h"




void VSTS_dispatcher(
		     struct VSTClient *vstclient,
		     struct VSTS_controlRequest *cr
		     )
{

  switch(cr->opCode){
  case effEditOpen:
    vstclient->ws->pleaseclosethewindow=0;
    if(vstclient->ws->hWnd==NULL){
      if(pthread_create(&vstclient->winwin_thread,NULL,WINWIN_handler,vstclient->ws)!=0){
	fprintf(stderr,"VSTS_dispatcher: Could not make new thread.\n");
      }
    }
    break;
  case effEditClose:
    WINWIN_close(vstclient->ws);
    while(vstclient->ws->hWnd!=NULL) usleep(200);
    break;
  case effProcessEvents:
    s_effProcessEvents(vstclient,cr);
    return;
  default:
    cr->value=vstclient->ws->effect->dispatcher(
						vstclient->ws->effect,
						cr->opCode,
						cr->index,
						cr->value,
						&cr->ptr,
						cr->opt
						);
    break;
  }


  C_send(vstclient->control_output,cr,sizeof(struct VSTS_controlRequest));

}

