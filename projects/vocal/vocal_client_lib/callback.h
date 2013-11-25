#pragma once

#include <functional>
#include <string>
#include <utils/buffer.h>

#include <projects/vocal/vocal_lib/vocal.pb.h>
#include <projects/vocal/vocal_lib/defines.h>

#include "message.h"

/** This file containes callbacks declarations */

namespace NVocal {

typedef std::function<void(const TBuffer& /*data*/)> TDataCallback;
typedef std::function<void(const std::string& /*string*/)> TStringCallback;
typedef TDataCallback TNamedCallback;
typedef std::function<void(const std::string& /*name*/, const TBuffer& /*data*/)> TNamedDataCallback;
typedef std::function<std::string(size_t)> TDataRequireCallback;
typedef std::function<void(bool)> TBoolCallback;
typedef std::function<void(const std::string& /*name*/, bool /*result*/)> TNamedBoolCallback;
typedef std::function<void(ELoginResult)> TLoginCallback;
typedef std::function<void(ERegisterResult)> TRegisterCallback;
typedef std::function<void()> TCallBack;
typedef std::function<void(const TMessage&)> TMessageCallback;

} // NVocal
