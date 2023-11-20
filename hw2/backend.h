#pragma once

#include <string>
#include "request.h"

namespace Router {
class Endpoint {
  public:
    std::string URI;
    bool getFile;
    Method method;

    // Constructor with default values
    Endpoint(const std::string &uri, bool getFileFlag = false,
             Method methodType = GET);

    // Other member functions or data as needed
};
enum EndpointIndex {
    Root,
    UploadFile,
    UploadVideo,
    File,
    Video,
    VideoPath,
    ApiFile,
    ApiFilePath,
    ApiVideo,
    ApiVideoPath,
    Unknown
};
struct Result {
    EndpointIndex idx;
    std::string filePath;

    Result(EndpointIndex idx, std::string filePath = "");
};

extern Endpoint endpoints[];
Result route(const Request &request);
} // namespace Router
