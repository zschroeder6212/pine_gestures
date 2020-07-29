/* 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 * Author: Zachary Schroeder
 * Created: July 28 2020
 */

#include <iostream>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <cstring>
#include <sys/time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define THRESHOLD 32000


//reads file and converts to int
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
	
	return std::stoi(buffer);
}

int main()
{
    bool flash_on = false; //flashlight state
    int last_y_accel = ftoi("/sys/bus/i2c/drivers/inv-mpu6050-i2c/2-0068/iio\:device2/in_accel_y_raw"); //the last accel value, initialized to current accel value

    struct timeval start, now;
	long mtime, seconds, useconds = 0; 

    gettimeofday(&start, NULL);


    //number of shakes detected. The phone must be shaken at least twice to enable the flashlight.
    int shakes = 0;
	
    while(true)
    {
        //update accel value
        int y_accel = ftoi("/sys/bus/i2c/drivers/inv-mpu6050-i2c/2-0068/iio\:device2/in_accel_y_raw");

        gettimeofday(&now, NULL);

        seconds  = now.tv_sec  - start.tv_sec;
		useconds = now.tv_usec - start.tv_usec;

		mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;

        //check for shake
        if(abs(y_accel-last_y_accel) > THRESHOLD)
        {
            gettimeofday(&start, NULL);
            shakes++;
            usleep(100000);
        }

        if(shakes >= 2)
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

            //wait 1 second before continuing to avoid multiple detections
            usleep(1000000);
        }

        //took too long
        if(mtime > 1000)
        {
            shakes = 0;
        }

        //printf("%d\n", abs(y_accel-last_y_accel) );
        last_y_accel = y_accel;
        usleep(25000);
    }
}