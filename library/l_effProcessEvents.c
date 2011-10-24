
#include <stdio.h>
#include <stdlib.h>

#include <vstserver.h>
#include <k_communicate.h>

#include "vstfuncs.h"
#include "l_.h"


void l_effProcessEvents(
			struct MyAEffect *myeffect,
			struct VSTS_controlRequest *cr,
			void *ptr
			)
{
  int lokke;

  struct VSTLib *vstlib=myeffect->vstlib;
  struct VstEvents *vstevents=(struct VstEvents*)ptr;

  int num_events = cr->value = vstevents->numEvents;

  C_send(vstlib->control_output,cr,sizeof(struct VSTS_controlRequest));


  if(num_events > vstlib->vstevents_arraylen){

    free(vstlib->vstevent_array);
    free(vstlib->vstevents);

    vstlib->vstevent_array=malloc(num_events*sizeof(struct VstEvent));

    if(num_events<=2){
      vstlib->vstevents=calloc(1,sizeof(struct VstEvents));
    }else{
      vstlib->vstevents=calloc(1,sizeof(struct VstEvents) + sizeof(struct VstEvent*)*(num_events-2));
    }

    for(lokke=0;lokke<num_events;lokke++){
      vstlib->vstevents->events[lokke]=&vstlib->vstevent_array[lokke];
    }

    vstlib->vstevents_arraylen=num_events;
  }

  for(lokke=0;lokke<num_events;lokke++){
    memcpy(vstlib->vstevents->events[lokke],vstevents->events[lokke],sizeof(struct VstEvent));
  }

  C_send(vstlib->control_output,vstlib->vstevent_array,sizeof(struct VstEvent)*num_events);

}
