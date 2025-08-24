#define _POSIX_C_SOURCE 200112L
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

static void die(const char* msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <bind_ip> <port>\n", argv[0]);
        return 1;
    }

    const char* bind_ip = argv[1];
    int port = atoi(argv[2]);

    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) die("socket");

    int yes = 1;
    if (setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        die("setsockopt");
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    if (inet_pton(AF_INET, bind_ip, &addr.sin_addr) != 1) {
        fprintf(stderr, "Invalid bind IP\n");
        close(srv);
        return 1;
    }

    if (bind(srv, (struct sockaddr*)&addr, sizeof(addr)) < 0) die("bind");
    if (listen(srv, 16) < 0) die("listen");

    printf("Echo server listening on %s:%d\n", bind_ip, port);

    for (;;) {
        struct sockaddr_in cli;
        socklen_t cli_len = sizeof(cli);
        int cfd = accept(srv, (struct sockaddr*)&cli, &cli_len);
        if (cfd < 0) {
            if (errno == EINTR) continue;
            die("accept");
        }

        char ipstr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &cli.sin_addr, ipstr, sizeof(ipstr));
        printf("Client connected from %s:%d\n", ipstr, ntohs(cli.sin_port));

        char buf[4096];
        for (;;) {
            ssize_t n = recv(cfd, buf, sizeof(buf), 0);
            if (n == 0) {
                // client closed
                break;
            } else if (n < 0) {
                if (errno == EINTR) continue;
                perror("recv");
                break;
            }
            // echo back
            ssize_t off = 0;
            while (off < n) {
                ssize_t m = send(cfd, buf + off, (size_t)(n - off), 0);
                if (m < 0) {
                    if (errno == EINTR) continue;
                    perror("send");
                    break;
                }
                off += m;
            }
            if (off < n) break; // send error
        }

        printf("Client disconnected\n");
        close(cfd);
    }

    close(srv);
    return 0;
}
