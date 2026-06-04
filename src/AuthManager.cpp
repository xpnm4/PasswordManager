#include "AuthManager.h"
#include "crypto/picosha2.h"

std::string AuthManager::hashPassword(const std::string& password) {
    return picosha2::hash256_hex_string(password);
}

bool AuthManager::verifyPassword(const std::string& password, const std::string& storedHash) {
    std::string enteredHash = hashPassword(password);
    return (enteredHash == storedHash);
}
