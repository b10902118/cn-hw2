#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <poll.h>
#include "request.h"
#include "utils.h"

#define PORT 9999
#define MAXFD 1024
#define BUFSZ 1024 * 1024 * 1024

const short poll_read = POLLIN | POLLPRI;
const short poll_write = POLLOUT | POLLWRBAND;
const short poll_err = POLLERR | POLLNVAL | POLLHUP;
const short poll_mask = poll_read | poll_write;

char req[BUFSZ];

int main(int argc, char *argv[]) {
    int listenfd, connfd;
    struct sockaddr_in server_addr, client_addr;
    int client_addr_len = sizeof(client_addr);
    char *message = "Hello World!";

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
    if (bind(listenfd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {
        ERR_EXIT("bind()");
    }

    // Listen on the server file descriptor
    if (listen(listenfd, 3) < 0) {
        ERR_EXIT("listen()");
    }

    int client_sockets[MAXFD];
    int client_n = 0;
    struct pollfd conns[MAXFD];
    for (int i = 1; i < MAXFD; ++i)
        conns[i] = (struct pollfd){-1, poll_mask, 0};
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
            if ((connfd = accept(listenfd, (struct sockaddr *)&client_addr,
                                 (socklen_t *)&client_addr_len)) < 0) {
                ERR_EXIT("accept()");
            }
            client_sockets[connfd] = connfd;
            conns[connfd].fd = connfd;
            conns[connfd].events = poll_mask;
        }

        // Check for activity on client sockets
        for (int i = 1; i < MAXFD; i++) {
            if (conns[i].fd < 0 || conns[i].fd == listenfd) continue;
            if (conns[i].revents & POLLIN) { // can be close (EOF)
                // puts("DATA");
                int n_recv = recv(conns[i].fd, req, sizeof(req), 0);
                if (n_recv > 0) {
                    parse_request(req);
                    puts(type == 0 ? "GET" : "POST");
                    puts(URI);
                    for (int j = 0; j < n_header; ++j) {
                        puts(headers[j]);
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

    close(connfd);
    close(listenfd);
}
