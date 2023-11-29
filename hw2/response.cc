#include "response.h"
#include "request.h"
#include <cstddef>

// Status code descriptions map
const std::map<int, std::string> Response::statusDescriptions = {
{200, "OK"},
{401, "Unauthorized"},
{404, "Not Found"},
{405, "Method Not Allowed"},
{500, "Internal Server Error"}
// Add more status descriptions as needed
};

// Constructor
Response::Response()
    : statusCode(200), serverHeader("CN2023Server/1.0"), contentType("text/plain"), contentLength(0),
      Allow(GET) {}

bool Response::isOK() { return statusCode == 200; }

void Response::setContentType(const std::string &type) { contentType = type; }

// void Response::setContentLength(size_t length) { contentLength = length; }

void Response::setAllow(Method method) { Allow = method; }

/*
void Response::setBody(const char *buffer, size_t size) {
    body.assign(buffer, buffer + size);
    contentLength = size;
}
*/

// Set status code
void Response::setStatusCode(int code) { statusCode = code; }

// TODO make fromatted response raw buffer
// Get the formatted HTTP response

std::vector<char> Response::getFormattedResponse() {
    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " " << statusDescriptions.at(statusCode) << "\r\n";
    response << formatHeaders();
    response << "\r\n"; // Empty line before the body

    // Get the response as a string
    std::string responseStr = response.str();

    // Create a vector<char> from the string
    std::vector<char> responseVector(responseStr.begin(), responseStr.end());
    // no body

    return responseVector;
}
std::vector<char> Response::getFormattedResponse(const char *body, size_t n) {
    contentLength = n;
    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " " << statusDescriptions.at(statusCode) << "\r\n";
    response << formatHeaders();
    response << "\r\n"; // Empty line before the body

    // Get the response as a string
    std::string responseStr = response.str();

    // Create a vector<char> from the string
    std::vector<char> responseVector(responseStr.begin(), responseStr.end());

    // Append the body data
    if (body != nullptr) responseVector.insert(responseVector.end(), body, body + n);

    return responseVector;
}

std::vector<char> Response::getFormattedResponse(const std::string body) {
    contentLength = body.length();
    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " " << statusDescriptions.at(statusCode) << "\r\n";
    response << formatHeaders();
    response << "\r\n"; // Empty line before the body

    // Get the response as a string
    std::string responseStr = response.str();

    // Create a vector<char> from the string
    std::vector<char> responseVector(responseStr.begin(), responseStr.end());

    // Append the body data
    responseVector.insert(responseVector.end(), body.begin(), body.end());

    return responseVector;
}

// Helper function to format headers
std::string Response::formatHeaders() const {
    std::ostringstream headers;
    headers << "Server: " << serverHeader << "\r\n";
    headers << "Content-Length: " << contentLength << "\r\n";
    if (statusCode == 401) {
        headers << "Content-Type: " << contentType << "\r\n";
        headers << "WWW-Authenticate: Basic realm=\"B10902118\""
                << "\r\n";
    }
    else if (statusCode == 405) {
        headers << "Allow: " << methodToString(Allow) << "\r\n";
    }
    else if (statusCode != 500) {
        headers << "Content-Type: " << contentType << "\r\n";
    }

    return headers.str();
}

std::vector<char> Response::res_401() {

    setStatusCode(401);
    return getFormattedResponse("Unauthorized\n", strlen("Unauthorized\n"));
}

std::vector<char> Response::res_404() {

    setStatusCode(404);
    return getFormattedResponse("Not Found\n", strlen("Not Found\n"));
}

std::vector<char> Response::res_500() {

    setStatusCode(500);
    return getFormattedResponse();
}

std::vector<char> Response::res_invalid() {
    switch (statusCode) {
    case 401:
        return res_401();
        break;
    case 404:
        return res_404();
        break;
    case 405:
        return getFormattedResponse();
        break;
    case 500:
    default:
        return res_500();
    }
}

std::vector<char> Response::retHtml(const std::string html) {

    setStatusCode(200);
    setContentType("text/html");
    return getFormattedResponse(html);
}
