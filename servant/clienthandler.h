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

#include <k_communicate.h>
#include "win/windowsstuff.h"
#include "shmhandler.h"

#define SHAREDMEM_ELEMENTSIZE 1024

struct VSTClient{
  char *name;

  struct Communicate *control_output;
  struct Communicate *process_output;

  struct Communicate *newdelete_input;
  struct Communicate *control_input;
  struct Communicate *process_input;

  pthread_t control_thread;
  pthread_t process_thread;

  int control_thread_status; //0=not connected. 1=connected.
  int process_thread_status; //------------""---------------

  bool isprocessing;
  pid_t processthread_pid;
  pthread_t watchdog_thread;
  bool watchdog_thread_started;
  int processthreadisalive; // Used by the watchdog. Must change value at least once every 10 seconds.
  bool endwatchdog;

  struct WindowsStuff *ws;
  struct SharedMemStuff *sms;

  float **inputs;
  float **outputs;

  pthread_t winwin_thread;

  int realtime;

  int vstevents_arraylen;
  struct VstEvent *vstevent_array;
  struct VstEvents *vstevents;
};




void CH_delete(struct VSTClient *client);
struct VSTClient *CH_new(
			 void *wws,
			 int client_control_id,
			 int client_process_id,
			 int client_realtime,
			 char *client_libname
			 );
