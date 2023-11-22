#pragma once

#include <iostream>
#include <cstddef>
#include <cstdlib>
#include <string>
#include <cstring>
#include <regex>
#include "utils.h"
#include "utils/base64.h"

enum Method { GET, POST };
enum HeaderType {
    Connection,
    Content_Type,
    Authorization,
    ContentLength,
    ContentDisposition,
    Other
};

enum ServeStage { HEADER, BODY, ROUTE };

std::string methodToString(Method method);

std::string decodeURI(const std::string &uri);

class Request {
  public:
    Method method;
    bool valid;
    ServeStage stage;
    bool connected;
    int contentLen;
    std::string boundary;
    std::string URI;
    std::string credential;
    std::string contentType;
    std::string filePath;
    std::string Response;

    Request();
    //~Request();
    std::string recvHeader(int socket);
    void parseHeader(const std::string &httpHeader);
    void showRequest();

  private:
    static const std::string headerName[5];

    static bool matchHeaderName(const std::string &header,
                                const std::string &name);
    static HeaderType getHeaderType(const std::string &header);
    static std::string extractHeaderValue(const std::string &header);
    bool parseRequestLine(const std::string request_line);
};
