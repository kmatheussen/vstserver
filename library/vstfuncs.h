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


struct VSTLib{
  bool connectedtoserver;

  struct Communicate *control_input;
  struct Communicate *control_output;

  struct Communicate *newdelete_output;

  struct Communicate *process_input;
  struct Communicate *process_output;

  int sharedmem_elementsize;
  float *mem;
  float **inputs;
  float **outputs;

  int vstevents_arraylen;
  struct VstEvent *vstevent_array;
  struct VstEvents *vstevents;
}; 


struct MyAEffect{
  struct AEffect effect;
  struct VSTLib *vstlib;
  char *name;
};


void VSTLIB_setParameter(
		      AEffect *effect, 
		      long index,
		      float parameter
		      );

float VSTLIB_getParameter(
		      AEffect *effect, 
		      long index
		      );

void VSTLIB_process(
		 AEffect *effect,
		 float **inputs,
		 float **outputs,
		 long sampleframes
		 );
void VSTLIB_processReplacing_byprocess(
		 AEffect *effect,
		 float **inputs,
		 float **outputs,
		 long sampleframes
		 );

void VSTLIB_processReplacing(
		 AEffect *effect,
		 float **inputs,
		 float **outputs,
		 long sampleframes
		 );

long VSTLIB_dispatcher(
		    AEffect *effect,
		    long opCode,
		    long index,
		    long value,
		    void *ptr,
		    float opt
		    );
