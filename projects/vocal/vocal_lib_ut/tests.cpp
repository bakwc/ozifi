// tests.cpp

#include <boost/filesystem/operations.hpp>
#include <gtest/gtest.h>

#include <projects/vocal/vocal_lib/crypto.h>
#include <projects/vocal/vocal_lib/compress.h>

using namespace std;
using namespace boost::filesystem;
using namespace NVocal;

TEST(vocal_lib, Compression) {
    string data = "Hello, compression!";
    string compressed = Compress(data);
    string decompressed = Decompress(compressed);
    ASSERT_EQ(data, decompressed);
}

TEST(vocal_lib, AssymetricalEncription) {
    string data = "Hello, crypto!";

    pair<string, string> keys = GenerateKeys();
    string encrypted = EncryptAsymmetrical(keys.second, data);
    string decrypted = DecryptAsymmetrical(keys.first, encrypted);

    ASSERT_EQ(data, decrypted);
}

TEST(vocal_lib, Signature) {
    string data = "Hello, crypto!";
    string badData = "Corrupted, crypto!";
    pair<string, string> keys = GenerateKeys();
    string signature = Sign(keys.first, data);

    ASSERT_TRUE(CheckSignature(keys.second, data, signature));
    ASSERT_FALSE(CheckSignature(keys.second, badData, signature));
}

int main(int argc, char **argv) {
    setlocale(LC_CTYPE, "en_US.UTF-8");
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
