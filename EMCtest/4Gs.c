#include<stdio.h>
#include<sys/types.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<time.h>
#include<syslog.h>
#include <netinet/tcp.h>
#include <pthread.h>

#define DEBUG
#define SIZE 100

static FILE *pfd;
static time_t timep;
static char *PORT = "10850";
static socklen_t len_client;
static int connect_fd[100], listen_fd;
static struct sockaddr_in server_addr, client_addr;
static pthread_t pthreadid[100];

void *function(void *arg)
{
	printf("connect ok\n");

	int connect_fd = *(int *)arg;

	struct tcp_info info;
	int nret;
	int length = sizeof(struct tcp_info);
	char preadBuff[SIZE], pwriteBuff[SIZE];
	bzero(&pwriteBuff, sizeof(pwriteBuff));
	strcpy(pwriteBuff, "helloworld");

	while(1)
	{
		while(1)
		{
			nret = send(connect_fd, pwriteBuff, strlen(pwriteBuff), 0);
			printf("send: %s\n", pwriteBuff);
			if(nret != strlen(pwriteBuff))
			{
				time(&timep);
				fprintf(pfd, "%ssend fail. return value: [%d]\n", asctime(gmtime(&timep)), nret);
				fflush(pfd);
				perror("send fail");
				sleep(1);
			}
			if(nret > 0)
				break;
		}
		while(1)
		{
			getsockopt(connect_fd, IPPROTO_TCP, TCP_INFO, (void *)&info, (socklen_t *)&length);
			if(info.tcpi_state != TCP_ESTABLISHED)
			{
				close(connect_fd);
				time(&timep);
				fprintf(pfd, "%sIP: %s\nconnection break.\n", asctime(gmtime(&timep)), inet_ntoa(client_addr.sin_addr));
				fflush(pfd);
				printf("Waiting for the connection\n");
				if((connect_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &len_client)) < 0)
				{
					time(&timep);
					fprintf(pfd, "%saccept fail. return value: [%d]\n", asctime(gmtime(&timep)), connect_fd);
					fflush(pfd);
					perror("accept");
					return NULL;
				}
				time(&timep);
				fprintf(pfd, "%sIP: %s\nreconnection success.\n", asctime(gmtime(&timep)), inet_ntoa(client_addr.sin_addr));
				fflush(pfd);
				break;
			}
			bzero(&preadBuff, SIZE);
			nret = recv(connect_fd, preadBuff, SIZE, 0);
			printf("recv: %s\n", preadBuff);
			if(nret < 0)
			{
				time(&timep);
				fprintf(pfd, "%srecv fail. return value: [%d]\n", asctime(gmtime(&timep)), nret);
				fflush(pfd);
				return NULL;
			}
			else if(nret == 0)
				sleep(1);
			else
			{
				nret = strcmp(pwriteBuff, preadBuff);
				if(nret == 0)
					break;
				else
				{
					time(&timep);
					fprintf(pfd, "%sread FAIL! msg: [%s]. return value: [%d]\n", asctime(gmtime(&timep)), preadBuff, nret);
					fflush(pfd);
					break;
				}
			}
			/*getsockopt(connect_fd, IPPROTO_TCP, TCP_INFO, (void *)&info, (socklen_t *)&length);
			if(info.tcpi_state != TCP_ESTABLISHED)
			{
				close(connect_fd);
				time(&timep);
				fprintf(pfd, "%sIP: %s\nconnection break.\n", asctime(gmtime(&timep)), inet_ntoa(client_addr.sin_addr));
				fflush(pfd);
				printf("Waiting for the connection\n");
				if((connect_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &len_client)) < 0)
				{
					time(&timep);
					fprintf(pfd, "%saccept fail. return value: [%d]\n", asctime(gmtime(&timep)), connect_fd);
					fflush(pfd);
					perror("accept");
					return NULL;
				}
				time(&timep);
				fprintf(pfd, "%sIP: %s\nreconnection success.\n", asctime(gmtime(&timep)), inet_ntoa(client_addr.sin_addr));
				fflush(pfd);
				break;
			}*/
		}
	}
	fclose(pfd);
	close(connect_fd);
	pthread_exit(NULL);
}

void *func(void *arg)
{
	FILE *pfd = (FILE *)arg;
	time_t timep;
	while(1)
	{
		sleep(30);
		time(&timep);
		fprintf(pfd, "%s4Gserver is working.\n", asctime(gmtime(&timep)));
		fflush(pfd);
	}
	pthread_exit(NULL);
}

void waitconnect()
{
	pthread_t pthid;
	pfd = fopen("/home/pi/log/4G.log", "a");
	//setbuf(pfd, NULL);
	if(pfd == NULL)
	{
		perror("fopen 4G.log fail");
		return;
	}

	time(&timep);
	fprintf(pfd, "%s4Gserver start work.\n", asctime(gmtime(&timep)));
	fflush(pfd);

	pthread_create(&pthid, NULL, func, (void *)pfd);

	int i = 0;
	int j = 0;
	int nresult;
	if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket fail");
		return;
	}

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi("10850"));
	//server_addr.sin_addr.s_addr = inet_addr("192.168.8.85");
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int option = 1;
	if(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) != 0)
		perror("setsockopt");

	if(bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("bind");
		return;
	}

#ifdef DEBUG
	printf("bind ok\n");
#endif

	if(listen(listen_fd, 10) < 0)
	{
		perror("listen");
		return;
	}

#ifdef DEBUG
	printf("listen ok\n");
#endif

	len_client = sizeof(client_addr);
/*
	while(1)
	{
		if((connect_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &len_client)) < 0)
		{
			perror("accept");
			close(connect_fd);
		}
		else
			function();
	}*/
	while(1)
	{
		if((connect_fd[j] = accept(listen_fd, (struct sockaddr *)&client_addr, &len_client)) < 0)
		{
			perror("accept");
			close(connect_fd[j]);
			continue;
		}
		else
		{
			while(1)
			{
				nresult = pthread_create(&pthreadid[i++], NULL, function, (void *)(&connect_fd[j]));
				if(nresult != 0)
				{
					time(&timep);
					fprintf(pfd, "%spthread_create fail\n", asctime(gmtime(&timep)));
					fflush(pfd);
					perror("pthread_create");
					i--;
				}
				else
				{
					time(&timep);
					fprintf(pfd, "%sIP: %s\nconnect success!\n", asctime(gmtime(&timep)), inet_ntoa(client_addr.sin_addr));
					fflush(pfd);
					break;
				}
			}
			j++;
		}
	}
}


int main(int argc, char *argv[])
{
	waitconnect();

	return 0;
}

