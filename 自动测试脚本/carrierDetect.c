#include <stdio.h>  
#include <string.h>  
#include <sys/types.h>  
#include <errno.h>  
#include <sys/stat.h>  
#include <fcntl.h>  
#include <unistd.h>  
#include <termios.h>  
#include <stdlib.h>  
#include <time.h>

#define TRUE 1  
#define FALSE 0  

int analysis(char *buff);  
int OpenDev(char *Dev);  
void set_speed(int fd, int speed);  
int set_Parity(int fd,int databits,int stopbits,int parity);  

int speed_arr[] = {115200,38400,19200,9600,4800,2400,1200,300};  
int name_arr[] =  {115200,38400,19200,9600,4800,2400,1200,300};  
int OpenDev(char *Dev)  
{  
    int fd = open(Dev,O_RDWR | O_NOCTTY | O_NONBLOCK);  
    if(-1 == fd)  
    {  
        perror("Can't Open Serial Port");  
        return -1;  
    }   
    else   
    {  
        printf("Open com success!!!!!!!!!!!\n");  
        return fd;  
    }  
}   
void set_speed(int fd, int speed)  
{   
    int i;   
    int status;   
    struct termios Opt;  
    tcgetattr(fd, &Opt);   
    for ( i= 0; i < sizeof(speed_arr) / sizeof(int); i++)  
    {   
        if (speed == name_arr[i])   
        {   
            tcflush(fd, TCIOFLUSH);   
            cfsetispeed(&Opt, speed_arr[i]);  
            cfsetospeed(&Opt, speed_arr[i]);  
            status = tcsetattr(fd, TCSANOW, &Opt);   
            if (status != 0) perror("tcsetattr fd1");  
            return;  
        }   
        tcflush(fd,TCIOFLUSH);  
    }  
}  
int set_Parity(int fd,int databits,int stopbits,int parity)   
{   
    struct termios options;   
    if ( tcgetattr( fd,&options) != 0)   
    {  
        perror("SetupSerial 1");  
        return(FALSE);  
    }   
    bzero(&options,sizeof(options));   
    options.c_cflag |= CLOCAL | CREAD;  
    options.c_cflag &= ~CSIZE;   
    switch (databits)   
    {   
        case 7:   
            options.c_cflag |= CS7;  
            break;  
        case 8:  
            options.c_cflag |= CS8;  
            break;   
        default: fprintf(stderr,"Unsupported data size\n");  
                 return (FALSE);   
    }   
    switch (parity)   
    {  
        case 'n':   
        case 'N':  
            options.c_cflag &= ~PARENB;  
            options.c_iflag &= ~INPCK;   
            break;   
        case 'o':  
        case 'O':   
            options.c_cflag |= (PARODD | PARENB);  
            options.c_iflag |= (INPCK | ISTRIP);   
            break;   
        case 'e':   
        case 'E':   
            options.c_cflag |= PARENB;  
            options.c_cflag &= ~PARODD;   
            options.c_iflag |= (INPCK | ISTRIP);   
            break;   
        case 'S':   
        case 's':   
            options.c_cflag &= ~PARENB;   
            options.c_cflag &= ~CSTOPB;  
            break;  
        default: fprintf(stderr,"Unsupported parity\n");   
                 return (FALSE);   
    }   
    switch (stopbits)  
    {   
        case 1:  
            options.c_cflag &= ~CSTOPB;   
            break;   
        case 2:   
            options.c_cflag |= CSTOPB;  
            break;   
        default: fprintf(stderr,"Unsupported stop bits\n");   
                 return (FALSE);   
    }   
    if (parity != 'n')   
        options.c_iflag |= INPCK;   
    options.c_cc[VTIME] = 0;  
    options.c_cc[VMIN] = 0;  
    tcflush(fd,TCIFLUSH);   
    if (tcsetattr(fd,TCSANOW,&options) != 0)  
    {    
        perror("SetupSerial 3");   
        return (FALSE);  
    }   
    return (TRUE);  
}  

int analysis (char *buff)
{  
    int i;  
    char *p;
    p=buff;
    printf("read : [%s]\n", buff);  
    return 0;  
}  

