#Summary
A simple application for the pinephone to launch applications using gestures

#Compiling
```
on phone or computer:
	make
	
on phone:
	sudo make install
```

#Usage
```
pine_gestures [--shake_cmd <cmd>] [--twist_cmd <cmd>];
	--shake_cmd		path to cmd to be triggered by shake gesture or 'none' to disable. default: /usr/bin/toggleflash
    --twist_cmd		path to cmd to be triggered by twist gesture or 'none' to disable. default: /usr/bin/pinhole
```