
# Basics about ESP32 development boards

Currently we use goouuuu-esp32 for demonstration. 
There might be some features not avaliable on this board. Please check [SnapAirUnit Hardware](./How_to_Connect.md)

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




