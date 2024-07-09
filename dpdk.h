#include <rte_ethdev.h>

struct rte_mempool* init_dpdk(int argc, char** argv);
void send_udp_packet(struct rte_mempool* mbuf_pool, char* message, int len);
void send_udp_packet_batch(struct rte_mempool* mbuf_pool, char** message, int items_len, int len);

