#pragma once

#include <string>
#include <chrono>
#include <functional>
#include <boost/array.hpp>

struct TMessage {
    std::string Message;
    std::string From;                       // sender login
    std::string To;                         // destination login or chat id
    std::chrono::microseconds Timestamp;    // sender timestamp (consider is real; auto-synced with server)
    boost::array<char, 16> UID;             // random id
};

typedef std::function<void(const std::string&, const TMessage&)> TMessageCallback;
