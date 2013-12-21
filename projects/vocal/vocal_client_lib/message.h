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
public:
    std::string Text;                           // message text
    std::string From;                           // sender login
    std::string To;                             // destination login or chat id
    TDuration Time;                             // sender timestamp (consider is real; auto-synced with server)
public:
    inline std::string CalcSignature() const {  // message signature, unique for each message
        ui64 hash = LittleHash(Text + From + To);
        ui64 time = Time.GetValue();
        return Pack(time) + Pack(hash);
    }
};

} // NVocal
