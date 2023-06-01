#!/bin/sh

cd /workspaces
git clone -b 1.4.0 --recursive https://github.com/raspberrypi/pico-sdk.git
git clone -b V10.5.0 --recursive  https://github.com/FreeRTOS/FreeRTOS-Kernel.git
cd Anjay-pico-client
git submodule update --init --recursive
