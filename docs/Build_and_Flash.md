# **How to Buil and Flash**

# 1. Hot to get source code and setup environment

**Step 1: get source**

> $ git clone git@github.com:lida2003/SnapAirUnit.git
>
> $ git clone git@github.com:espressif/esp-idf.git
>
> $ cd esp-idf
>
> $ git checkout release/v5.0
>

**Step 2: install compiling tools**

> $ ./install.sh

**Step 3: setup environment**

> $ . ./export.sh

[Detailed info about ESP-IDF, click HERE!!!](https://github.com/espressif/esp-idf/blob/master/README_CN.md)

*Note: setup environment before compile/flash/debug the code.*

# 2. How to build node.js application

**Step 1: install npm environment**

> $ cd SnapAirUnit/front/web-demo
>
> $ npm install

**Step 2: build web root**

> $ npm run build

# 3. How to build Snap Air Unit application code

**Step 1: set target board**

> $ cd SnapAirUnit
>
> $ idf.py set-target esp32

**Support following targets:**
- esp32 **-- esp-idf v5.0 ok --**
- esp32s2  **-- unclear --**
- esp32c3  **-- unclear --**
- esp32s3  **-- unclear --**
- esp32c2  **-- unclear --**

**Step 2: configure the project**

*Note: if you are using default, just ignore this step.*

> $ idf.py menuconfig

**Step 3: build the project**

> $ idf.py build


# 4. How to flash & debug the Snap Air Unit firmware

**4.1 flash the entire binary to target board**

> $ idf.py -p /dev/ttyUSB0 flash

**4.2 flash application binary to target board**

> $ idf.py -p /dev/ttyUSB0 app-flash

**4.3 run & monitor**

> $ idf.py -p /dev/ttyUSB0 monitor

**4.4 export entire binary firmware for release, etc.**

*Note: this is used for 4MB external SPI flash only!!!*

> $ esptool.py -p /dev/ttyUSB0 -b 460800 read_flash 0 0x400000 SnapAirUnit_Factory.bin

# 5. How to flash the Snap Air Unit firmware on Windows

**Step 1: Please download [flash_download_tool_3.9.3_0.zip](https://www.espressif.com/sites/default/files/tools/flash_download_tool_3.9.3_0.zip)**

**Step 2: Select ESP32 target**

<image src="./images/windows_tool_burn_image_1.png" width="600">

**Step 3: Input 7 parameters and choose the release firmware version**

<image src="./images/windows_tool_burn_image_2.png" width="600">

**Step 4: Wait and see burning process is FINISHED**

<image src="./images/windows_tool_burn_image_3.png" width="600">
