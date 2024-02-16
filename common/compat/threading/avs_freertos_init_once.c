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

#include <avs_commons_init.h>

#include "avs_freertos_structs.h"
#include <avsystem/commons/avs_defs.h>
#include <avsystem/commons/avs_init_once.h>

#include "pico/critical_section.h"
#include <stdbool.h>
#include <stdlib.h>

AVS_STATIC_ASSERT(sizeof(avs_init_once_handle_t) >= sizeof(bool),
                  avs_init_once_handle_too_small);
AVS_STATIC_ASSERT(AVS_ALIGNOF(avs_init_once_handle_t) >= AVS_ALIGNOF(bool),
                  avs_init_once_alignment_incompatible);

static avs_mutex_t g_mutex;

void static __attribute__((constructor)) init_mutex(void) {
    g_mutex.handle = xSemaphoreCreateMutexStatic(&g_mutex.buffer);
    if (!g_mutex.handle) {
        abort();
    }
}

void static __attribute__((destructor)) deinit_mutex(void) {
    vSemaphoreDelete(g_mutex.handle);
}

int avs_init_once(volatile avs_init_once_handle_t *handle,
                  avs_init_once_func_t *func,
                  void *func_arg) {
    volatile bool *state = (volatile bool *) handle;

    if (xSemaphoreTake(g_mutex.handle, portMAX_DELAY) != pdTRUE) {
        return -1;
    }

    int result = 0;
    if (!*state) {
        result = func(func_arg);
        if (result) {
            *state = false;
        } else {
            *state = true;
        }
    }

    xSemaphoreGive(g_mutex.handle);

    return result;
}
