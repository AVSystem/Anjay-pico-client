# Copyright 2022-2023 AVSystem <avsystem@avsystem.com>
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

add_executable(time_object
               main.c
               time_object.c
               time_object.h
               )

target_link_libraries(time_object
                      pico_stdlib
                      anjay-pico
                      FreeRTOS
                      )

target_include_directories(time_object PRIVATE
                           ${COMMON_DIR}/config
                           )

target_compile_definitions(time_object PRIVATE
                           WIFI_SSID=\"${WIFI_SSID}\"
                           WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
                           ENDPOINT_NAME=\"${ENDPOINT_NAME}\"
                           PSK_IDENTITY=\"${PSK_IDENTITY}\"
                           PSK_KEY=\"${PSK_KEY}\"
                           )
pico_enable_stdio_usb(time_object 1)
pico_enable_stdio_uart(time_object 0)

pico_add_extra_outputs(time_object)
