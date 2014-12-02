#!/bin/sh

#PLX
./p

#libusb
cd libusb-0.1.12/
make clean
./configure

cd ..
#pci driver
./build.sh drv

#lld
./build.sh lld

#hld
./build.sh hld

#Application
./build.sh app
