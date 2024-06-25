#include <sys/types.h>
#include <sys/socket.h>

struct addrinfo* get_addrinfo(const char host[], const char port[]);
int connect_(const char host[], const char port[]); 
int send_(int sfd, const char *message);

