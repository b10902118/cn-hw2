#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <regex>
#include "utils.h"

using namespace std;

enum Method { GET, POST, INVALID };

#define INVALID_REQUEST(message)                                            \
    do {                                                                    \
        method = INVALID;                                                   \
        cerr << "Invalid request: " << message << endl;                     \
        return;                                                             \
    } while (false)

class Request {
  public:
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
        cout << "Method: " << methodToString() << endl;

        // Display connection status
        cout << "Connected: " << (connected ? "true" : "false") << endl;

        // Display URI
        cout << "URI: " << URI << endl;

        // Display credential
        cout << "Credential: " << credential << endl;

        // Display body (if available)
        if (method == POST) {
            if (body) cout << "Body: " << body << endl;
            else cout << "Body: [empty]" << endl;
        }
    }

  private:
    Method method;
    bool connected;
    string URI;
    string credential;
    char *body;

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
        cout << version_str << endl;
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
            if (isConnectionClose(s)) connected = false;
            else if (isAuthorizationBasic(s))
                credential = extractBasicCredentials(s);
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
        if (method == POST) body = s + 2;
    }

    // Convert Method enum to string
    string methodToString() const {
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
