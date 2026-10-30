#include "hostinfo.h"
#include "check.h"

static void test_accepts_report(void) {
    host_info_t   info     = {0};
    const uint8_t data[32] = {HOST_INFO_COMMAND, HOST_INFO_VERSION, 13, 37, 42, 100};
    CHECK(host_info_receive(&info, data, sizeof(data), 1000));
    CHECK(info.valid);
    CHECK(info.hour == 13 && info.minute == 37 && info.cpu == 42 && info.ram == 100);
    CHECK(info.received_at == 1000);
}

static void test_leaves_other_commands_to_via(void) {
    host_info_t   info     = {0};
    const uint8_t data[32] = {0x01, HOST_INFO_VERSION, 13, 37, 42, 50};
    CHECK(!host_info_receive(&info, data, sizeof(data), 0));
    CHECK(!host_info_receive(&info, data, 0, 0));
    CHECK(!info.valid);
}

static void test_consumes_but_ignores_bad_reports(void) {
    host_info_t   info        = {.valid = true, .hour = 1, .received_at = 5};
    const uint8_t version[32] = {HOST_INFO_COMMAND, HOST_INFO_VERSION + 1, 13, 37, 42, 50};
    const uint8_t hour[32]    = {HOST_INFO_COMMAND, HOST_INFO_VERSION, 24, 0, 0, 0};
    const uint8_t minute[32]  = {HOST_INFO_COMMAND, HOST_INFO_VERSION, 0, 60, 0, 0};
    const uint8_t cpu[32]     = {HOST_INFO_COMMAND, HOST_INFO_VERSION, 0, 0, 101, 0};
    const uint8_t ram[32]     = {HOST_INFO_COMMAND, HOST_INFO_VERSION, 0, 0, 0, 101};
    CHECK(host_info_receive(&info, version, sizeof(version), 9));
    CHECK(host_info_receive(&info, hour, sizeof(hour), 9));
    CHECK(host_info_receive(&info, minute, sizeof(minute), 9));
    CHECK(host_info_receive(&info, cpu, sizeof(cpu), 9));
    CHECK(host_info_receive(&info, ram, sizeof(ram), 9));
    CHECK(host_info_receive(&info, version, 3, 9));
    CHECK(info.hour == 1 && info.received_at == 5);
}

static void test_goes_stale(void) {
    host_info_t   info     = {0};
    const uint8_t data[32] = {HOST_INFO_COMMAND, HOST_INFO_VERSION, 0, 0, 0, 0};
    CHECK(!host_info_fresh(&info, 0));
    host_info_receive(&info, data, sizeof(data), 0xFFFFF000u);
    CHECK(host_info_fresh(&info, 0xFFFFF000u + HOST_INFO_TIMEOUT_MS - 1));
    CHECK(!host_info_fresh(&info, 0xFFFFF000u + HOST_INFO_TIMEOUT_MS));
}

int main(void) {
    test_accepts_report();
    test_leaves_other_commands_to_via();
    test_consumes_but_ignores_bad_reports();
    test_goes_stale();
    return CHECK_DONE();
}
