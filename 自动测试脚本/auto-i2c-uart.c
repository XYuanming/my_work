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
#include <sys/wait.h>
#include <sys/sem.h>

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


/*union semun
{
	int val;
	struct semid_ds *pbuf;
	unsigned short *parry;
};

*/
int
main()
{
	time_t timep;
	time(&timep);
	openlog("testserial", LOG_CONS|LOG_NDELAY, LOG_USER);
	pid_t fpid;

	fpid=fork();
	if(fpid < 0)
	{
		syslog(LOG_DEBUG, "error in fork!");
		printf("error in fork!\n");
		return 0;
	}
	else if (fpid == 0)
	{
		usleep(100000);
		int fd0, ret;
		int num = 0;
		char *portname0 = "/dev/ttyACM3";
		char buff_Read[100];

		fd0 = open (portname0, O_RDWR | O_NOCTTY | O_SYNC);
		if(fd0 <= 0)
		{
			printf("open %s fail\n", portname0);
			syslog(LOG_DEBUG, "DEBUG : open %s fail.", portname0);
			return 0;
		}

		set_interface_attribs (fd0, B9600, 0);  // set speed to 115,200 bps, 8n1 (no parity)
		set_blocking (fd0, 0);                // set no blocking
		
		bzero(&buff_Read, sizeof(buff_Read));
		while(1)
		{
			ret = read(fd0, buff_Read, sizeof(buff_Read));
			if(ret < 0)
			{
				perror("ret < 0");
				syslog(LOG_DEBUG, "DEBUG : %s, ret < 0.", portname0);
				return 0;
			}
			else if(ret == 0)
				break;
		}
		syslog(LOG_DEBUG, "%s, READ : [%s]", portname0, buff_Read);

		ret = write(fd0, buff_Read, strlen(buff_Read));
		if(ret < 0)
		{
			syslog(LOG_DEBUG, "%s, [write() fail.]", portname0);
			printf("write() fail\n");
			return 0;
		}
		syslog(LOG_DEBUG, "%s, WRITE : [%s]", portname0, buff_Read);
	}
	
	else
	{
		char *portname2 = "/dev/ttyXRM0";
		char buff_Write[100];
		char buff_Read[100];
		int fd2, ret;
		int num2 = 0;

		fd2 = open (portname2, O_RDWR | O_NOCTTY | O_SYNC);
		if(fd2 <= 0)
		{
			printf("open %s fail\n", portname2);
			syslog(LOG_DEBUG, "DEBUG : open %s fail.", portname2);
			return 0;
		}

		set_interface_attribs (fd2, B9600, 0);  // set speed to 115,200 bps, 8n1 (no parity)
		set_blocking (fd2, 0);                // set no blocking

		bzero(&buff_Write, sizeof(buff_Write));
		strcpy(buff_Write, asctime(gmtime(&timep)));
		ret = write (fd2, buff_Write, strlen(buff_Write));
		if(ret < 0)
		{
			syslog(LOG_DEBUG, "%s, [write() fail!]", portname2);
			printf("write fail!\n");
			return 0;
		}
		syslog(LOG_DEBUG, "%s, WRITE : [%s]", portname2, buff_Write);
		
		sleep(1);

		bzero(&buff_Read, sizeof(buff_Read));
		/*while(1)
		{
			ret = read(fd2, buff_Read, sizeof(buff_Read));
			if(ret < 0)
			{
				perror("ret < 0");
				syslog(LOG_DEBUG, "DEBUG : %s, ret < 0.", portname2);
				return 0;
			}
			else if(ret == 0)
				break;
		}*/
		while((read(fd2, buff_Read, strlen(buff_Write))) > 0);
		syslog(LOG_DEBUG, "%s, READ : [%s]", portname2, buff_Read);
		if((ret = strcmp(buff_Read, buff_Write)) == 0)
			printf("IIC扩UART OK!\n");
		else
			printf("IIC扩UART FAIL!\n");
		wait(NULL);
	}
}
