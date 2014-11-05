#!/bin/bash
# Verify that we're running in sudo (or su)
if [ "$(whoami)" != "root" ]
then
	echo "This script requires root privileges. Please run as: 'sudo $0'"
	exit 1
fi

