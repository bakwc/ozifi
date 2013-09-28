// todo: implement this

#include "memory.h"
#include "unordered_map"
#include "kwstorage.h"

namespace NKwStorage {

typedef std::unordered_map<std::string, std::string> TData;

class TMemoryStorage: public TKwStorage {
public:
    TMemoryStorage(const TMemoryStorageOptions& options)
        : FileName(options.FileName)
    {
    }
    virtual void Put(const std::string& key, const std::string& value) {
        Data[key] = value;
    }
    virtual boost::optional<std::string> Get(const std::string& key) {
        TData::iterator it = Data.find(key);
        if (it != Data.end()) {
            return it->second;
        }
        return boost::optional<std::string>();
    }
    virtual bool Exists(const std::string& key) {
        return Data.find(key) != Data.end();
    }
private:
    boost::optional<std::string> FileName;
    TData Data;
};

// todo: implement thread-safe storage
class TMemoryStorageThreadsafe: public TMemoryStorage {
public:
private:
};

TKwStorage* CreateMemoryStorage(const TMemoryStorageOptions& options) {
    return new TMemoryStorage(options);
}

} // NKwStorage
