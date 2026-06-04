#ifndef FILE_STORAGE_H
#define FILE_STORAGE_H

#include <string>

class FileStorage {
private:
    std::string filepath;

public:
    explicit FileStorage(std::string filepath);

    // Checks if the vault file exists
    bool exists() const;

    // Saves the encrypted vault data (master password hash and encrypted entries ciphertext)
    void save(const std::string& masterHash, const std::string& ciphertext);

    // Loads the encrypted vault data
    bool load(std::string& masterHash, std::string& ciphertext);
};

#endif // FILE_STORAGE_H
