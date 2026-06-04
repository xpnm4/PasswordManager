#include "PasswordManager.h"
#include "AuthManager.h"
#include "Encryptor.h"
#include "nlohmann/json.hpp"
#include <chrono>
#include <ctime>
#include <algorithm>
#include <stdexcept>

using json = nlohmann::json;

// Define custom serialization for PasswordEntry in global namespace for ADL
void to_json(json& j, const PasswordEntry& e) {
    j = json{
        {"serviceName", e.serviceName},
        {"username", e.username},
        {"password", e.password},
        {"url", e.url},
        {"note", e.note},
        {"createdAt", e.createdAt},
        {"updatedAt", e.updatedAt}
    };
}

void from_json(const json& j, PasswordEntry& e) {
    e.serviceName = j.at("serviceName").get<std::string>();
    e.username = j.at("username").get<std::string>();
    e.password = j.at("password").get<std::string>();
    e.url = j.contains("url") ? j.at("url").get<std::string>() : "";
    e.note = j.contains("note") ? j.at("note").get<std::string>() : "";
    e.createdAt = j.contains("createdAt") ? j.at("createdAt").get<std::string>() : "";
    e.updatedAt = j.contains("updatedAt") ? j.at("updatedAt").get<std::string>() : "";
}

namespace {
    // Helper for case-insensitive string comparison
    bool containsIgnoreCase(std::string str, std::string query) {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        std::transform(query.begin(), query.end(), query.begin(), ::tolower);
        return (str.find(query) != std::string::npos);
    }
}


PasswordManager::PasswordManager(const std::string& filepath) 
    : storage(filepath), isUnlocked(false) {}

bool PasswordManager::hasVault() const {
    return storage.exists();
}

std::string PasswordManager::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm* tm_ptr = std::localtime(&now_time);
    char buf[80];
    if (tm_ptr) {
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_ptr);
        return std::string(buf);
    }
    return "";
}

void PasswordManager::createVault(const std::string& masterPassword) {
    if (masterPassword.empty()) {
        throw std::invalid_argument("Master password cannot be empty.");
    }
    masterHash = AuthManager::hashPassword(masterPassword);
    masterKey = Encryptor::deriveKey(masterPassword);
    isUnlocked = true;
    entries.clear();
    
    // Save an empty list of entries encrypted
    save();
}

bool PasswordManager::unlock(const std::string& masterPassword) {
    std::string loadedHash;
    std::string loadedCiphertext;

    if (!storage.load(loadedHash, loadedCiphertext)) {
        return false;
    }

    if (!AuthManager::verifyPassword(masterPassword, loadedHash)) {
        return false;
    }

    // Password matches hash, now decrypt the entries
    masterHash = loadedHash;
    masterKey = Encryptor::deriveKey(masterPassword);

    try {
        std::string decryptedJson = Encryptor::decrypt(loadedCiphertext, masterKey);
        json j = json::parse(decryptedJson);
        entries = j.get<std::vector<PasswordEntry>>();
        isUnlocked = true;
        return true;
    } catch (const std::exception& e) {
        // Even if hash matched, decryption failed (e.g. file corrupted)
        lock();
        throw std::runtime_error("Failed to decrypt vault content: " + std::string(e.what()));
    }
}

void PasswordManager::lock() {
    entries.clear();
    // Zero-out keys for security
    std::fill(masterKey.begin(), masterKey.end(), 0);
    masterKey.clear();
    masterHash.clear();
    isUnlocked = false;
}

void PasswordManager::save() {
    if (!isUnlocked) {
        throw std::runtime_error("Vault is locked. Cannot save changes.");
    }

    json j = entries;
    std::string plaintext = j.dump();
    std::string ciphertext = Encryptor::encrypt(plaintext, masterKey);
    storage.save(masterHash, ciphertext);
}

void PasswordManager::addEntry(const PasswordEntry& entry) {
    if (!isUnlocked) {
        throw std::runtime_error("Vault is locked.");
    }

    // Verify service name doesn't already exist
    auto it = std::find_if(entries.begin(), entries.end(), [&entry](const PasswordEntry& e) {
        return e.serviceName == entry.serviceName;
    });

    if (it != entries.end()) {
        throw std::runtime_error("An entry for '" + entry.serviceName + "' already exists.");
    }

    PasswordEntry newEntry = entry;
    newEntry.createdAt = getCurrentTimestamp();
    newEntry.updatedAt = newEntry.createdAt;
    entries.push_back(newEntry);
}

bool PasswordManager::editEntry(const std::string& serviceName, const PasswordEntry& newDetails) {
    if (!isUnlocked) {
        throw std::runtime_error("Vault is locked.");
    }

    auto it = std::find_if(entries.begin(), entries.end(), [&serviceName](const PasswordEntry& e) {
        return e.serviceName == serviceName;
    });

    if (it == entries.end()) {
        return false;
    }

    // Preserve creation date
    std::string oldCreatedAt = it->createdAt;

    *it = newDetails;
    it->createdAt = oldCreatedAt;
    it->updatedAt = getCurrentTimestamp();
    return true;
}

bool PasswordManager::deleteEntry(const std::string& serviceName) {
    if (!isUnlocked) {
        throw std::runtime_error("Vault is locked.");
    }

    auto it = std::find_if(entries.begin(), entries.end(), [&serviceName](const PasswordEntry& e) {
        return e.serviceName == serviceName;
    });

    if (it == entries.end()) {
        return false;
    }

    entries.erase(it);
    return true;
}

std::vector<PasswordEntry> PasswordManager::searchEntries(const std::string& serviceQuery) const {
    if (!isUnlocked) {
        throw std::runtime_error("Vault is locked.");
    }

    std::vector<PasswordEntry> results;
    for (const auto& e : entries) {
        if (containsIgnoreCase(e.serviceName, serviceQuery)) {
            results.push_back(e);
        }
    }
    return results;
}

std::vector<PasswordEntry> PasswordManager::getAllEntries() const {
    if (!isUnlocked) {
        throw std::runtime_error("Vault is locked.");
    }
    return entries;
}
