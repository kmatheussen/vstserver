/*

The code here is based on the winemine source in the wine distribution.
I dont understand what happens, I have just copied code. If you know windows
programming, it would be nice if you could have a look at it.


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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include "resource.h"

#include "winwin.h"

#include "windowsstuff.h"

#include "../mainhandler.h"



static LRESULT WINAPI WINWIN_proc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch(msg){
  case WM_DESTROY:
    PostQuitMessage( 0 );
    return 0;
  }
  
  return( DefWindowProc( hWnd, msg, wParam, lParam ));
}




/* 
   This is where the program starts. Theres no main().
 */

int WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdline, int cmdshow ){
    struct WinWinStuff wws;

    
    wws.hInst=hInst;
    wws.hPrevInst=hPrevInst;
    wws.cmdline=cmdline;
    wws.cmdshow=cmdshow;



    LoadString( hInst,IDS_APPNAME, wws.appname, sizeof(wws.appname));
    //sprintf(wws.appname,"gakk");

    wws.wc.style = 0;
    wws.wc.lpfnWndProc = WINWIN_proc;
    //wc.lpfnWndProc = NULL_HANDLE;
    wws.wc.cbClsExtra = 0;
    wws.wc.cbWndExtra = 0;
    wws.wc.hInstance = hInst;
    wws.wc.hIcon = LoadIcon( hInst, wws.appname );
    wws.wc.hCursor = LoadCursor( NULL_HANDLE, IDI_APPLICATION );
    wws.wc.hbrBackground = (HBRUSH) GetStockObject( BLACK_BRUSH );
    wws.wc.lpszMenuName = "MENU_WINEMINE";
    wws.wc.lpszClassName = wws.appname;
    
    if (!RegisterClass(&wws.wc)){
      fprintf(stderr,"Seems to a problem with some windows-stuff.\n");
      return 1;
    }

    return MAINHANDLER_control(&wws,cmdline);
}
