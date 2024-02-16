/*
 * Copyright 2022-2024 AVSystem <avsystem@avsystem.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"

#include <anjay/anjay.h>
#include <avsystem/commons/avs_log.h>

#ifndef RUN_FREERTOS_ON_CORE
#    define RUN_FREERTOS_ON_CORE 0
#endif

#define ANJAY_TASK_PRIORITY (tskIDLE_PRIORITY + 1UL)

#define ANJAY_TASK_SIZE (4000U)

static anjay_t *g_anjay;
static StackType_t anjay_stack[ANJAY_TASK_SIZE];
static StaticTask_t anjay_task_buffer;

static void init_wifi(void) {
    if (cyw43_arch_init()) {
        printf("Failed to initialise CYW43 modem\n");
        exit(1);
    }
    cyw43_arch_enable_sta_mode();
    printf("Connecting to WiFi...\n");
    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD,
                                              CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        const int sleep_time_ms = 1000;
        printf("Failed to connect, trying again in %d ms\n", sleep_time_ms);
        sleep_ms(sleep_time_ms);
    }
    printf("Connected.\n");
}

void anjay_task(__unused void *params) {
    init_wifi();

    anjay_configuration_t config = {
        .endpoint_name = ENDPOINT_NAME,
        .in_buffer_size = 2048,
        .out_buffer_size = 2048,
        .msg_cache_size = 2048,
    };

    if (!(g_anjay = anjay_new(&config))) {
        avs_log(main, ERROR, "Could not create Anjay object");
        exit(1);
    }

    // Event loop placeholder
    while (true)
        ;

    anjay_delete(g_anjay);
}

int main(void) {
    stdio_init_all();

    xTaskCreateStatic(anjay_task, "AnjayLwM2MTask", ANJAY_TASK_SIZE, NULL,
                      ANJAY_TASK_PRIORITY, anjay_stack, &anjay_task_buffer);

    /* Start the tasks and timer running. */
    vTaskStartScheduler();

    return 0;
}
