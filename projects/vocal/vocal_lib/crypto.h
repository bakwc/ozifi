#pragma once

#include <utility>
#include <string>
#include "defines.h"

namespace NVocal {

std::pair<std::string /*privKey*/, std::string /*pubKey*/> GenerateKeys();
std::string GenerateKey(); // key for symmetrical crypto
std::string GenerateKey(const std::string& password); // password-based key for symmetrical crypto
std::string EncryptAsymmetrical(const std::string& pubKey, const std::string& data);
std::string DecryptAsymmetrical(const std::string& privKey, const std::string& data);
std::string EncryptSymmetrical(const std::string& key, const std::string& data);
std::string DecryptSymmetrical(const std::string& key, const std::string& data);
std::string Sign(const std::string& privKey, const std::string& message);
bool CheckSignature(const std::string& pubKey,
                    const std::string& message,
                    const std::string& signature);
std::string GenerateRandomSequence(size_t length = DEFAULT_RANDOM_SEQUNCE_BITS);
std::string Hash(const std::string& data);
ui64 LittleHash(const std::string& data);

}
