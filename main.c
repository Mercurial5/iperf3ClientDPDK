#include <rte_ethdev.h>

#define PORT_ID 0
#define QUEUE_ID 0
#define TXONLY_DEF_PACKET_LEN 64
#define IP_DEFTTL  64   /* from RFC 1340. */
#define RTE_CPU_TO_BE_16(cpu_16_v) (uint16_t) ((((cpu_16_v) & 0xFF) << 8) | ((cpu_16_v) >> 8))

static struct rte_ether_addr cfg_ether_src = {{ 0x82, 0x89, 0x56, 0x16, 0x91, 0x5b }};
static struct rte_ether_addr cfg_ether_dst = {{ 0x02, 0x42, 0xde, 0xb4, 0xa3, 0xf0 }};
static uint32_t cfg_ip_src = RTE_IPV4(127, 0, 0, 1);
static uint32_t cfg_ip_dst = RTE_IPV4(172, 17, 0, 1);
static uint16_t cfg_udp_src	= 8888;
static uint16_t cfg_udp_dst	= 5201;

struct InitMessage {
    char data[4];
};

struct Message {
    char data[1012];
};

void send_init_message(struct rte_mbuf *pkt, struct InitMessage *message) {
    struct InitMessage* msg;
    struct rte_ether_hdr *eth_hdr;
    /* Initialize Ethernet header. */
    eth_hdr = rte_pktmbuf_mtod(pkt, struct rte_ether_hdr *);
    rte_ether_addr_copy(&cfg_ether_dst, &eth_hdr->dst_addr);
    rte_ether_addr_copy(&cfg_ether_src, &eth_hdr->src_addr);
    eth_hdr->ether_type = rte_cpu_to_be_16(RTE_ETHER_TYPE_IPV4);

    int pkt_size = sizeof(struct rte_ether_hdr) + sizeof(struct rte_ipv4_hdr) + sizeof(struct rte_udp_hdr) + sizeof(struct InitMessage);

    struct rte_ipv4_hdr *ip_hdr;
    /* Initialize IP header. */
    ip_hdr = (struct rte_ipv4_hdr *)(eth_hdr + 1);
    memset(ip_hdr, 0, sizeof(*ip_hdr));
    ip_hdr->version_ihl	= RTE_IPV4_VHL_DEF;
    ip_hdr->type_of_service	= 0;
    ip_hdr->fragment_offset	= 0;
    ip_hdr->time_to_live	= IP_DEFTTL;
    ip_hdr->next_proto_id	= IPPROTO_UDP;
    ip_hdr->packet_id	= 0;
    ip_hdr->src_addr	= rte_cpu_to_be_32(cfg_ip_src);
    ip_hdr->dst_addr	= rte_cpu_to_be_32(cfg_ip_dst);
    ip_hdr->total_length	= RTE_CPU_TO_BE_16(pkt_size - sizeof(*eth_hdr));
    ip_hdr->hdr_checksum	= rte_ipv4_cksum(ip_hdr);

    struct rte_udp_hdr *udp_hdr;
    /* Initialize UDP header. */
    udp_hdr = (struct rte_udp_hdr *)(ip_hdr + 1);
    udp_hdr->src_port	= rte_cpu_to_be_16(cfg_udp_src);
    udp_hdr->dst_port	= rte_cpu_to_be_16(cfg_udp_dst);
    udp_hdr->dgram_cksum	= 0; /* No UDP checksum. */
    udp_hdr->dgram_len	= RTE_CPU_TO_BE_16(pkt_size - sizeof(*eth_hdr) - sizeof(*ip_hdr));

    msg = (struct InitMessage*) (rte_pktmbuf_mtod(pkt, char*) + pkt_size - sizeof(struct InitMessage));
    *msg = *message;

    uint64_t ol_flags = 0;

    pkt->nb_segs		= 1;
    pkt->pkt_len		= pkt_size;
    pkt->ol_flags		&= RTE_MBUF_F_EXTERNAL;
    pkt->ol_flags		|= ol_flags;
    pkt->l2_len		= sizeof(struct rte_ether_hdr);
    pkt->l3_len		= sizeof(struct rte_ipv4_hdr);

    pkt->data_len = pkt_size;
    pkt->pkt_len = pkt_size;

    struct rte_mbuf* tx_packets[] = {pkt};
    int ret = rte_eth_tx_burst(PORT_ID, QUEUE_ID, tx_packets, 1);
    printf("returned value: %d\n", ret);
    printf("packet size: %d\n", pkt_size);
    printf("rte_ether_hdr struct size: %lu\n", sizeof(struct rte_ether_hdr));
}

