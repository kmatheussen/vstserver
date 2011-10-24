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
#include <string.h>
#include <unistd.h>

#include <pthread.h>

#include <vstserver.h>



#include "controlthread.h"
#include "processthread.h"

#include "clienthandler.h"

#include "win/winwin_proc.h"


static void CH_deleteReal(struct VSTClient *client){

  if(client->ws!=NULL){
    WINWIN_close(client->ws);
    while(client->ws->hWnd!=NULL) usleep(200); // Dont want to use unix stuff in the windows code.
  }

  if(client->control_thread!=0){
    pthread_join(client->control_thread,NULL);
  }

  if(client->process_thread!=0){
    pthread_join(client->process_thread,NULL);
  }

  if(client->control_output!=NULL) C_delete(client->control_output);
  if(client->process_output!=NULL) C_delete(client->process_output);
  if(client->control_input!=NULL) C_delete(client->control_input);
  if(client->process_input!=NULL) C_delete(client->process_input);

  if(client->newdelete_input!=NULL) C_delete(client->newdelete_input);

  if(client->ws!=NULL) WINDOWS_delete(client->ws);
  if(client->sms!=NULL) SMS_delete(client->sms);

  if(client->inputs!=NULL) free(client->inputs);
  if(client->outputs!=NULL) free(client->outputs);


  if(client->name!=NULL) free(client->name);
  free(client);
}

void CH_delete(struct VSTClient *client){
  CH_deleteReal(client);
}



/* Returns NULL if WINDOWS_new returned NULL and (void *)-1 if there was a different error. */

struct VSTClient *CH_new(
			 void *winwinstuff,
			 int client_control_id,
			 int client_process_id,
			 int client_realtime,
			 char *pluginname
			 )
{
  struct VSTS_serverAnswerRequest ar;
  struct VSTClient *vstclient;

  char socketfilename[500];
  sprintf(socketfilename,"%s/.vstserver/sockets/s_",getenv("HOME"));

  ar.plugin_ok=0;

  vstclient=calloc(1,sizeof(struct VSTClient));

  vstclient->realtime=client_realtime;

  /******** Connect to the clients sockets. ****************/

  vstclient->control_output=C_newOutput(socketfilename,client_control_id);
  vstclient->process_output=C_newOutput(socketfilename,client_process_id);

  if(vstclient->control_output==NULL || vstclient->process_output==NULL){
    fprintf(stderr,"VSTSERVANT/CH_new: Could not connect to clients sockets.\n");
    CH_deleteReal(vstclient);
    return (void*)-1;
  }

  /******** Open plugin. **************/

  vstclient->name=calloc(1,strlen(pluginname)+1);
  sprintf(vstclient->name,"%s",pluginname);

  vstclient->ws=WINDOWS_new(vstclient->name,winwinstuff);
  if(vstclient->ws==NULL){
      C_send(vstclient->control_output,&ar,sizeof(struct VSTS_serverAnswerRequest));
      CH_deleteReal(vstclient);
      return NULL;
  }

  /******* Create the input sockets. ***********/

  vstclient->newdelete_input=C_newInput(socketfilename,-1);
  vstclient->control_input=C_newInput(socketfilename,-1);
  vstclient->process_input=C_newInput(socketfilename,-1);

  if(
     vstclient->process_input==NULL
     || vstclient->control_input==NULL
     || vstclient->newdelete_input==NULL
     )
    {
      fprintf(stderr,"VSTSERVANT/CH_new: Could not create sockets.\n");
      C_send(vstclient->control_output,&ar,sizeof(struct VSTS_serverAnswerRequest));
      CH_deleteReal(vstclient);
      return (void*)-1;
    }



  /******* Set up shared memory. ***********/

  vstclient->sms=SMS_new(
			 SHAREDMEM_ELEMENTSIZE,
			 vstclient->ws->effect->numInputs + vstclient->ws->effect->numOutputs
			 );
  if(vstclient->sms==NULL){
    fprintf(stderr,"VSTSERVANT/CH_new: Could not set up shared memory.\n");
    C_send(vstclient->control_output,&ar,sizeof(struct VSTS_serverAnswerRequest));
    CH_deleteReal(vstclient);
    return (void*)-1;
  }


  if(vstclient->ws->effect->numInputs>0){
    int lokke;
    struct AEffect *effect=vstclient->ws->effect;
    float *mem=(float *)vstclient->sms->addr;

    vstclient->inputs=malloc(sizeof(float *) * effect->numInputs);
    for(lokke=0;lokke<effect->numInputs;lokke++){
      vstclient->inputs[lokke]=mem + (lokke*vstclient->sms->size);
    }
  }

  if(vstclient->ws->effect->numOutputs>0){
    int lokke;
    struct AEffect *effect=vstclient->ws->effect;
    float *mem=(float *)vstclient->sms->addr;

    vstclient->outputs=malloc(effect->numOutputs * sizeof(float*));
    for(lokke=effect->numInputs;lokke<effect->numInputs+effect->numOutputs;lokke++){
      vstclient->outputs[lokke-effect->numInputs]=mem + (lokke*vstclient->sms->size);
    }
  }




  /****** Set up threads. ************/

  if(pthread_create(&vstclient->control_thread,NULL,CONTROL_thread,vstclient)!=0){
    fprintf(stderr,"VSTSERVANT/CH_new: Could not create control thread.\n");
    C_send(vstclient->control_output,&ar,sizeof(struct VSTS_serverAnswerRequest));
    CH_deleteReal(vstclient);
    return (void*)-1;
  }

  if(pthread_create(&vstclient->process_thread,NULL,PROCESS_thread,vstclient)!=0){
    fprintf(stderr,"VSTSERVANT/CH_new: Could not create process thread.\n");
    C_send(vstclient->control_output,&ar,sizeof(struct VSTS_serverAnswerRequest));
    CH_deleteReal(vstclient);
    return (void*)-1;
  }





  /****** Set up the answerRequest. **********/

  ar.plugin_ok=1;

  ar.server_newdelete_id=vstclient->newdelete_input->id;
  ar.server_control_id=vstclient->control_input->id;
  ar.server_process_id=vstclient->process_input->id;

  ar.sharedmem_key=vstclient->sms->key;
  ar.sharedmem_elementsize=vstclient->sms->size; // This is number of floats, not bytes.

  memcpy(&ar.effect,vstclient->ws->effect,sizeof(struct AEffect));





  /****** Sending answer request. **********/

  if(
     C_send(vstclient->control_output,&ar,sizeof(struct VSTS_serverAnswerRequest))
     != sizeof(struct VSTS_serverAnswerRequest)
     )
    {
      fprintf(stderr,"VSTSERVANT/CH_new: Could not send proper answer..\n");
      CH_deleteReal(vstclient);
      return (void*)-1;
    }




  /********** Accept client connections on our sockets. ****************/

  /* (control_thread and process_thread is done at the start of the threads) */

  while(C_acceptIncomingConnection(vstclient->newdelete_input)==false);



  /*********** Waiting for the threads to be initialized. ****************/

  while(vstclient->control_thread_status==0) usleep(100);
  while(vstclient->process_thread_status==0) usleep(100);

  /*********** Sending final message to the client that everything is initialized. ****************/

  C_sendInt(vstclient->control_output,1);


  return vstclient;
}









