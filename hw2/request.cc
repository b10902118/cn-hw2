#include "request.h"
#include <cstddef>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdio>

std::string decodeURI(const std::string &uri) {
    std::ostringstream decoded;
    decoded.fill('0');

    for (std::size_t i = 0; i < uri.length(); ++i) {
        if (uri[i] == '%' && i + 2 < uri.length()) {
            try {
                // TODO invalid case
                auto hexChar = uri.substr(i + 1, 2);
                auto decodedChar = static_cast<char>(std::stoi(hexChar, nullptr, 16));
                decoded << decodedChar;
                i += 2; // Skip the next two characters
            } catch (...) {
                return "";
                // Invalid hex value, ignore and continue
                decoded << uri[i];
            }
        }
        else if (uri[i] == '?' || uri[i] == '#') {
            // Stop decoding at query parameters or fragments
            break;
        }
        else {
            decoded << uri[i];
        }
    }

    return decoded.str();
}

Request::Request() : method(GET), valid(true), stage(HEADER), connected(false), received(0) {}
int Request::fileCounter = 0;

#define INVALID_REQUEST(message)                                                                             \
    do {                                                                                                     \
        valid = false;                                                                                       \
        std::cerr << "Invalid request: " << message << std::endl;                                            \
    } while (false)

// Display the parsed request
void Request::showRequest() {
    // Display method
    std::cout << "Method: " << methodToString(method) << std::endl;

    // Display connection status
    std::cout << "Connected: " << (connected ? "true" : "false") << std::endl;

    // Display URI
    std::cout << "URI: " << URI << std::endl;

    // Display contentType
    std::cout << "Content-type: " << contentType << std::endl;

    // Display credential
    std::cout << "Credential: " << credential << std::endl;

    std::cout << "boundary: " << boundary << std::endl;

    std::cout << "Contenti-Length: " << contentLen << std::endl;
    // std::cout << "Content-Disposition: " << ContentDisposition <<
    // std::endl;

    /*
// Display body (if available)
if (method == POST) {
    if (body)
        std::cout << "Body:\n"
                  << "--------------------------\n"
                  << body << "--------------------------\n"
                  << std::endl;
    else std::cout << "Body: [empty]" << std::endl;
}
    */
}

// Function to receive HTTP header until \r\n\r\n and preserve content after
// the delimiter
// assume header are all sent together
std::string Request::recvHeader(int socket) {
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
            // return "";
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
            // std::cout << "clean up kernel buffer" << std::endl;
            recv(socket, buffer, len, 0);
            // std::cout << "cleaned" << std::endl;

            break; // Exit the loop once the delimiter is found
        }
        else {
            int bytesAvailable, err = ioctl(socket, FIONREAD, &bytesAvailable);
            if (err == -1) {
                INVALID_REQUEST("recvHeader: ioctl");
                // return "";
                break;
            }
            readable = bytesAvailable > 0;
        }
    }

    // std::cout << "returned" << std::endl;
    /*
if (httpHeader[httpHeader.length() - 1] != '\n' ||
    httpHeader[httpHeader.length() - 2] != '\r' ||
    httpHeader[httpHeader.length() - 3] != '\n' ||
    httpHeader[httpHeader.length() - 4] != '\r') {
    std::cout << "not end right" << std::endl;
    return "";
}
    */
    return httpHeader;
}

const std::string Request::headerName[5] = {[Connection] = "Connection",
                                            [Content_Type] = "Content-Type",
                                            [Authorization] = "Authorization",
                                            [ContentLength] = "Content-Length",
                                            [ContentDisposition] = "Content-Disposition"};

bool Request::matchHeaderName(const std::string &header, const std::string &name) {
    return strncasecmp(header.c_str(), name.c_str(), name.length()) == 0;
}
HeaderType Request::getHeaderType(const std::string &header) {
    for (size_t i = 0; i < 5; ++i) {
        if (matchHeaderName(header, headerName[i] + ":")) return static_cast<HeaderType>(i);
    }
    return Other;
}

