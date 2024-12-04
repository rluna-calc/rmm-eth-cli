#include <chrono>


int64_t time_since_epoch_seconds() {
    auto now = std::chrono::system_clock::now();
    auto seconds_since_epoch = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    return seconds_since_epoch;
}

int64_t time_since_eqoch_microsecs() {
     auto now = std::chrono::system_clock::now();
    auto microseconds_since_epoch = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
    return microseconds_since_epoch;
}