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

#include <avsystem/commons/avs_time.h>

#include "FreeRTOS.h"

#include "lwip/sys.h"

avs_time_monotonic_t avs_time_monotonic_now(void) {
    static uint64_t prev_ms = 0;
    uint64_t ms = sys_now();
    while (ms < prev_ms) {
        ms += portMAX_DELAY + 1;
    }
    prev_ms = ms;

    return avs_time_monotonic_from_scalar((int64_t) ms, AVS_TIME_MS);
}

avs_time_real_t avs_time_real_now(void) {
    avs_time_real_t result = {
        .since_real_epoch = avs_time_monotonic_now().since_monotonic_epoch
    };
    return result;
}
