#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define maxlen 1024
#define port 6789

int wgs_atoi(char * str)
{
    int ret=0;
    while((*str)!='\0')
    {
        ret=((*str)-'0') + ret*10;
        str++;
    }
    return ret;
}

int main(int argc, char** argv) {

	int flag=0;
	FILE * fp;
	pid_t pid1;
	pid_t pid2;
	pid_t pid3;
    //int sin_len;
    socklen_t sin_len;
    char message[256];

	flag=0;
	//fp=fopen("/data/allserverdata.txt","w+");

	pid1=fork();
	if(pid1<0)
	{
		printf("fork error!\n");
		exit(1);
	}
	else if(pid1 == 0)
	{
		printf("GPS child process\n");
		if(execl("mytestgps","mytestgps",/*"-g",argv[2],"stop",*/NULL)<0)
		{
			fprintf(stderr,"execl failed: %s\n",strerror(errno));
			exit(0);
		}
	}

	pid2=fork();
	if(pid2<0)
	{
		printf("fork error!\n");
		exit(1);
	}
	else if(pid2 == 0)
	{
		printf("Sensor child process\n");
		if(execl("mytestsensor","mytestsensor",/*"-g",argv[4],argv[5],*/NULL)<0)
		{
			fprintf(stderr,"execl failed: %s\n",strerror(errno));
			exit(0);
		}
	}

	printf("server start to wait\n");

    exit(0);

    return (EXIT_SUCCESS);
}
