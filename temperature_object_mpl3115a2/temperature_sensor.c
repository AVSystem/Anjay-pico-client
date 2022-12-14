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

#include <assert.h>
#include <stdbool.h>

#include <anjay/anjay.h>
#include <anjay/ipso_objects.h>

#include <avsystem/commons/avs_defs.h>
#include <avsystem/commons/avs_log.h>

#include "mpl3115a2.h"
#include "temperature_sensor.h"

static int
temperature_sensor_get_value(anjay_iid_t iid, void *_ctx, double *value) {
    (void) iid;
    (void) _ctx;
    assert(value);

    double read_value;
    if (!temperature_read_data() && !temperature_get_data(&read_value)) {
        *value = read_value;
        return 0;
    } else {
        return -1;
    }
}

void temperature_sensor_install(anjay_t *anjay) {
    if (mpl3115a2_init()) {
        avs_log(ipso_object,
                WARNING,
                "Driver for MPL3115A2 could not be initialized!");
        return;
    }

    if (anjay_ipso_basic_sensor_install(anjay, 3303, 1)) {
        avs_log(ipso_object,
                WARNING,
                "Object: Temperature sensor could not be installed");
        return;
    }

    if (anjay_ipso_basic_sensor_instance_add(
                anjay,
                3303,
                0,
                (anjay_ipso_basic_sensor_impl_t) {
                    .unit = "Cel",
                    .min_range_value = NAN,
                    .max_range_value = NAN,
                    .get_value = temperature_sensor_get_value
                })) {
        avs_log(ipso_object,
                WARNING,
                "Instance of Temperature sensor object could not be added");
    }
}

void temperature_sensor_update(anjay_t *anjay) {
    anjay_ipso_basic_sensor_update(anjay, 3303, 0);
}

void temperature_sensor_release(void) {
    mpl3115a2_release();
}
