#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace Fs {
void init();
std::vector<std::string> readLines(const std::string &filePath);
std::vector<char> readBinary(const std::string &filePath);
std::string readText(const std::string &filePath);
std::vector<std::string> listDir(const std::string &dirPath);

bool createDirectory(const std::string &path);
bool fileExists(const std::string &filePath);

extern const std::unordered_map<std::string, std::string> mimeTypes;
std::string getMimeType(const std::string &filePath);
} // namespace Fs
