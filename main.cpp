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

#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <errno.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <linux/input.h>

#include <dirent.h>

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

//find file containing string in dir
bool find(char* path, char* contains, char* newPath)
{
    DIR *dir;
	struct dirent *ent;

	if ((dir = opendir (path)) != NULL) {
		bool found = false;

		while ((ent = readdir (dir)) != NULL) {
			if(strstr(ent->d_name, contains) != NULL)
			{
                sprintf(newPath, "%s/%s", path, ent->d_name);
                closedir (dir);
				return true;
			}
		}
		closedir (dir);
        return false;
	}

}

int open_event_dev(const char* name_needle, int flags)
{
	char path[256];
	char name[256];
	int fd, ret;

	// find the right device and open it
	for (int i = 0; i < 10; i++) {
        snprintf(path, sizeof path, "/dev/input/event%d", i);
        fd = open(path, flags);
        if (fd < 0)
            continue;

        ret = ioctl(fd, EVIOCGNAME(256), name);
        if (ret < 0)
            continue;
        
        if (strstr(name, name_needle))
            return fd;
        
        close(fd);
	}
	
	errno = ENOENT;
	return -1;
}

void syscall_error(int is_err, const char* fmt, ...)
{
	va_list ap;

	if (!is_err)
		return;

	printf("ERROR: ");
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	printf(": %s\n", strerror(errno));
}


extern "C" {
	struct ff_effect e;
	struct input_event play;
}


//vibrate code adapted from https://git.sr.ht/~mil/sxmo-utils/tree/master/programs/sxmo_vibratepine.c
void vibrate(int durationMs, int strength)
{

    int fd, ret;
	struct pollfd pfds[1];
	int effects;

    fd = open_event_dev("vibrator", O_RDWR | O_CLOEXEC);
	syscall_error(fd < 0, "Can't open vibrator event device");
	ret = ioctl(fd, EVIOCGEFFECTS, &effects);
	syscall_error(ret < 0, "EVIOCGEFFECTS failed");

	e.type = FF_RUMBLE;
	e.id = -1;
	e.u.rumble.strong_magnitude = strength;

	ret = ioctl(fd, EVIOCSFF, &e);
	syscall_error(ret < 0, "EVIOCSFF failed");

	play.type = EV_FF;
	play.code = e.id;
	play.value = 3;

	ret = write(fd, &play, sizeof play);
	syscall_error(ret < 0, "write failed");

	usleep(durationMs * 1000);

	ret = ioctl(fd, EVIOCRMFF, e.id);
	syscall_error(ret < 0, "EVIOCRMFF failed");

	close(fd);
}


int main()
{

    char accel_path[256];

    //scan for accelerometer
    if(find("/sys/bus/i2c/drivers/inv-mpu6050-i2c", "0068", accel_path))
    {
        if(find(accel_path, "iio:device", accel_path))
        {
            sprintf(accel_path, "%s/in_accel_y_raw", accel_path);
        }else{
            printf("could not find accel path\n");
            return 1;
        }
    }else{
        printf("could not find accel path\n");
        return 1;
    }

    printf("%s\n", accel_path);

    bool flash_on = false; //flashlight state
    int last_y_accel = ftoi(accel_path); //the last accel value, initialized to current accel value
    struct timeval start, now;
	long mtime, seconds, useconds = 0; 

    gettimeofday(&start, NULL);


    //number of shakes detected. The phone must be shaken at least twice to enable the flashlight.
    int shakes = 0;
	
    while(true)
    {
        //update accel value
        int y_accel = ftoi(accel_path);

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

            int fd = open("/sys/devices/platform/led-controller/leds/white:flash/brightness", O_RDWR);

            if(fd != NULL)
            {
                write(fd, brightness, strlen(brightness));
            }

            close(fd);

            vibrate(200, 4000);

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