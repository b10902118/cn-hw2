#include "fs.h"
#include <fstream>
#include <dirent.h>
#include <sstream>
#include <unordered_map>
#include <sys/stat.h>
#include <iostream>
#include <filesystem>
#include <unistd.h>
#include <cstdlib>
#include <limits.h> // For PATH_MAX

using namespace std;

namespace Fs {

const std::string FileRoot = "web/files/";
const std::string VideoRoot = "web/videos/";
const std::string TmpDir = "web/tmp/";
const std::string baseDir = "./";

void init() {
    if (!createDirectory(TmpDir)) std::cerr << "cannot create dir";
    if (!createDirectory(FileRoot)) std::cerr << "cannot create dir";
    if (!createDirectory(VideoRoot)) std::cerr << "cannot create dir";
}

std::string validPath(const std::string root, const std::string filePath) {
    char resolvedPath[PATH_MAX];
    static const std::string fullBase = [&resolvedPath] {
        char *absBase = realpath(baseDir.c_str(), resolvedPath);
        if (absBase == nullptr) {
            std::cerr << "realpath(baseDir) error" << std::endl;
        }
        return std::string(absBase);
    }();

    // Use realpath to get the absolute path
    std::string fullPath = root + filePath;
    // cerr << "fullPath" << fullPath << endl;
    char *absolutePath = realpath(fullPath.c_str(), resolvedPath);
    // cerr << "resolved " << absolutePath << endl;
    if (absolutePath == nullptr) { // if the path does not exist
        return "";
    }
    std::string absPath = std::string(absolutePath);
    std::string fullRoot = fullBase + "/" + root;
    // cerr << "fullBase " << fullBase << endl;
    // std::cerr << "absPath " << absPath << std::endl;
    // cerr << "fullRoot " << fullRoot << endl;

    // Check if the resolved path is within the root path
    if (absPath.compare(0, fullBase.length(), fullBase) == 0) {
        return absPath;
    }
    else {
        return "";
    }
}

void parse_upload() {
    //  TODO write to file, handle boundary & content length
    //  & filesize state: file too big
    /*
// parse file
std::ifstream file("./web/tmp/" + request.tmpName, std::ios::binary);
std::string line, disposition;
getline(file, line);
getline(file, disposition);
// extract name
size_t pos = disposition.find("filename=\"");
if (pos == string::npos) cerr << "disposition no filename" << endl;
pos += strlen("filename=\"");
request.filePath = disposition.substr(pos, disposition.length() - strlen("\"\r\n") - pos);
Fs::createDirectory("./web/videos/" + request.filePath);

getline(file, line);
getline(file, line);
bool first = true;
string fullPath = "./web/tmp/" + request.filePath;
while (std::getline(file, line, '\r')) {
    if (line.substr(1) == request.boundary) break;
    if (!first) {
        line = "\r" + line;
        first = false;
    }
    Fs::appendData(fullPath, line.c_str(), line.length());
}
pid_t pid = fork();

if (pid == -1) {
    // Handle fork error
    std::cerr << "Error forking process." << std::endl;
    return 1;
}
else if (pid == 0) {
    // This is the child process
    execl("/bin/sh", "sh", "-c", command, nullptr);
    // If execl fails
    std::cerr << "Error executing the command." << std::endl;
    _exit(1);
}
else {
}
    */
}

std::vector<std::string> readLines(const std::string &filePath) {
    std::vector<std::string> lines;
    std::ifstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filePath << std::endl;
        return lines; // Return an empty vector on error
    }

    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    file.close();
    return lines;
}

/*
std::vector<char> readBinary(const std::string &filePath, std::size_t chunkSize = 4096) {
    std::vector<char> content;
    std::ifstream file(filePath, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Error opening binary file: " << filePath << std::endl;
        return content; // Return an empty vector on error
    }

    // Read the file in chunks
    char buffer[chunkSize];
    while (file.read(buffer, chunkSize)) {
        content.insert(content.end(), buffer, buffer + chunkSize);
    }

    // Read the remaining part
    content.insert(content.end(), buffer, buffer + file.gcount());

    file.close();
    return content;
}
*/
bool appendData(const std::string filename, const char *data, std::size_t len) {
    std::ofstream file(filename, std::ios::out | std::ios::app | std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return false;
    }

    file.write(data, len);

    if (!file.good()) {
        std::cerr << "Error writing to file: " << filename << std::endl;
        return false;
    }

    file.close();
    return true;
}

std::vector<char> readBinary(const std::string &filePath) {
    std::vector<char> content;
    std::ifstream file(filePath, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Error opening binary file: " << filePath << std::endl;
        return content; // Return an empty vector on error
    }

    // Get the length of the file
    file.seekg(0, std::ios::end);
    std::streamsize fileSize = file.tellg();
    std::cout << "file size:" << fileSize << std::endl;
    file.seekg(0, std::ios::beg);

    // Resize the vector to hold the entire file
    content.resize(static_cast<size_t>(fileSize));

    // Read the file into the vector
    file.read(content.data(), fileSize);

    file.close();
    return content;
}

std::string readText(const std::string &filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        // Handle error: unable to open file
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool fileExists(const std::string &filePath) { return access(filePath.c_str(), F_OK) == 0; }

std::vector<std::string> listDir(const std::string &dirPath) {
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
// Define a mapping of file extensions to MIME types
const std::unordered_map<std::string, std::string> mimeTypes = {
{"html", "text/html"},          {"rhtml", "text/html"},       {"mp4", "video/mp4"},
{"m4v", "video/mp4"},           {"m4s", "video/iso.segment"}, {"m4a", "audio/mp4"},
{"mpd", "application/dash+xml"}
// Add more mappings as needed
};

std::string getMimeType(const std::string &filePath) {
    // Extract file extension from the file path
    size_t dotPosition = filePath.find_last_of('.');
    if (dotPosition == std::string::npos) {
        // No file extension found, default to text/plain
        return "text/plain";
    }

    std::string extension = filePath.substr(dotPosition + 1);

    // Look up the MIME type based on the file extension
    auto it = mimeTypes.find(extension);
    if (it != mimeTypes.end()) {
        return it->second;
    }
    else {
        // If the extension is not found, default to text/plain
        return "text/plain";
    }
}

bool createDirectory(const std::string &path) {
    // Create a directory with read, write, and execute permissions for the
    // owner and read and execute permissions for others
    int status = mkdir(path.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

    if (status == 0) {
        // Directory created successfully
        return true;
    }
    else {
        // Check if the directory already exists
        if (errno == EEXIST) {
            return true;
        }
        else {
            // Other error occurred
            return false;
        }
    }
}

} // namespace Fs
