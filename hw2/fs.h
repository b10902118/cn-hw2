#pragma once
#include <string>
#include <vector>

namespace Fs {
std::string readFile(const std::string &filePath);
std::vector<std::string> listDir(const std::string &dirPath);
// namespace Fs
