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

#include <string.h>

#include "ds18b20.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

#define DS18B20_SCRATCHPAD_SIZE 9
#define DS18B20_ROM_CODE_SIZE 8

#define DS18B20_READ_ROM 0x33
#define DS18B20_MATCH_ROM 0x55
#define DS18B20_SKIP_ROM 0xCC

#define DS18B20_CONVERT_T 0x44
#define DS18B20_READ_SCRATCHPAD 0xBE

static double temperature;
static uint8_t ds18b20_addr[DS18B20_ROM_CODE_SIZE];

static uint8_t byte_crc(uint8_t crc, uint8_t byte) {
    uint8_t i;
    for (i = 0; i < 8; i++) {
        uint8_t b = crc ^ byte;
        crc >>= 1;
        if (b & 0x01) {
            crc ^= 0x8c;
        }
        byte >>= 1;
    }
    return crc;
}

static uint8_t wire_crc(const uint8_t *data, size_t len) {
    size_t i;
    uint8_t crc = 0;

    for (i = 0; i < len; i++) {
        crc = byte_crc(crc, data[i]);
    }

    return crc;
}

static int wire_reset(void) {
    gpio_set_dir(ONEWIRE_PIN, GPIO_OUT);
    gpio_put(ONEWIRE_PIN, 0);
    sleep_us(480);
    gpio_put(ONEWIRE_PIN, 1);
    sleep_us(70);
    gpio_set_dir(ONEWIRE_PIN, GPIO_IN);
    bool rc = gpio_get(ONEWIRE_PIN);
    sleep_us(410);

    if (rc) {
        return 1;
    }
    return 0;
}

static void write_bit(int value) {
    gpio_set_dir(ONEWIRE_PIN, GPIO_OUT);
    if (value) {
        gpio_put(ONEWIRE_PIN, 0);
        sleep_us(6);
        gpio_put(ONEWIRE_PIN, 1);
        sleep_us(64);
    } else {
        gpio_put(ONEWIRE_PIN, 0);
        sleep_us(60);
        gpio_put(ONEWIRE_PIN, 1);
        sleep_us(10);
    }
}

static bool read_bit(void) {
    gpio_set_dir(ONEWIRE_PIN, GPIO_OUT);
    gpio_put(ONEWIRE_PIN, 0);
    sleep_us(6);
    gpio_put(ONEWIRE_PIN, 1);
    sleep_us(9);
    gpio_set_dir(ONEWIRE_PIN, GPIO_IN);
    bool rc = gpio_get(ONEWIRE_PIN);
    sleep_us(55);
    return rc;
}

static void wire_write(uint8_t byte) {
    uint8_t i;
    for (i = 0; i < 8; i++) {
        write_bit(byte & 0x01);
        byte >>= 1;
    }
}

static uint8_t wire_read(void) {
    uint8_t value = 0;
    uint8_t i;
    for (i = 0; i < 8; i++) {
        value >>= 1;
        if (read_bit()) {
            value |= 0x80;
        }
    }
    return value;
}

static int ds18b20_read_address(uint8_t *rom_code) {
    int i;
    uint8_t crc;

    if (wire_reset()) {
        return 1;
    }

    wire_write(DS18B20_READ_ROM);

    for (i = 0; i < DS18B20_ROM_CODE_SIZE; i++) {
        rom_code[i] = wire_read();
    }

    crc = wire_crc(rom_code, DS18B20_ROM_CODE_SIZE - 1);
    if (rom_code[DS18B20_ROM_CODE_SIZE - 1] != crc) {
        return 1;
    }

    return 0;
}

static int send_cmd(const uint8_t *rom_code, uint8_t cmd) {
    int i;

    if (wire_reset()) {
        return 1;
    }

    if (!rom_code) {
        wire_write(DS18B20_SKIP_ROM);
    } else {
        wire_write(DS18B20_MATCH_ROM);
        for (i = 0; i < DS18B20_ROM_CODE_SIZE; i++) {
            wire_write(rom_code[i]);
        }
    }
    wire_write(cmd);
    return 0;
}

static int ds18b20_read_scratchpad(const uint8_t *rom_code,
                                   uint8_t *scratchpad) {
    int i;
    uint8_t crc;

    if (send_cmd(rom_code, DS18B20_READ_SCRATCHPAD)) {
        return 1;
    }

    for (i = 0; i < DS18B20_SCRATCHPAD_SIZE; i++) {
        scratchpad[i] = wire_read();
    }

    crc = wire_crc(scratchpad, DS18B20_SCRATCHPAD_SIZE - 1);
    if (scratchpad[DS18B20_SCRATCHPAD_SIZE - 1] != crc) {
        return 1;
    }

    return 0;
}

static double ds18b20_get_temp(const uint8_t *rom_code) {
    uint8_t scratchpad[DS18B20_SCRATCHPAD_SIZE];
    int16_t temp;

    if (ds18b20_read_scratchpad(rom_code, scratchpad) != 0) {
        return 85.0;
    }

    memcpy(&temp, &scratchpad[0], sizeof(temp));

    return temp / 16.0;
}

static int ds18b20_start_measure(const uint8_t *rom_code) {
    return send_cmd(rom_code, DS18B20_CONVERT_T);
}

int ds18b20_init(void) {
    gpio_init(ONEWIRE_PIN);
    gpio_pull_up(ONEWIRE_PIN);
    if (ds18b20_read_address(ds18b20_addr)) {
        return 1;
    }

    return 0;
}

int ds18b20_release(void) {
    gpio_deinit(ONEWIRE_PIN);
}

int temperature_read_data(void) {
    ds18b20_start_measure(ds18b20_addr);
    sleep_ms(750);
    temperature = ds18b20_get_temp(ds18b20_addr);
    if (temperature >= 80.0) {
        return 1;
    }

    return 0;
}

int temperature_get_data(double *sensor_data) {
    *sensor_data = temperature;
    return 0;
}
