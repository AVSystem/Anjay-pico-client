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

#include "hardware/structs/rosc.h"

#include <avsystem/commons/avs_time.h>

#include "mbedtls/timing.h"
#include <mbedtls/entropy.h>

/*
 * The easiest way of obtaining a pseuro-random number on RPI Pico W is reading
 * RANDOMBIT register of ROSC n times. This, however, is not a secure source.
 * Quoting the RP2040 chip Datasheet: "This does not meet the requirements of
 * randomness for security systems because it can be compromised, but it may be
 * useful in less critical applications. If the cores are running from the ROSC
 * then the value will not be random because the timing of the register read
 * will be correlated to the phase of the ROSC."
 */
static int entropy_callback(void *dev,
                            unsigned char *out_buf,
                            size_t out_buf_len,
                            size_t *out_buf_out_len) {
    for (size_t i = 0; i < out_buf_len; i++) {
        uint8_t random_number = 0;
        for (uint8_t bit = 0; bit < 8; bit++) {
            random_number |= rosc_hw->randombit << bit;
        }
        out_buf[i] = random_number;
    }
    *out_buf_out_len = out_buf_len;

    return 0;
}

void anjay_pico_mbedtls_entropy_init__(struct mbedtls_entropy_context *ctx) {
    int result = mbedtls_entropy_add_source(ctx, entropy_callback, NULL, 1,
                                            MBEDTLS_ENTROPY_SOURCE_STRONG);
    (void) result;
    AVS_ASSERT(!result, "Failed to add entropy source");
}

unsigned long mbedtls_timing_hardclock(void) {
    int64_t result;
    avs_time_monotonic_to_scalar(&result, AVS_TIME_MS,
                                 avs_time_monotonic_now());
    return (unsigned long) result;
}

/*
 * mbedtls_timing_set_delay, mbedtls_timing_get_delay and
 * mbedtls_timing_get_timer implementations based on mbedtls/library/timing.c
 */

/*
 * Set delays to watch
 */
void mbedtls_timing_set_delay(void *data, uint32_t int_ms, uint32_t fin_ms) {
    mbedtls_timing_delay_context *ctx = (mbedtls_timing_delay_context *) data;

    ctx->int_ms = int_ms;
    ctx->fin_ms = fin_ms;

    if (fin_ms != 0)
        (void) mbedtls_timing_get_timer(&ctx->timer, 1);
}

/*
 * Get number of delays expired
 */
int mbedtls_timing_get_delay(void *data) {
    mbedtls_timing_delay_context *ctx = (mbedtls_timing_delay_context *) data;
    unsigned long elapsed_ms;

    if (ctx->fin_ms == 0)
        return (-1);

    elapsed_ms = mbedtls_timing_get_timer(&ctx->timer, 0);

    if (elapsed_ms >= ctx->fin_ms)
        return (2);

    if (elapsed_ms >= ctx->int_ms)
        return (1);

    return (0);
}

unsigned long mbedtls_timing_get_timer(struct mbedtls_timing_hr_time *val,
                                       int reset) {
    AVS_STATIC_ASSERT(sizeof(struct mbedtls_timing_hr_time)
                              >= sizeof(avs_time_monotonic_t),
                      avs_time_monotonic_fits);
    avs_time_monotonic_t *start = (avs_time_monotonic_t *) val;
    avs_time_monotonic_t offset = avs_time_monotonic_now();

    if (reset) {
        *start = offset;
        return 0;
    }

    int64_t delta;
    avs_time_duration_to_scalar(&delta, AVS_TIME_MS,
                                avs_time_monotonic_diff(offset, *start));
    return (unsigned long) delta;
}
