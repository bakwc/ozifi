#include "crypto.h"

// todo: implement this

namespace NVocal {

std::pair<std::string /*privKey*/, std::string /*pubKey*/> GenerateKeys() {
}

std::string EncryptAsymmetrical(const std::string& pubKey, const std::string& data) {
}

std::string DecryptAsymmetrical(const std::string& privKey, const std::string& data) {
}

std::string EncryptSymmetrical(const std::string& key, const std::string& data) {
}

std::string DecryptSymmetrical(const std::string& key, const std::string& data) {
}

std::string GenerateRandomSequence(size_t length) {
}

std::string Hash(const std::string &data)
{
}

std::string Sign(const std::string &privKey, const std::string &message)
{
}

bool CheckSignature(const std::string &pubKey, const std::string &message, const std::string &signature)
{
}

std::string GenerateKey()
{
}

}