std::string Request::extractHeaderValue(const std::string &header) {
    // fixed may not strip wsp:
    // fixed header match first ':'
    std::regex pattern("^[^:]*:\\s*(.*[^\\s])\\s*$");
    std::smatch matches;

    // seems that regex does not support rvalue string
    if (std::regex_search(header, matches, pattern) && matches.size() > 1) {
        return matches[1].str();
    }

    return "";
}

bool Request::parseRequestLine(const std::string request_line) {
    std::string stripped_method;
    if (request_line.compare(0, 4, "GET ") == 0) {
        method = GET;
        // stage = HEADER;
        stripped_method = request_line.substr(4); // skip "GET "
    }
    // Can cause error
    else if (request_line.compare(0, 5, "POST ") == 0) {
        method = POST;
        stage = BODY;
        stripped_method = request_line.substr(5);
    }
    else {
        INVALID_REQUEST("Method");
        return false;
    }
    size_t space_pos = stripped_method.find(" ");
    if (space_pos == std::string::npos || space_pos == request_line.length() - 1) {
        INVALID_REQUEST("request line invalid space");
        return false;
    }
    URI = decodeURI(stripped_method.substr(0, space_pos));

    std::string httpVersion = stripped_method.substr(space_pos + 1);

    if (httpVersion != "HTTP/1.1") {
        INVALID_REQUEST("HTTP version");
        return false;
    }
    return true;
}
void Request::parseHeader(const std::string &httpHeader) {
    // headers is valid, so must end with \r\n\r\n
    size_t end_reqline = httpHeader.find("\r\n");

    std::string request_line = httpHeader.substr(0, end_reqline); // no \r\n
    if (!parseRequestLine(request_line)) return;

    std::string headers = httpHeader.substr(end_reqline + 2); // skip \r\n
    // std::cout << headers << std::endl;
    // return;

    connected = true;
    // not allow multiline header
    size_t cur = 0;
    /*
if (headers[headers.length() - 1] != '\n' ||
    headers[headers.length() - 2] != '\r' ||
    headers[headers.length() - 3] != '\n' ||
    headers[headers.length() - 4] != '\r') {
    std::cout << (int)headers[headers.length() - 4] << ' '
              << (int)headers[headers.length() - 3] << ' '
              << (int)headers[headers.length() - 2] << ' '
              << (int)headers[headers.length() - 1] << std::endl;
    return;
}
    */
    while (cur != headers.length() - 2) {
        size_t pos = headers.find("\r\n", cur);
        std::string header = headers.substr(cur, pos - cur);
        /*
std::cout << pos << std::endl;
std::cout << '"' << header << '"' << std::endl;
if (header == "\r\n") break;
if (pos > headers.length()) {
    std::cout << "cur: " << cur << std::endl;
    std::cout << "headers len: " << headers.length() << std::endl;
    if (headers[cur] != '\r' || headers[cur + 1] != '\n') {
        std::cout << (int)headers[cur] << (int)headers[cur]
                  << std::endl;
    }
    break;
}
        */
        cur = pos + 2;

        // gather required header
        std::string value;
        HeaderType type = getHeaderType(header);
        if (type != Other) value = extractHeaderValue(header);
        switch (type) {
        case Connection:
            // require length same
            if (icaseCmp(value, "Close")) connected = false;
            break;
        case Content_Type:
            contentType = std::string(value); // TODO  may contain form boundary
            if (contentType.compare(0, strlen("multipart/form-data"), "multipart/form-data") == 0) {
                size_t pos = contentType.find("boundary=");
                if (pos != std::string::npos) boundary = contentType.substr(pos + strlen("boundary="));
            }
            break;
        case Authorization:
            if (value.substr(0, 6) != "Basic ") {
                std::cerr << "basic not match" << std::endl;
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
                if (pt != nullptr) {
                    credential = std::string(reinterpret_cast<char *>(pt), len);
                    free(pt);
                }
                else credential = "";
            }
            break;
        case ContentLength:
            contentLen = atoi(value.c_str());
            break;
            // TODO
        case ContentDisposition:
            break;
        case Other:
        default:
            break;
        }
    }
}

// Convert Method enum to string
std::string methodToString(Method method) {
    switch (method) {
    case GET:
        return "GET";
    case POST:
        return "POST";
    default:
        return "Error";
    }
}
