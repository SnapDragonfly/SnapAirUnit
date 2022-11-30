# How to Buil and Flash

# Hot to get source code and setup environment

## Step 1: get source
> $ git clone git@github.com:lida2003/SnapAirUnit.git
>
> $ git clone git@github.com:espressif/esp-idf.git
>
> $ cd esp-idf
>
> $ git checkout release/v5.0

## Step 2: setup environment

> . ./export.sh

[Detailed info, click HERE!!!](https://github.com/espressif/esp-idf/blob/master/README_CN.md)


# How to build node.js application

## Step 1: install npm environment

> $ cd SnapAirUnit/front/web-demo
>
> $ npm install

## Step 2: build web root

> $ npm run build

# How to build Snap Air Unit application code

## Step 1: set target board

> $ cd SnapAirUnit
>
> $ idf.py set-target ESP32

## Step 2: configure the project

> $ idf.py menuconfig

## Step 3: build the project

> $ idf.py build

## Step 4: flash binary to target board

> $ idf.py flash

## Step 5: run & monitor 

> $ idf.py monitor

Note: setup environment before compile the code.