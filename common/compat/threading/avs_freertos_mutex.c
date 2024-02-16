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

#include <avsystem/commons/avs_defs.h>
#include <avsystem/commons/avs_memory.h>
#include <avsystem/commons/avs_mutex.h>

#include "FreeRTOS.h"
#include "avs_freertos_structs.h"
#include "semphr.h"

int _avs_mutex_init(avs_mutex_t *mutex) {
    mutex->handle = xSemaphoreCreateMutexStatic(&mutex->buffer);
    if (!mutex->handle) {
        return -1;
    }

    return 0;
}

int avs_mutex_create(avs_mutex_t **out_mutex) {
    AVS_ASSERT(!*out_mutex, "possible attempt to reinitialize a mutex");

    *out_mutex = (avs_mutex_t *) avs_calloc(1, sizeof(avs_mutex_t));
    if (!*out_mutex) {
        return -1;
    }

    if (_avs_mutex_init(*out_mutex)) {
        avs_free(*out_mutex);
        *out_mutex = NULL;
        return -1;
    }

    return 0;
}

int avs_mutex_lock(avs_mutex_t *mutex) {
    if (xSemaphoreTake(mutex->handle, portMAX_DELAY) == pdTRUE) {
        return 0;
    } else {
        return -1;
    }
}

int avs_mutex_try_lock(avs_mutex_t *mutex) {
    if (xSemaphoreTake(mutex->handle, 0) == pdTRUE) {
        return 0;
    } else {
        // No distinction between mutex being unavailable or invalid
        return -1;
    }
}

int avs_mutex_unlock(avs_mutex_t *mutex) {
    if (xSemaphoreGive(mutex->handle) == pdTRUE) {
        return 0;
    } else {
        return -1;
    }
}

void _avs_mutex_destroy(avs_mutex_t *mutex) {
    AVS_ASSERT(xSemaphoreGetMutexHolder(mutex->handle) == NULL,
               "Mutex requested for deletion is taken");
    vSemaphoreDelete(mutex->handle);
}

void avs_mutex_cleanup(avs_mutex_t **mutex) {
    if (!*mutex) {
        return;
    }

    _avs_mutex_destroy(*mutex);
    avs_free(*mutex);
    *mutex = NULL;
}
