#ifndef ARP_STRUCT_H_
#define ARP_STRUCT_H_



#endif /* ARP_STRUCT_H_ */

//********************For ARP*******************************************//
#define ARP_PROTO 0x0806
#define ETH2_HEADER_LEN 14
#define HW_TYPE 1
#define MAC_LENGTH 6
#define IPV4_LENGTH 4
#define ARP_REQUEST 0x01
#define ARP_REPLY 0x02
#define IP4_HDRLEN 20
#define ARP_HDRLEN 28
//**********************************************************************//

struct arp_header {
    unsigned short hardware_type;
    unsigned short protocol_type;
    unsigned char hardware_len;
    unsigned char protocol_len;
    unsigned short opcode;
    unsigned char sender_mac[MAC_LENGTH];
    unsigned char sender_ip[IPV4_LENGTH];
    unsigned char target_mac[MAC_LENGTH];
    unsigned char target_ip[IPV4_LENGTH];
    unsigned char padding[10];
};
