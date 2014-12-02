#!/bin/sh

myparam=$1
case "$myparam" in
   102) module="tsp102";;
   140) module="tvb140";;
   150) module="tqk150";;
   160) module="tom160";;
   360) module="tvb360";;
   380) module="tvb380v4";;
   390) module="tvb380v4";;
   590) module="tvb380v4";;
   370) module="tvb370v6";;
   110) module="tse110v1";;
esac

if [ $# = 0 ]; then
   echo "    USAGE: ./l 102|140|150|160|360|380|390|590|370|110"
   exit 0
else
   echo Loading $module
fi

device=$module
group="wheel"
mode="666"

# invoke insmod with all arguments we got
#/sbin/insmod -f ./$module.ko $* || exit 1
/sbin/insmod -f ./$module.ko || exit 1

major=`cat /proc/devices | awk "\\$2==\"$module\" {print \\$1}"`

echo $device found on major number $major
rm -f /dev/${device}*
mknod /dev/${device}-1 c $major 1
mknod /dev/${device}-2 c $major 2
mknod /dev/${device}-3 c $major 3
mknod /dev/${device}-4 c $major 4
mknod /dev/${device}-5 c $major 5
mknod /dev/${device}-6 c $major 6
mknod /dev/${device}-7 c $major 7
mknod /dev/${device}-8 c $major 8
ln -s /dev/${device}-1 /dev/${device}
chgrp $group /dev/${device}*
chmod $mode  /dev/${device}*
