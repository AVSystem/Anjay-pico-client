## Firmware Update over The Air

This application demonstrates an example use case of object 5 - Firmware
Update. It utilizes the
[pico_fota_bootloader](https://github.com/JZimnol/pico_fota_bootloader) to swap
the flash partitions after downloading the appropriate binary file from [Coiote
IoT
DM](https://www.avsystem.com/products/coiote-iot-device-management-platform/).

### Flashing the bootloader and the application

After compiling the application, you should have output similar to:

```
build
└── firmware_update
    ├── CMakeFiles
    ├── cmake_install.cmake
    ├── Makefile
    ├── firmware_update.bin
    ├── firmware_update.dis
    ├── firmware_update.elf
    ├── firmware_update.elf.map
    ├── firmware_update.hex
    ├── firmware_update.uf2
    ├── Makefile
    └── pico_fota_bootloader
        ├── CMakeFiles
        ├── cmake_install.cmake
        ├── libpico_fota_bootloader_lib.a
        ├── Makefile
        ├── pico_fota_bootloader.bin
        ├── pico_fota_bootloader.dis
        ├── pico_fota_bootloader.elf
        ├── pico_fota_bootloader.elf.map
        ├── pico_fota_bootloader.hex
        └── pico_fota_bootloader.uf2
```

To flash the application, set Raspberry Pi Pico W to the `BOOTSEL` state (by
powering it up with the `BOOTSEL` button pressed) and copy the
`pico_fota_bootloader.uf2` file into it. Right now the Raspberry Pi Pico W is
flashed with the bootloader but does not have proper application in the
application FLASH memory slot yet. Then, set Raspberry Pi Pico W to the
`BOOTSEL` state again and copy the `firmware_update.uf2` file. The board should
reboot and start the `firmware_update` application.

**NOTE**: you can also look at the serial output logs to monitor the
application state.

### Performing Firmware Update

To perform a Firmware Update Over the Air, the `firmware_update.bin` file
should be sent to or downloaded by the Raspberry Pi Pico W. To do so, open your
Device Page in Coiote DM Platform and go to the `Firmware update` tab. Then
click the `Update firmware` button and then select `Basic Firmware Update`.
Upload `firmware_update.bin` file to Coiote DM, click `Next` button twice and
then `Schedule Update`. After doing so, a Firmware Update process will begin.
Check the serial output logs - the `INFO [fw_update]
[/anjay-pico-client/firmware_update/firmware_update.c]: Downloaded X bytes`
logs should appear. For more detailed information, see [AVSystem
Devzone](https://iotdevzone.avsystem.com/docs/Coiote_IoT_DM/firmware_update/).

After downloading the file, Raspberry Pi Pico W will reboot and the bootloader
will execute the application from the downloaded `.bin` file. After connecting
to the Wi-Fi, the `INFO [fw_update]
[/anjay-pico-client/firmware_update/firmware_update.c]: Running on a new
firmware` log will appear.

**Note that while rebuilding the application, the linker scripts' contents
should not be changed or should be changed carefully to maintain the memory
layout backward compatibility.**
