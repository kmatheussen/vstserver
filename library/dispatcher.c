
#include <string.h>
#include <stdio.h>

#include <vstserver.h>
#include <k_communicate.h>

#include "vstfuncs.h"
#include "l_.h"

long VSTLIB_dispatcher(
		    AEffect *effect,
		    long opCode,
		    long index,
		    long value,
		    void *ptr,
		    float opt
		    )
{
  struct MyAEffect *myeffect=(struct MyAEffect*)effect;
  struct VSTLib *vstlib=myeffect->vstlib;
  //  struct VSTLib *vstlib=(struct VSTLib *)effect->user;
  struct VSTS_controlRequest cr={
    VSTP_dispatcher,
    opCode,
    index,
    value,
    {0},
    opt,
    0.0f
  };

  int lokke;
  char *string=(char *)ptr;

  switch(opCode){
  case effProcessEvents:
    l_effProcessEvents(myeffect,&cr,ptr);
    return 0;
  default:
    break;
  }



  /* Standard treatment. (should work for most of the requests) */

  if(string!=NULL){
    for(lokke=0;lokke<32;lokke++){
      cr.ptr[lokke]=string[lokke];
      if(cr.ptr[lokke]==0) break;
    }
  }

  C_send(vstlib->control_output,&cr,sizeof(struct VSTS_controlRequest));
  C_receive(vstlib->control_input,&cr,sizeof(struct VSTS_controlRequest));

  if(string!=NULL){
    for(lokke=0;lokke<32;lokke++){
      string[lokke]=cr.ptr[lokke];
      if(cr.ptr[lokke]==0) break;
    }
  }

  return cr.value;

}
