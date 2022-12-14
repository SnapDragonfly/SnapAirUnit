#!/bin/bash

# for partitions dir
mkdir -p ./build/storage
mkdir -p ./build/nvs
mkdir -p ./build/phy_init

# for git repo dirty control
rm -f version.h
if [[ $(git diff --stat) != '' ]]; then
	echo -e "#define APP_DIRTYFLAG  \"dirty\"" > version.h
	echo "dirty"
else
	echo -e "#define APP_DIRTYFLAG  \"clean\"" > version.h
	echo "clean"
fi


