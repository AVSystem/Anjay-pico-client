# Anjay Raspberry Pico W Client
## Overview
This repository contains a LwM2M Client application example for Raspberry Pico devices, based on the open-source [Anjay](https://github.com/AVSystem/Anjay) library and [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk).

## Examples

Example applications refer to [Anjay Basic Client Tutorial](https://avsystem.github.io/Anjay-doc/BasicClient.html) and can guide through the creation of a basic LwM2M Client based on Anjay library step by step.

Application|Description|Reference
---|---|---
[Anjay Initialization](anjay_init)|A minimum build environment for example client|[doc link](https://avsystem.github.io/Anjay-doc/BasicClient/BC-Initialization.html)
[Mandatory Objects](mandatory_objects)|Mandatory LwM2M Objects necessary for setting up a connection with a server and an implementation of custom Anjay event loop|[doc link](https://avsystem.github.io/Anjay-doc/BasicClient/BC-MandatoryObjects.html)
[Secure Communication](secure_communication)|Secure communication using PSK mode<br>Note: randomness source does not meet requirements of security systems, see [comments in the code](secure_communication/main.c#L2)|[doc link](https://avsystem.github.io/Anjay-doc/BasicClient/BC-Security.html)
[Temperature Object](temperature_object)|Example Temperature Sensor object implementation using Adafruit MPL3115A2|[doc link](https://avsystem.github.io/Anjay-doc/AdvancedTopics/AT-IpsoObjects.html)

## Compiling and launching

To compile the client, you need Raspberry Pi Pico SDK and FreeRTOS kernel cloned into a base directory, parallel to this repository. Creating a workspace from scratch could look like this:
```
mkdir pico
cd pico
git clone -b 1.4.0 https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk/ && git submodule update --init && cd ..
git clone -b V10.5.0 https://github.com/FreeRTOS/FreeRTOS-Kernel.git
git clone https://github.com/AVSystem/Anjay-pico-client.git
```
Your final working tree should look like this:
```
$ ls -1
Anjay-pico-client
FreeRTOS-Kernel
pico-sdk
```
To compile the application, you will need some tools.
```
sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib
```
After that, you can go to the `Anjay-pico-client` repository and build the project with `<ssid>` and `<pass>` replaced with your WIFI name and password respectively. LwM2M Client Endpoint Name is also configured by `<endpoint_name>` parameter.
```
cd Anjay-pico-client
git submodule update --init --recursive
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DWIFI_SSID="<ssid>" -DWIFI_PASSWORD="<pass>" -DENDPOINT_NAME="<endpoint_name>" .. && make -j
```
When building examples with security enabled (Secure Communication and Temperature Object), you need to provide PSK Identity and Key (by replacing `<identity>` and `<psk>` respectively with values entered in [Coiote LwM2M Server](#connecting-to-the-lwm2m-server))
```
cmake -DCMAKE_BUILD_TYPE=Debug -DWIFI_SSID="<ssid>" -DWIFI_PASSWORD="<pass>" -DENDPOINT_NAME="<endpoint_name>" -DPSK_IDENTITY="<identity>" -DPSK_KEY="<psk>" .. && make -j
```
This should generate directories named after examples that contain, among others, files with `.uf2` and `.hex` extensions. `.uf2` files can be programmed through the bootloader and `.hex` are for programming using a debugger and SWD connection.

### Flashing
#### Bootloader
To program using the bootloader, press `BOOTSEL` button while connecting Raspberry Pi Pico W through a USB cable. It should be recognized then as a Mass Storage device, where you can copy the `.uf2` file, Pico will be programmed, reset and start running the code.

### Serial output
Default serial port connection is located at pins `GP0 - TX` and `GP1 - RX`, use for example Serial to USB converter and connect it to those pins to receive `stdout` messages.

## Connecting to the LwM2M Server
To connect to [Coiote IoT Device Management](https://www.avsystem.com/products/coiote-iot-device-management-platform/) LwM2M Server, please register at [https://eu.iot.avsystem.cloud/](https://eu.iot.avsystem.cloud/). There is a [guide showing basic usage of Coiote DM](https://iotdevzone.avsystem.com/docs/Coiote_DM_Device_Onboarding/Quick_start/)
available on IoT Developer Zone. For the [Mandatory Objects](mandatory_objects) example that has no connection security enabled, simply select `NoSec` as `Security Mode` in the Guide step 3.

## Links
* [Anjay source repository](https://github.com/AVSystem/Anjay)
* [Anjay documentation](https://avsystem.github.io/Anjay-doc/index.html)
* [Doxygen-generated API documentation](https://avsystem.github.io/Anjay-doc/api/index.html)
* [AVSystem IoT Devzone](https://iotdevzone.avsystem.com/)
* [AVSystem Discord server](https://discord.avsystem.com)
* [Raspberry Pico Documentation](https://www.raspberrypi.com/documentation/microcontrollers/raspberry-pi-pico.html)
