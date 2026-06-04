#include "FileStorage.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

FileStorage::FileStorage(std::string filepath) : filepath(std::move(filepath)) {}

bool FileStorage::exists() const {
    std::ifstream file(filepath);
    return file.good();
}

void FileStorage::save(const std::string& masterHash, const std::string& ciphertext) {
    json vaultJson;
    vaultJson["master_hash"] = masterHash;
    vaultJson["ciphertext"] = ciphertext;

    std::ofstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open vault file for writing: " + filepath);
    }

    file << vaultJson.dump(4); // Indented JSON
}

bool FileStorage::load(std::string& masterHash, std::string& ciphertext) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return false;
    }

    try {
        json vaultJson;
        file >> vaultJson;

        if (vaultJson.contains("master_hash") && vaultJson.contains("ciphertext")) {
            masterHash = vaultJson["master_hash"].get<std::string>();
            ciphertext = vaultJson["ciphertext"].get<std::string>();
            return true;
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Vault file is corrupted or invalid: " + std::string(e.what()));
    }

    return false;
}
