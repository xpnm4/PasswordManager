#include "PasswordGenerator.h"
#include <random>
#include <vector>
#include <stdexcept>
#include <algorithm>

std::string PasswordGenerator::generate(
    size_t length,
    bool useUpper,
    bool useLower,
    bool useDigits,
    bool useSpecial
) {
    if (length < 8 || length > 64) {
        throw std::invalid_argument("Password length must be between 8 and 64 characters.");
    }

    std::string upperChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string lowerChars = "abcdefghijklmnopqrstuvwxyz";
    std::string digitChars = "0123456789";
    std::string specialChars = "!@#$%^&*()_+-=[]{}|;:,.<>?";

    std::string allowedPool = "";
    std::vector<std::string> mandatoryPools;

    if (useUpper) {
        allowedPool += upperChars;
        mandatoryPools.push_back(upperChars);
    }
    if (useLower) {
        allowedPool += lowerChars;
        mandatoryPools.push_back(lowerChars);
    }
    if (useDigits) {
        allowedPool += digitChars;
        mandatoryPools.push_back(digitChars);
    }
    if (useSpecial) {
        allowedPool += specialChars;
        mandatoryPools.push_back(specialChars);
    }

    if (allowedPool.empty()) {
        throw std::invalid_argument("At least one character set must be selected.");
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    
    std::string password = "";
    
    // Ensure we have at least one character from each selected category
    for (const auto& pool : mandatoryPools) {
        std::uniform_int_distribution<size_t> dist(0, pool.size() - 1);
        password += pool[dist(gen)];
    }

    // Fill the rest of the password length
    std::uniform_int_distribution<size_t> distAll(0, allowedPool.size() - 1);
    while (password.size() < length) {
        password += allowedPool[distAll(gen)];
    }

    // Shuffle the password characters to randomize positions of mandatory chars
    std::shuffle(password.begin(), password.end(), gen);

    return password;
}
