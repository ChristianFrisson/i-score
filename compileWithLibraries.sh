#!/bin/bash

## USAGE : 
## ./compile.sh LINUX
##         OR
## ./compile.sh LINUX64
##         OR
## ./compile.sh MACOS
##         OR
## ./compile.sh MACOS64

CURRENT_PATH=`pwd`

OS=$1

if [ "$OS" = "LINUX" ] || [ "$OS" = "LINUX64" ]
then
{
DEVICE_MANAGER_PATH='/home/luc/Documents/workspace/Device-Manager'
LIB_ISCORE_PATH='/home/luc/Documents/workspace/libIscore'
ACOUSMOSCRIBE_PATH='/home/luc/Documents/workspace/acousmoscribe'
}
elif [ "$OS" = "MACOS" ] || [ "$OS" = "MACOS64" ]
then
{
DEVICE_MANAGER_PATH='/Users/luc/Documents/workspace/Device-Manager'
LIB_ISCORE_PATH='/Users/luc/Documents/workspace/libIscore'
ACOUSMOSCRIBE_PATH='/Users/luc/Documents/workspace/acousmoscribe'
}
fi

QMAKE_LINUX='qmake -spec linux-g++'
QMAKE_LINUX_64='qmake -spec linux-g++-64'
QMAKE_MACOS='qmake -spec macx-g++'
MAKE='make'
MAKE_LINUX='make linux'
MAKE_LINUX64='make linux64'
MAKE_MAC='make mac'
MAKE_MAC64='make mac64'
MAKE_CLEAN='make clean'

echo '-=[START]=-'

echo 'Device-Manager changes tranfert'

cp -r $ACOUSMOSCRIBE_PATH/devManChanges/* $DEVICE_MANAGER_PATH

## Device-Manager Compilation

echo '-=[Device Manager : Library]=- Compiling ...'
cd $DEVICE_MANAGER_PATH
if [ "$OS" = "LINUX" -o "$OS" = "LINUX64" ]
then
{
echo '-=[Device Manager : LINUX]=-'
echo '-=[Device Manager : Library]=- Cleaning ...'
$MAKE_CLEAN
echo '-=[Device Manager : Library]=- Compiling  ...'
$MAKE_LINUX
echo '-=[Device Manager : Library]=- Installing ...'
sudo cp libDeviceManager.a /usr/local/lib/
sudo mkdir -p /usr/local/include/DeviceManager/
sudo cp DeviceManager.h /usr/local/include/DeviceManager/
echo '-=[Device Manager : Library]=- Done.'
cd Plugins/Linux_MacOS/
echo '-=[Device Manager : Plugins]=- Cleaning ...'
$MAKE_CLEAN
echo '-=[Device Manager : Plugins]=- Compiling  ...'
if [ "$OS" = "LINUX" ]
then
    {
	chmod u+x compileForLinux.sh
	./compileForLinux.sh
	$MAKE_LINUX
    }
elif [ "$OS" = "LINUX64" ]
then
{
    chmod u+x compileForLinux64.sh
    ./compileForLinux64.sh
    $MAKE_LINUX64
}
fi
echo '-=[Device Manager : Plugins]=- Installing ...'
sudo mkdir -p /usr/local/lib/IScore/
sudo cp Plugins/*.so /usr/local/lib/IScore/
}
elif [ "$OS" = "MACOS" ] || [ "$OS" = "MACOS64" ]
then
{
echo '-=[Device Manager : MACOS]=-'
echo '-=[Device Manager : Library]=- Cleaning ...'
$MAKE_CLEAN
echo '-=[Device Manager : Library]=- Compiling  ...'
pwd
$MAKE_MAC
echo '-=[Device Manager : Library]=- Installing ...'
sudo cp libDeviceManager.a /usr/local/lib/
sudo mkdir -p /usr/local/include/DeviceManager/
sudo cp DeviceManager.h /usr/local/include/DeviceManager/
echo '-=[Device Manager : Library]=- Done.'
cd Plugins/Linux_MacOS/
echo '-=[Device Manager : Plugins]=- Cleaning ...'
$MAKE_CLEAN
echo '-=[Device Manager : Plugins]=- Compiling  ...'
if [ "$OS" = "MACOS" ]
then
    {
	chmod u+x compileForMac.sh
	./compileForMac.sh
	$MAKE_MAC
    }
elif [ "$OS" = "MACOS64" ]
then
    {
	chmod u+x compileForMac64.sh
	./compileForMac64.sh
	$MAKE_MAC64
    }
fi
echo '-=[Device Manager : Plugins]=- Installing ...'
sudo mkdir -p /usr/local/lib/IScore/
sudo cp Plugins/*.dylib /usr/local/lib/IScore/
}
fi

## LibIscore Compilation

cd $LIB_ISCORE_PATH
echo '-=[LibIScore]=- Cleaning ...'
$MAKE_CLEAN
if [ "$OS" = "LINUX" ]
then
{
echo '-=[LibIScore]=- Generating makefile ...'
$QMAKE_LINUX iscore.pro
}
elif [ "$OS" = "LINUX64" ]
then
{
echo '-=[LibIScore]=- Generating makefile ...'
$QMAKE_LINUX_64 iscore.pro
}
elif [ "$OS" = "MACOS" ] || [ "$OS" = "MACOS64" ]
then
{
echo '-=[LibIScore]=- Generating makefile ...'
$QMAKE_MACOS iscore.pro
}
else
{
echo '-=[LibIScore]=- ERROR : Unhandled OS'
}
fi
echo '-=[LibIScore]=- Compiling  ...'
$MAKE
echo '-=[LibIScore]=- Installing ...'
chmod u+x install.sh
./install.sh

## Acousmoscribe Compilation
cd $ACOUSMOSCRIBE_PATH
echo '-=[Acousmoscribe]=- Cleaning ...'
$MAKE_CLEAN
if [ "$OS" = "LINUX" ]
then
{
echo '-=[Acousmocribe]=- Generating makefile ...'
$QMAKE_LINUX acousmoscribe.pro
}
elif [ "$OS" = "LINUX64" ]
then
{
echo '-=[Acousmocribe]=- Generating makefile ...'
$QMAKE_LINUX_64 acousmoscribe.pro
}
elif [ "$OS" = "MACOS" ] || [ "$OS" = "MACOS64" ]
then
{
echo '-=[Acousmoscribe]=- Generating makefile ...'
$QMAKE_MACOS acousmoscribe.pro
}
else
{
echo '-=[Acousmoscribe]=- ERROR : Unhandled OS'
}
fi
echo '-=[Acousmoscribe]=- Compiling  ...'
$MAKE

cd $CURRENT_PATH
echo '-=[END]=-'
