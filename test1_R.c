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


int
main()
{
	time_t timep;
	time(&timep);
	char *portname = "/dev/ttyACM0";
	char buff_Write[100];
	char buff_Read[100];
	openlog("testserial", LOG_CONS|LOG_NDELAY, LOG_USER);
	int fd, nret = 0, ret = 0;
	int timedelay;

	fd = open (portname, O_RDWR | O_NOCTTY | O_SYNC);
	if(!fd)
	{
		syslog(LOG_DEBUG, "DEBUG : open /dev/ttyACM0 fail.");
		return 0;
	}
	set_interface_attribs (fd, B9600, 0);  // set speed to 115,200 bps, 8n1 (no parity)
	set_blocking (fd, 0);                // set no blocking

	while(1)
	{
		bzero(&buff_Read, sizeof(buff_Read));
		while(nret <= 0)
		{
			nret = read (fd, buff_Read, sizeof(buff_Read));  // read up to 100 characters if ready to read
			if(nret < 0)
			{
				syslog(LOG_DEBUG, "%s, [read() fail!]", portname);
				printf("[read fail!]\n");
				return 0;
			}
		}
		syslog(LOG_DEBUG, "%s, READ: [%s]", portname, buff_Read);

		bzero(&buff_Write, sizeof(buff_Write));
		strcpy(buff_Write, "timep");
		ret = write (fd, buff_Write, strlen(buff_Write));
		if(!ret)
		{
			syslog(LOG_DEBUG, "%s, [write() fail!]", portname);
			printf("write fail!\n");
			return 0;
		}
		syslog(LOG_DEBUG, "%s, WRITE: [%s]", portname, buff_Write);		

	}
}
