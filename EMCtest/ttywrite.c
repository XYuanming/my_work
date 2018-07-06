#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <syslog.h>
#include <sys/wait.h>
#include <pthread.h>
	int
set_interface_attribs (int fd, int speed, int parity)
{
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	if (tcgetattr (fd, &tty) != 0)
	{
		perror("tcgetattr fail");
		//error_message ("error %d from tcgetattr", errno);
		return -1;
	}

	cfsetospeed (&tty, speed);
	cfsetispeed (&tty, speed);

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
	// disable IGNBRK for mismatched speed tests; otherwise receive break
	// as \000 chars
	tty.c_iflag &= ~IGNBRK;         // disable break processing
	tty.c_lflag = 0;                // no signaling chars, no echo,
	// no canonical processing
	tty.c_oflag = 0;                // no remapping, no delays
	tty.c_cc[VMIN]  = 0;            // read doesn't block
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

	tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
	// enable reading
	tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
	tty.c_cflag |= parity;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;

	if (tcsetattr (fd, TCSANOW, &tty) != 0)
	{
		perror("tcgetattr fail");
		//error_message ("error %d from tcsetattr", errno);
		return -1;
	}
	return 0;
}

	void
set_blocking (int fd, int should_block)
{
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	if (tcgetattr (fd, &tty) != 0)
	{
		perror("tcgetattr fail");
		//error_message ("error %d from tggetattr", errno);
		return;
	}

	tty.c_cc[VMIN]  = should_block ? 1 : 0;
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

	if (tcsetattr (fd, TCSANOW, &tty) != 0)
		perror("tcgetattr fail");
	//error_message ("error %d setting term attributes", errno);
}

static char *portname = NULL;

void *func(void *arg)
{
	FILE *pfd = (FILE *)arg;
	time_t timep;
	while(1)
	{
		sleep(30);
		time(&timep);
		fprintf(pfd, "%s%s is working.\n", asctime(gmtime(&timep)), portname);
		fflush(pfd);
	}
	pthread_exit(NULL);
}

int main(int argc, char **argv)
{
	time_t timep;
	int fd, ret, wrt;
	char buff[100];
	char pbuf[100];
	bzero(&pbuf, sizeof(pbuf));
	strcpy(pbuf, "helloword");
	FILE *pfd;
	int flag;
	int value;
	pthread_t pthid;

	if(argc < 2)
	{
		printf("Usage: %s <path>\n", argv[0]);
		return 0;
	}
	char *portname = argv[1];

	pfd = fopen("/root/log/tty.log", "a");
	setbuf(pfd, NULL);
	if(pfd == NULL)
	{
		perror("fopen tty.log fail");
		return 0;
	}

	time(&timep);
	fprintf(pfd, "%s%s start work.\n", asctime(gmtime(&timep)), portname);
	fflush(pfd);

	pthread_create(&pthid, NULL, func, (void *)pfd);

	fd = open (portname, O_RDWR | O_NOCTTY | O_SYNC);
	if(fd <= 0)
	{
		time(&timep);
		fprintf(pfd, "%s%s\nopen %s fail. return value: [%d]\n", asctime(gmtime(&timep)), portname, portname, fd);
		fflush(pfd);
		printf("open %s fail\n", portname);
		return 0;
	}

	set_interface_attribs (fd, B9600, 0);  // set speed to 115,200 bps, 8n1 (no parity)
	set_blocking (fd, 0);                // set no blocking

	while(1)
	{
		value = 0;
		time(&timep);
		wrt = write(fd, pbuf, strlen(pbuf));
		if(wrt != strlen(pbuf))
		{
			time(&timep);
			fprintf(pfd, "%s%s\nwrite fail. return value: [%d].\n", asctime(gmtime(&timep)), portname, ret);
			fflush(pfd);
		}
		sleep(1);
		while(1)
		{
			time(&timep);
			bzero(&buff, sizeof(buff));
			ret = read(fd, buff, sizeof(buff));
			printf("%s   read: [%s]\n", portname, buff);
			if (ret < 0)
			{
				fprintf(pfd, "%s%s\nread fail. return value: [%d]\n", asctime(gmtime(&timep)), portname, ret);
				fflush(pfd);
				printf("read() fail ; ret = %d\n", ret);
				return 0;
			}
			else if (ret > 0)
			{
				if((strcmp(pbuf, buff)) != 0)
				{
					fprintf(pfd, "%s%s\nread fail. return value: [%d]. read msg: [%s]\n", asctime(gmtime(&timep)), portname, ret, buff);
					fflush(pfd);
				}
				break;
			}
			else
			{
				fprintf(pfd, "%s%s\nread fail. return value: [%d]. read msg: [%s]\n", asctime(gmtime(&timep)), portname, ret, buff);
				fflush(pfd);
				sleep(1);
				if(value > 2)
					break;
				value++;
			}
		}
	}
	fclose(pfd);
	close(fd);
	return 0;
}
