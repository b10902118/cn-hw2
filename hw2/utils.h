#pragma once

#include <cstddef>
#include <string>
#include <cstring>
#define ERR_EXIT(a)                                                         \
    {                                                                       \
        perror(a);                                                          \
        exit(1);                                                            \
    }

/*
bool icaseCmp(const char *str1, const char *str2) {
    // Check for null pointers
    if (str1 == nullptr || str2 == nullptr) {
        return false;
    }

    // Use strncasecmp for case-insensitive comparison
    return strncasecmp(str1, str2,
                       std::min(std::strlen(str1), std::strlen(str2))) == 0;
}
*/

bool icaseCmp(const std::string &str1, const std::string &str2);

bool icaseNCmp(const std::string &str1, const std::string &str2, size_t n);
