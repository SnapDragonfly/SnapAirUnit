#!/bin/bash

# for git repo dirty control
rm -rf version.h
rm -rf build/esp-idf/main/CMakeFiles/__idf_main.dir/factory_setting.c.obj

if [[ $(git diff --stat) != '' ]]; then
	echo -e "#define APP_DIRTYFLAG  \"dirty\"" > version.h
	echo "dirty"
else
	echo -e "#define APP_DIRTYFLAG  \"clean\"" > version.h
	echo "clean"
fi
