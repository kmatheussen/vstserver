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
#include <string.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <vst/AEffect.h>
#include <vstserver.h>
#include <k_communicate.h>

#include "vstfuncs.h"

#include "newdelete.h"

static void VSTLIB_deleteReal(struct VSTLib *vstlib){
  if(vstlib->control_output!=NULL) C_delete(vstlib->control_output);
  if(vstlib->control_input!=NULL) C_delete(vstlib->control_input);
  if(vstlib->process_output!=NULL) C_delete(vstlib->process_output);
  if(vstlib->process_input!=NULL) C_delete(vstlib->process_input);

  if(vstlib->inputs!=NULL) free(vstlib->inputs);
  if(vstlib->outputs!=NULL) free(vstlib->outputs);

  free(vstlib);

}


void VSTLIB_delete(struct AEffect *effect){

  struct MyAEffect *myeffect=(struct MyAEffect*)effect;
  struct VSTLib *vstlib=myeffect->vstlib;

  if(vstlib->connectedtoserver==true){
    struct VSTS_serverRequest sr;
    struct VSTS_controlRequest cr;

    struct Communicate *newdelete_output=vstlib->newdelete_output;


    sr.reqtype=VSTP_delete;
    sr.client_control_id=vstlib->control_input->id;
    
    cr.reqtype=VSTP_delete;
    
    C_send(vstlib->control_output,&cr,sizeof(struct VSTS_controlRequest));
    C_sendInt(vstlib->process_output,0);
    
    VSTLIB_deleteReal(vstlib);

    if(C_sendInt(newdelete_output,0)==false){
      fprintf(stderr,"VSTLIB/delete: Could not send proper request to server.\n");
    }

    C_delete(newdelete_output);

  }else{
    free(vstlib);
  }

  if(myeffect->name!=NULL) free(myeffect->name);
  free(myeffect);

}




struct AEffect *VSTLIB_newReal(const char *name,bool realtime){

  struct Communicate *main_output;
  struct MyAEffect *myeffect;
  struct AEffect *effect;
  struct VSTLib *vstlib=calloc(1,sizeof(struct VSTLib));

  struct VSTS_serverRequest sr;
  struct VSTS_serverAnswerRequest ar;

  int segid;
  int lokke;

  char socketfilename[500];
  sprintf(socketfilename,"%s/.vstserver/sockets/s_",getenv("HOME"));

  if(name==NULL) return NULL;


  vstlib->connectedtoserver=true;

  /********** Connect to server socket. ****************/

  main_output=C_newOutput(socketfilename,0);
  if(main_output==NULL){
    fprintf(stderr,"VSTLIB/new: Could not connect to server.\n");
    free(vstlib);
    return NULL;
  }


  /********** Create input sockets. ****************/

  vstlib->control_input=C_newInput(socketfilename,-1);
  vstlib->process_input=C_newInput(socketfilename,-1);
  if(vstlib->control_input==NULL || vstlib->process_input==NULL){
    fprintf(stderr,"VSTLIB/new: Could not create socket.\n");
    C_delete(main_output);
    VSTLIB_deleteReal(vstlib);
    return NULL;
  }


  /********** Sending request to server. ****************/

  sr.reqtype=VSTP_new;

  sr.version_major=VERSION_MAJOR;
  sr.version_minor=VERSION_MINOR;
  sr.version_minor_minor=VERSION_MINOR_MINOR;

  sr.client_control_id=vstlib->control_input->id;
  sr.client_process_id=vstlib->process_input->id;
  sprintf(sr.pluginname,"%s",name);

  sr.realtime=realtime;

  if(C_send(main_output,&sr,sizeof(struct VSTS_serverRequest)) != sizeof(struct VSTS_serverRequest)){
    fprintf(stderr,"VSTLIB/new: Could not send proper request to server.\n");
    C_delete(main_output);
    VSTLIB_deleteReal(vstlib);
    return NULL;
  }

  C_delete(main_output);


  /********** Accept server connection on our sockets. ****************/
  
  if(
     C_acceptIncomingConnection(vstlib->control_input)==false
     || C_acceptIncomingConnection(vstlib->process_input)==false
     )
    {
      fprintf(stderr,"VSTLIB/new: Unable to accept sockets connection on our sockets.\n");
      VSTLIB_deleteReal(vstlib);
      return NULL;
    }




