#pragma once

#include <cstddef>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include "request.h"

// TODO merge with request.h

class Response {
  public:
    // Constructor
    Response();

    // Setters for response headers
    void setServerHeader(const std::string &server);
    void setContentType(const std::string &contentType);
    void setAllow(Method method);

    // Setters for response body
    // void setBody(const std::string &responseBody);
    // void setBody(const std::vector<char> &binaryBody);
    void setBody(const char *buffer, size_t size);

    // Set status code
    void setStatusCode(int statusCode);

    // Get the formatted HTTP response
    // std::vector<char>
    // getFormattedResponse(const std::vector<char> body) const;
    std::vector<char> getFormattedResponse();
    std::vector<char> getFormattedResponse(const char *body, size_t n);
    std::vector<char> getFormattedResponse(const std::string body);
    std::vector<char> res_401();
    std::vector<char> res_404();
    std::vector<char> res_500();
    std::vector<char> retHtml(const std::string html);

  private:
    // Helper function to format headers
    std::string formatHeaders() const;

  private:
    int statusCode;
    const std::string serverHeader;
    std::string contentType;
    size_t contentLength;
    Method Allow;

    // Status code descriptions
    static const std::map<int, std::string> statusDescriptions;
    // void setContentLength(size_t contentLength);
};
