#pragma once

#include <functional>
#include <string>
#include <utils/buffer.h>

namespace NVocal {

typedef std::function<void(const TBuffer& /*data*/)> TDataCallback;
typedef TDataCallback TNamedCallback;
typedef std::function<void(const std::string& /*name*/, const TBuffer& /*data*/)> TNamedDataCallback;
typedef std::function<std::string(size_t)> TDataRequireCallback;
typedef std::function<void(bool)> TBoolCallback;
typedef std::function<void(const std::string& /*name*/, bool /*result*/)> TNamedBoolCallback;

} // NVocal
