#include <rte_ethdev.h>

struct rte_mempool* init_dpdk(int argc, char** argv);
void send_udp_packet(struct rte_mempool* mbuf_pool, char* message, int len);

