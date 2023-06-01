/*
 * Copyright 2022-2023 AVSystem <avsystem@avsystem.com>
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

#include <hardware/adc.h>
#include <hardware/gpio.h>
#include <pico/stdlib.h>

#include "lm35.h"

#if (LM35_GPIO_PIN < 26) || (LM35_GPIO_PIN > 28)
#    error "Invalid ADC GPIO pin selected for LM35 sensor"
#endif

int lm35_init(void) {
    adc_init();
    adc_gpio_init(LM35_GPIO_PIN);
    return 0;
}

int temperature_get_data(double *sensor_data) {
    adc_select_input(LM35_ADC_CHANNEL);
    uint adc_val = adc_read();
    double milli_volts = (double) adc_val * (3300. / 4096.);
    *sensor_data = milli_volts / 10.;
    return 0;
}
