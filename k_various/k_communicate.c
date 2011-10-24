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


/*

  Easy and fast communication between two processes.



  Usage
  *****

  Connection
  ----------
  Process 1 calls c1=C_newInput(filename,k)
  Process 2 calls c2=C_newOutput(filename,k)
  Process 1 calls C_acceptIncomingConnection(c1)
  -or:
  Process 1 calls c1=C_newInput(filename,k)
  Process 1 calls C_acceptIncomingConnection(c1)
  Process 2 calls c2=C_newOutput(filename,k)

  Sending
  -------
  Process 1 calls C_receive(c1,..)
  Process 2 calls C_send(c2,..)
  -or:
  Process 2 calls C_send(c2,..)
  Process 1 calls C_receive(c1,..)

  Closing
  -------
  Process 1 calls C_delete(c1)
  Process 2 calls C_delete(c2)
  -or:
  Process 2 calls C_delete(c2)
  Process 1 calls C_delete(c1)


  C_acceptIncomingConnection and C_receive blocks until
  a new connection or message arrives.

  C_acceptIncomingConnection can be called again to accept
  a new connection. The previous connection is closed.



 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>

#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <k_locks.h>

#include <k_communicate.h>


struct Communicate *C_newInput(char *socketfilename,int id){
  struct sockaddr_un addr;
  char temp[500];
  struct Communicate *c;

  if(socketfilename==NULL || id==-1){
    int lokke;
    struct Locks *locks;
    char lockfilename[500];

    if(socketfilename==NULL){
      socketfilename="/tmp/k_communicate_socket";
    }

    sprintf(lockfilename,"%s_lock_",socketfilename);
    locks=LOCKS_new(lockfilename,1);

    if(locks==NULL) return NULL;
 
    for(lokke=1;;lokke++){
      sprintf(temp,"%s%d",socketfilename,lokke);
      if(access(temp,F_OK)!=0){
	id=lokke;
	break;
      }
      if(lokke>=100000){
	fprintf(
		stderr,
		"k_communicate.c/C_newInput: Could not find free socket. Some directory is probably not present.\n"
		);
	LOCKS_delete(locks);
	return NULL;
      }
    }

    LOCKS_delete(locks);
  }

  c=calloc(sizeof(struct Communicate),1);
  c->isInput=true;
  c->client_socket=-1;
  c->id=id;

  sprintf(temp,"%s%d",socketfilename,id);

  c->name=malloc(strlen(temp)+1);
  sprintf(c->name,"%s",temp);
  
  c->fd=socket(AF_UNIX,SOCK_STREAM,0);
  
  if(c->fd<0){
    fprintf(stderr,"k_communicate.c/C_newInput: Could not create socket.\n");
    free(c->name);
    free(c);
    return NULL;
  }
  

  addr.sun_family=AF_UNIX;
  sprintf(addr.sun_path,c->name);

  
  if( bind(c->fd,(struct sockaddr*)&addr,sizeof(struct sockaddr_un)) <0 ){
    fprintf(stderr,"k_communicate.c/C_newInput: Creation of InputSocket -%s- failed 1.\n",c->name);
    close(c->fd);
    free(c->name);
    free(c);
    return NULL;
  }
  
  if (listen (c->fd, 10000) < 0) {
    fprintf(stderr,"k_communicate.c/C_newInput: Creation of InputSocket -%s- failed 2.\n",c->name);
    close(c->fd);
    free(c->name);
    free(c);
    return NULL;
  }
  
  return c;
}



struct Communicate *C_newOutput(char *socketfilename,int id){
  char temp[500];
  struct Communicate *c;
  struct sockaddr_un addr;

  c=calloc(sizeof(struct Communicate),1);
  c->isInput=false;
  c->id=id;

  sprintf(temp,"%s%d",socketfilename,id);

  c->name=malloc(strlen(temp)+1);
  sprintf(c->name,"%s",temp);

  c->fd=socket(AF_UNIX,SOCK_STREAM,0);
  if(c->fd<0){
    fprintf(stderr,"k_communicate.c/C_newOutput: Could not connect to socket.\n");
    free(c->name);
    free(c);
    return NULL;
  }
  
  addr.sun_family=AF_UNIX;
  sprintf(addr.sun_path,c->name);

  if (connect (c->fd, (struct sockaddr *) &addr, sizeof (addr)) < 0) {
    fprintf(stderr,"k_communicate.c/C_newOutput: Could not connect -%s- %s\n",c->name, strerror (errno));
    free(c->name);
    free(c);
    close (c->fd);
    return NULL;
  }


  return c;
}


void C_delete(struct Communicate *c){
  char temp[500];

  if(c->client_socket!=-1 && c->client_socket!=0){
    close(c->client_socket);
  }

  close(c->fd);

  if(c->isInput==true){
    sprintf(temp,"rm -f %s",c->name);
    system(temp);
    //    unlink(c->name);
  }

  free(c->name);
  free(c);
}

int C_send(struct Communicate *c,void *buf,int size){
  return write(c->fd,buf,size);
}

bool C_sendInt(struct Communicate *c,int dasint){
  int s=dasint;
  if(write(c->fd,&s,sizeof(int))!=sizeof(int)){
    return false;
  }
  return true;
}

bool C_sendFloat(struct Communicate *c,float dasfloat){
  float s=dasfloat;
  if(write(c->fd,&s,sizeof(float))!=sizeof(float)){
    return false;
  }
  return true;
}



bool C_acceptIncomingConnection(struct Communicate *c){
  if(c->client_socket!=-1) close(c->client_socket);

  c->client_addrlen=sizeof(struct sockaddr_un);
  c->client_socket=accept(c->fd,(struct sockaddr *)&c->client_addr,&c->client_addrlen);
  if(c->client_socket<0){
    fprintf(
	    stderr,
	    "k_communicate.c/C_acceptIncomingConnection: Not able to accept socket for %s. \n\tErrornum: %d / %s\n",
	    c->name,
	    c->client_socket,
	    strerror (errno)
	    );
    return false;
  }

  return true;

}


int C_receive(struct Communicate *c,void *buf,int maxsize){

  if(c->client_socket==-1){
    fprintf(stderr,"k_communicate.c/C_receive: No incoming sockets are bounded to this socket. %s.\n",c->name);
    return -1;
  }

  return read(c->client_socket,buf,maxsize);
}



