#include "dpdk.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "http.h"

unsigned char COOKIE[] = { 'e', 'x', 'f', 'o', 'y', 'c', 'p', 't', 's', '4', 'l', '4', 'r', 't', '2', 'y', '6', '2', '2', 't', 'l', 'x', 'g', 'd', 'b', 'x', '7', 'k', 'm', 'o', 'u', 'j', '2', 'e', 'l', '2', 0};

char* gen_random(const int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    char* tmp_s = (char *)malloc(len * sizeof(char));

    for (int i = 0; i < len; ++i) {
        tmp_s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    
    return tmp_s;
}

int main(int argc, char** argv) {
    struct rte_mempool* mempool = init_dpdk(argc, argv);

    const char host[] = "192.168.137.97";
    const char port[] = "5201";

    int sfd = connect_(host, port);
    if (sfd == -1) {
        printf("Could not connect\n");
        return 0;
    }

    write(sfd, COOKIE, 37);

    unsigned char message_one[] = {0, 0, 0, 143};
    write(sfd, message_one, 4);

    char message_second[] = "{\"udp\":true,\"omit\":0,\"time\":10,\"num\":0,\"blockcount\":0,\"parallel\":1,\"len\":1000,\"bandwidth\":1048576,\"pacing_timer\":1000,\"client_version\":\"3.16+\"}";
    write(sfd, (char *)&message_second, sizeof(message_second) - 1);
    
    char** messages = (char **)(malloc(10 * sizeof(char*)));
    for (int i = 0; i < 10; i++) {
        messages[i] = gen_random(1000);
    }

    for (int i = 0; i < 1000000; i++) {
        send_udp_packet_batch(mempool, messages, 10, 1000);
        if (i % 10000 == 0) printf("Sent %d packets\n", i);
    }    

    int x = 4;
    write(sfd, (char *) &x, sizeof(char));

    x = 16;
    write(sfd, (char *) &x, sizeof(char));

    char* response = malloc(1025 * sizeof(char));
    int r = read(sfd, response, 1024); 
    response[1024] = '\0';

    printf("response: %s\n", response);

    for (int i = 0; i < 10; i++) {
        free(messages[i]);
    }

    free(messages);
    return 0;
}
