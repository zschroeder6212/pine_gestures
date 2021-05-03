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

#define SHAKE_THRESHOLD 32000
#define TWIST_THRESHOLD 15000

char twist_command[256];
char shake_command[256];

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

//get the value of a given argument
int get_arg(int argc, char **argv, char *arg_name, char *arg_value)
{
    for(int i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], arg_name) == 0 && i < argc-1)
        {
            sprintf(arg_value, argv[i+1]);
            return 0;
        }else if(strcmp(argv[i], arg_name) == 0 && i == argc-1)
        {
            return 2;
        }
    }
    return 1;
}

void usage(char *name)
{
    printf("Usage: %s [--shake_cmd <cmd>] [--twist_cmd <cmd>]\n");
    printf("\t--shake_cmd\tpath to cmd or 'none' to disable. default: /usr/bin/toggleflash\n");
    printf("\t--twist_cmd\tpath to cmd or 'none' to disable. default: /usr/bin/megapixels\n");
    exit(1);
}

int main(int argc, char **argv)
{
    char IMU_path[256];

    char gyro_x_path[256];
    char accel_y_path[256];

    int ret = get_arg(argc, argv, "--shake_cmd", shake_command);

    if(ret == 1){
        sprintf(shake_command, "none");
    }else if(ret == 2)
    {
        usage(argv[0]);
    }

    ret = get_arg(argc, argv, "--twist_cmd", twist_command);
    if(ret == 1){
        sprintf(twist_command, "none");
    }else if(ret == 2)
    {
        usage(argv[0]);
    }


    if(strcmp(twist_command, "none") != 0)
    {
        sprintf(twist_command, "%s &", twist_command);
    }
    
    if(strcmp(shake_command, "none") != 0)
    {
        sprintf(shake_command, "%s &", shake_command);
    }

    //scan for IMU
    if(find("/sys/bus/i2c/drivers/inv-mpu6050-i2c", "0068", IMU_path))
    {
        if(!find(IMU_path, "iio:device", IMU_path))
        {
            printf("could not find IMU path\n");
            return 1;
        }
    }else{
        printf("could not find IMU path\n");
        return 1;
    }

    sprintf(gyro_x_path, "%s/in_anglvel_x_raw", IMU_path);
    sprintf(accel_y_path, "%s/in_accel_y_raw", IMU_path);

    //the last accel values, initialized to current accel value
    int last_accel_y = ftoi(accel_y_path); 

    struct timeval start, now;
	long mtime, seconds, useconds = 0; 

    gettimeofday(&start, NULL);


    //number of shakes detected. The phone must be shaken at least twice to enable the flashlight.
    int shakes = 0;
    int twists = 0;
	
    while(true)
    {
        //update accel value
        int gyro_x = ftoi(gyro_x_path);
        int accel_y = ftoi(accel_y_path);

        gettimeofday(&now, NULL);

        seconds  = now.tv_sec  - start.tv_sec;
		useconds = now.tv_usec - start.tv_usec;

		mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;

        //check for shake
        if(abs(accel_y-last_accel_y) > SHAKE_THRESHOLD)
        {
            gettimeofday(&start, NULL);
            shakes++;
            usleep(100000);
        }

        if(abs(gyro_x) > TWIST_THRESHOLD)
        {
            gettimeofday(&start, NULL);
            twists++;
            usleep(100000);
        }

        if(shakes >= 2)
        {
            shakes = 0;
            printf(shake_command);
            if(strcmp(shake_command, "none") != 0)
                system(shake_command);

            vibrate(200, 4000);

            //wait 1 second before continuing to avoid multiple detections
            usleep(1000000);
        }

        if(twists >= 2)
        {
            twists = 0;
            
            printf(twist_command);
            if(strcmp(twist_command, "none") != 0)
                system(twist_command);

            vibrate(200, 4000);

            //wait 1 second before continuing to avoid multiple detections
            usleep(1000000);
        }

        //took too long
        if(mtime > 1000)
        {
            shakes = 0;
            twists = 0;
        }

        //printf("%d\n", abs(accel_y-last_accel_y) );
        last_accel_y = accel_y;
        usleep(25000);
    }
}