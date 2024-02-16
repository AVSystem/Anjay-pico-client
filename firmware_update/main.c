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

#include "lwip/sockets.h"
#include <anjay/anjay.h>
#include <anjay/core.h>
#include <anjay/security.h>
#include <anjay/server.h>
#include <avsystem/commons/avs_list.h>
#include <avsystem/commons/avs_log.h>
#include <avsystem/commons/avs_prng.h>
#include <avsystem/commons/avs_time.h>

#include "firmware_update.h"

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

static int setup_security_object() {
    if (anjay_security_object_install(g_anjay)) {
        return -1;
    }

    static const char psk_identity[] = PSK_IDENTITY;
    static const char psk_key[] = PSK_KEY;

    const anjay_security_instance_t security_instance = {
        .ssid = 1,
        .server_uri = "coaps://eu.iot.avsystem.cloud:5684",
        .security_mode = ANJAY_SECURITY_PSK,
        .public_cert_or_psk_identity = (const uint8_t *) psk_identity,
        .public_cert_or_psk_identity_size = strlen(psk_identity),
        .private_cert_or_psk_key = (const uint8_t *) psk_key,
        .private_cert_or_psk_key_size = strlen(psk_key)
    };

    anjay_iid_t security_instance_id = ANJAY_ID_INVALID;
    return anjay_security_object_add_instance(g_anjay, &security_instance,
                                              &security_instance_id);
}

static int setup_server_object() {
    if (anjay_server_object_install(g_anjay)) {
        return -1;
    }

    const anjay_server_instance_t server_instance = {
        .ssid = 1,
        .lifetime = 50,
        .default_min_period = -1,
        .default_max_period = -1,
        .disable_timeout = -1,
        .binding = "U"
    };

    anjay_iid_t server_instance_id = ANJAY_ID_INVALID;
    return anjay_server_object_add_instance(g_anjay, &server_instance,
                                            &server_instance_id);
}

void main_loop(void) {
    while (true) {
        AVS_LIST(avs_net_socket_t *const) sockets = NULL;
        sockets = anjay_get_sockets(g_anjay);

        size_t numsocks = AVS_LIST_SIZE(sockets);
        struct pollfd pollfds[numsocks];
        size_t i = 0;
        AVS_LIST(avs_net_socket_t *const) sock;
        AVS_LIST_FOREACH(sock, sockets) {
            pollfds[i].fd = *(const int *) avs_net_socket_get_system(*sock);
            pollfds[i].events = POLLIN;
            pollfds[i].revents = 0;
            ++i;
        }

        const int max_wait_time_ms = 1000;
        int wait_ms = max_wait_time_ms;
        wait_ms = anjay_sched_calculate_wait_time_ms(g_anjay, max_wait_time_ms);

        if (poll(pollfds, numsocks, wait_ms) > 0) {
            int socket_id = 0;
            AVS_LIST(avs_net_socket_t *const) socket = NULL;
            AVS_LIST_FOREACH(socket, sockets) {
                if (pollfds[socket_id].revents) {
                    if (anjay_serve(g_anjay, *socket)) {
                        avs_log(tutorial, ERROR, "anjay_serve() failed");
                    }
                }
                ++socket_id;
            }
        }

        anjay_sched_run(g_anjay);
    }
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

    if (setup_security_object() || setup_server_object()) {
        avs_log(main, ERROR, "Failed to initialize basic objects");
        exit(1);
    }

    if (fw_update_install(g_anjay)) {
        avs_log(main, ERROR, "Failed to initialize FOTA object");
        exit(1);
    }

    main_loop();

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
