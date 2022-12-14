#!/bin/bash

# for git repo dirty control
rm -f version.h
if [[ $(git diff --stat) != '' ]]; then
	echo -e "#define APP_DIRTYFLAG  \"dirty\"" > version.h
	echo "dirty"
else
	echo -e "#define APP_DIRTYFLAG  \"clean\"" > version.h
	echo "clean"
fi


