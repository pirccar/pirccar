#!/bin/bash
#Autostart script
#exec 3>&1 1>>/home/pi/bin/network.log 2>&1
echo "Autostarting network script"
(sleep 5;sudo /home/pi/bin/network_monitor.sh)
