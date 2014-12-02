#!/bin/sh
TLV_SDK_DIR=$PWD
PLX_SDK_DIR=$TLV_SDK_DIR/PlxSdk
export PLX_SDK_DIR

cd $PLX_SDK_DIR/Linux/PlxApi
make clean;make
rm -f $TLV_SDK_DIR/lld/PlxApi.a
cp -f ./Library/PlxApi.a $TLV_SDK_DIR/lld
cd $TLV_SDK_DIR

cd $PLX_SDK_DIR/Linux/Driver
./builddriver Svc clean
./builddriver Svc
$PLX_SDK_DIR/Bin/Plx_unload Svc
$PLX_SDK_DIR/Bin/Plx_load Svc
cd $TLV_SDK_DIR
