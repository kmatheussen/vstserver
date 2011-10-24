

#include <dirent.h>

int myselect(const struct dirent *dasdirent){
  if(!strcmp("..",dasdirent->d_name)) return 0;
  if(!strcmp(".",dasdirent->d_name)) return 0;
  if(!strcmp("vstservant.so",dasdirent->d_name)) return 0;
  return 1;
}

main(){
  struct dirent **namelist;
  int n;
  
  n = scandir(".", &namelist, myselect, alphasort);
  if (n < 0)
    perror("scandir");
  else {
    while(n--) {
      printf("%s\n", namelist[n]->d_name);
      free(namelist[n]);
    }
  }
}

