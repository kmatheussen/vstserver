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
#include <string.h>

#include <vst/AEffect.h>
#include <vstserver.h>
#include <k_communicate.h>

#include "vstfuncs.h"

#include "processing.h"


/*
"[3]" wrote:
> As far as process/replacing goes
> Insert plugins call process(),
>     out+= effect..
> where as send plugins and vst-synths call process-replacing,
>     out=effect.
>
*/



#define ISREPLACE(a) ((a)<0)
#define ISPROCESS(a) ((a)>0)

static void VSTLIB_processing_do(
			      struct VSTLib *vstlib,
			      AEffect *effect,
			      float **inputs,
			      float **outputs,
			      long sampleframes,
			      int replace // If this is -1, do processreplace. If 1, do processing
			      )
{
  int das_sampleframes=replace*sampleframes;
  int ret;
  int ch;

  for(ch=0;ch<effect->numInputs;ch++){
    memcpy(vstlib->inputs[ch],inputs[ch],sampleframes*sizeof(float));
  }

  if(ISPROCESS(replace)){
    for(ch=0;ch<effect->numOutputs;ch++){
      memcpy(vstlib->outputs[ch],outputs[ch],sampleframes*sizeof(float));
    }
  }

  C_send(vstlib->process_output,&das_sampleframes,sizeof(int));
  C_receive(vstlib->process_input,&ret,sizeof(int));

  for(ch=0;ch<effect->numOutputs;ch++){
    memcpy(outputs[ch],vstlib->outputs[ch],sampleframes*sizeof(float));
  }
}


static void VSTLIB_processing_many(
				   struct VSTLib *vstlib,
				   AEffect *effect,
				   float **inputs,
				   float **outputs,
				   long sampleframes,
				   int replace // If this is -1, do processreplace. If 1, do processing
				   )
{
  int ch;
  float *new_inputs[effect->numInputs];
  float *new_outputs[effect->numOutputs];

  for(ch=0;ch<effect->numInputs;ch++){
    new_inputs[ch]=inputs[ch];
  }
  for(ch=0;ch<effect->numOutputs;ch++){
    new_outputs[ch]=outputs[ch];
  }

  for(;;){
    VSTLIB_processing_do(
		      vstlib,
		      effect,
		      new_inputs,
		      new_outputs,
		      MIN(sampleframes,vstlib->sharedmem_elementsize),
		      replace
		      );
    
    sampleframes-=vstlib->sharedmem_elementsize;
    if(sampleframes<=0) break;

    for(ch=0;ch<effect->numInputs;ch++){
      new_inputs[ch]+=vstlib->sharedmem_elementsize;
    }
    for(ch=0;ch<effect->numOutputs;ch++){
      new_outputs[ch]+=vstlib->sharedmem_elementsize;
    }
  }
}

void VSTLIB_processing(
		       struct VSTLib *vstlib,
		       AEffect *effect,
		       float **inputs,
		       float **outputs,
		       long sampleframes,
		       int replace // If this is -1, do processreplace. If 1, do processing
		       )
{
  if(sampleframes > vstlib->sharedmem_elementsize){
    VSTLIB_processing_many(vstlib,effect,inputs,outputs,sampleframes,replace);
  }else{
    VSTLIB_processing_do(vstlib,effect,inputs,outputs,sampleframes,replace);
  }
}
