#ifndef PASSWORD_GENERATOR_H
#define PASSWORD_GENERATOR_H

#include <string>

class PasswordGenerator {
public:
    static std::string generate(
        size_t length,
        bool useUpper,
        bool useLower,
        bool useDigits,
        bool useSpecial
    );
};

#endif // PASSWORD_GENERATOR_H
