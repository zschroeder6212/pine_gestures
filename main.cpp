#include <iostream>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <cstring>
#include <sys/time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int ftoi(char* path)
{
	FILE *f = fopen(path, "r");
	
	char buffer[10];
	
	if(f == NULL)
	{
		return NULL;
	}
	
	if(fgets(buffer, 10, f) == NULL)
	{
		return NULL;
	}
	fclose(f);
	
	return std::stol(buffer);
}

int main()
{
    bool flash_on = false;
    int last_y_accel = ftoi("/sys/bus/i2c/drivers/inv-mpu6050-i2c/2-0068/iio\:device2/in_accel_y_raw");

    struct timeval start, now;
	long mtime, seconds, useconds = 0; 

    gettimeofday(&start, NULL);

    int shakes = 0;
	
    while(true)
    {
        int y_accel = ftoi("/sys/bus/i2c/drivers/inv-mpu6050-i2c/2-0068/iio\:device2/in_accel_y_raw");
        
        if(abs(y_accel-last_y_accel) > 32000)
        {
            gettimeofday(&start, NULL);
            shakes++;
            usleep(100000);
        }

        gettimeofday(&now, NULL);

        seconds  = now.tv_sec  - start.tv_sec;
		useconds = now.tv_usec - start.tv_usec;

		mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;

        if(shakes >= 3)
        {
            shakes = 0;
            flash_on = !flash_on;

            char brightness[2];

            sprintf(brightness, "%d", flash_on);

            int fd = open("/sys/devices/platform/led-controller/leds/white\:flash/brightness", O_RDWR);

            if(fd != NULL)
            {
                write(fd, brightness, strlen(brightness));
            }

            close(fd);
        }

        if(mtime > 500)
        {
            shakes = 0;
        }
        //printf("%d\n", abs(y_accel-last_y_accel) );
        last_y_accel = y_accel;
        usleep(25000);
    }
}