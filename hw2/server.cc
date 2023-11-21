#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <poll.h>
// mine
#include "request.h"
#include "utils.h"
#include "backend.h"

using namespace std;

const int MAXFD = 1024;

const short poll_read = POLLIN | POLLPRI;
const short poll_write = POLLOUT | POLLWRBAND;
const short poll_err = POLLERR | POLLNVAL | POLLHUP;
const short poll_mask = poll_read | poll_write;

char ReqBuf[MAXFD][BUFSZ];

int main(int argc, char *argv[]) {

    if (argv[1] == nullptr) {
        cerr << "Usage: ./ server [port]";
        return -1;
    }
    // assume valid port
    const int PORT = atoi(argv[1]);

    int listenfd, connfd;
    sockaddr_in server_addr, client_addr;
    int client_addr_len = sizeof(client_addr);

    // Get socket file descriptor
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        ERR_EXIT("socket()");
    }

    // Set server address information
    bzero(&server_addr, sizeof(server_addr)); // erase the data
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    // Bind the server file descriptor to the server address
    if (bind(listenfd, (sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        ERR_EXIT("bind()");
    }

    // Listen on the server file descriptor
    if (listen(listenfd, 3) < 0) {
        ERR_EXIT("listen()");
    }

    pollfd conns[MAXFD];
    for (int i = 1; i < MAXFD; ++i) conns[i] = (pollfd){-1, poll_mask, 0};
    conns[listenfd].fd = listenfd;
    conns[listenfd].events = POLLIN;

    while (1) {
        // Call poll
        int pollret = poll(conns, MAXFD, -1); // Wait indefinitely for events

        if (pollret == -1 || conns[listenfd].revents & poll_err) {
            ERR_EXIT("poll");
        }

        // Check for activity on the server socket
        if (conns[listenfd].revents & POLLIN) {
            // Accept the client and get client file descriptor
            if ((connfd = accept(listenfd, (sockaddr *)&client_addr,
                                 (socklen_t *)&client_addr_len)) < 0) {
                ERR_EXIT("accept()");
            }
            conns[connfd].fd = connfd;
            conns[connfd].events = poll_mask;
        }

        // Check for activity on client sockets
        for (int i = 1; i < MAXFD; i++) {
            if (conns[i].fd < 0 || conns[i].fd == listenfd) continue;
            if (conns[i].revents & POLLIN) { // can be data or close (EOF)
                // puts("DATA");
                int n_recv = recv(conns[i].fd, ReqBuf[i], BUFSZ, 0);
                if (n_recv > 0) {
                    // believe that buffer is big enough
                    Request request(ReqBuf);
                    if (request.valid) request.showRequest();

                    Router::Result ret = Router::route(request);
                    cout << "filePath: " << ret.filePath << endl;
                    switch (ret.idx) {
                    case Router::Root:
                        std::cout << "Routing to Root" << std::endl;
                        // Code for Router::Root
                        break;
                    case Router::UploadFile:
                        std::cout << "Routing to UploadFile" << std::endl;
                        // Code for Router::UploadFile
                        break;
                    case Router::UploadVideo:
                        std::cout << "Routing to UploadVideo" << std::endl;
                        // Code for Router::UploadVideo
                        break;
                    case Router::File:
                        std::cout << "Routing to File" << std::endl;
                        // Code for Router::File
                        break;
                    case Router::Video:
                        std::cout << "Routing to Video" << std::endl;
                        // Code for Router::Video
                        break;
                    case Router::VideoPath:
                        std::cout << "Routing to VideoPath" << std::endl;
                        // Code for Router::VideoPath
                        break;
                    case Router::ApiFile:
                        std::cout << "Routing to ApiFile" << std::endl;
                        // Code for Router::ApiFile
                        break;
                    case Router::ApiFilePath:
                        std::cout << "Routing to ApiFilePath" << std::endl;
                        // Code for Router::ApiFilePath
                        break;
                    case Router::ApiVideo:
                        std::cout << "Routing to ApiVideo" << std::endl;
                        // Code for Router::ApiVideo
                        break;
                    case Router::ApiVideoPath:
                        std::cout << "Routing to ApiVideoPath" << std::endl;
                        // Code for Router::ApiVideoPath
                        break;
                    default:
                        std::cout << "Unknown routing result" << std::endl;
                        // Default case for unknown result
                        if (ret.idx != Router::Unknown) {
                            perror("Router::route");
                        }
                    }
                }
                else if (n_recv == 0) {
                    // puts("EOF");
                    close(conns[i].fd);
                    conns[i].fd = -1;
                }
                else {
                    perror("recv");
                    // ERR_EXIT("recv");
                }
            }
            /*
if (conns[i].revents & POLLOUT) {
    puts("POLLOUT");
}
            */
        }
    }
}
