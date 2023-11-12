#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <regex>
#include "utils.h"
using namespace std;

enum Method { GET, POST, INVALID };

struct Request {
    Method method;
    bool connected;
    string URI;
    string credential;
    char *body;
} request;

#define INVALID_REQUEST(message)                                            \
    do {                                                                    \
        request.method = INVALID;                                           \
        cerr << "Invalid request: " << message << endl;                     \
        return;                                                             \
    } while (false)

/*
bool caseInsensitiveIdentical(const std::string &str1,
                              const std::string &str2) {
    size_t len = std::min(str1.length(), str2.length());
    for (size_t i = 0; i < len; ++i) {
        if (std::tolower(str1[i]) != std::tolower(str2[i])) {
            return false;
        }
    }
    return true;
}

bool caseInsensitiveIdentical(const std::string &str1, const char *str2) {
    size_t len1 = str1.length();
    size_t len2 = std::strlen(str2);
    size_t len = std::min(str1.length(), str2.length());

    for (size_t i = 0; i < len; ++i) {
        if (std::tolower(str1[i]) != std::tolower(str2[i])) {
            return false;
        }
    }

    return true;
}
*/

void showRequest(const Request &request) {
    // Display method
    std::cout << "Method: ";
    switch (request.method) {
    case GET:
        std::cout << "GET";
        break;
    case POST:
        std::cout << "POST";
        break;
    // Add cases for other methods if needed
    default:
        std::cout << "Unknown";
        break;
    }
    // std::cout << std::endl;

    // Display connection status
    std::cout << "Connected: " << (request.connected ? "true" : "false")
              << std::endl;

    // Display URI
    std::cout << "URI: " << request.URI << std::endl;

    // Display credential
    std::cout << "Credential: " << request.credential << std::endl;

    // Display body (if available)
    if (request.method == POST) {
        if (request.body) std::cout << "Body: " << request.body << std::endl;
        else std::cout << "Body: [empty]" << std::endl;
    }
}

bool isConnectionClose(const char *header) {
    std::regex pattern("Connection:\\s*close\\s*",
                       std::regex_constants::icase);
    return std::regex_search(header, pattern);
}

bool isAuthorizationBasic(const char *header) {
    std::regex pattern("Authorization:\\s*Basic\\s.*",
                       std::regex_constants::icase);
    return std::regex_search(header, pattern);
}

std::string extractBasicCredentials(char *header) {
    // Define a regex pattern to match Authorization: Basic followed by
    // Base64 string
    // std::regex
    // pattern("Authorization:\\s*Basic\\s+([A-Za-z0-9+/]+={0,2})");
    std::regex pattern("Authorization:\\s*Basic\\s+([^\\s]+)\\s*");

    // Use std::smatch to capture the matched groups
    std::smatch matches;

    std::string input(header);
    // Check if the pattern is found in the input
    if (std::regex_search(input, matches, pattern)) {
        // Check if there's a captured group (Base64 string)
        if (matches.size() > 1) {
            // Return the Base64 string as a C++ string
            return matches[1].str();
        }
        else {
            return ""; // No Base64 credentials found
        }
    }
    else {
        return ""; // Authorization header not found or does not contain
                   // Basic credentials
    }
}

void parse_request(char *const buf) {
    // Parse the request line
    char *request_line = strtok(buf, "\r");
    if (request_line == NULL) INVALID_REQUEST("No request line");

    if (strncmp("GET", request_line, 3) == 0) {
        request.method = GET;
        request_line += 4; // skip "GET "
    }
    else if (strncmp("POST", request_line, 4) == 0) {
        request.method = POST;
        request_line += 5;
    }
    else INVALID_REQUEST("Method");

    // check " HTTP/1.1" and copy uri
    char *const version_str =
    request_line + strlen(request_line) - strlen(" HTTP/1.1");
    if (version_str <= request_line) INVALID_REQUEST("No URI");
    if (strcmp(version_str, " HTTP/1.1") != 0)
        INVALID_REQUEST("HTTP version");

    *version_str = '\0';
    string URI = string(request_line);

    /*
           Parse headers
           stop when this:
           ...\r\n\r\n
                   ^
     */
    // not allow multiline header
    char *s;
    for (; (s = strtok(NULL, "\n")) != NULL && *s != '\r';) {
        if (isConnectionClose(s)) request.connected = false;
        else if (isAuthorizationBasic(s))
            request.credential = extractBasicCredentials(s);
        // string header = string(s);
        /*
if (caseInsensitiveIdentical(header, "Connection:")) {
    // to lower case
    transform(header.begin(), header.end(), header.begin(),
              [](unsigned char c) { return std::tolower(c); });
    if (header.find("close") != string::npos) }
                */
    }
    if (s == NULL) INVALID_REQUEST("no double CRLF");
    if (request.method == POST) request.body = s + 2;
    showRequest(request);
}
