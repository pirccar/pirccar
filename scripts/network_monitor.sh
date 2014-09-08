#!/bin/bash

while true ; do
	if /sbin/ifconfig wlan0 | grep -q "inet addr:"; then
		sleep 60
	else
		echo "Network connection down! Attempting reconnection."
		/sbin/ifdown --force wlan0
		/sbin/ifup --force wlan0
		sleep 10
	fi
done
