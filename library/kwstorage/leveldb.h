#pragma once

#include "kwstorage.h"

namespace NKwStorage {

struct TLevelDbStorageOptions {
    TLevelDbStorageOptions()
        : CreateUnexistDirectory(true)
    {
    }
    std::string DirectoryName;    // Directory to store internal leveldb files
    bool CreateUnexistDirectory;  // Create directory if it's not exists
};

/**
 * @brief Creates leveldb-based disk storage
 * @param options - leveldb options
 * @return - KwStorage interface
 */
TKwStorage* CreateLevelDbStorage(const TLevelDbStorageOptions& options = TLevelDbStorageOptions());

} // NKwStorage
