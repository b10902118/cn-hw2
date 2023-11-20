#include "utils.h"

bool icaseCmp(const std::string &str1, const std::string &str2) {
    if (str1.length() != str2.length()) return false;
    return strncasecmp(str1.c_str(), str2.c_str(), str1.length()) == 0;
}

bool icaseNCmp(const std::string &str1, const std::string &str2, size_t n) {
    return strncasecmp(str1.c_str(), str2.c_str(), n) == 0;
}

std::string readHtmlFile(const std::string &filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filePath << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::vector<char> readBinaryFile(const std::string &filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filePath << std::endl;
        return {};
    }

    // Determine the file size
    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Read the file content into a vector
    std::vector<char> fileContent(static_cast<size_t>(fileSize));
    file.read(fileContent.data(), fileSize);

    return fileContent;
}

/*
bool icaseCmp(const std::string &str1, const std::string &str2) {
    size_t len1 = std::strlen(str1);
    size_t len2 = std::strlen(str2);
    size_t len = min(len1, len2);

    for (size_t i = 0; i < len; ++i) {
        if (std::tolower(str1[i]) != std::tolower(str2[i])) {
            return false;
        }
    }
    return true;
}
*/
