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

add_executable(mandatory_objects
               main.c
               )

target_link_libraries(mandatory_objects
                      pico_stdlib
                      anjay-pico
                      FreeRTOS
                      )

target_include_directories(mandatory_objects PRIVATE
                           ${COMMON_DIR}/config
                           )

target_compile_definitions(mandatory_objects PRIVATE
                           WIFI_SSID=\"${WIFI_SSID}\"
                           WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
                           ENDPOINT_NAME=\"${ENDPOINT_NAME}\"
                           )

pico_enable_stdio_usb(mandatory_objects 1)
pico_enable_stdio_uart(mandatory_objects 0)

pico_add_extra_outputs(mandatory_objects)
