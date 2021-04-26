# Summary
A simple application for the pinephone to launch applications using gestures. Note that the autostart only works on mobian.

# Compiling
```
on phone or computer:
	make
	
on phone:
	sudo make install
```

# Usage
```
pine_gestures [--shake_cmd <cmd>] [--twist_cmd <cmd>];
	--shake_cmd		path to cmd to be triggered by shake gesture or 'none' to disable. default: /usr/bin/toggleflash
    --twist_cmd		path to cmd to be triggered by twist gesture or 'none' to disable. default: /usr/bin/pinhole
```

# Autostart
Copy the service file to the systemd folder, ensure that you are in the pine_gestures directory
```
sudo cp gestures.service /etc/systemd/system/gestures.service
```
Then simple start the service with
```
sudo systemctl start gestures.service
```
And make sure it is active with
```
sudo systemctl status gestures.service
```
