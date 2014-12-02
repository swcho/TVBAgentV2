#!/bin/sh

#device driver
if [ "$1" = "drv" ]; then
make -f Makefile clean
make -f Makefile
fi


#lld
if [ "$1" = "lld" ]; then
cd lld
#source lldenv.sh
make  clean
make  
make install
cd ..
fi

#hld


if [ "$1" = "hld" ]; then
cd hld
make  clean
make 
make install
cd ..
fi

#app
if [ "$1" = "app" ]; then
make -f makefile.agent clean
make -f makefile.agent 
fi

#installation 
if [ "$1" = "install" ]; then
cp -a tlib/* /srv/nfs_srv/tvb597lanv2/rootfs/usr/lib/
cp ./app/tvbagent /srv/nfs_srv/tvb597lanv2/rootfs/bin/
fi

