
#include <stdio.h>
#include <stdlib.h>

#include <vstserver.h>

#include "s_.h"


/*
  int vstevents_arraylen;
  struct VstEvent *vstevent_array;
  struct VstEvents *vstevents;
*/

void s_effProcessEvents(
			struct VSTClient *vstclient,
			struct VSTS_controlRequest *cr
			)
{
  int num_events=cr->value;
  int num_read;

  if(num_events > vstclient->vstevents_arraylen){
    int lokke;

    free(vstclient->vstevent_array);
    free(vstclient->vstevents);

    vstclient->vstevent_array=malloc(num_events*sizeof(struct VstEvent));

    if(num_events<=2){
      vstclient->vstevents=calloc(1,sizeof(struct VstEvents));
    }else{
      vstclient->vstevents=calloc(1,sizeof(struct VstEvents) + sizeof(struct VstEvent*)*(num_events-2));
    }

    for(lokke=0;lokke<num_events;lokke++){
      vstclient->vstevents->events[lokke]=&vstclient->vstevent_array[lokke];
    }

    vstclient->vstevents_arraylen=num_events;

  }

  vstclient->vstevents->numEvents=num_events;

  num_read=C_receive(
		     vstclient->control_input,
		     vstclient->vstevent_array,
		     sizeof(struct VstEvent) * num_events
		     );

  if(num_read!=sizeof(struct VstEvent) * num_events){
    fprintf(stderr,"s_effProcessEvents/s_effProcessEvents: Could not read events. Only got %d.\n",num_read);
    return;
  }

  vstclient->ws->effect->dispatcher(
				    vstclient->ws->effect,
				    effProcessEvents,
				    0,
				    0,
				    vstclient->vstevents,
				    0.0f
				    );


}

