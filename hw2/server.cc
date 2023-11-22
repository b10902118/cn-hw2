#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/types.h>
#include <unistd.h>
#include <poll.h>
#include <csignal>
// mine
#include "request.h"
#include "response.h"
#include "utils.h"
#include "backend.h"
#include "html.h"
#include "fs.h"
#include "auth.h"

using namespace std;
void emptyHandler(int signum) {}
const int MAXFD = 1024;
const int BUFSZ = 1024 * 4;

const short poll_read = POLLIN | POLLPRI;
const short poll_write = POLLOUT | POLLWRBAND;
const short poll_err = POLLERR | POLLNVAL | POLLHUP;
const short poll_mask = poll_read | poll_write;

Request requests[MAXFD];
char bodyBuf[BUFSZ];

int main(int argc, char *argv[]) {

    if (argv[1] == nullptr) {
        cerr << "Usage: ./ server [port]";
        return -1;
    }
    std::signal(SIGPIPE, emptyHandler);
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

    // Auth::init();
    Html::init();
    Fs::init();

    while (1) {
        // Call poll
        int pollret = poll(conns, MAXFD, -1); // Wait indefinitely for events

        if (pollret == -1 || conns[listenfd].revents & poll_err) {
            ERR_EXIT("poll");
        }

        // Check for activity on the server socket
        if (conns[listenfd].revents & POLLIN) {
            // Accept the client and get client file descriptor
            if ((connfd =
                 accept(listenfd, (sockaddr *)&client_addr, (socklen_t *)&client_addr_len)) < 0) {
                ERR_EXIT("accept()");
            }
            conns[connfd].fd = connfd;
            conns[connfd].events = poll_mask;
        }

        // Check for activity on client sockets
        for (int i = 1; i < MAXFD; i++) {
            if (conns[i].fd < 3 || conns[i].fd == listenfd) continue;
            // handle POLLOUT and response local kernel buffer may be
            // full so write block.
            if (conns[i].revents & poll_err) { // have to close it
                                               // causing some fd broken
                close(conns[i].fd);
                conns[i].fd = -1;
            }
            else if (conns[i].revents & POLLIN) { // can be data or close (EOF)

                // TODO unexpected close should not crash
                Request &request = requests[i];
                Response response;
                // check recv header or body
                cout << request.stage << endl;
                if (request.stage == HEADER) {
                    string httpHeader = request.recvHeader(i);

                    if (httpHeader == "" || !request.valid) {
                        vector<char> raw_resp = response.res_500();
                        send(i, raw_resp.data(), raw_resp.size(), 0);
                        cerr << "header invalid" << endl;
                        continue;
                    }
                    request.parseHeader(httpHeader);
                    if (!request.valid) {
                        vector<char> raw_resp = response.res_500();
                        send(i, raw_resp.data(), raw_resp.size(), 0);
                        cerr << "request invalid" << endl;
                        continue;
                    }
                    request.showRequest();
                }
                else if (request.stage == BODY) { // receiving body
                    ssize_t to_recv = min(BUFSZ, request.contentLen);
                    // recv 0 still hang
                    int n_recv = recv(i, bodyBuf, to_recv, 0);
                    if (n_recv < 0) {
                        perror("recv:");
                    }
                    else if (n_recv == 0) {
                        cerr << "unexpected close of connection" << endl;
                    }
                    bodyBuf[n_recv] = '\0';
                    cout << bodyBuf << endl;
                    if (request.contentLen - to_recv == 0) {
                        request.stage = ROUTE;
                    }
                    //  TODO write to file, handle boundary & content length
                    //  & filesize state: file too big
                }
                if (request.stage == ROUTE) { // Routing
                    Router::Result ret = Router::route(request);
                    vector<char> raw_resp;
                    vector<char> fileBuf;
                    cout << "filePath: " << request.filePath << endl;
                    if (!ret.valid) {
                        if (ret.idx == Router::Unknown) { // 404
                            raw_resp = response.res_404();
                        }
                        else { // 405
                            response.setStatusCode(405);
                            response.setAllow(Router::endpoints[ret.idx].method);
                            raw_resp = response.getFormattedResponse();
                        }
                    }
                    else { // default valid routing
                        response.setStatusCode(200);
                        // TODO Auth
                        switch (ret.idx) {
                        case Router::Root:
                            std::cout << "Routing to Root" << std::endl;
                            raw_resp = response.retHtml(Html::index);
                            break;
                        case Router::UploadFile:
                            std::cout << "Routing to UploadFile" << std::endl;
                            if (!Auth::authorized(request.credential)) {
                                raw_resp = response.res_401();
                            }
                            else {
                                raw_resp = response.retHtml(Html::uploadf);
                            }
                            break;
                        case Router::UploadVideo:
                            std::cout << "Routing to UploadVideo" << std::endl;
                            if (!Auth::authorized(request.credential))
                                raw_resp = response.res_401();
                            raw_resp = response.retHtml(Html::uploadv);
                            break;

                        case Router::File:
                            std::cout << "Routing to File" << std::endl;
                            raw_resp = response.retHtml(
                            Html::tagToList(Html::listf, "FILE_LIST", "/api/file/", "./web/files"));

                            break;
                        case Router::Video:
                            std::cout << "Routing to Video" << std::endl;
                            raw_resp = response.retHtml(
                            Html::tagToList(Html::listv, "VIDEO_LIST", "/video/", "./web/videos"));
                            break;
                        case Router::VideoPath:
                            // TODO check existance
                            std::cout << "Routing to VideoPath" << std::endl;
                            // clang-format off
							if (!Fs::fileExists("./web/videos/"+request.filePath)){//404
                            	raw_resp = response.res_404();
							}
							else{
								raw_resp = response.retHtml(
									Html::replaceTag(
										Html::replaceTag(Html::player, "VIDEO_NAME", request.filePath),
										"MPD_PATH",
										"/api/video/"+request.filePath
									)
								);
							}
                            // clang-format on
                            break;

                        case Router::ApiFile:
                            std::cout << "Routing to ApiFile" << std::endl;
                            break;
                        case Router::ApiFilePath:
                            std::cout << "Routing to ApiFilePath" << std::endl;
                            if (!Auth::authorized(request.credential))
                                raw_resp = response.res_401();
                            else {
                                // check exist
                                if (!Fs::fileExists("./web/files/" + request.filePath)) { // 404
                                    raw_resp = response.res_404();
                                }
                                else {
                                    response.setContentType(Fs::getMimeType(request.filePath));
                                    fileBuf = Fs::readBinary("./web/files/" + request.filePath);
                                    raw_resp =
                                    response.getFormattedResponse(fileBuf.data(), fileBuf.size());
                                }
                            }
                            break;
                        case Router::ApiVideo:
                            std::cout << "Routing to ApiVideo" << std::endl;
                            if (!Auth::authorized(request.credential))
                                raw_resp = response.res_401();
                            break;
                        case Router::ApiVideoPath:
                            std::cout << "Routing to ApiVideoPath" << std::endl;
                            if (!Auth::authorized(request.credential))
                                raw_resp = response.res_401();
                            else {
                                // check exist
                                if (!Fs::fileExists("./web/videos/" + request.filePath)) { // 404
                                    raw_resp = response.res_404();
                                }
                                else {
                                    response.setContentType(Fs::getMimeType(request.filePath));
                                    fileBuf = Fs::readBinary("./web/videos/" + request.filePath);
                                    raw_resp =
                                    response.getFormattedResponse(fileBuf.data(), fileBuf.size());
                                }
                            }
                            break;
                        default:
                            std::cout << "Unknown routing result" << std::endl;
                            // Default case for unknown result
                            if (ret.idx != Router::Unknown) {
                                perror("Router::route");
                            }
                        }
                    }
                    send(i, raw_resp.data(), raw_resp.size(), 0);
                    if (!request.connected) {
                        close(conns[i].fd);
                        conns[i].fd = -1;
                        continue;
                    }
                    request.stage = HEADER;
                }
            }
            // if (conns[i].revents & POLLOUT && request.response != "") {
            //}
        }
    }
}
