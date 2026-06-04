#ifndef AUTH_MANAGER_H
#define AUTH_MANAGER_H

#include <string>

class AuthManager {
public:
    // Hashes a master password using SHA-256
    static std::string hashPassword(const std::string& password);

    // Verifies a master password against a stored SHA-256 hash
    static bool verifyPassword(const std::string& password, const std::string& storedHash);
};

#endif // AUTH_MANAGER_H
