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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include <k_locks.h>


void LOCKS_delete(struct Locks *locks){
  close(locks->fd);
  unlink(locks->lockfilename);

  free(locks);
}


struct Locks *LOCKS_new(char *lockfilename,int id){
  struct Locks *locks=calloc(1,sizeof(struct Locks));
  int numretries=0;

  locks->id=id;

  sprintf(locks->lockfilename,"%s%d",lockfilename,locks->id);

  locks->fd=open(locks->lockfilename,O_RDWR|O_CREAT|O_EXCL,S_IRWXU);
  while(locks->fd==-1){
    if(errno!=EEXIST){
      fprintf(
	      stderr,
	      "k_locks.c/LOCKS_new: Could not create lock-file \"%s\", error: %s.\n",
	      locks->lockfilename,
	      strerror(errno)
	      );
      free(locks);
      return NULL;
    }
    usleep(2000);
    locks->fd=open(locks->lockfilename,O_RDWR|O_CREAT|O_EXCL,S_IRWXU);
    numretries++;
    if(numretries>(5000000/2000)){
      printf(
	     "k_locks.c/LOCKS_new: Warning, possible deadlock while trying to open lock-file \"%s\".\n",
	     locks->lockfilename
	     );
      numretries=0;
    }
  }

  return locks;
}



