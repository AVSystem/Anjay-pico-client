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

add_executable(temperature_object_ds18b20
               main.c
               temperature_sensor.c
               ds18b20.c
               )

target_link_libraries(temperature_object_ds18b20
                      pico_stdlib
                      hardware_i2c
                      anjay-pico
                      FreeRTOS
                      )

target_include_directories(temperature_object_ds18b20 PRIVATE
                           ${COMMON_DIR}/config
                           )

target_compile_definitions(temperature_object_ds18b20 PRIVATE
                           WIFI_SSID=\"${WIFI_SSID}\"
                           WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
                           ENDPOINT_NAME=\"${ENDPOINT_NAME}\"
                           PSK_IDENTITY=\"${PSK_IDENTITY}\"
                           PSK_KEY=\"${PSK_KEY}\"
                           )

pico_enable_stdio_usb(temperature_object_ds18b20 1)
pico_enable_stdio_uart(temperature_object_ds18b20 0)

pico_add_extra_outputs(temperature_object_ds18b20)
