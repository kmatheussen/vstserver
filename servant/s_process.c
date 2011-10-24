
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
#include <vstserver.h>

#include "s_.h"

void VSTS_process(
		  struct VSTClient *vstclient,
		  int sampleframes
		  )
{
  struct AEffect *effect=vstclient->ws->effect;

  effect->process(effect,vstclient->inputs,vstclient->outputs,sampleframes);
  C_sendInt(vstclient->process_output,0);
}
