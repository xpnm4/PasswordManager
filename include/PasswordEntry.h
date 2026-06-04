#ifndef PASSWORD_ENTRY_H
#define PASSWORD_ENTRY_H

#include <string>

struct PasswordEntry {
    std::string serviceName;
    std::string username;
    std::string password;
    std::string url;
    std::string note;
    std::string createdAt;
    std::string updatedAt;
};

#endif // PASSWORD_ENTRY_H
