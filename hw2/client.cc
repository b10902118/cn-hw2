#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <netdb.h>
#include "request.h"
#include "fs.h"
using namespace std;
#define BUFF_SIZE 1024
//#define PORT 9999
#define ERR_EXIT(a)                                                                                          \
    {                                                                                                        \
        perror(a);                                                                                           \
        exit(1);                                                                                             \
    }

std::string resolveDomainToIP(const std::string &domain, int myport) {
    const char *hostname = domain.c_str();
    const char *port = std::to_string(myport).c_str();

    struct addrinfo hints, *result, *rp;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;       /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Stream socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0; /* Any protocol */

    int s = getaddrinfo(hostname, port, &hints, &result);
    if (s != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(s) << std::endl;
        return "";
    }

    std::string ipAddress;

    // Iterate through the list and return the first IP address as a string
    for (rp = result; rp != nullptr; rp = rp->ai_next) {
        if (rp->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)rp->ai_addr;
            char ipstr[INET_ADDRSTRLEN];

            // Convert the IPv4 address to a string
            inet_ntop(rp->ai_family, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
            ipAddress = ipstr;
            break; // Break after the first IPv4 address is found
        }
    }
    return ipAddress;
}

enum Command { Put, Putv, Get, Quit, Unknown };

Command parseCommand(const std::string &command) {
    if (command == "put") {
        return Put;
    }
    else if (command == "putv") {
        return Putv;
    }
    else if (command == "get") {
        return Get;
    }
    else if (command == "quit") {
        return Quit;
    }
    else {
        return Unknown;
    }
}

void printHelp(Command commandType) {
    switch (commandType) {
    case Put:
        cerr << "Usage: put [file]" << endl;
        break;
    case Putv:
        cerr << "Usage: putv [file]" << endl;
        break;
    case Get:
        cerr << "Usage: get [file]" << endl;
        break;
    default:
        cerr << "Error" << endl;
    }
}

std::string urlEncode(const std::string &input) {
    std::ostringstream encoded;
    encoded.fill('0');
    encoded << std::hex;

    for (char c : input) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            // These characters are allowed as is
            encoded << c;
        }
        else {
            // Encode other characters using percent encoding (%XX)
            encoded << '%' << std::setw(2) << int(static_cast<unsigned char>(c));
        }
    }

    return encoded.str();
}

string host;
int port;
string credential;
std::string createHttpHeader(Method method, const std::string &URI, const std::string &contentType = "",
                             int contentLen = 0) {
    std::ostringstream request;
    // Request line
    request << (method == GET ? "GET" : "POST") << " " << urlEncode(URI) << " HTTP/1.1\r\n";

    // Host header
    request << "Host: " << host + ":" + to_string(port) << "\r\n";

    // User-Agent header
    request << "User-Agent: "
            << "CN2023Client/1.0"
            << "\r\n";

    // Connection header
    request << "Connection: "
            << "keep-alive"
            << "\r\n";

    // Content-Type header
    if (!contentType.empty()) {
        request << "Content-Type: " << contentType << "\r\n";
    }

    // Content-Length header
    if (contentLen != 0 || method == POST) {
        request << "Content-Length: " << content.length() << "\r\n";
    }

    // Empty line before the body
    request << "\r\n";

    return request.str();
}

const string start_usage = "Usage: ./client [host] [port] [username:password]";

int main(int argc, char *argv[]) {
    if (argc != 3 && argc != 4) {
        cerr << start_usage;
        return -1;
    }
    host = string(argv[1]);
    port = atoi(argv[2]);

    // unbuffered stdin
    std::cout << std::unitbuf;
    std::ios_base::sync_with_stdio(false);

    if (argc == 4) credential = string(argv[3]);

    string serverIP = resolveDomainToIP(host, port);
    cout << serverIP << endl;

    int sockfd;
    struct sockaddr_in addr;
    char buffer[BUFF_SIZE] = {};

    // Get socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        ERR_EXIT("socket()");
    }

    // Set server address
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(serverIP.c_str());
    addr.sin_port = htons(port);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        ERR_EXIT("connect()");
    }

    std::string input, cmd, arg;
    bool quit = false;
    while (!quit) {
        cout << "> ";
        std::getline(std::cin, input);

        size_t spacePos = input.find(' ');
        if (spacePos != string::npos) {
            // Extract command and argument
            cmd = input.substr(0, spacePos);
            arg = (spacePos != std::string::npos) ? input.substr(spacePos + 1) : "";
        }
        else {
            cmd = input;
            arg = "";
        }

        Command commandType = parseCommand(cmd);
        if (commandType != Unknown && commandType != Quit && arg == "") {
            printHelp(commandType);
            continue;
        }
        string header;
        vector<char> file, request;
        switch (commandType) {
        case Put:
            if (!Fs::fileExists(arg)) {
                cout << "Command failed." << endl;
                break;
            }
            file = Fs::readBinary(arg);
            header = createHttpHeader(POST, "/api/file", Fs::getMimeType(arg), file.size());
            request = vector<char>(header.begin(), header.end());
            for (int i = 0; i < file.size(); i++) request.push_back(file[i]);
            send(sockfd, request.data(), request.size(), MSG_NOSIGNAL);
            recv(sockfd, buf, sizeof(buf), 0);
            break;

        case Putv:
            if (!Fs::fileExists(arg)) {
                cout << "Command failed." << endl;
                break;
            }
            // /api/video
            break;

        case Get:
            // /api/file/{filepath}
            break;

        case Quit:
            std::cout << "Bye." << std::endl;
            quit = true;
            break;

        case Unknown:
            std::cerr << "Command Not Found." << std::endl;
            break;
        }
    }

    close(sockfd);

    return 0;
}
