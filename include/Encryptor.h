#ifndef ENCRYPTOR_H
#define ENCRYPTOR_H

#include <string>
#include <vector>

class Encryptor {
public:
    // Derives a 32-byte key from a master password using SHA-256
    static std::vector<uint8_t> deriveKey(const std::string& password);

    // Encrypts plaintext and returns a hex-encoded string containing IV + encrypted data
    static std::string encrypt(const std::string& plaintext, const std::vector<uint8_t>& key);

    // Decrypts a hex-encoded string containing IV + encrypted data
    static std::string decrypt(const std::string& ciphertextHex, const std::vector<uint8_t>& key);

    // Hex encoding/decoding helpers
    static std::string toHex(const std::vector<uint8_t>& data);
    static std::vector<uint8_t> fromHex(const std::string& hex);
};

#endif // ENCRYPTOR_H
