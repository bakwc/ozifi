#pragma once

#include <string>
#include <chrono>
#include <functional>
#include <boost/array.hpp>

#include <utils/date_time.h>
#include <utils/cast.h>
#include <utils/pack.h>

#include <projects/vocal/vocal_lib/crypto.h>

namespace NVocal {

struct TMessage {
    std::string Text;
    std::string From;                       // sender login
    std::string To;                         // destination login or chat id
    TDuration Time;                         // sender timestamp (consider is real; auto-synced with server)
    inline std::string CalcSignature() const {
        ui64 hash = LittleHash(Text + From + To);
        ui64 time = Time.GetValue();
        return Pack(time) + Pack(hash);
    }
};

typedef std::function<void(const TMessage&)> TMessageCallback;

} // NVocal
