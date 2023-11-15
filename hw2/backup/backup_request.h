#pragma once

#include <iostream>
#include <cstddef>
#include <cstdlib>
#include <string>
#include <cstring>
#include <regex>
#include "utils.h"
#include "utils/base64.h"

enum Method { GET, POST, INVALID };
enum HeaderType { Connection, Content_Type, Authorization, Other };

#define INVALID_REQUEST(message)                                            \
    do {                                                                    \
        method = INVALID;                                                   \
        std::cerr << "Invalid request: " << message << std::endl;           \
        return;                                                             \
    } while (false)

class Request {
  public:
    Method method;
    bool connected;
    std::string URI;
    std::string credential;
    std::string contentType;
    char *body;
    // Constructor
    Request(char *buf) : method(INVALID), connected(true), body(nullptr) {
        parseRequest(buf);
    }

    // Destructor
    //~Request() { delete[] body; }

    bool isValid() { return method != INVALID; }
    // Display the parsed request
    void showRequest() {
        // Display method
        std::cout << "Method: " << methodToString() << std::endl;

        // Display connection status
        std::cout << "Connected: " << (connected ? "true" : "false")
                  << std::endl;

        // Display URI
        std::cout << "URI: " << URI << std::endl;

        // Display contentType
        std::cout << "Content-type: " << contentType << std::endl;

        // Display credential
        std::cout << "Credential: " << credential << std::endl;

        // Display body (if available)
        if (method == POST) {
            if (body) std::cout << "Body: " << body << std::endl;
            else std::cout << "Body: [empty]" << std::endl;
        }
    }

  private:
    const std::string headerName[3] = {[Connection] = "Connection",
                                       [Content_Type] = "Content-Type",
                                       [Authorization] = "Authorization"};

    static bool matchHeaderName(char *header, const std::string &name) {
        return strncasecmp(header, name.c_str(), name.length()) == 0;
    }
    HeaderType getHeaderType(char *header) {
        for (size_t i = 0; i < 3; ++i) {
            if (matchHeaderName(header, headerName[i] + ":"))
                return static_cast<HeaderType>(i);
        }
        return Other;
    }

    static std::string extractHeaderValue(char *header) {
        // fixed may not strip wsp:
        // fixed header match first ':'
        std::regex pattern("^[^:]*:\\s*(.*[^\\s])\\s*$");
        std::smatch matches;

        // seems that regex does not support rvalue string
        std::string s(header);
        if (std::regex_search(s, matches, pattern) && matches.size() > 1) {
            return matches[1].str();
        }

        return "";
    }

    // Parse the entire request
    void parseRequest(char *buf) {
        // Parse the request line
        char *request_line = strtok(buf, "\r");
        if (request_line == NULL) INVALID_REQUEST("No request line");

        if (strncmp("GET ", request_line, 4) == 0) {
            method = GET;
            request_line += 4; // skip "GET "
        }
        else if (strncmp("POST ", request_line, 5) == 0) {
            method = POST;
            request_line += 5;
        }
        else INVALID_REQUEST("Method");

        // check " HTTP/1.1" and copy uri
        // cout << strlen(request_line) << ' ' << strlen(" HTTP/1.1");
        // cannot jsut enter, require /r/n
        char *const version_str =
        request_line + strlen(request_line) - strlen(" HTTP/1.1");
        // cout << version_str << std::endl;
        if (version_str <= request_line) INVALID_REQUEST("No URI");
        if (std::strcmp(version_str, " HTTP/1.1") != 0)
            INVALID_REQUEST("HTTP version");

        *version_str = '\0';
        URI = std::string(request_line);

        /*
               Parse headers
               stop when this:
               ...\r\n\r\n
                       ^
         */
        // not allow multiline header
        char *s;
        HeaderType type;
        std::string value;
        for (; (s = strtok(NULL, "\n")) != NULL && *s != '\r';) {
            type = getHeaderType(s);
            if (type != Other) value = extractHeaderValue(s);
            switch (type) {
            case Connection:
                // require length same
                if (icaseCmp(value, "Close")) connected = false;
                break;
            case Content_Type:
                contentType = std::string(value);
                break;
            case Authorization:
                if (value.substr(0, 6) != "Basic ") {
                    // cerr << "basic not match" << std::endl;
                    credential = "";
                }
                else {
                    const char *enc = value.c_str() + strlen("Basic ");
                    /*
printf("%p\n", enc);
cerr << '"' << enc << '"' << std::endl;
printf("%p\n", value.c_str());
cerr << '"' << value.c_str() << '"' << std::endl;
                    */
                    size_t len;
                    // must store ptr to free and wait the function set len
                    unsigned char *pt =
                    base64_decode(enc, strlen(enc), &len);
                    credential =
                    std::string(reinterpret_cast<char *>(pt), len);
                    free(pt);
                }
                break;
            }
        }
        if (s == NULL) INVALID_REQUEST("no double CRLF");
        if (method == POST) body = s + 2;
    }

    // Convert Method enum to string
    std::string methodToString() const {
        switch (method) {
        case GET:
            return "GET";
        case POST:
            return "POST";
        default:
            return "Unknown";
        }
    }
};
