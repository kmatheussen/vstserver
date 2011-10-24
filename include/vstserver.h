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
#include <pthread.h>


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#define AEFFECTX_H_LINUXWORKAROUND
#include "vst/aeffectx.h"

//#include <vst/AEffect.h>


#ifdef MAX
#  undef MAX
#endif
#ifdef MIN
#  undef MIN
#endif

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))


enum  {
  VSTP_new,
  VSTP_delete,
  VSTP_dispatcher,
  VSTP_process,
  VSTP_setParameter,
  VSTP_getParameter,
  VSTP_processReplacing
};


struct VSTS_controlRequest{
  int reqtype;

  long opCode;
  long index;
  long value;
  char ptr[32];
  float opt;

  float parameter;
};

/* From client to server. */
struct VSTS_serverRequest{
  int reqtype;

  int version_major;
  int version_minor;
  int version_minor_minor;

  int client_control_id;
  int client_process_id;

  char pluginname[500];

  bool realtime;
};


/* From server to client. */
struct VSTS_serverAnswerRequest{
  int plugin_ok; // 1=fine, 0=not
  int server_newdelete_id;
  int server_control_id;
  int server_process_id;
  int sharedmem_key;
  int sharedmem_elementsize; // Total size of the shared memory is (numinput+numoutput) * this variable * sizeof(float).

  struct AEffect effect;
};


