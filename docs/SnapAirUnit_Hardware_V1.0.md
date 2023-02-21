
# Basics about SnapAirUnit V1.0

* CP2102 USB-to-TTL
* 2 Buttons: reboot + mode
* 3 LEDs: Power LED/Status LED/Mode LED
* ESP-WROOM-32 
> 1. ESP32-D0WDQ6 
> 2. Xtensa® 32-bit LX6 MCU x 2 
> 3. Clock Freq. 80 ~ 240 MHz 
> 4. 448 KB ROM
> 5. 520 KB SRAM
> 6. RTC 8 KB SRAM
> 7. 1 kbit  eFuse, 256 bit for sysem use (MAC and chip settings), 768 bit reserved for user
> 8. 4MB SPIFlash

 # How to Connect Flight Controller

 * 6pin connector for power and communication (7-24V, GND, OSD_Rx, OSD_Tx, MSP_Rx, MSP_Tx)
 * Power Supply: 7-24V, GND
 * Communication: MSP_Rx, MSP_Tx
 > 1. MSP_Rx --> FC Rx
 > 2. MSP_Tx --> FC Tx
 * Reserved: OSD_Rx, OSD_Tx
 > which we plan to use for ELRS receiver, but apprently hardware doesn't have 2.54mm 4pin connector designed for ELRS receiver.


 # Hardware Images

 Here is a video shot:

https://www.bilibili.com/video/BV1Gv4y1t76r/
 

 <image src="./images/snap_air_unit_hw_v10_front.png" width="200"> <image src="./images/snap_air_unit_hw_v10_back.png" width="200">

 <image src="./images/snap_air_unit_hw_v10_front_rel.png" width="200"> <image src="./images/snap_air_unit_hw_v10_back_rel.png" width="200">

 To Do:
 
 1. solid connector of external antenna: MMCX or IPX
 2. internal PCB antenna or external MMCX antenna selectable by 0 ohm resistor
 3. 4 pin connector (4.5V, GND, RX, TX) and solid fixer for ELRS receiver
 4. 2.54mm Jumper for channel selection (OSD_RX/OSD_TX to esp32 or ELRS receiver)
 5. support camera interface, FPV HD(720P)
 6. RF amplifier for long range 2.4G (LOS) transimission
 7. IO controls: dry contact or wet contact

 *Note: Make sure to avoid IO confliction.*
