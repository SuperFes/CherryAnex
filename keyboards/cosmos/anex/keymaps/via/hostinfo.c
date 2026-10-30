#include "hostinfo.h"

bool host_info_receive(host_info_t *info, const uint8_t *data, uint8_t length, uint32_t now) {
    if (length < 1 || data[0] != HOST_INFO_COMMAND) {
        return false;
    }
    if (length < HOST_INFO_LENGTH || data[1] != HOST_INFO_VERSION || data[2] > 23 || data[3] > 59 || data[4] > 100 || data[5] > 100) {
        return true;
    }
    *info = (host_info_t){
        .valid       = true,
        .hour        = data[2],
        .minute      = data[3],
        .cpu         = data[4],
        .ram         = data[5],
        .received_at = now,
    };
    return true;
}

bool host_info_fresh(const host_info_t *info, uint32_t now) {
    return info->valid && now - info->received_at < HOST_INFO_TIMEOUT_MS;
}
