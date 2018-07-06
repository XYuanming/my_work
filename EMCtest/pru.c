#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <pthread.h>

#define DEBUG
#define SIZE 512
#define BIT_COUNT 62 + 36

void *func(void *arg)
{
	time_t timep;
	FILE * pfd = (FILE *)arg;
	while(1)
	{
		sleep(30);
		fprintf(pfd, "%spru is working.\n", asctime(gmtime(&timep)));
		fflush(pfd);
	}
	pthread_exit(NULL);
}


int main(int argc, char **argv)
{
	struct pollfd pollfds[1];
	char *portname = "/root/log/pru.log";
	char *path = "/dev/rpmsg_pru31";
	FILE *pfd;
	time_t timep;
	int nret = 0;
	int i, j;
	char preadBuff[SIZE];
	int flag;
	pthread_t pthid;

	pfd = fopen(portname, "a");
	setbuf(pfd, NULL);
	if(pfd == NULL)
	{
		perror("fopen pru.log fail.");
		return 0;
	}

	time(&timep);
	fprintf(pfd, "%spru start work.\n", asctime(gmtime(&timep)));
	fflush(pfd);

	pthread_create(&pthid, NULL, func, (void *)pfd);


	pollfds[0].fd = open(path, O_RDWR);
	if(pollfds[0].fd <= 0)
	{
		time(&timep);
		fprintf(pfd, "%s[open %s fail.]\n", asctime(gmtime(&timep)), path);
		fflush(pfd);
		perror("open rpmsg_pru31 fail.");
		return 0;
	}

	//设置read函数阻塞
	flag = fcntl(pollfds[0].fd, F_GETFL, 0);
	flag &= ~O_NONBLOCK;
	fcntl(pollfds[0].fd, F_SETFL, flag);

	nret = write(pollfds[0].fd, "12", 3);
	if(nret < 0)
	{
		time(&timep);
		fprintf(pfd, "%s[write %s fail. return value: %d]\n", asctime(gmtime(&timep)), path, nret);
		fflush(pfd);
		perror("open rpmsg_pru31 fail.");
		return 0;
	}
	while(1)
	{
		nret = read(pollfds[0].fd, preadBuff, SIZE);
		if(nret < 0)
		{
			time(&timep);
			fprintf(pfd, "%s[read %s fail. return value: %d]\n", asctime(gmtime(&timep)), path, nret);
			fflush(pfd);
			perror("read rpmsg_pru31 fail.");
			return 0;
		}
		else if(nret == 48)
		{
			time(&timep);
			fprintf(pfd, "%s\n", asctime(gmtime(&timep)));
			fflush(pfd);
			for(j=0; j<nret-1; j++)
			{
				fprintf(pfd, "%02x ", preadBuff[j]);
			}
			fprintf(pfd, "\n");
			fflush(pfd);
		}
#ifdef DEBUG
		else
		{
			printf("nret = %d\n", nret);
			for(j=0;j<nret;j++)
			{
				printf("%02x ", preadBuff[j]);
			}
			printf("\n");
		}
#endif
	}
	return 0;
}
