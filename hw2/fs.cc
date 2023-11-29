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
#include <cstring>
#include <limits.h> // For PATH_MAX

using namespace std;

namespace Fs {

const std::string FileRoot = "hw2/web/files/";
const std::string VideoRoot = "hw2/web/videos/";
const std::string TmpDir = "hw2/web/tmp/";
const std::string baseDir = "./";

void init() {
    removeDir(TmpDir);
    if (!createDirectory(TmpDir)) std::cerr << "cannot create dir";
    if (!createDirectory(FileRoot)) std::cerr << "cannot create dir";
    if (!createDirectory(VideoRoot)) std::cerr << "cannot create dir";
}

void removeDir(const string p) { system(("rm -rf " + p).c_str()); }

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

const int max_size = 200 * 1024 * 1024;
char fileData[max_size + 1024];

// clang-format off
const string cmd[] = {
"ffmpeg -re -i " ,
//video file
" -c:a aac -c:v libx264 "
"-map 0 -b:v:1 6M -s:v:1 1920x1080 -profile:v:1 high "
"-map 0 -b:v:0 144k -s:v:0 256x144 -profile:v:0 baseline " 
"-bf 1 -keyint_min 120 -g 120 -sc_threshold 0 -b_strategy 0 "
"-ar:a:1 22050 -use_timeline 1 -use_template 1 "
"-adaptation_sets \"id=0,streams=v id=1,streams=a\" -f dash ",
// video dir/dash.mpd
//"/dash.mpd"
};
// clang-format on

void parseUpload(std::string tmpName, std::string boundary, bool isApiVideo) {
    //  TODO write to file, handle boundary & content length
    //  & filesize state: file too big
    // parse file
    string tmpPath = TmpDir + tmpName;
    std::ifstream file(tmpPath, std::ios::binary);
    std::string line, disposition;
    getline(file, line);
    getline(file, disposition);
    // extract name
    size_t pos = disposition.find("filename=\"");
    if (pos == string::npos) cerr << "disposition no filename" << endl;
    pos += strlen("filename=\"");
    // \n stripped
    string fileName = disposition.substr(pos, disposition.length() - strlen("\"\r") - pos);
    string strippedName; // for video
    cout << fileName << endl;
    if (isApiVideo) {
        strippedName = fileName.substr(0, fileName.length() - strlen(".mp4"));
        cout << strippedName << endl;
    }

    getline(file, line);
    getline(file, line);

    streamsize fileStart = file.tellg(), end;

    // Get the size of the file
    file.seekg(0, std::ios::end);
    std::streamsize bodyEnd = file.tellg();

    streamsize fileEnd =
    bodyEnd - (strlen("\r\n") + boundary.length() +
               strlen("\r\nContent-Disposition: form-data; name=\"submit\"\r\n\r\nUpload\r\n") +
               boundary.length() + strlen("--\r\n"));
    streamsize fileSize = fileEnd - fileStart;
    if (fileSize > max_size) {
        // abort and rm
        return;
    }

    file.seekg(fileStart);
    file.read(fileData, fileSize);
    file.close();
    // unlink(tmpPath.c_str());

    if (isApiVideo) {
        const string tmpVideo = TmpDir + fileName;
        writeFile(tmpVideo, fileData, fileSize);
        const string videoFolder = VideoRoot + strippedName + "/";
        removeDir(videoFolder);
        Fs::createDirectory(videoFolder);
        pid_t pid = fork();
        cout << "forking" << endl;
        if (pid == -1) {
            // Handle fork error
            std::cerr << "Error forking process." << std::endl;
            return;
        }
        else if (pid == 0) {
            // This is the child process
            string command =
            cmd[0] + "\"" + tmpVideo + "\"" + cmd[1] + "\"" + videoFolder + "dash.mpd" + "\"";
            cout << command << endl;
            int ret = system(command.c_str());
            cout << "done" << endl;
            if (ret == -1) {
                std::cerr << "system: create new process fail" << std::endl;
            }
            else if (ret != 0) {
                std::cerr << "return value: " << ret << std::endl;
            }
            _exit(1);
        }
    }
    else { // normal file
        // cout << "writing to " << FileRoot + fileName << endl;
        writeFile(FileRoot + fileName, fileData, fileSize);
    }
    return;
}

bool writeFile(const std::string filename, const char *data, std::size_t len) {
    std::ofstream file(filename, std::ios::out | std::ios::binary);

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
{"html", "text/html"},        {"rhtml", "text/html"}, {"mp4", "video/mp4"},           {"m4v", "video/mp4"},
{"m4s", "video/iso.segment"}, {"m4a", "audio/mp4"},   {"mpd", "application/dash+xml"}
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
