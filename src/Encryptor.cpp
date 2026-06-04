#include "Encryptor.h"
#include "crypto/picosha2.h"

// Set AES256 definition before including aes.h
#ifndef AES256
#define AES256 1
#endif
extern "C" {
#include "crypto/aes.h"
}

#include <random>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <cstring>

std::vector<uint8_t> Encryptor::deriveKey(const std::string& password) {
    std::vector<uint8_t> key(32);
    picosha2::hash256(password.begin(), password.end(), key.begin(), key.end());
    return key;
}

std::string Encryptor::toHex(const std::vector<uint8_t>& data) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (uint8_t b : data) {
        ss << std::setw(2) << static_cast<int>(b);
    }
    return ss.str();
}

std::vector<uint8_t> Encryptor::fromHex(const std::string& hex) {
    std::vector<uint8_t> data;
    if (hex.length() % 2 != 0) {
        throw std::invalid_argument("Hex string must have an even length.");
    }
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(strtol(byteString.c_str(), nullptr, 16));
        data.push_back(byte);
    }
    return data;
}

std::string Encryptor::encrypt(const std::string& plaintext, const std::vector<uint8_t>& key) {
    if (key.size() != 32) {
        throw std::invalid_argument("AES-256 key must be 32 bytes.");
    }

    // Generate random 16-byte IV
    std::vector<uint8_t> iv(16);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 255);
    for (int i = 0; i < 16; ++i) {
        iv[i] = static_cast<uint8_t>(dist(gen));
    }

    // Prepare plaintext with PKCS#7 padding
    std::vector<uint8_t> buffer(plaintext.begin(), plaintext.end());
    size_t padLength = 16 - (buffer.size() % 16);
    for (size_t i = 0; i < padLength; ++i) {
        buffer.push_back(static_cast<uint8_t>(padLength));
    }

    // Initialize AES-256-CBC context
    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key.data(), iv.data());

    // Encrypt in-place
    AES_CBC_encrypt_buffer(&ctx, buffer.data(), static_cast<uint32_t>(buffer.size()));

    // Prepend IV to ciphertext
    std::vector<uint8_t> ivAndCiphertext = iv;
    ivAndCiphertext.insert(ivAndCiphertext.end(), buffer.begin(), buffer.end());

    return toHex(ivAndCiphertext);
}

std::string Encryptor::decrypt(const std::string& ciphertextHex, const std::vector<uint8_t>& key) {
    if (key.size() != 32) {
        throw std::invalid_argument("AES-256 key must be 32 bytes.");
    }

    std::vector<uint8_t> data = fromHex(ciphertextHex);
    if (data.size() < 32) { // 16 bytes IV + at least 16 bytes padded ciphertext
        throw std::runtime_error("Invalid or corrupted ciphertext (too short).");
    }

    // Extract IV (first 16 bytes)
    std::vector<uint8_t> iv(data.begin(), data.begin() + 16);
    // Extract Ciphertext
    std::vector<uint8_t> ciphertext(data.begin() + 16, data.end());

    if (ciphertext.size() % 16 != 0) {
        throw std::runtime_error("Ciphertext length must be a multiple of 16.");
    }

    // Initialize AES-256-CBC context
    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key.data(), iv.data());

    // Decrypt in-place
    AES_CBC_decrypt_buffer(&ctx, ciphertext.data(), static_cast<uint32_t>(ciphertext.size()));

    // Verify and remove PKCS#7 padding
    if (ciphertext.empty()) {
        throw std::runtime_error("Decrypted buffer is empty.");
    }
    
    uint8_t padLength = ciphertext.back();
    if (padLength < 1 || padLength > 16) {
        throw std::runtime_error("Invalid PKCS#7 padding (incorrect size).");
    }

    for (size_t i = ciphertext.size() - padLength; i < ciphertext.size(); ++i) {
        if (ciphertext[i] != padLength) {
            throw std::runtime_error("Invalid PKCS#7 padding (mismatched bytes).");
        }
    }

    ciphertext.resize(ciphertext.size() - padLength);

    return std::string(ciphertext.begin(), ciphertext.end());
}
