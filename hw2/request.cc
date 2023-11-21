#include "request.h"
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>

Request::Request(char *buf)
    : method(GET), valid(true), connected(true), body(nullptr) {
    parseRequest(buf);
}

// Display the parsed request
void Request::showRequest() {
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
        if (body)
            std::cout << "Body:\n"
                      << "--------------------------\n"
                      << body << "--------------------------\n"
                      << std::endl;
        else std::cout << "Body: [empty]" << std::endl;
    }
}

// Function to receive HTTP header until \r\n\r\n and preserve content after
// the delimiter
// assume header are all sent together
std::string recvHeader(int socket) {
    const int bufferSize = 4096;
    char buffer[bufferSize];

    std::string httpHeader;
    bool readable = true;

    while (readable) {
        // Peek into the socket to read the data without removing it
        ssize_t bytesRead = recv(socket, buffer, bufferSize - 1, MSG_PEEK);

        if (bytesRead <= 0) {
            // Error or connection closed
            INVALID_REQUEST("recvHeader: recv bytesRead<=0");
            break;
        }

        buffer[bytesRead] = '\0'; // Null-terminate the buffer

        // Search for the \r\n\r\n delimiter in the buffer
        char *delimiter = strstr(buffer, "\r\n\r\n");

        if (delimiter != nullptr) {
            // Include the delimiter in the HTTP header
            httpHeader.append(buffer, delimiter + 4 - buffer);

            // Remove the data from the socket up to and including the
            // delimiter
            ssize_t len = delimiter + 4 - buffer;
            recv(socket, buffer, len, 0);

            break; // Exit the loop once the delimiter is found
        }
        else {
            int bytesAvailable,
            err = ioctl(socket, FIONREAD, &bytesAvailable);
            if (err == -1) {
                INVALID_REQUEST("recvHeader: ioctl");
                break;
            }
            readable = bytesAvailable > 0;
        }
    }

    return httpHeader;
}

const std::string Request::headerName[3] = {[Connection] = "Connection",
                                            [Content_Type] = "Content-Type",
                                            [Authorization] =
                                            "Authorization"};

bool Request::matchHeaderName(char *header, const std::string &name) {
    return strncasecmp(header, name.c_str(), name.length()) == 0;
}
HeaderType Request::getHeaderType(char *header) {
    for (size_t i = 0; i < 3; ++i) {
        if (matchHeaderName(header, headerName[i] + ":"))
            return static_cast<HeaderType>(i);
    }
    return Other;
}

std::string Request::extractHeaderValue(char *header) {
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

void Request::parseHeader(char *buf) {
    // Parse the request line
    char *request_line = strtok(buf, "\r");
    if (request_line == NULL) {
        INVALID_REQUEST("No request line");
        return;
    }

    if (strncmp("GET ", request_line, 4) == 0) {
        method = GET;
        request_line += 4; // skip "GET "
    }
    else if (strncmp("POST ", request_line, 5) == 0) {
        method = POST;
        request_line += 5;
    }
    else {
        INVALID_REQUEST("Method");
        return;
    }

    // check " HTTP/1.1" and copy uri
    // cout << strlen(request_line) << ' ' << strlen(" HTTP/1.1");
    // cannot jsut enter, require /r/n
    char *const version_str =
    request_line + strlen(request_line) - strlen(" HTTP/1.1");
    // cout << version_str << std::endl;
    if (version_str <= request_line) {
        INVALID_REQUEST("No URI");
        return;
    }
    if (std::strcmp(version_str, " HTTP/1.1") != 0) {
        INVALID_REQUEST("HTTP version");
        return;
    }

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
                unsigned char *pt = base64_decode(enc, strlen(enc), &len);
                credential = std::string(reinterpret_cast<char *>(pt), len);
                free(pt);
            }
            break;
        }
    }
    // if (s == NULL) INVALID_REQUEST("no double CRLF");
    // if (method == POST) body = s + 2;
}

// Convert Method enum to string
std::string Request::methodToString() const {
    switch (method) {
    case GET:
        return "GET";
    case POST:
        return "POST";
    default:
        return "Error";
    }
}
