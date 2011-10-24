

#define NULL_HANDLE 0

struct WinWinStuff{
  HINSTANCE hInst;
  HINSTANCE hPrevInst;
  LPSTR cmdline;
  int cmdshow;
  char appname[9];

  WNDCLASS wc;
};



