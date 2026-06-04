#ifndef PASSWORD_MANAGER_H
#define PASSWORD_MANAGER_H

#include "PasswordEntry.h"
#include "FileStorage.h"
#include <vector>
#include <string>

class PasswordManager {
private:
    std::vector<PasswordEntry> entries;
    FileStorage storage;
    std::string masterHash;
    std::vector<uint8_t> masterKey;
    bool isUnlocked;

    // Helper to get current timestamp string
    std::string getCurrentTimestamp() const;

public:
    explicit PasswordManager(const std::string& filepath);

    // Checks if a vault file already exists
    bool hasVault() const;

    // Initializes a new vault with a master password
    void createVault(const std::string& masterPassword);

    // Unlocks the vault with the master password
    bool unlock(const std::string& masterPassword);

    // Locks the vault and clears keys from memory
    void lock();

    // Saves changes to the vault file
    void save();

    // Check if vault is currently unlocked
    bool unlocked() const { return isUnlocked; }

    // Password entries operations
    void addEntry(const PasswordEntry& entry);
    bool editEntry(const std::string& serviceName, const PasswordEntry& newDetails);
    bool deleteEntry(const std::string& serviceName);
    
    std::vector<PasswordEntry> searchEntries(const std::string& serviceQuery) const;
    std::vector<PasswordEntry> getAllEntries() const;
};

#endif // PASSWORD_MANAGER_H
