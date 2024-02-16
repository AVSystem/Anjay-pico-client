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

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <avsystem/commons/avs_utils.h>

#include "flash_aligned_writer.h"

void flash_aligned_writer_new(uint8_t *batch_buf,
                              size_t batch_buf_max_len_bytes,
                              flash_aligned_writer_cb_t *writer_cb,
                              flash_aligned_writer_t *out_writer) {
    assert(batch_buf);
    assert(batch_buf_max_len_bytes);
    assert(writer_cb);

    out_writer->batch_buf = batch_buf;
    out_writer->batch_buf_max_len_bytes = batch_buf_max_len_bytes;
    out_writer->batch_buf_len_bytes = 0;
    out_writer->write_offset_bytes = 0;
    out_writer->writer_cb = writer_cb;
}

int flash_aligned_writer_write(flash_aligned_writer_t *writer,
                               const uint8_t *data,
                               size_t length_bytes) {
    while (length_bytes > 0) {
        const size_t bytes_to_copy = AVS_MIN(
                writer->batch_buf_max_len_bytes - writer->batch_buf_len_bytes,
                length_bytes);
        memcpy(writer->batch_buf + writer->batch_buf_len_bytes, data,
               bytes_to_copy);
        data += bytes_to_copy;
        length_bytes -= bytes_to_copy;
        writer->batch_buf_len_bytes += bytes_to_copy;

        if (writer->batch_buf_len_bytes == writer->batch_buf_max_len_bytes) {
            int res = writer->writer_cb(writer->batch_buf,
                                        writer->write_offset_bytes,
                                        writer->batch_buf_len_bytes);
            if (res) {
                return res;
            }
            writer->write_offset_bytes += writer->batch_buf_len_bytes;
            writer->batch_buf_len_bytes = 0;
        }
    }

    return 0;
}

int flash_aligned_writer_flush(flash_aligned_writer_t *writer) {
    if (writer->batch_buf_len_bytes == 0) {
        return 0;
    }

    int res = writer->writer_cb(writer->batch_buf, writer->write_offset_bytes,
                                writer->batch_buf_len_bytes);
    if (res) {
        return res;
    }
    writer->write_offset_bytes += writer->batch_buf_len_bytes;
    writer->batch_buf_len_bytes = 0;

    return 0;
}
