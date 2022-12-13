# **How to Use Snap Air Unit**

This is education version of Snap Air Unit, which is designed for youth (kids and adult) to get a basic understanding of auto pilot. And we hope it wil inspire people from knowing more about automation, physics, electronics，
mathematics, computer, aviation, and etc. Ultimately, it might cultivate talents from generation to generation in scientific aeras.

<image src="./images/snap_air_unit_draft_edu_hw_digram.png" width="600">

** Note: MSP(DJI) Protocol is NOT used, since there is NO camera on education version hardware of Snap Air Unit.**

# 101 lessons on How to use Snap Air Unit Module

## Step 1: Preparation of hardware and firmware

Please consult: 
1. [Build_and_Flash.md](./docs/Build_and_Flash.md)
2. [How_to_Connect.md](./docs/How_to_Connect.md)

## Step 2: Software mode switching

**Mode LED**
> 1. [Mandatory] WiFi-AP mode: Always bright 
> 2. [Mandatory] WiFi-STA mode: Slow flash
> 3. [Mandatory] BT-SPP mode: flash

**Status LED**
> 1. [Mandatory] Time out for Connection with Factory mode: Quick flash
> 2. [Mandatory] Wait for Binding: Two flash
> 3. [Mandatory] Wait for Connection: Slow flash
> 4. [Mandatory] Connection Established: Always bright
> 5. [Mandatory] Exception or Error: Three flash

### 2.1 by hardware

**Short press to switch mode**
> sequecne：WiFi-STA ==> BT-SPP(UART) ==> WiFi-AP ==> Loop

*Note: Currently is for devlopement choices; please re-order to AP->STA->BT when release.*

### 2.2 by udp command

**bluetooth**
> Switch to BT SPP(UART) mode

**ap <ssid> <pass>**
> Switch to WiFi STA mode with ssid and password

**wifi <ssid> <pass>**
> Switch to WiFi AP mode with ssid and password

*Note: Currently, different mode switch is deemed effective way. Please switch to BT first and back to WiFi AP or STA mode.*

### 2.3 by debug command

**bluetooth**
> Switch to BT SPP(UART) mode

**ap <ssid> <pass>**
> Switch to WiFi STA mode with ssid and password

**wifi <ssid> <pass>**
> Switch to WiFi AP mode with ssid and password

*Note: Currently, different mode switch is deemed effective way. Please switch to BT first and back to WiFi AP or STA mode.*

## Step 3: Configure BT SPP(UART) on windows

please consult: [蓝牙无线自制串口模块连接穿越机配置工具](https://blog.csdn.net/lida2003/article/details/127901773)

## Step 4: Find WiFi Address of the module

Currently, we do NOT support [Need Bonjour, broadcast, UPNP protocol for device finding when in STA/Ap mode](https://github.com/lida2003/SnapAirUnit/issues/10).

So......

a) It's easy for us to get WiFi address when in AP mode.

b) For WiFi STA mode, please check seiral debug log, something much more like below bold ip address

> I (1912) wifi:connected with AutoLab, aid = 13, channel 6, 40D, bssid = d0:c7:c0:5c:ce:50
> 
> I (1912) wifi:security: WPA2-PSK, phy: bgn, rssi: -37
> 
> I (1922) wifi:pm start, type: 1
> 
> I (1952) wifi:AP's beacon interval = 102400 us, DTIM period = 1
>
> W (2482) sta: UNEXPECTED EVENT 00000000
>
> I (2482) sta: Rechecking in 9 seconds...
>
> I (3502) esp_netif_handlers: sta ip: 192.168.78.229, mask: 255.255.255.0, gw: 192.168.78.1
>
> **I (3502) sta: got ip:192.168.78.229**
>
> I (3502) sta: connected to ap SSID:AutoLab password:68686868
>
> I (3512) http: Starting HTTP Server


## Step 5: Communicate with Snap Air Unit with iNav configurator

### 5.1 by virtual BT com port

please consult: [蓝牙无线自制串口模块连接穿越机配置工具](https://blog.csdn.net/lida2003/article/details/127901773)

<image src="./images/snap_air_unit_bt_spp_inva_configurator.png" width="600">

### 5.2 by UDP port

<image src="./images/snap_air_unit_udp_inva_configurator.png" width="600">

## Step 6: Communicate with Snap Air Unit with UDP tools

<image src="./images/snap_air_unit_udp_tool_1.png" width="600">

<image src="./images/snap_air_unit_udp_tool_2.png" width="600">


## Step 7: Communicate with Snap Air Unit with debug serial port

<image src="./images/snap_air_unit_debug_serial.png" width="600">

## Step 8: Scratch Programming

TBD
