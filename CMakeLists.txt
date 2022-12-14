# Copyright 2022 AVSystem <avsystem@avsystem.com>
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

set(PICO_BOARD pico_w)
set(PICO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/../pico-sdk" CACHE PATH "Path to the Raspberry Pi Pico SDK")
set(FREERTOS_KERNEL_PATH "${CMAKE_CURRENT_LIST_DIR}/../FreeRTOS-Kernel" CACHE PATH "Path to the FreeRTOS kernel sources")
set(WIFI_SSID "wifi_ssid" CACHE STRING "SSID of the WiFi network to connect to")
set(WIFI_PASSWORD "wifi_password" CACHE STRING "PSK passphrase of the WiFi network to connect to")
set(ENDPOINT_NAME "endpoint_name" CACHE STRING "Endpoint Name dor LwM2M Client")
set(PSK_IDENTITY "psk_identity" CACHE STRING "PSK Identity for secure communication")
set(PSK_KEY "psk_key" CACHE STRING "PSK Key for secure communication")
set(WIFI_PASSWORD "wifi_password" CACHE STRING "PSK passphrase of the WiFi network to connect to")
set(COMMON_DIR ${CMAKE_CURRENT_LIST_DIR}/common)
set(MBEDTLS_CONFIG_FILE "mbedtls.h")

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)
include(FreeRTOS_Kernel_import.cmake)

project(anjay-pico)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

file(GLOB_RECURSE ANJAY_SOURCES "deps/anjay/src/*.c")
file(GLOB_RECURSE AVS_COMMONS_SOURCES "deps/anjay/deps/avs_commons/src/*.c")
file(GLOB_RECURSE AVS_COAP_SOURCES "deps/anjay/deps/avs_coap/src/*.c")
file(GLOB_RECURSE AVS_OS_COMPAT_SOURCES ${COMMON_DIR}/compat/*.c)
file(GLOB FREERTOS_SOURCES ${FREERTOS_KERNEL_PATH}/*.c)
file(GLOB MBEDTLS_SOURCES "deps/mbedtls/library/*.c")
        
add_library(anjay-pico 
            ${ANJAY_SOURCES}
            ${AVS_COMMONS_SOURCES}
            ${AVS_COAP_SOURCES}
            ${AVS_OS_COMPAT_SOURCES}
            )

add_library(FreeRTOS
            ${FREERTOS_SOURCES}
            ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/port.c
            ${COMMON_DIR}/src/freertos.c
            )

add_library(mbedtls
            ${MBEDTLS_SOURCES}
            ${COMMON_DIR}/compat/mbedtls/mbedtls_timing.c
            )

target_include_directories(FreeRTOS PUBLIC
                           ${FREERTOS_KERNEL_PATH}/include
                           ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/include
                           ${COMMON_DIR}/config
                           )

target_include_directories(anjay-pico PRIVATE
                           deps/anjay/src
                           deps/anjay/deps/avs_commons/src
                           deps/anjay/deps/avs_coap/src
                           )

target_include_directories(anjay-pico PUBLIC
                           deps/anjay/include_public
                           deps/anjay/deps/avs_commons/include_public
                           deps/anjay/deps/avs_coap/include_public
                           ${COMMON_DIR}/config
                           )

target_include_directories(mbedtls PUBLIC
                           "deps/mbedtls/include"
                           ${COMMON_DIR}/config
                           ${COMMON_DIR}/compat/mbedtls
                           )

target_include_directories(mbedtls PRIVATE
                           deps/anjay/include_public
                           deps/anjay/deps/avs_commons/include_public
                           deps/anjay/deps/avs_coap/include_public
                           )

target_compile_definitions(mbedtls PRIVATE
                           MBEDTLS_CONFIG_FILE=\"${MBEDTLS_CONFIG_FILE}\"
                           )

target_link_libraries(mbedtls
                      pico_stdlib
                      )

target_link_libraries(FreeRTOS
                      pico_cyw43_arch_lwip_sys_freertos
                      FreeRTOS-Kernel-Heap4
                      )

target_link_libraries(anjay-pico
                      FreeRTOS
                      mbedtls
                      )

add_subdirectory(anjay_init)
add_subdirectory(mandatory_objects)
add_subdirectory(secure_communication)
add_subdirectory(temperature_object_mpl3115a2)
add_subdirectory(temperature_object_ds18b20)
