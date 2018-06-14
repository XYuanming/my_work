#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/watchdog.h>
#include <sys/ioctl.h>
int main(int argc, char **argv){
    int fd;
    int num;
    fd = open("/dev/watchdog",O_RDONLY);
    if(fd < 0){
        perror("/dev/watchdog");
        return -1;
    }
    for(num=0;num<5;num++){
        printf("feed dog!\n");
        ioctl(fd, WDIOC_KEEPALIVE);
        sleep(5);
    }
    close(fd);
    return 0;
}
