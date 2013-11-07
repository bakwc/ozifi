#include <limits>
#include <chrono>

#include "date_time.h"

TDuration::TDuration()
    : Value(0)
{
}

TDuration::TDuration(ui64 value)
    : Value(value)
{
}

ui64 TDuration::GetValue() {
    return Value;
}

ui64 TDuration::MicroSeconds() {
    return Value;
}

ui64 TDuration::MilliSeconds() {
    return Value / 1000;
}

ui64 TDuration::Seconds() {
    return Value / 1000000;
}

ui64 TDuration::Minutes() {
    return Value / 60000000;
}

ui64 TDuration::Hours() {
    return Value / 3600000000;
}

ui64 TDuration::Days() {
    return Value / 86400000000;
}

TDuration TDuration::MicroSeconds(ui64 microSeconds) {
    return TDuration(microSeconds);
}

TDuration TDuration::MilliSeconds(ui64 milliSeconds) {
    return TDuration(milliSeconds * 1000);
}

TDuration TDuration::Seconds(ui64 seconds) {
    return TDuration(seconds * 1000000);
}

TDuration TDuration::Minutes(ui64 minutes) {
    return TDuration(minutes * 60000000);
}

TDuration TDuration::Hours(ui64 hours) {
    return TDuration(hours * 3600000000);
}

TDuration TDuration::Days(ui64 days) {
    return TDuration(days * 86400000000);
}

TDuration TDuration::Now() {
    auto now = std::chrono::system_clock::now().time_since_epoch();
    return TDuration(std::chrono::duration_cast<std::chrono::microseconds>(now).count());
}

TDuration TDuration::Min() {
    return TDuration(0);
}

TDuration TDuration::Max() {
    return TDuration(std::numeric_limits<ui64>::max());
}

TDuration Now() {
    return TDuration::Now();
}
