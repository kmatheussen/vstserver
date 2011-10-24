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

#include <vst/AEffect.h>
#include <vstserver.h>
#include <k_communicate.h>

#include "vstfuncs.h"

void VSTLIB_setParameter(
		      AEffect *effect, 
		      long index,
		      float parameter
		      )
{
  struct MyAEffect *myeffect=(struct MyAEffect*)effect;
  struct VSTLib *vstlib=myeffect->vstlib;
  //  struct VSTLib *vstlib=(struct VSTLib *)effect->user;
  struct VSTS_controlRequest cr;

  cr.reqtype=VSTP_setParameter;
  cr.index=index;
  cr.parameter=parameter;

  C_send(vstlib->control_output,&cr,sizeof(struct VSTS_controlRequest));
}
