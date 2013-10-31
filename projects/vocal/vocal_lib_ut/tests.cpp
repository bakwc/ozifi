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

TEST(vocal_lib, AsymmetricalEncription) {
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

TEST(vocal_lib, SymmetricalEncryption) {
    string password = "absdef";
    string data = "Some text message!";

    string encrypted = EncryptSymmetrical(GenerateKey(password), data);
    string decrypted = DecryptSymmetrical(GenerateKey(password), encrypted);
    ASSERT_NE(data, encrypted);
    ASSERT_EQ(data, decrypted);
}

TEST(vocal_lib, SymmetricalEncryptionBig) {
    string password = "qwerty";
    string data = GenerateRandomSequence(4096);
    ASSERT_EQ(data.size(), 4096);

    string encrypted = EncryptSymmetrical(GenerateKey(password), data);
    string decrypted = DecryptSymmetrical(GenerateKey(password), encrypted);
    ASSERT_NE(data, encrypted);
    ASSERT_EQ(data, decrypted);
}

TEST(vocal_lib, LittleHashTest) {
    string message1 = "abc oO";
    string message2 = "abc Oo";
    string message3 = "abc oO";
    ASSERT_NE(LittleHash(message1), LittleHash(message2));
    ASSERT_EQ(LittleHash(message1), LittleHash(message3));
}

int main(int argc, char **argv) {
    setlocale(LC_CTYPE, "en_US.UTF-8");
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
