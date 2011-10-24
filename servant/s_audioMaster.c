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

#include <stdbool.h>
#include <stdio.h>

#include "s_audioMaster.h"

/* The following audioMaster opcode handlings are copied (with a light modification)
   from the vst tilde source made by Mark Williamson.
*/


long VSTS_audioMaster(AEffect* effect,
			      long opcode,
			      long index,
			      long value,
			      void* ptr,
			      float opt)
{
#if 1 // If vst_tilde audioMaster (else plugin_tilde audioMaster)
  static float sample_rate = 44100.0f;
  static VstTimeInfo _timeInfo;

	// Support opcodes
  switch(opcode){
  case audioMasterAutomate:			
    return 0;		// index, value, returns 0
    
  case audioMasterVersion:			
    return 9;		// vst version, currently 7 (0 for older)
    
  case audioMasterCurrentId:			
    return 'AASH';	// returns the unique id of a plug that's currently loading
    
  case audioMasterIdle:
    effect->dispatcher(effect, effEditIdle, 0, 0, NULL, 0.0f);
    return 0;		// call application idle routine (this will call effEditIdle for all open editors too) 
    
  case audioMasterPinConnected:	
    return false;	// inquire if an input or output is beeing connected;
    
  case audioMasterWantMidi:			
    return 0;
    
  case audioMasterProcessEvents:		
    return 0; 	// Support of vst events to host is not available
    
    
  case audioMasterGetTime:
#if 0
    fprintf(stderr,"VST master dispatcher: audioMasterGetTime\n");
    break;
#else
    memset(&_timeInfo, 0, sizeof(_timeInfo));
    _timeInfo.samplePos = 0;
    _timeInfo.sampleRate = sample_rate;
    return (long)&_timeInfo;
#endif		
    
  case audioMasterTempoAt:			
    return 0;
    
  case audioMasterNeedIdle:	
    effect->dispatcher(effect, effIdle, 0, 0, NULL, 0.0f);
    return 1;
    
  case audioMasterGetSampleRate:		
    return sample_rate;	
    
  case audioMasterGetVendorString:	// Just fooling version string
    strcpy((char*)ptr,"Radium");
    return 0;
    
  case audioMasterGetVendorVersion:	
    return 5000;	// HOST version 5000
    
  case audioMasterGetProductString:	// Just fooling product string
    strcpy((char*)ptr,"A mighty crack from the past.");
    return 0;
    
  case audioMasterVendorSpecific:		
    {
      return 0;
    }
    
    
  case audioMasterGetLanguage:		
    return kVstLangEnglish;
    
  case audioMasterUpdateDisplay:
    //fprintf(stderr,"audioMasterUpdateDisplay\n");
    effect->dispatcher(effect, effEditIdle, 0, 0, NULL, 0.0f);
    return 0;
    
  case 	audioMasterSetTime:
    fprintf(stderr,"VST master dispatcher: Set Time\n");
    break;
  case 	audioMasterGetNumAutomatableParameters:
    fprintf(stderr,"VST master dispatcher: GetNumAutPar\n");
    break;
  case 	audioMasterGetParameterQuantization:
    fprintf(stderr,"VST master dispatcher: ParamQuant\n");
    break;
  case 	audioMasterIOChanged:		
    fprintf(stderr,"VST master dispatcher: IOchanged\n");
    break;
  case 	audioMasterSizeWindow:		
    fprintf(stderr,"VST master dispatcher: Size Window\n");
    break;
  case 	audioMasterGetBlockSize:	
    fprintf(stderr,"VST master dispatcher: GetBlockSize\n");
    break;
  case 	audioMasterGetInputLatency:	
    fprintf(stderr,"VST master dispatcher: GetInLatency\n");
    break;
  case 	audioMasterGetOutputLatency:	
    fprintf(stderr,"VST master dispatcher: GetOutLatency\n");
    break;
  case 	audioMasterGetPreviousPlug:	
    fprintf(stderr,"VST master dispatcher: PrevPlug\n");
    break;
  case 	audioMasterGetNextPlug:		
    fprintf(stderr,"VST master dispatcher: NextPlug\n");
    break;
  case 	audioMasterWillReplaceOrAccumulate:	
    fprintf(stderr,"VST master dispatcher: WillReplace"); 
    break;
  case 	audioMasterGetCurrentProcessLevel:	
    return 0;
    break;
  case 	audioMasterGetAutomationState:		
    fprintf(stderr,"VST master dispatcher: GetAutState\n");
    break;
  case 	audioMasterOfflineStart:	
    fprintf(stderr,"VST master dispatcher: Offlinestart\n");
    break;
  case 	audioMasterOfflineRead:		
    fprintf(stderr,"VST master dispatcher: Offlineread\n");
    break;
  case 	audioMasterOfflineWrite:	
    fprintf(stderr,"VST master dispatcher: Offlinewrite\n");
    break;
  case 	audioMasterOfflineGetCurrentPass:	
    fprintf(stderr,"VST master dispatcher: OfflineGetcurrentpass\n");
    break;
  case 	audioMasterOfflineGetCurrentMetaPass:	
    fprintf(stderr,"VST master dispatcher: GetGetCurrentMetapass\n");
    break;
  case 	audioMasterSetOutputSampleRate:		
    fprintf(stderr,"VST master dispatcher: Setsamplerate\n");
    break;
  case 	audioMasterGetSpeakerArrangement:	
    fprintf(stderr,"VST master dispatcher: Getspeaker\n");
    break;
  case 	audioMasterSetIcon:			
    fprintf(stderr,"VST master dispatcher: seticon\n");
    break;
  case 	audioMasterCanDo:	
    fprintf(stderr,"VST master dispatcher: Can Do\n");
    break;
  case 	audioMasterOpenWindow:		
    fprintf(stderr,"VST master dispatcher: OpenWindow\n");
    break;
  case	audioMasterCloseWindow:		
    fprintf(stderr,"VST master dispatcher: CloseWindow\n");
    break;
  case	audioMasterGetDirectory:	
    fprintf(stderr,"VST master dispatcher: GetDirectory\n");
    break;
    
  default:
    fprintf(stderr,"VST master dispatcher: undefed: %d , %d\n",opcode , effKeysRequired );
    break;
  }	
  
  
  return 0;


#else
    char param_name[9];
#if 0 /*PLUGIN_TILDE_DEBUG*/
    /* The following procedure is copied (with a light modification) from the plugin tilde source
       made by Jarno Seppänen. I dont know what it does, but it needs
       to be here. (Got to read that VST documentation soon. :)
    */



    fprintf(stderr,"DEBUG plugin~_vst: audioMaster(0x%p, %ld, %ld, %ld, 0x%p, %f)",
	   effect, opcode, index, value, ptr, opt);
#endif

    switch (opcode) {
	case audioMasterAutomate:
#if 0
	    if(effect==NULL){
	      fprintf(stderr,"effect==NULL\n");
	    }
	    if(effect->user==NULL){
	      fprintf(stderr,"effect->user==NULL\n");
	    }
#endif
	    effect->setParameter (effect, index, opt);
	    /* Send "control" messages from here */
	    memset (param_name, 0, 9);
	    effect->dispatcher (effect, effGetParamName, index, 0, param_name, 0);
	    //	  plugin_tilde_emit_control_output (effect->user, param_name, opt);
	    return 0;
	    break;
	case audioMasterVersion:
	    return 1;
	    break;
	case audioMasterCurrentId:
	    return 0;
	    break;
	case audioMasterIdle:
	  // This is a different thread, isnt it? The winwin thread should provide enough idle cycles.
	  //	    effect->dispatcher (effect, effEditIdle, 0, 0, NULL, 0);
	    return 0;
	    break;
	case audioMasterPinConnected:
	    /* return 0="true" for all inquiries for now */
	    return 0;
	    break;
    }
#if 0 /*PLUGIN_TILDE_DEBUG*/
    fprintf(stderr,"DEBUG plugin~_vst: warning: unsupported audioMaster opcode");
#endif
    return 0;
#endif
}

