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


#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <sys/mman.h>
#include <signal.h>
#include <unistd.h>

#include <vstserver.h>

#include "s_.h"

#include "processthread.h"




static void *watchdog(void *arg){
  struct VSTClient *client=(struct VSTClient*)arg;
  struct sched_param par;

  system("/usr/bin/givertcap");

  par.sched_priority = sched_get_priority_max(SCHED_FIFO);
  if(sched_setscheduler(0,SCHED_FIFO,&par)==-1){
    fprintf(stderr,"VSTSERVANT/PROCESS_watchdog: Unable to set SCHED_FIFO realtime priority for the watchdog thread. No watchdog.\n");
    goto exit;
  }

  client->watchdog_thread_started=true;

  for(;;){
    int last;
    if(client->isprocessing==false)
      last=-1;
    else
      last=client->processthreadisalive;   
    sleep(10);
    if(client->endwatchdog==true) goto exit;
    if(client->isprocessing==true && client->processthreadisalive==last){
      kill(client->processthread_pid,SIGKILL);
      fprintf(stderr,"VSTSERVANT/PROCESS_watchdog: Plugin \"%s\" used more than 10 seconds in the process-thread. Killed it.\n",client->name);
      goto exit;
    }
  }
 exit:
  //fprintf(stderr,"Watchdog exiting\n");
  return NULL;
}


static void set_realtime(struct VSTClient *client){
    struct sched_param par;
    par.sched_priority = (sched_get_priority_max(SCHED_FIFO) + sched_get_priority_min(SCHED_FIFO) ) / 2;

    if(sched_setscheduler(0,SCHED_FIFO,&par)==-1){
      fprintf(stderr,"VSTSERVANT/PROCESS_thread: Unable to set SCHED_FIFO realtime priority for \"%s\".\n",client->name);
    }else{
      printf("VSTSERVANT/PROCESS_thread: Realtime priority set for \"%s\".\n",client->name);
      client->processthread_pid=getpid();
      client->endwatchdog=false;
      client->watchdog_thread_started=false;
      if(pthread_create(&client->watchdog_thread,NULL,watchdog,client)!=0){
	fprintf(stderr,"VSTSERVANT_PROCESS_thread: Unable to start watchdog thread for \"%s\". No watchdog.\n",client->name);
      }
    }

    if(mlockall(MCL_FUTURE)==-1){
    	fprintf(stderr,"VSTSERVANT/PROCESS_thread: Unable to lock memory for \"%s\".\n",client->name);
    }
}


void *PROCESS_thread(void *arg){
  struct VSTClient *client=(struct VSTClient *)arg;
  char name[500];
  
  while(C_acceptIncomingConnection(client->process_input)==false);

  sprintf(name,"%s",client->name);

  if(client->realtime==1){
    system("/usr/bin/givertcap");
    set_realtime(client);
  }
  
  client->process_thread_status=1;

  for(;;){
    int sampleframes;
    int num_read=C_receive(client->process_input,&sampleframes,sizeof(int));
    if(num_read!=sizeof(int)){
      if(num_read==0){
	fprintf(stderr,"VSTSERVANT/PROCESS_thread: Seems like client has died. Exiting \"%s\".\n",name);
	goto exit;
      }else{
	fprintf(stderr,"VSTSERVANT/PROCESS_thread: num_read not %d but %d for \"%s\".\n",sizeof(int),num_read,name);
      }
      continue;
    }

    if(sampleframes==0){
      printf("VSTSERVANT/PROCESS_thread: \"%s\" received delete.\n",name);
      goto exit;
    }

    client->isprocessing=true;
    client->processthreadisalive++;
#ifdef TESTING_WATCHDOG
    while(1){
      int ak=2;
      int ak2=3;
      for(;;){
	ak++;
	ak2=ak+ak2/2;
      }
    }
#endif
    if(sampleframes>0){
      VSTS_process(client,sampleframes);
    }else{
      VSTS_processReplacing(client,-sampleframes);
    }
    client->isprocessing=false;
  }

 exit:
  if(client->watchdog_thread_started==true){
    client->endwatchdog=true;
    pthread_join(client->watchdog_thread,NULL);
  }

  //fprintf(stderr,"Process exiting\n");
  return NULL;
}

