



#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <vstlib.h>


void vst_print (AEffect *plugin)
{
    unsigned i;

    for (i = 0; i < plugin->numParams; i++) {
	/* the Steinberg(tm) way... */
	char name[9]={0};
	char display[25]={0};
	char label[9]={0};
	if (i == 0) {
	    printf ("Control input/output(s):\n");
	}
	memset (name, 0, 9);
	memset (display, 0, 25);
	memset (label, 0, 9);
	plugin->dispatcher (plugin,
					    effGetParamName,
					    i, 0, name, 0);

	plugin->dispatcher (plugin,
					    effGetParamDisplay,
					    i, 0, display, 0);
	plugin->dispatcher (plugin,
					    effGetParamLabel,
					    i, 0, label, 0);
	printf (" #%d \"%s\" (%s %s)\n",
		i + 1, name, display, label);
    }
}


float in1[200];
float in2[200];
float out1[200];
float out2[200];

int main(){
  float **in=calloc(2,sizeof(float*));
  float **out=calloc(2,sizeof(float*));
  char temp[500];
  //  struct AEffect *effect=VST_new("Dynasone.dll");
  //  struct AEffect *effect=VST_new("dB Tremelo.dll");
  //  struct AEffect *effect=VSTLIB_new("Comp_S_01.dll");
  struct AEffect *effect=VSTLIB_new("PitchAccumStereo.dll");

  if(effect==NULL) return -1;

  in[0]=&in1[0];
  in[1]=&in2[0];
  out[0]=&out1[0];
  out[1]=&out2[0];

  //  printf("2effect: %x %d\n",(unsigned int)effect,effect->numParams);

  //  gets(temp);

  vst_print(effect);

  //gets(temp);

  //  fprintf(stderr,"inputs: %d, outputs: %d\n",effect->numInputs,effect->numOutputs);

  in[1][199]=1.4f;
  effect->process(effect,in,out,200);

  effect->dispatcher (effect,
		      effEditOpen,
		      0, 0, NULL, 0);
  


  fprintf(stderr,"2 %f\n",out[1][199]);
  gets(temp);
  VSTLIB_delete(effect);
  //gets(temp);
  return 0;
}
