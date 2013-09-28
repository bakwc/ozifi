#include <memory>
#include <contrib/leveldb/db.h>
#include <utils/exception.h>

#include "leveldb.h"

namespace NKwStorage {

class TLevelDbStorage: public TKwStorage {
public:
    TLevelDbStorage(const TLevelDbStorageOptions& options) {
        leveldb::DB* db;
        leveldb::Options dbOptions;
        dbOptions.create_if_missing = options.CreateUnexistDirectory;
        leveldb::Status status = leveldb::DB::Open(dbOptions, options.DirectoryName, &db);
        if (!status.ok()) {
            throw UException(status.ToString());
        }
        Db.reset(db);
    }
    void Put(const std::string& key, const std::string& value) {
        leveldb::Status status;
        status = Db->Put(leveldb::WriteOptions(),
                         leveldb::Slice(key.c_str(), key.size()),
                         leveldb::Slice(value.c_str(), value.size()));
        if (!status.ok()) {
            throw UException(status.ToString());
        }
    }
    boost::optional<std::string> Get(const std::string& key) {
        leveldb::Status status;
        std::string value;
        status = Db->Get(leveldb::ReadOptions(),
                         leveldb::Slice(key.c_str(), key.size()), &value);
        if (status.ok()) {
            return value;
        } else {
            if (!status.IsNotFound()) {
                throw UException(status.ToString());
            }
        }
        return boost::optional<std::string>();
    }
    bool Exists(const std::string& key) {
        leveldb::Status status;
        std::string value;
        status = Db->Get(leveldb::ReadOptions(),
                         leveldb::Slice(key.c_str(), key.size()), &value);
        if (status.ok()) {
            return true;
        } else {
            if (!status.IsNotFound()) {
                throw UException(status.ToString());
            }
        }
        return false;
    }
private:
    std::unique_ptr<leveldb::DB> Db;
};

TKwStorage* CreateLevelDbStorage(const TLevelDbStorageOptions& options) {
    return new TLevelDbStorage(options);
}

} // NKwStorage
