#include "fs.h"
#include <fstream>
#include <dirent.h>

std::string Fs::readFile(const std::string &filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        // Handle error: unable to open file
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::vector<std::string> Fs::listDir(const std::string &dirPath) {
    std::vector<std::string> files;

    DIR *directory = opendir(dirPath.c_str());
    if (!directory) {
        // Handle error: unable to open directory
        return files;
    }

    dirent *entry;
    while ((entry = readdir(directory)) != nullptr) {
        std::string fileName = entry->d_name;
        if (fileName != "." && fileName != "..") {
            files.push_back(fileName);
        }
    }

    closedir(directory);
    return files;
}
