#!/bin/sh
FILE=/etc/iotsample-raspberrypi/device.cfg
if [ -e "$FILE" ]
then
	echo Running in registered mode
	grep "^id=" $FILE
else 
	devId=`/sbin/ifconfig | grep 'eth0' | tr -s ' ' | cut -d ' ' -f5 |  tr -d ':'`
	echo id=$devId
	echo For Real-time visualization of the data, visit http://quickstart.internetofthings.ibmcloud.com/?deviceId=$devId
        return 1
fi



