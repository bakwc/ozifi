#include <contrib/cryptopp/cryptlib.h>
#include <contrib/cryptopp/osrng.h>
#include <contrib/cryptopp/rsa.h>
#include "crypto.h"

// todo: implement this

using namespace CryptoPP;
using namespace std;

namespace NVocal {

pair<string /*privKey*/, string /*pubKey*/> GenerateKeys() {
    AutoSeededRandomPool rng;
    InvertibleRSAFunction params;
    params.GenerateRandomWithKeySize(rng, DEFAULT_ASSYMETRICAL_KEY_LENGTH);

    RSA::PrivateKey privateKey(params);
    RSA::PublicKey publicKey(params);


    string privKey;
    string pubKey;
    privateKey.Save(StringSink(privKey).Ref());
    publicKey.Save(StringSink(pubKey).Ref());

    return pair<string, string>(privKey, pubKey);
}

std::string EncryptAsymmetrical(const std::string& pubKey, const std::string& data) {
    assert(!data.empty());
    string result;

    AutoSeededRandomPool rng;
    RSA::PublicKey publicKey;
    publicKey.Load(StringSource(pubKey, true).Ref());
    RSAES_OAEP_SHA_Encryptor e(publicKey);
    StringSource ss(data, true,
                    new PK_EncryptorFilter(rng, e,
                    new StringSink(result)));
    assert(!result.empty());
    return result;
}

std::string DecryptAsymmetrical(const std::string& privKey, const std::string& data) {
    assert(!data.empty());
    string result;

    AutoSeededRandomPool rng;
    RSA::PrivateKey privateKey;
    privateKey.Load(StringSource(privKey, true).Ref());
    RSAES_OAEP_SHA_Decryptor d(privateKey);
    StringSource ss(data, true,
                    new PK_DecryptorFilter(rng, d,
                    new StringSink(result)));

    assert(!result.empty());
    return result;
}

std::string EncryptSymmetrical(const std::string& key, const std::string& data) {
}

std::string DecryptSymmetrical(const std::string& key, const std::string& data) {
}

std::string GenerateRandomSequence(size_t length) {
}

std::string Hash(const std::string& data) {
}

std::string Sign(const std::string& privKey, const std::string& message) {
    AutoSeededRandomPool rng;
    string signature;
    RSA::PrivateKey privateKey;
    privateKey.Load(StringSource(privKey, true).Ref());
    RSASSA_PKCS1v15_SHA_Signer signer(privateKey);
    StringSource(message, true,
                 new SignerFilter(rng, signer,
                 new StringSink(signature)));
    return signature;
}

bool CheckSignature(const std::string& pubKey,
                    const std::string& message,
                    const std::string& signature)
{
    RSA::PublicKey publicKey;
    publicKey.Load(StringSource(pubKey, true).Ref());

    RSASSA_PKCS1v15_SHA_Verifier verifier(publicKey);
    try {
        StringSource(message+signature, true,
                     new SignatureVerificationFilter(
                     verifier, NULL,
                     SignatureVerificationFilter::THROW_EXCEPTION));
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

std::string GenerateKey()
{
}

}
