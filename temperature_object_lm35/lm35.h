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

#pragma once

#define ADC_PIN_TO_CHANNEL(Pin) ((Pin) - (26))

/* Temperature sensor ADC channel and pin */
#define LM35_GPIO_PIN 26
#define LM35_ADC_CHANNEL ADC_PIN_TO_CHANNEL(LM35_GPIO_PIN)

int lm35_init(void);
int temperature_get_data(double *sensor_data);
