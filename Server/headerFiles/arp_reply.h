/*
 * arp_reply.h
 *
 *  Created on: Oct 17, 2017
 *      Author: root
 */

#ifndef ARP_REPLY_H_
#define ARP_REPLY_H_

#endif /* ARP_REPLY_H_ */

#ifndef ARP_STRUCT_H_
#include "arp_struct.h"
#endif

#ifndef __NETINET_IP_H
#include <netinet/ip.h>
#endif

unsigned int crc32b(unsigned char *message, int framelen);

unsigned char src_mac[6] ={0xc4,0x34,0x6b,0x56,0xcd,0xb4};
char* send_arp_reply(char* request){
	unsigned char ether_frame[IP_MAXPACKET];
	unsigned int checksum = 0;
	struct arp_header *req = (struct arp_header *)(request+ETH2_HEADER_LEN);
	struct arp_header rep ;
	rep.hardware_type = htons(HW_TYPE);
	rep.protocol_type = htons(ARP_PROTO);
	rep.hardware_len = MAC_LENGTH;
	rep.protocol_len = IPV4_LENGTH;
	rep.opcode = htons(ARP_REPLY);

	memcpy(rep.sender_mac,src_mac,6*sizeof(unsigned char));
	memcpy(rep.target_mac,req->sender_mac,6*sizeof(unsigned char));
	memcpy(rep.sender_ip,req->target_ip,4*sizeof(unsigned char));
	memcpy(rep.target_ip,req->sender_ip,4*sizeof(unsigned char));
	bzero(rep.padding,10);


	int frame_length = 6+6+2+ARP_HDRLEN+10;

	memcpy(ether_frame,req->sender_mac,6*sizeof(unsigned char));
	memcpy(ether_frame+6,src_mac,6*sizeof(unsigned char));

    // ETHERNET TYPE code ARP_PROTO for ARP

	ether_frame[12] = ARP_PROTO/256;
	ether_frame[13] = ARP_PROTO%256;

	// ethernet frame data( ARP Header)

	memcpy(ether_frame+ETH2_HEADER_LEN,&rep,ARP_HDRLEN * sizeof(unsigned char));

	checksum = crc32b(&ether_frame,frame_length);
	return &rep;

}
unsigned int crc32b(unsigned char *message, int framelen) {
   int i, j;
   unsigned int byte, crc, mask;

   i = 0;
   crc = 0xFFFFFFFF;
   while (i<framelen) {
      byte = message[i];            // Get next byte.
      crc = crc ^ byte;
      for (j = 7; j >= 0; j--) {    // Do eight times.
         mask = -(crc & 1);
         crc = (crc >> 1) ^ (0xEDB88320 & mask);
      }
      i = i + 1;
   }
   return ~crc;
}

