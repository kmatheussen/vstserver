
#include <unistd.h>

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(){
  	int fd1=open("/asdf/gakk",O_RDWR|O_CREAT|O_EXCL,S_IRWXU);
	//	int fd2=open("gakk",O_RDWR|O_CREAT|O_EXCL,S_IRWXU);

	fprintf(stderr,"fd1: %d, errno: %d, strerror: %s\n",fd1,errno,strerror(errno));

	//	fprintf(stderr,"fd2: %d -%s-\n",fd2,strerror (errno));

	

	close(fd1);

	//	unlink("gakk");

	return 0;
}

