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
using namespace std;
#define BUFF_SIZE 1024
//#define PORT 9999
#define ERR_EXIT(a)                                                         \
    {                                                                       \
        perror(a);                                                          \
        exit(1);                                                            \
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
            inet_ntop(rp->ai_family, &(ipv4->sin_addr), ipstr,
                      sizeof(ipstr));
            ipAddress = ipstr;
            break; // Break after the first IPv4 address is found
        }
    }
    return ipAddress;
}

enum Command {
    Put,
    Putv,
    Get,
    Quit,
};

const string start_usage =
"Usage: ./client [host] [port] [username:password]";

string host;
int port;
string credential;
int main(int argc, char *argv[]) {
    if (argc != 3 && argc != 4) {
        cerr << start_usage;
        return -1;
    }
    host = string(argv[1]);
    port = atoi(argv[2]);

    if (argc == 4) credential = string(argv[3]);

    string serverIP = resolveDomainToIP(host, port);
    cout << serverIP << endl;
    return 0;

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

    while (true) {
    }
    // Receive message from server
    ssize_t n;
    if ((n = read(sockfd, buffer, sizeof(buffer))) < 0) {
        ERR_EXIT("read()");
    }

    printf("%s\n", buffer);

    close(sockfd);

    return 0;
}
