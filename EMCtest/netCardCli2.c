#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <syslog.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <pthread.h>

#define DEBUG
#define SIZE 100

static struct sockaddr_in server_addr;
static int socketFd;
static char *PORT, *IP;
static time_t timep;
static FILE *pfd;

void reconnect()
{
	int nret;
	if((socketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		time(&timep);
		fprintf(pfd, "%sIP: [%s], PORT: [%s]\nsocket() fail. return value: [%d]\n", asctime(gmtime(&timep)), IP, PORT, socketFd);
		fflush(pfd);
		return;
	}
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(PORT));
	server_addr.sin_addr.s_addr = inet_addr(IP);
	while(1)
	{
		nret = connect(socketFd, (struct sockaddr *)&server_addr, sizeof(server_addr));
		if(nret == 0)
			break;
		else
		{
			time(&timep);
			fprintf(pfd, "%sIP: [%s]; PORT: [%s]\nreconnection fail. return value: [%d]\n", asctime(gmtime(&timep)), IP, PORT, nret);
			fflush(pfd);
			printf("reconnection fail\n");
			printf("nret = %d\n", nret);
			sleep(1);
		}
	}

}

void *func()
{
	while(1)
	{
		sleep(30);
		time(&timep);
		fprintf(pfd, "%snetclient2 is working.\n", asctime(gmtime(&timep)));
		fflush(pfd);
	}
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	int nret;
	socklen_t len_server_addr;
	char preadBuff[SIZE], pwriteBuff[SIZE];
	struct tcp_info info;
	int length = sizeof(struct tcp_info);
	bzero(&pwriteBuff, sizeof(pwriteBuff));
	strcpy(pwriteBuff, "helloworld");
	pthread_t pthid;

	if(argc < 3)
	{
		printf("Usage : %s <port> <ip>\n", argv[0]);
		return 0;
	}
	PORT = argv[1];
	IP = argv[2];

	pfd = fopen("/home/pi/log/netclient2.log", "a");
	setbuf(pfd, NULL);
	if(pfd == NULL)
	{
		perror("fopen netclient2.log fail");
		return 0;
	}

	time(&timep);
	fprintf(pfd, "%snetclint2 start work.\n", asctime(gmtime(&timep)));
	fflush(pfd);

	pthread_create(&pthid, NULL, func, NULL);

	if((socketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		time(&timep);
		fprintf(pfd, "%sIP: [%s]; PORT: [%s]\nsocket() fail. return value: [%d]\n", asctime(gmtime(&timep)), IP, PORT, socketFd);
		fflush(pfd);
		return 0;
	}
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[1]));
	server_addr.sin_addr.s_addr = inet_addr(argv[2]);

	while(1)
	{
		nret = connect(socketFd, (struct sockaddr *)&server_addr, sizeof(server_addr));
		if(nret == 0)
			break;
		else
		{
			time(&timep);
			fprintf(pfd, "%sIP: [%s]; PORT: [%s]\nconnect fail. return value: [%d]\n", asctime(gmtime(&timep)), IP, PORT, nret);
			fflush(pfd);
			sleep(1);
		}
	}

#ifdef DEBUG
	time(&timep);
	fprintf(pfd, "%sIP: [%s], PORT: [%s]\nconnect success.\n", asctime(gmtime(&timep)), IP, PORT);
	fflush(pfd);
	printf("connect ok\n");
#endif
	struct timeval timeout = {5,0};
	nret = setsockopt(socketFd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
	if (nret != 0)
		printf("set recv timeout fail\n");
	while(1)
	{
		while(1)
		{
			getsockopt(socketFd, IPPROTO_TCP, TCP_INFO, (void *)&info, (socklen_t *)&length);
			if(info.tcpi_state != TCP_ESTABLISHED)
			{
				time(&timep);
				fprintf(pfd, "%sIP: [%s]; PORT: [%s]\nsocket break.\n", asctime(gmtime(&timep)), IP, PORT);
				fflush(pfd);
				close(socketFd);
				reconnect();
			}
			bzero(&preadBuff, sizeof(preadBuff));
			nret = recv(socketFd, preadBuff, SIZE, 0);
			printf("preadBuff: [%s]\n", preadBuff);
			if(nret == -1)
			{
				time(&timep);
				fprintf(pfd, "%sIP: [%s]; PORT: [%s]\nrecv timeout.\n", asctime(gmtime(&timep)), IP, PORT);
				fflush(pfd);
			}
			else if (nret < 0)
			{
				time(&timep);
				fprintf(pfd, "%sIP: [%s]; PORT: [%s]\nrecv() fail; return value = [%d].\n", asctime(gmtime(&timep)), IP, PORT, nret);
				fflush(pfd);
				perror("recv fail");
				return 0;
			}
			else if (nret == 0)
				sleep(1);
			else
			{
				if((strcmp(pwriteBuff, preadBuff)) != 0)
				{
					time(&timep);
					fprintf(pfd, "%sIP: [%s]; PORT: [%s]\nread FAIL!, msg : [%s]\n", asctime(gmtime(&timep)), IP, PORT, preadBuff);
					fflush(pfd);
				}
				else
					break;
			}
		}
		sleep(1);
		while(1)
		{
			nret = send(socketFd, pwriteBuff, strlen(preadBuff), 0);
			if(nret <= 0)
			{
				time(&timep);
				fprintf(pfd, "%sIP: [%s]; PORT: [%s]\nsend() fail; return value = [%d].\n", asctime(gmtime(&timep)), IP, PORT, nret);
				fflush(pfd);
			}
			else
				break;
		}
	}
	fclose(pfd);
	closelog();
	return 0;
}
