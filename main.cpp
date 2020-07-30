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

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

#include <dirent.h>

#define THRESHOLD 32000

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

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

int main()
{

    char accel_path[256];

    //scan for accelerometer
    if(find("/sys/bus/i2c/drivers/inv-mpu6050-i2c", "0068", accel_path))
    {
        if(find(accel_path, "iio\:device", accel_path))
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