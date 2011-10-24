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
#include <sys/ipc.h>
#include <sys/shm.h>


#include "shmhandler.h"


void SMS_delete(struct SharedMemStuff *sms){
  shmctl(sms->id,IPC_RMID,NULL);
  free(sms);
}


struct SharedMemStuff *SMS_new(int size,int num_elements){
  struct SharedMemStuff *sms=calloc(1,sizeof(struct SharedMemStuff));
  sms->size=size;
  sms->num_elements=num_elements;

  sms->key=random();
  sms->id=shmget(sms->key,size*num_elements*sizeof(float),IPC_CREAT|0600);
  if(sms->id==-1){
    fprintf(stderr,"VSTSERVANT/SMS_new: Unable to allocate %d bytes of shared memory.\n",size*num_elements*sizeof(float));
    free(sms);
    return NULL;
  }

  sms->addr=shmat(sms->id,0,0);
  if(sms->addr==(void*)-1){
    fprintf(stderr,"VSTSERVANT/SMS_new: Unable to allocate shared memory. 2\n");
    shmctl(sms->id,IPC_RMID,NULL);
    free(sms);
    return NULL;
  }


  return sms;

}




