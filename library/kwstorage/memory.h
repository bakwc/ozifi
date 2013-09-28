#pragma once

#include <memory>

#include "kwstorage.h"

namespace NKwStorage {

struct TMemoryStorageOptions {
    TMemoryStorageOptions()
        : ThreadSafe(false)
    {
    }
    boost::optional<std::string> FileName; // File name to store data
    bool ThreadSafe;                       // Single / multithread usage
};

/**
 * @brief Creates im-memory storage (hash-based)
 * @param options - MemoryStorage options
 * @return - KwStorage interface
 */
TKwStorage* CreateMemoryStorage(const TMemoryStorageOptions& options = TMemoryStorageOptions());

} // NKwStorage
