#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <syslog.h>
#include <pthread.h>

void *read_another_1(void *arg);
void *read_another_2(void *arg);
void *read_another_3(void *arg);
void *read_another_4(void *arg);
void *write_another_1(void *arg);
void *write_another_2(void *arg);
void *write_another_3(void *arg);
void *write_another_4(void *arg);

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

time_t timep;

char *portname1 = "/dev/ttyACM0";
char *portname2 = "/dev/ttyACM1";
char *portname3 = "/dev/ttyACM2";
char *portname4 = "/dev/ttyACM3";

int
main()
{
	pthread_mutex_init(&mutex, NULL);
	openlog("testserial", LOG_CONS|LOG_NDELAY, LOG_USER);

	time(&timep);
	int fd1, fd2, fd3, fd4;

#if 1 
	fd1 = open (portname1, O_RDWR | O_NOCTTY | O_SYNC);
	if(!fd1)
	{
		syslog(LOG_DEBUG, "Time : %s. DEBUG : open /dev/ttyACM0 fail. ", ctime(&timep));
		return 0;
	}
#endif
#if 1
	fd2 = open (portname2, O_RDWR | O_NOCTTY | O_SYNC);
	if(!fd2)
	{
		syslog(LOG_DEBUG, "Time : %s. DEBUG : open /dev/ttyACM1 fail. ", ctime(&timep));
		return 0;
	}
#endif

#if 0
	fd3 = open (portname3, O_RDWR | O_NOCTTY | O_SYNC);
	if(!fd3)
	{
		syslog(LOG_DEBUG, "Time : %s. DEBUG : open /dev/ttyACM2 fail. ", ctime(&timep));
		return 0;
	}
#endif
#if 0
	fd4 = open (portname4, O_RDWR | O_NOCTTY | O_SYNC);
	if(!fd4)
	{
		syslog(LOG_DEBUG, "Time : %s. DEBUG : open /dev/ttyACM3 fail. ", ctime(&timep));
		return 0;
	}
#endif
#if 0
	set_interface_attribs (fd1, B9600, 0);  // set speed to 115,200 bps, 8n1 (no parity)
	set_blocking (fd1, 0);                // set no blocking
#endif
#if 0
	set_interface_attribs (fd2, B19200, 0);  // set speed to 115,200 bps, 8n1 (no parity)
	set_blocking (fd2, 0);                // set no blocking
#endif
#if 0
	set_interface_attribs (fd3, B38400, 0);  // set speed to 115,200 bps, 8n1 (no parity)
	set_blocking (fd3, 0);                // set no blocking
#endif
#if 0
	set_interface_attribs (fd4, B57600, 0);  // set speed to 115,200 bps, 8n1 (no parity)
	set_blocking (fd4, 0);                // set no blocking
#endif
	char *buf_write1 = "helloworld\n";
	int ret;
	pthread_t pthid_1, pthid_2, pthid_3, pthid_4;

#if 1
	pthread_create(&pthid_1, NULL, read_another_1, (void *)(&fd1));
	pthread_create(&pthid_1, NULL, write_another_1, (void *)(&fd1));
#endif

#if 1
	pthread_create(&pthid_2, NULL, read_another_2, (void *)(&fd2));
	pthread_create(&pthid_2, NULL, write_another_2, (void *)(&fd2));
#endif

#if 0
	pthread_create(&pthid_3, NULL, read_another_3, (void *)(&fd3));
	pthread_create(&pthid_3, NULL, write_another_3, (void *)(&fd3));
#endif

#if 0
	pthread_create(&pthid_4, NULL, read_another_4, (void *)(&fd4));
	pthread_create(&pthid_4, NULL, write_another_4, (void *)(&fd4));
#endif

	while(1)
	{
	/*	
		ret = write (fd1, buf_write1, strlen(buf_write1));           
		syslog(LOG_DEBUG, "%s,  Time : %s. write : %s; write byte : %d", portname1, ctime(&timep), buf_write1, ret);		
		ret = read (fd, buf_read, sizeof(buf_read));  // read up to 100 characters if ready to read
		syslog(LOG_DEBUG, "Time : %s. read : %s; read byte : %d", ctime(&timep), buf_read, ret);

		usleep (1000000);             // sleep enough to transmit the 7 plus
                                     // receive 25:  approx 100 uS per char transmit
	*/
	}
	return 0;
}

void *read_another_1(void *arg)
{
	int fd;
	fd = (*(int *)arg);
	int ret;
	char buf_read[100];

	set_interface_attribs (fd, B9600, 0);  // set speed to 115,200 bps, 8n1 (no parity)
	set_blocking (fd, 0);                // set no blocking

	while(1)
	{
		bzero(&buf_read, sizeof(buf_read));
		ret = read (fd, buf_read, sizeof(buf_read));  // read up to 100 characters if ready to read
                syslog(LOG_DEBUG, "%s,  Time : %s. read : %s; read byte : %d", portname1, ctime(&timep), buf_read, ret);

		usleep(1000000);
	}
	return NULL;
}

