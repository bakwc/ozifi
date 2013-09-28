#pragma once

#include <string>
#include <boost/optional.hpp>
#include <boost/noncopyable.hpp>
#include <utils/unused.h>

namespace NKwStorage {

class TKwStorage: public boost::noncopyable {
public:
    virtual void Put(const std::string& key, const std::string& value) {
        UNUSED(key);
        UNUSED(value);
    }
    virtual boost::optional<std::string> Get(const std::string& key) {
        UNUSED(key);
        return boost::optional<std::string>();
    }
    virtual bool Exists(const std::string& key) {
        return false;
    }
    virtual void Flush() {
    }
protected:
    TKwStorage() {
    }
};

} // NKwStorage
