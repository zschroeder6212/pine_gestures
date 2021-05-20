Pine_Gestures[![TravisCI](https://travis-ci.com/zschroeder6212/pine_gestures.svg?branch=master)](https://travis-ci.com/zschroeder6212/pine_gestures)
==============================

# Summary
A simple application for the pinephone to launch applications using gestures. Note that the camera will not work when running as a service.

# Compiling
```
on phone or computer:
	make
	
on phone:
	sudo make install
```

# Usage
To customize actions, simply edit /opt/pine_gestures/startup_gestures.sh using the command line arguments shown below.

```
pine_gestures [--shake_cmd <cmd>] [--twist_cmd <cmd>];
	--shake_cmd		path to cmd to be triggered by shake gesture or 'none' to disable. default: /usr/bin/toggleflash
	--twist_cmd		path to cmd to be triggered by twist gesture or 'none' to disable. default: /usr/bin/megapixels
```