#pragma once

#include "types.h"

class TDuration {
public:
    TDuration();
    TDuration(ui64 value);
    ui64 GetValue() const;
    ui64 MicroSeconds();
    ui64 MilliSeconds();
    ui64 Seconds();
    ui64 Minutes();
    ui64 Hours();
    ui64 Days();
    static TDuration MicroSeconds(ui64 microSeconds);
    static TDuration MilliSeconds(ui64 milliSeconds);
    static TDuration Seconds(ui64 seconds);
    static TDuration Minutes(ui64 minutes);
    static TDuration Hours(ui64 hours);
    static TDuration Days(ui64 days);
    static TDuration Now();
    static TDuration Min();
    static TDuration Max();
private:
    ui64 Value;
};

TDuration Now();
