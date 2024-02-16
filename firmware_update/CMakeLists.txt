# Copyright 2022-2024 AVSystem <avsystem@avsystem.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.13)

add_subdirectory(pico_fota_bootloader)

add_executable(firmware_update
               firmware_update.c
               flash_aligned_writer.c
               main.c
               )

target_link_libraries(firmware_update
                      pico_stdlib
                      pico_fota_bootloader_lib
                      anjay-pico
                      FreeRTOS
                      )

target_include_directories(firmware_update PRIVATE
                           ${COMMON_DIR}/config
                           )

target_compile_definitions(firmware_update PRIVATE
                           WIFI_SSID=\"${WIFI_SSID}\"
                           WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
                           ENDPOINT_NAME=\"${ENDPOINT_NAME}\"
                           PSK_IDENTITY=\"${PSK_IDENTITY}\"
                           PSK_KEY=\"${PSK_KEY}\"
                           )

target_link_options(firmware_update PRIVATE "-L${CMAKE_CURRENT_SOURCE_DIR}/pico_fota_bootloader/linker_common")
pico_set_linker_script(firmware_update ${CMAKE_CURRENT_SOURCE_DIR}/pico_fota_bootloader/linker_common/application.ld)

pico_enable_stdio_usb(firmware_update 1)
pico_enable_stdio_uart(firmware_update 0)

pico_add_extra_outputs(firmware_update)