void *write_another_1(void *arg)
{
	int fd;
	fd = (*(int *)arg);
	int ret;

	char buf_write[100] = "from ttyACM0, this is a test msg";

	set_interface_attribs (fd, B9600, 0);  // set speed to 115,200 bps, 8n1 (no parity)
	set_blocking (fd, 0);                // set no blocking

	while(1)
	{
		ret = read (fd, buf_write, strlen(buf_write));  // read up to 100 characters if ready to read
                syslog(LOG_DEBUG, "%s,  Time : %s. write : %s; write byte : %d", portname1, ctime(&timep), buf_write, ret);

		usleep(1000000);
	}
	return NULL;
}

void *read_another_2(void *arg)
{
	int fd;
	fd = (*(int *)arg);
	int ret;
	char buf_read[100];

	set_interface_attribs (fd, B9600, 0);  // set speed to 115,200 bps, 8n1 (no parity)
	set_blocking (fd, 0);                // set no blocking

	while(1)
	{
		bzero(&buf_read, sizeof(buf_read));
		ret = read (fd, buf_read, sizeof(buf_read));  // read up to 100 characters if ready to read
                syslog(LOG_DEBUG, "%s,  Time : %s. read : %s; read byte : %d", portname2, ctime(&timep), buf_read, ret);

		usleep(1000000);
	}
	pthread_exit(NULL);
}

void *write_another_2(void *arg)
{
	int fd;
	fd = (*(int *)arg);
	int ret;
	char buf_write[100] = "from ttyACM1, this is a test msg";

	set_interface_attribs (fd, B9600, 0);  // set speed to 115,200 bps, 8n1 (no parity)
	set_blocking (fd, 0);                // set no blocking

	while(1)
	{
		ret = write(fd, buf_write, strlen(buf_write));
                syslog(LOG_DEBUG, "%s,  Time : %s. write : %s; write byte : %d", portname2, ctime(&timep), buf_write, ret);

		usleep(1000000);
	}
	return NULL;
}

void *read_another_3(void *arg)
{
	int fd;
	fd = (*(int *)arg);
	int ret;
	char buf_read[100];

	set_interface_attribs (fd, B9600, 0);  // set speed to 115,200 bps, 8n1 (no parity)
	set_blocking (fd, 0);                // set no blocking

	while(1)
	{
		bzero(&buf_read, sizeof(buf_read));
		ret = read (fd, buf_read, sizeof(buf_read));  // read up to 100 characters if ready to read
                syslog(LOG_DEBUG, "%s,  Time : %s. read : %s; read byte : %d", portname3, ctime(&timep), buf_read, ret);

		usleep(1000000);
	}
	return NULL;
}

void *write_another_3(void *arg)
{
	int fd;
	fd = (*(int *)arg);
	int ret;
	char buf_write[100] = "from ttyACM2, this is a test msg";

	set_interface_attribs (fd, B9600, 0);  // set speed to 115,200 bps, 8n1 (no parity)
	set_blocking (fd, 0);                // set no blocking

	while(1)
	{
		ret = write(fd, buf_write, strlen(buf_write));
                syslog(LOG_DEBUG, "%s,  Time : %s. write : %s; write byte : %d", portname3, ctime(&timep), buf_write, ret);

		usleep(1000000);
	}
	return NULL;
}

void *read_another_4(void *arg)
{
	int fd;
	fd = (*(int *)arg);
	int ret;
	char buf_read[100];

	set_interface_attribs (fd, B57600, 0);  // set speed to 115,200 bps, 8n1 (no parity)
	set_blocking (fd, 0);                // set no blocking

	while(1)
	{
		bzero(&buf_read, sizeof(buf_read));
		ret = read (fd, buf_read, sizeof(buf_read));  // read up to 100 characters if ready to read
                syslog(LOG_DEBUG, "%s,  Time : %s. read : %s; read byte : %d", portname4, ctime(&timep), buf_read, ret);

		usleep(1000000);
	}
	return NULL;
}

void *write_another_4(void *arg)
{
	int fd;
	fd = (*(int *)arg);
	int ret;
	char buf_write[100] = "from ttyACM3, this is a test msg";

	set_interface_attribs (fd, B57600, 0);  // set speed to 115,200 bps, 8n1 (no parity)
	set_blocking (fd, 0);                // set no blocking

	while(1)
	{
		ret = write(fd, buf_write, strlen(buf_write));
                syslog(LOG_DEBUG, "%s,  Time : %s. write : %s; write byte : %d", portname4, ctime(&timep), buf_write, ret);

		usleep(1000000);
	}
	return NULL;
}