  /********** Getting answer from server. ****************/

  if(
     C_receive(vstlib->control_input,&ar,sizeof(struct VSTS_serverAnswerRequest))
     != sizeof(struct VSTS_serverAnswerRequest)
     )
    {
      fprintf(stderr,"VSTLIB/new: Did not receive proper answer from server for %s.\n",name);
      VSTLIB_deleteReal(vstlib);
      return NULL;
    }


  if(ar.plugin_ok==0){ // Plugin could not be loaded, did not exist, was not found, etc.
    VSTLIB_deleteReal(vstlib);
    return NULL;
  }



  /********** Connecting to control and process sockets on server. ****************/

  vstlib->control_output=C_newOutput(socketfilename,ar.server_control_id);
  vstlib->process_output=C_newOutput(socketfilename,ar.server_process_id);

  vstlib->newdelete_output=C_newOutput(socketfilename,ar.server_newdelete_id);
  
  if(
     vstlib->control_output==NULL
     || vstlib->process_output==NULL
     || vstlib->newdelete_output==NULL
     )
    {
      fprintf(stderr,"VSTLIB/new: Unable to connect to server sockets %d %d %d. (this is bad)\n",
	      vstlib->newdelete_output==NULL?0:1,
	      vstlib->control_output==NULL?0:1,
	      vstlib->process_output==NULL?0:1
	      );
      VSTLIB_deleteReal(vstlib);
      return NULL;
    }




  /********** Setting up the AEffect VST struct. ****************/

  myeffect=calloc(1,sizeof(struct MyAEffect));
  myeffect->vstlib=vstlib;
  myeffect->name=strdup(name);

  effect=&myeffect->effect;
  memcpy(effect,&ar.effect,sizeof(struct AEffect));


  effect->dispatcher=VSTLIB_dispatcher;
  effect->process=VSTLIB_process;
#if 0
  // VSTLIB_process works both non-replacing and replacing, since
  // we must copy the samples to and from our own buffer anyway.
  // No, it doesn't work that way...
  effect->processReplacing=VSTLIB_process;
#else

#endif
  effect->setParameter=VSTLIB_setParameter;
  effect->getParameter=VSTLIB_getParameter;

  /* If the plugin doesn't provide a replacing-function, we simulate one in the library. */
  if ( (effect->flags & effFlagsCanReplacing) == 0) {
    effect->flags |=  effFlagsCanReplacing ;
    effect->processReplacing=VSTLIB_processReplacing_byprocess;
  }else{
    effect->processReplacing=VSTLIB_processReplacing;
  }


  /********** Setting up shared memory. ****************/


  segid=shmget(ar.sharedmem_key,0,0);
  if(segid==-1){
    fprintf(stderr,"VSTLIB/new: Problem getting shared mem from server. No seg-id.\n");
    VSTLIB_delete(effect);
    return NULL;
  }



  vstlib->mem=shmat(segid,0,0);
  if(vstlib->mem==NULL){
    fprintf(stderr,"VSTLIB/new: Problem getting shared mem from server. No mem.\n");
    VSTLIB_delete(effect);
    return NULL;
  }

  vstlib->sharedmem_elementsize=ar.sharedmem_elementsize;


  if(effect->numInputs>0){
    vstlib->inputs=malloc(sizeof(float *) * effect->numInputs);

    for(lokke=0;lokke<effect->numInputs;lokke++){
      vstlib->inputs[lokke]=vstlib->mem + (lokke*ar.sharedmem_elementsize);
    }
  }

  if(effect->numOutputs>0){
    vstlib->outputs=malloc(sizeof(float*) * effect->numOutputs);

    for(lokke=effect->numInputs;lokke<effect->numInputs+effect->numOutputs;lokke++){
      vstlib->outputs[lokke-effect->numInputs]=vstlib->mem + (lokke*ar.sharedmem_elementsize);
    }
  }



  /*************** Waiting for the server to have initialized everything. **************/

  C_receive(vstlib->control_input,&lokke,sizeof(int));


  return effect;

}

struct AEffect *VSTLIB_new(const char *name){
  return VSTLIB_newReal(name,true);
}

