#!/bin/sh

if [ $# -ne 1 ]; then
  echo "Invalid parameter count"
  exit 1
fi
echo $1

./compile.sh Server
./buildServer/ServerApp/buildServer/FileServiceServer $1
