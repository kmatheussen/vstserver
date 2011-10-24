
/* 
The code here is based on the winemine source in the wine distribution, and
some guessing.

I dont understand all what happens, I have just copied code. If you know windows
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


#include <unistd.h>

#include <stdio.h>

#include <windows.h>
#include "resource.h"

#include "windowsstuff.h"
#include "winwin.h"
#include "winwin_proc.h"


/* This struct is from a .hpp include file in include/vst/. I just write it again here to avoid
   mixing c and c++.
 */
struct ERect{
  short top;
  short left;
  short bottom;
  short right;
};



void *WINWIN_handler(void *windowsstuff){
  struct WindowsStuff *ws=(struct WindowsStuff*)windowsstuff;
  struct WinWinStuff *wws=(struct WinWinStuff*)ws->winwinstuff;

  MSG msg;

  //  HACCEL haccel;
  
  struct ERect *er;
  

  ws->hWnd = CreateWindow( wws->appname, wws->appname,
			   WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
			   CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			   NULL_HANDLE, NULL_HANDLE, wws->hInst, NULL_HANDLE );
  

  if ( ws->hWnd==NULL) return(NULL);
  
  ws->effect->dispatcher(ws->effect,
			 effEditOpen,
			 0, 0, ws->hWnd, 0
			 );
  


  ws->effect->dispatcher(ws->effect,effEditGetRect, 0, 0, &er, 0.0f);
  SetWindowPos(ws->hWnd, 0, 0, 0, er->right-er->left+8, er->bottom-er->top+26, SWP_NOMOVE | SWP_NOZORDER);

  ShowWindow( ws->hWnd, wws->cmdshow );

  UpdateWindow( ws->hWnd );

  /*
rcEffClient.left, rcEffClient.top,
                    rcEffClient.right, rcEffClient.bottom);
  */
  
  //  haccel = LoadAccelerators( wws->hInst, appname );
  SetTimer( ws->hWnd, ID_TIMER, 10, NULL_HANDLE );
  

  while( GetMessage(&msg, NULL_HANDLE, 0, 0) ) {
#if 0
    if (!TranslateAccelerator( ws->hWnd, haccel, &msg ))
      TranslateMessage( &msg );
#endif


    DispatchMessage( &msg );

    if(ws->pleaseclosethewindow==1) break;

    ws->effect->dispatcher(ws->effect,
			   effEditIdle,
			   0, 0, NULL, 0
			   );
  



  }

  CloseWindow((HWND)ws->hWnd);
  ws->hWnd=NULL;


  return NULL;
}



// The code that calls this function must wait until ws->hWnd is NULL after
// calling before continuing execution.

void WINWIN_close(struct WindowsStuff *ws){
  if(ws->hWnd==NULL) return;
  //  CloseWindow((HWND)ws->hWnd);
  ws->pleaseclosethewindow=1;
  return;
}