int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)   
{   
    struct termios newtio,oldtio;   
    /*保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息*/   
    if  ( tcgetattr( fd,&oldtio)  !=  0) {    
        perror("SetupSerial 1");  
        printf("tcgetattr( fd,&oldtio) -> %d\n",tcgetattr( fd,&oldtio));   
        return -1;   
    }   
    bzero( &newtio, sizeof( newtio ) );   
    /*步骤一，设置字符大小*/   
    newtio.c_cflag  |=  CLOCAL | CREAD;    
    newtio.c_cflag &= ~CSIZE;    
    /*设置停止位*/   
    switch( nBits )   
    {   
        case 7:   
            newtio.c_cflag |= CS7;   
            break;   
        case 8:   
            newtio.c_cflag |= CS8;   
            break;   
    }   
    /*设置奇偶校验位*/   
    switch( nEvent )   
    {   
        case 'o':  
        case 'O': //奇数   
            newtio.c_cflag |= PARENB;   
            newtio.c_cflag |= PARODD;   
            newtio.c_iflag |= (INPCK | ISTRIP);   
            break;   
        case 'e':  
        case 'E': //偶数   
            newtio.c_iflag |= (INPCK | ISTRIP);   
            newtio.c_cflag |= PARENB;   
            newtio.c_cflag &= ~PARODD;   
            break;  
        case 'n':  
        case 'N':  //无奇偶校验位   
            newtio.c_cflag &= ~PARENB;   
            break;  
        default:  
            break;  
    }   
    /*设置波特率*/   
    switch( nSpeed )   
    {   
        case 2400:   
            cfsetispeed(&newtio, B2400);   
            cfsetospeed(&newtio, B2400);   
            break;   
        case 4800:   
            cfsetispeed(&newtio, B4800);   
            cfsetospeed(&newtio, B4800);   
            break;   
        case 9600:   
            cfsetispeed(&newtio, B9600);   
            cfsetospeed(&newtio, B9600);   
            break;   
        case 115200:   
            cfsetispeed(&newtio, B115200);   
            cfsetospeed(&newtio, B115200);   
            break;   
        case 460800:   
            cfsetispeed(&newtio, B460800);   
            cfsetospeed(&newtio, B460800);   
            break;   
        default:   
            cfsetispeed(&newtio, B9600);   
            cfsetospeed(&newtio, B9600);   
            break;   
    }   
    /*设置停止位*/   
    if( nStop == 1 )   
        newtio.c_cflag &=  ~CSTOPB;   
    else if ( nStop == 2 )   
        newtio.c_cflag |=  CSTOPB;   
    /*设置等待时间和最小接收字符*/   
    newtio.c_cc[VTIME]  = 0;   
    newtio.c_cc[VMIN] = 0;   
    /*处理未接收字符*/   
    tcflush(fd,TCIFLUSH);   
    /*激活新配置*/   
    if((tcsetattr(fd,TCSANOW,&newtio))!=0)   
    {   
        perror("com set error");   
        return -1;   
    }   
    //printf("set done!\n");   
    return 0;   
}

void main(void)  
{  
    time_t timep;
    time(&timep);
    int fd;  
    int nread;  
    int wrote = 0;
    char buff[255];
    char buff_write[100];

    //char *dev_name = "/dev/chn/1";//根据实际情况选择串口  
    char *dev_name = "/dev/ttyO2";//根据实际情况选择串口  
    while(1)   
    {    
        fd = OpenDev(dev_name); //打开串口   

        if(fd>0)   
            set_opt(fd, 9600, 8, 'n', 1); //设置波特率   
            //set_speed(fd,115200); //设置波特率   
        else   
        {   
            printf("Can't Open Serial Port!\n");   
            sleep(1);  
            continue;   
        }   
        break;  
    }  
/*
    if(set_Parity(fd,8,1,'N')==FALSE) //设置校验位   
    {  
        printf("Set Parity Error\n");   
        exit(1);  
    }  
*/
    while(1)   
    {  	
	time(&timep);
	bzero(&buff_write, sizeof(buff_write)); 
	strcpy(buff_write, asctime(gmtime(&timep)));
        wrote =  write(fd, buff_write, strlen(buff_write));
        printf("write : [%d byte: %s]\n", wrote, buff_write);
        sleep(3);
	bzero(&buff, sizeof(buff));
        nread = read(fd,buff,sizeof(buff));
        printf("read : [%d byte]\n",nread);
        if((nread>0))  
        {       
            analysis(buff);
	    break;
        }  
    }  
}
