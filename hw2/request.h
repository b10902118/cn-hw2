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
enum HeaderType { Connection, Content_Type, Authorization, Other };

#define INVALID_REQUEST(message)                                            \
    do {                                                                    \
        valid = false;                                                      \
        std::cerr << "Invalid request: " << message << std::endl;           \
        return;                                                             \
    } while (false)

class Request {
  public:
    Method method;
    bool valid;
    bool connected;
    std::string URI;
    std::string credential;
    std::string contentType;
    char *body;

    Request(char *buf);
    //~Request();
    void showRequest();

  private:
    static const std::string headerName[3];

    static bool matchHeaderName(char *header, const std::string &name);
    static HeaderType getHeaderType(char *header);
    static std::string extractHeaderValue(char *header);
    void parseRequest(char *buf);
    std::string methodToString() const;
};
