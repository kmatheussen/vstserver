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

#include <vstserver.h>


#include "s_.h"

#include "controlthread.h"


void *CONTROL_thread(void *arg){
  struct VSTClient *client=(struct VSTClient *)arg;
  char name[500];

  while(C_acceptIncomingConnection(client->control_input)==false);

  sprintf(name,"%s",client->name);

  client->control_thread_status=1;

  for(;;){
    struct VSTS_controlRequest cr;

    int num_read=C_receive(client->control_input,&cr,sizeof(struct VSTS_controlRequest));
    if(num_read!=sizeof(struct VSTS_controlRequest)){
      if(num_read==0){
	fprintf(stderr,"VSTSERVANT/CONTROL_thread: Seems like client has died. Exiting \"%s\".\n",name);
	return NULL;
      }else{
	fprintf(stderr,"VSTSERVANT/CONTROL_thread: num_read not %d but %d for \"%s\".\n",sizeof(int),num_read,name);
      }
      continue;
    }

    switch(cr.reqtype){
    case VSTP_delete:
      printf("VSTSERVANT/CONTROL_thread: \"%s\" received delete.\n",name);
      return NULL;
      break;
    case VSTP_dispatcher:
      VSTS_dispatcher(client,&cr);
      break;
    case VSTP_setParameter:
      VSTS_setParameter(client,&cr);
      break;
    case VSTP_getParameter:
      VSTS_getParameter(client,&cr);
      break;
    default:
      fprintf(stderr,"VSTSERVANT/CONTROL_thread: received unknown request type %d %d for \"%s\".\n",cr.reqtype,num_read,name);
      break;
    }
  }
  return NULL;
}

