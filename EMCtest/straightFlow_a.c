#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <syslog.h>
#include <unistd.h>
#include <pthread.h>

#define ADC_DIR "/sys/bus/iio/devices/"
#define K 0.0057488
#define F 20
#define S 12
#define T 4
#define DEBUG

void *func(void *arg)
{
	FILE *pfd = (FILE *)arg;
	time_t timep;
	while(1)
	{
		sleep(30);
		time(&timep);
		fprintf(pfd, "%sstraightFlow is working.\n", asctime(gmtime(&timep)));
		fflush(pfd);
	}
	pthread_exit(NULL);
}

int main(int argc, char **argv)
{
	FILE *fd3, *fd5, *pfd;
	float value3, value5;
	float val3[3] = {0}, val5[3] = {0};
	float I3, I5;
	time_t timep;
	pthread_t pthid;
#ifdef DEBUG
	pfd = fopen("/root/log/straightFlow.log", "a");
	setbuf(pfd, NULL);
	if(pfd == NULL)
	{
		perror("fopen straightFlow.log fail");
		return 0;
	}

	time(&timep);
	fprintf(pfd, "%sstraightFlow start work.\n", asctime(gmtime(&timep)));
	fflush(pfd);

	pthread_create(&pthid, NULL, func, (void *)pfd);
#endif
	while(1)
	{
		time(&timep);
		fd5 = fopen(ADC_DIR"iio:device0/in_voltage5_raw", "r");
		setbuf(fd5, NULL);
		if(fd5 == NULL)
		{
			printf("fopen in_voltafe5_raw fail.\n");
			return 0;
		}
		fscanf(fd5, "%f", &value5);
		fclose(fd5);

		I5 = value5 * K;	
		val5[0] = fabs(I5-F);
		val5[1] = fabs(I5-S);
		val5[2] = fabs(I5-T);
		if ((val5[0] > 0.15) && (val5[1] > 0.15) && (val5[2] > 0.15))
		{
#ifdef DEBUG
			time(&timep);
			fprintf(pfd, "%sinvoltage5_raw: [%f]; I5: [%f]\n", asctime(gmtime(&timep)),value5, I5);
			fflush(pfd);
#endif
			printf("%s involtage5_raw: [%f]; I5: [%f]\n", asctime(gmtime(&timep)), value5, I5);
		}
		else
			printf("%sinvoltage5_raw: [%f]; I5: [%f]\n", asctime(gmtime(&timep)), value5, I5);
		sleep(3);
	}
	fclose(pfd);
	closelog();
	return 0;
}
