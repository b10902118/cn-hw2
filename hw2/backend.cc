#include "backend.h"
#include <cassert>

using namespace std;

namespace Router {

Endpoint::Endpoint(const std::string &uri, bool getFileFlag,
                   Method methodType)
    : URI(uri), getFile(getFileFlag), method(methodType) {}

// clang-format off
Endpoint endpoints[] = {
    {"/", false, GET},
    {"/upload/file", false, GET},
    {"/upload/video", false, GET},
    {"/file/", false, GET},
    {"/video/", false, GET},
    {"/video/", true, GET},
    {"/api/file", false, POST},
    {"/api/file/", true, GET},
    {"/api/video", false, POST},
    {"/api/video/", true, GET}
};
// clang-format on

Result::Result(EndpointIndex idx, std::string filePath)
    : idx(idx), filePath(filePath) {}

Result route(const Request &request) {
    // Iterate over endpoints to find a match

    assert(endpoints[ApiVideoPath].URI == "/api/video/");

    for (EndpointIndex i = Root; i != Unknown;
         i = static_cast<EndpointIndex>(i + 1)) {
        Endpoint &endpoint = endpoints[i];
        if (request.URI == endpoint.URI) {
            if (request.method == endpoint.method) return Result(i);
            return Result(Unknown);
        }
        else if (endpoint.getFile &&
                 request.URI.substr(0, endpoint.URI.length()) ==
                 endpoint.URI) {
            if (request.method == endpoint.method)
                return Result(i, request.URI.substr(endpoint.URI.length()));
            return Result(Unknown);
        }
    }
    // No matching route
    return Result(Unknown);
}

} // namespace Router