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



struct Communicate{
  char *name;
  int id;
  bool isInput;

  int fd;

  /* For input-type socket. */
  struct sockaddr_un client_addr;
  socklen_t client_addrlen;
  int client_socket;
};



struct Communicate *C_newInput(char *socketfilename,int id);
struct Communicate *C_newOutput(char *socketfilename,int id);

void C_delete(struct Communicate *c);
int C_send(struct Communicate *c,void *buf,int size);
bool C_sendInt(struct Communicate *c,int dasint);
bool C_sendFloat(struct Communicate *c,float dasfloat);


bool C_acceptIncomingConnection(struct Communicate *c);

int C_receive(struct Communicate *c,void *buf,int maxsize);
