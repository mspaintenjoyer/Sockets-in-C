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
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <server_ip> <port> <message>\n", argv[0]);
        return 1;
    }
    const char* server_ip = argv[1];
    int port = atoi(argv[2]);

    // Join remaining args as the message (so spaces work)
    size_t total_len = 0;
    for (int i = 3; i < argc; i++) total_len += strlen(argv[i]) + 1;
    char* msg = (char*)malloc(total_len + 1);
    if (!msg) die("malloc");
    msg[0] = '\0';
    for (int i = 3; i < argc; i++) {
        strcat(msg, argv[i]);
        if (i + 1 < argc) strcat(msg, " ");
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) die("socket");

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    if (inet_pton(AF_INET, server_ip, &addr.sin_addr) != 1) {
        fprintf(stderr, "Invalid server IP\n");
        close(fd);
        free(msg);
        return 1;
    }

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) die("connect");

    // Send the message
    size_t len = strlen(msg);
    const char* p = msg;
    while (len > 0) {
        ssize_t n = send(fd, p, len, 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            die("send");
        }
        p += n;
        len -= (size_t)n;
    }

    // Receive echo (up to 4KB)
    char buf[4096];
    ssize_t r = recv(fd, buf, sizeof(buf) - 1, 0);
    if (r < 0) die("recv");
    buf[r >= 0 ? r : 0] = '\0';

    printf("Echoed: %s\n", buf);

    close(fd);
    free(msg);
    return 0;
}
