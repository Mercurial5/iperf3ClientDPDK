#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

struct addrinfo* get_addrinfo(const char host[], const char port[]) {
    struct addrinfo hints;
    struct addrinfo* result;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    int error = getaddrinfo(host, port, &hints, &result);
    if (error != 0) {
        printf("Failed to fetch address info: %s\n", gai_strerror(error));
        return NULL;
    }

    return result;
}

int connect_(const char host[], const char port[]) {
    struct addrinfo* result = get_addrinfo(host, port);
    struct addrinfo* r = result;

    int sfd;
    int connected = 0;
    while (r) {
        // Here instead of result->... I could've used the constants such as AF_INET, SOCK_STREAM, 0.
        // But in documentation they decided to go with this more flexible way, so I stuck with it.
        sfd = socket(r->ai_family, r->ai_socktype, r->ai_protocol);
        if (sfd == -1) {
            freeaddrinfo(result);
            printf("Failed to open socket: %s\n", gai_strerror(errno));
            return 0;
        }

        int error = connect(sfd, r->ai_addr, r->ai_addrlen);
        if (error != -1) {
            connected = 1;
            break;
        }

        close(sfd);
        break;
    }

    if (!connected) {
        printf("Failed to connect\n");
        return 0;
    }


    return sfd;
}

int send_(int sfd, const char *message) {
    printf("strlen: %d\n", strlen(message));
    int error = write(sfd, message, strlen(message));
    if (error == -1) {
        printf("Failed to write: %s\n", gai_strerror(errno));
        return 0;
    }
    else if (error != strlen(message)) {
        printf("Partially failed to write: %s\n", gai_strerror(errno));
        return 0;
    }

    return 1;
}

