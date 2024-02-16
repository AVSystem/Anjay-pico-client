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

#pragma once

#include <stddef.h>
#include <stdint.h>

typedef int
flash_aligned_writer_cb_t(uint8_t *src, size_t offset_bytes, size_t len_bytes);

/**
 * Flash APIs rquires that the length of data to write will be a multiple of
 * 256, so we need to enforce that by additional buffering.
 */
typedef struct {
    uint8_t *batch_buf;
    size_t batch_buf_max_len_bytes;
    size_t batch_buf_len_bytes;
    size_t write_offset_bytes;
    flash_aligned_writer_cb_t *writer_cb;
} flash_aligned_writer_t;

void flash_aligned_writer_new(uint8_t *batch_buf,
                              size_t batch_buf_max_len_bytes,
                              flash_aligned_writer_cb_t *writer_cb,
                              flash_aligned_writer_t *out_writer);
int flash_aligned_writer_write(flash_aligned_writer_t *writer,
                               const uint8_t *data,
                               size_t length);
int flash_aligned_writer_flush(flash_aligned_writer_t *writer);
