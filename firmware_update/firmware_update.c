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

#include <anjay/anjay.h>
#include <anjay/fw_update.h>

#include <avsystem/commons/avs_defs.h>
#include <avsystem/commons/avs_log.h>
#include <avsystem/commons/avs_sched.h>
#include <avsystem/commons/avs_time.h>

#include <pico_fota_bootloader.h>

#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/watchdog.h"

#include "firmware_update.h"
#include "flash_aligned_writer.h"

static bool update_initialized;
static size_t downloaded_bytes;

static uint8_t writer_buf[PFB_ALIGN_SIZE];
static flash_aligned_writer_t writer;

static int fw_stream_open(void *user_ptr,
                          const char *package_uri,
                          const struct anjay_etag *package_etag) {
    (void) user_ptr;
    (void) package_uri;
    (void) package_etag;

    pfb_initialize_download_slot();
    flash_aligned_writer_new(writer_buf, AVS_ARRAY_SIZE(writer_buf),
                             pfb_write_to_flash_aligned_256_bytes, &writer);

    downloaded_bytes = 0;
    update_initialized = true;
    avs_log(fw_update, INFO, "Init successful");

    return 0;
}
static int fw_stream_write(void *user_ptr, const void *data, size_t length) {
    (void) user_ptr;

    assert(update_initialized);

    int res = flash_aligned_writer_write(&writer, data, length);
    if (res) {
        return res;
    }

    downloaded_bytes += length;
    avs_log(fw_update, INFO, "Downloaded %zu bytes.", downloaded_bytes);

    return 0;
}

static int fw_stream_finish(void *user_ptr) {
    (void) user_ptr;

    assert(update_initialized);
    update_initialized = false;

    int res = flash_aligned_writer_flush(&writer);
    if (res) {
        avs_log(fw_update, ERROR,
                "Failed to finish download: flash aligned writer flush failed, "
                "result: %d",
                res);
        return -1;
    }

    if (pfb_firmware_sha256_check(downloaded_bytes)) {
        avs_log(fw_update, ERROR, "SHA256 check failed");
        return -1;
    }

    return 0;
}

static void fw_reset(void *user_ptr) {
    (void) user_ptr;

    update_initialized = false;
}

static void fw_update_reboot(avs_sched_t *sched, const void *data) {
    (void) sched;
    (void) data;

    avs_log(fw_update, INFO, "Rebooting.....");
    pfb_perform_update();
}

static int fw_perform_upgrade(void *anjay) {
    pfb_mark_download_slot_as_valid();
    avs_log(fw_update, INFO,
            "The firmware will be updated at the next device reset");

    return AVS_SCHED_DELAYED(anjay_get_scheduler(anjay), NULL,
                             avs_time_duration_from_scalar(1, AVS_TIME_S),
                             fw_update_reboot, NULL, 0);
}

static const anjay_fw_update_handlers_t handlers = {
    .stream_open = fw_stream_open,
    .stream_write = fw_stream_write,
    .stream_finish = fw_stream_finish,
    .reset = fw_reset,
    .perform_upgrade = fw_perform_upgrade
};

int fw_update_install(anjay_t *anjay) {
    anjay_fw_update_initial_state_t state = { 0 };

    if (pfb_is_after_firmware_update()) {
        state.result = ANJAY_FW_UPDATE_INITIAL_SUCCESS;
        avs_log(fw_update, INFO, "Running on a new firmware");
    } else if (pfb_is_after_rollback()) {
        state.result = ANJAY_FW_UPDATE_INITIAL_NEUTRAL;
        avs_log(fw_update, WARNING, "Rollback performed");
    }

    return anjay_fw_update_install(anjay, &handlers, anjay, &state);
}
