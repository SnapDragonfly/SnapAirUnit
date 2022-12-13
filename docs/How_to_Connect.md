
# **How to connect Snap Air Unit to Flight Controller**

# Snap Air Unit Form Factor Draft

<image src="./images/snap_air_unit_draft_form_factor.png" width="400">

# Snap Air Unit Hardware Interface Definitions

* [Mandatory] LED x 2, status & mode
* [Mandatory] Button x 1, software mode switch
* [Mandatory] MMCX for RF antenna (2.4G WiFi & BT)
* [Mandatory] USB-C for PC connection
* [Mandatory] 6pin connector for power and communication (7-24V, GND, OSD_Rx, OSD_Tx, MSP_Rx, MSP_Tx)
* [Option] micro-SD CF for extended storage
* [Option] camera interface

# Basics about development boards

Currently we use goouuuu-esp32 for demonstration. 

* CP2102 USB-to-TTL
* 2 Buttons: reboot + GPIO
* ESP-WROOM-32 
> 1. ESP32-D0WDQ6 
> 2. Xtensa® 32-bit LX6 MCU x 2 
> 3. Clock Freq. 80 ~ 240 MHz 
> 4. 448 KB ROM
> 5. 520 KB SRAM
> 6. RTC 8 KB SRAM
> 7. 1 kbit  eFuse, 256 bit for sysem use (MAC and chip settings), 768 bit reserved for user
> 8. 4MB SPIFlash

<image src="./images/goouuuu-esp32-pcba.png" width="200"> <image src="./images/goouuuu-esp32-pcb.png" width="120">

 # How to Connect Flight Controller

 * Power Supply: GND & 5V
 * Communication: TTL
 > 1. G4 Tx --> FC Rx
 > 2. G5 Rx --> FC Tx

<image src="./images/goouuuu-esp32.png" width="400">






