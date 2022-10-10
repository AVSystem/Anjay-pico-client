/*
 * Copyright 2022 AVSystem <avsystem@avsystem.com>
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

#ifndef AVS_FREERTOS_STRUCTS_H
#define AVS_FREERTOS_STRUCTS_H

#include <avsystem/commons/avs_condvar.h>
#include <avsystem/commons/avs_mutex.h>

#include "FreeRTOS.h"
#include "semphr.h"

struct avs_mutex {
    StaticSemaphore_t buffer;
    SemaphoreHandle_t handle;
};

// we are not using AVS_LIST because we want to use stack allocation
typedef struct condvar_waiter_node_struct {
    StaticSemaphore_t buffer;
    SemaphoreHandle_t handle;
    struct condvar_waiter_node_struct *next;
} condvar_waiter_node_t;

struct avs_condvar {
    // first_waiter and every condvar_waiter_node_t::next are only accessed when
    // waiters_mutex is locked
    avs_mutex_t waiters_mutex;
    condvar_waiter_node_t *first_waiter;
};

int _avs_mutex_init(avs_mutex_t *mutex);
void _avs_mutex_destroy(avs_mutex_t *mutex);

#endif // AVS_FREERTOS_STRUCTS_H
