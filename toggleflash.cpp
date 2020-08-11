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
 * Created: July 31 2020
 */


#include <iostream>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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
    bool state = ftoi("/sys/devices/platform/led-controller/leds/white:flash/brightness"); //current flashlight state
    char brightness[2];

    sprintf(brightness, "%d", !state);

    int fd = open("/sys/devices/platform/led-controller/leds/white:flash/brightness", O_RDWR);

    if(fd != NULL)
    {
        write(fd, brightness, strlen(brightness));
    }

    close(fd);
}