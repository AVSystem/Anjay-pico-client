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

#include "FreeRTOS.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "semphr.h"

#ifndef i2c_default
#    define i2c_default PICO_DEFAULT_I2C_INSTANCE
#endif

// 7-bit sensor i2c address
#define MPL3115A2_I2C_ADDR (0x60)

#define MPL3115A2_REG_OUT_T_MSB_ADDR (0x04)
#define MPL3115A2_REG_WHO_AM_I_ADDR (0x0C)
#define MPL3115A2_REG_CTRLREG1_ADDR (0x26)

#define MPL3115A2_CTRLREG1_OS0 (0x08)
#define MPL3115A2_CTRLREG1_OS1 (0x10)
#define MPL3115A2_CTRLREG1_OS2 (0x20)
#define MPL3115A2_CTRLREG1_SBYB (0x01)
#define MPL3115A2_WHO_AM_I_VAL (0xC4)
#define MPL3115A2_CTRLREG1_CONFIG                                             \
    (MPL3115A2_CTRLREG1_OS0 | MPL3115A2_CTRLREG1_OS1 | MPL3115A2_CTRLREG1_OS2 \
     | MPL3115A2_CTRLREG1_SBYB)

#define MUTEX_TIME_TO_WAIT (200 / portTICK_PERIOD_MS)

static double temperature_sensor_data;
static SemaphoreHandle_t mutex;

int temperature_read_data(void) {
    int retval = -1;
    if (pdTRUE == xSemaphoreTake(mutex, MUTEX_TIME_TO_WAIT)) {
        uint8_t reg = MPL3115A2_REG_OUT_T_MSB_ADDR;
        uint8_t buf[2];

        if ((i2c_write_blocking(i2c_default, MPL3115A2_I2C_ADDR, &reg, 1, true)
             != 1)) {
            goto finish;
        }
        if (i2c_read_blocking(i2c_default, MPL3115A2_I2C_ADDR, buf, 2, false)
                != 2) {
            goto finish;
        }

        int16_t t = (int16_t) (((uint16_t) buf[0]) << 8 | buf[1]);
        temperature_sensor_data = ((float) t) / 256.f;
        retval = 0;
    finish:
        xSemaphoreGive(mutex);
    }
    return retval;
}

int temperature_get_data(double *sensor_data) {
    if (pdTRUE == xSemaphoreTake(mutex, MUTEX_TIME_TO_WAIT)) {
        *sensor_data = temperature_sensor_data;
        xSemaphoreGive(mutex);
        return 0;
    } else {
        return -1;
    }
}

int mpl3115a2_init(void) {
    // use default I2C0 at 400kHz, I2C is active low
    i2c_init(i2c_default, 400000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

    // add program information for picotool
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN,
                               PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));

    // verify chip's identity
    uint8_t reg = MPL3115A2_REG_WHO_AM_I_ADDR;
    uint8_t who_am_i;
    if (i2c_write_blocking(i2c_default, MPL3115A2_I2C_ADDR, &reg, 1, true)
            != 1) {
        return -1;
    }

    if (i2c_read_blocking(i2c_default, MPL3115A2_I2C_ADDR, &who_am_i, 1, false)
                    != 1
            && who_am_i != MPL3115A2_WHO_AM_I_VAL) {
        return -1;
    }

    // set device active
    uint8_t buf[] = { MPL3115A2_REG_CTRLREG1_ADDR, MPL3115A2_CTRLREG1_CONFIG };
    if (i2c_write_blocking(i2c_default, MPL3115A2_I2C_ADDR, buf, 2, false)
            != 2) {
        return -1;
    }

    mutex = xSemaphoreCreateMutex();

    return 0;
}

int mpl3115a2_release(void) {
    i2c_deinit(i2c_default);
}
