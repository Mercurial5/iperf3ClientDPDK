#include <rte_ethdev.h>

#define PORT_ID 0
#define QUEUE_ID 0
#define IP_DEFTTL 64
#define RTE_CPU_TO_BE_16(cpu_16_v) (uint16_t) ((((cpu_16_v) & 0xFF) << 8) | ((cpu_16_v) >> 8))

// static struct rte_ether_addr cfg_ether_dst = {{ 0x54, 0x8d, 0x5a, 0xac, 0xda, 0xa0 }};
static struct rte_ether_addr cfg_ether_dst = {{ 0x08, 0x00, 0x27, 0x8c, 0x1d, 0xa5 }};
static uint32_t cfg_ip_src = RTE_IPV4(0, 0, 0, 1);
static uint32_t cfg_ip_dst = RTE_IPV4(192, 168, 25, 125);
static uint16_t cfg_udp_src = 8888;
static uint16_t cfg_udp_dst = 5201;

struct rte_mempool* init_dpdk(int argc, char** argv) {
    int error;
    if ((error = rte_eal_init(argc, argv)) == -1) {
        rte_exit(EXIT_FAILURE, "Invalid EAL arguments: %s\n", rte_strerror(-error));
    }
    
    // create mbuf pool
    const char name[] = "mbpool";
    struct rte_mempool *mbuf_pool = rte_pktmbuf_pool_create(name, 4095, 250, 0, 4095, SOCKET_ID_ANY);
    
    // configure ethernet device
    struct rte_eth_conf dev_conf = {};
    if ((error = rte_eth_dev_configure(PORT_ID, 1, 1, &dev_conf)) < 0) {
        rte_exit(EXIT_FAILURE, "Could not configure eth device: %s\n", rte_strerror(-error));
    }
    
    // configure tx queue
    struct rte_eth_txconf tx_conf = {};
    if ((error = rte_eth_tx_queue_setup(PORT_ID, QUEUE_ID, 0, SOCKET_ID_ANY, &tx_conf)) < 0) {
        rte_exit(EXIT_FAILURE, "Could not configure tx queue: %s\n", rte_strerror(-error));
    }

    // start ethernet device
    if ((error = rte_eth_dev_start(PORT_ID)) < 0) {
        rte_exit(EXIT_FAILURE, "Could not start eth device: %s\n", rte_strerror(-error));
    }

    return mbuf_pool;
}

void send_udp_packet(struct rte_mempool* mbuf_pool, char* message, int len) {
    int pkt_size = sizeof(struct rte_ether_hdr) + 
        sizeof(struct rte_ipv4_hdr) + 
        sizeof(struct rte_udp_hdr) + 
        sizeof(char) * len;

    struct rte_mbuf *pkt = rte_mbuf_raw_alloc(mbuf_pool);
    
    // Get mac address of current ethernet device
    static struct rte_ether_addr cfg_ether_src;
    rte_eth_macaddr_get(PORT_ID, &cfg_ether_src);
    
    // Initialize ethernet header
    struct rte_ether_hdr *eth_hdr;
    eth_hdr = rte_pktmbuf_mtod(pkt, struct rte_ether_hdr*);
    rte_ether_addr_copy(&cfg_ether_dst, &eth_hdr->dst_addr);
    rte_ether_addr_copy(&cfg_ether_src, &eth_hdr->src_addr);
    eth_hdr->ether_type = rte_cpu_to_be_16(RTE_ETHER_TYPE_IPV4);
    
    // Initialize ipv4 header
    struct rte_ipv4_hdr *ip_hdr;
    ip_hdr = (struct rte_ipv4_hdr *) (eth_hdr + 1);
    memset(ip_hdr, 0, sizeof(*ip_hdr));
    ip_hdr->version_ihl = RTE_IPV4_VHL_DEF;
    ip_hdr->type_of_service = 0;
    ip_hdr->fragment_offset = 0;
    ip_hdr->time_to_live = IP_DEFTTL;
    ip_hdr->next_proto_id = IPPROTO_UDP;
    ip_hdr->packet_id = 0;
    ip_hdr->src_addr = rte_cpu_to_be_32(cfg_ip_src);
    ip_hdr->dst_addr = rte_cpu_to_be_32(cfg_ip_dst);
    ip_hdr->total_length = RTE_CPU_TO_BE_16(pkt_size - sizeof(*eth_hdr));
    ip_hdr->hdr_checksum = rte_ipv4_cksum(ip_hdr);
    
    struct rte_udp_hdr *udp_hdr;
    udp_hdr = (struct rte_udp_hdr *)(ip_hdr + 1);
    udp_hdr->src_port = rte_cpu_to_be_16(cfg_udp_src);
    udp_hdr->dst_port = rte_cpu_to_be_16(cfg_udp_src);
    udp_hdr->dgram_cksum = 0;
    udp_hdr->dgram_len = RTE_CPU_TO_BE_16(pkt_size - sizeof(*eth_hdr) - sizeof(*ip_hdr));
    
    char *msg = (char*)(rte_pktmbuf_mtod(pkt, char*) + pkt_size - sizeof(char) * len);
    *msg = *message;

    pkt->nb_segs = 1;
    pkt->pkt_len = pkt_size;
    pkt->ol_flags &= RTE_MBUF_F_EXTERNAL;
    pkt->l2_len = sizeof(struct rte_ether_hdr);
    pkt->l3_len = sizeof(struct rte_ipv4_hdr);

    pkt->data_len = pkt_size;
    pkt->pkt_len = pkt_size;
    struct rte_mbuf* tx_packets[] = { pkt };
    int ret = rte_eth_tx_burst(PORT_ID, QUEUE_ID, tx_packets, 1);
    if (ret != 1) {
        printf("ret is not 1: %d\n", ret);
    }

    rte_mbuf_raw_free(pkt);
}

