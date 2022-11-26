# SnapAirUnit

It's an education version of Snap Air Unit.

# Hot to get source code

> $ git clone git@github.com:lida2003/SnapAirUnit.git

# How to build node.js application

## Step 1: install npm environment

> $ cd SnapAirUnit/front/web-demo
>
> $ npm install

## Step 2: build web root

> $ npm run build

# How to build c-code application

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