#!/bin/bash

C_COMPILER=
CXX_COMPILER=
CMAKE_PREFIX_PATH=

if [ $# -eq 0 ]; then
    echo "Invalid argument count, please use --help"
    exit 1
fi

if [ "$1" = "--help" ]; then
    echo  "Usage: ./compile.sh APP"
    echo  " "
    echo  "Applications:"
    echo  "	Client"
    echo  "	Server"	        
    exit 0
fi

# Check application name
case $1 in
    Client)
    OUTPUT_PATH=./buildClient
    export OUTPUT_PATH
	echo "Client application is selected"
    if [ -d $OUTPUT_PATH ] && [ -f $OUTPUT_PATH/FileServiceClient ]; then
        echo "Erasing content from $OUTPUT_PATH/FileServiceClient"
        rm $OUTPUT_PATH/FileServiceClient
    fi

	;;
    Server)
    OUTPUT_PATH=./buildServer
    export OUTPUT_PATH
	echo "Server application is selected"
    if [ -d $OUTPUT_PATH ] && [ -f $OUTPUT_PATH/FileServiceServer ]; then
        echo "Erasing content from $OUTPUT_PATH/FileServiceServer"
        rm $OUTPUT_PATH/FileServiceServer
    fi
	;;
    *)
	echo "Unknown application name:" $1
	exit 1
	;;
esac

BUILD_TYPE=debug
# Create build folder if not exists
if [ ! -d $OUTPUT_PATH  ]; then
    echo "$OUTPUT_PATH directory does not exists, creating now..."
    mkdir $OUTPUT_PATH
fi

# Compile app
cd $OUTPUT_PATH
echo cmake -G Ninja -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=c++ -DAPP=$1 ..
cmake -G Ninja -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=c++ -DAPP=$1 ..
ninja -j 4
cd ..

echo "Build finished..."