void send_message(struct rte_mbuf *pkt, struct Message *message) {
    struct Message* msg;
    struct rte_ether_hdr *eth_hdr;
    /* Initialize Ethernet header. */
    eth_hdr = rte_pktmbuf_mtod(pkt, struct rte_ether_hdr *);
    rte_ether_addr_copy(&cfg_ether_dst, &eth_hdr->dst_addr);
    rte_ether_addr_copy(&cfg_ether_src, &eth_hdr->src_addr);
    eth_hdr->ether_type = rte_cpu_to_be_16(RTE_ETHER_TYPE_IPV4);

    int pkt_size = sizeof(struct rte_ether_hdr) + sizeof(struct rte_ipv4_hdr) + sizeof(struct rte_udp_hdr) + sizeof(struct Message);

    struct rte_ipv4_hdr *ip_hdr;
    /* Initialize IP header. */
    ip_hdr = (struct rte_ipv4_hdr *)(eth_hdr + 1);
    memset(ip_hdr, 0, sizeof(*ip_hdr));
    ip_hdr->version_ihl	= RTE_IPV4_VHL_DEF;
    ip_hdr->type_of_service	= 0;
    ip_hdr->fragment_offset	= 0;
    ip_hdr->time_to_live	= IP_DEFTTL;
    ip_hdr->next_proto_id	= IPPROTO_UDP;
    ip_hdr->packet_id	= 0;
    ip_hdr->src_addr	= rte_cpu_to_be_32(cfg_ip_src);
    ip_hdr->dst_addr	= rte_cpu_to_be_32(cfg_ip_dst);
    ip_hdr->total_length	= RTE_CPU_TO_BE_16(pkt_size - sizeof(*eth_hdr));
    ip_hdr->hdr_checksum	= rte_ipv4_cksum(ip_hdr);

    struct rte_udp_hdr *udp_hdr;
    /* Initialize UDP header. */
    udp_hdr = (struct rte_udp_hdr *)(ip_hdr + 1);
    udp_hdr->src_port	= rte_cpu_to_be_16(cfg_udp_src);
    udp_hdr->dst_port	= rte_cpu_to_be_16(cfg_udp_dst);
    udp_hdr->dgram_cksum	= 0; /* No UDP checksum. */
    udp_hdr->dgram_len	= RTE_CPU_TO_BE_16(pkt_size - sizeof(*eth_hdr) - sizeof(*ip_hdr));

    msg = (struct Message*) (rte_pktmbuf_mtod(pkt, char*) + pkt_size - sizeof(struct Message));
    *msg = *message;

    uint64_t ol_flags = 0;

    pkt->nb_segs		= 1;
    pkt->pkt_len		= pkt_size;
    pkt->ol_flags		&= RTE_MBUF_F_EXTERNAL;
    pkt->ol_flags		|= ol_flags;
    pkt->l2_len		= sizeof(struct rte_ether_hdr);
    pkt->l3_len		= sizeof(struct rte_ipv4_hdr);

    pkt->data_len = pkt_size;
    pkt->pkt_len = pkt_size;

    struct rte_mbuf* tx_packets[] = {pkt};
    int ret = rte_eth_tx_burst(PORT_ID, QUEUE_ID, tx_packets, 1);
    printf("returned value: %d\n", ret);
    printf("packet size: %d\n", pkt_size);
    printf("rte_ether_hdr struct size: %lu\n", sizeof(struct rte_ether_hdr));
}

int main(int argc, char** argv) {
    int ret;
    if ((ret = rte_eal_init(argc, argv)) == -1) {
        rte_exit(EXIT_FAILURE, "Invalid EAL arguments: err=%s\n", rte_strerror(-ret));
    }
    printf("Number of ports: %d\n", rte_eth_dev_count_avail());

    const char name[] = "mbpool";
    struct rte_mempool *pktmbuf_pool = rte_pktmbuf_pool_create(name, 4095, 250, 0, RTE_MBUF_DEFAULT_BUF_SIZE, SOCKET_ID_ANY);
    
    struct rte_eth_conf dev_conf = {};
    int RX_QUEUE = 1;
    int TX_QUEUE = 1;
    if((ret = rte_eth_dev_configure(PORT_ID, RX_QUEUE, TX_QUEUE, &dev_conf)) < 0) {
        rte_exit(EXIT_FAILURE, "Cannot configure device: err=%s, port=%u\r\n", rte_strerror(-ret), PORT_ID);
    }
    
    struct rte_eth_txconf tx_conf = {};
    if ((ret = rte_eth_tx_queue_setup(PORT_ID, QUEUE_ID, 0, SOCKET_ID_ANY, &tx_conf)) < 0) {
        rte_exit(EXIT_FAILURE, "TX Queue setup error: err=%s, port=%u\r\n", rte_strerror(-ret), PORT_ID);
    }

    if ((ret = rte_eth_dev_start(PORT_ID)) < 0) {
        rte_exit(EXIT_FAILURE, "Start error: err=%s, port=%u\r\n", rte_strerror(-ret), PORT_ID);
    }
    
    struct InitMessage init_message = {{'9', '8', '7', '6'}};
    // Creating a packet
    struct rte_mbuf *pkt = rte_mbuf_raw_alloc(pktmbuf_pool);
    if (!pkt) {    
        rte_exit(EXIT_FAILURE, "Packet allocation error: err=%s, port=%u\r\n", rte_strerror(-ret), PORT_ID);
    } 
 
    send_init_message(pkt, &init_message);

    for (int j = 0; j < 10000000; j++) {
        struct Message message;
        for (int i = 0; i < 8; i++) {
            message.data[i] = i;
        }

        message.data[8] = (j >> 24) & 0xFF;
        message.data[9] = (j >> 16) & 0xFF;
        message.data[10] = (j >> 8) & 0xFF;
        message.data[11] = j & 0xFF;


        for (int i = 12; i < 1000; i++) {
            message.data[i] = (i % 10) + '0';
        }    
        // Creating a packet
        pkt = rte_mbuf_raw_alloc(pktmbuf_pool);
        if (!pkt) {
            rte_exit(EXIT_FAILURE, "Packet allocation error: err=%s, port=%u\r\n", rte_strerror(-ret), PORT_ID);
        }
        send_message(pkt, &message);
    }

    return 0;
}
