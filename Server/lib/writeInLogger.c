/*
 *
 *
 *  Created on: Sep 7, 2017
 *      Author: deshanchathusanka
 */

#include <writeInLogger.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include "arp_struct.h"

void print_tcp_packet(unsigned char* Buffer, int Size, FILE *logfile) {
	unsigned short iphdrlen, tcphdrlen;
	int header_size, payloadSize, sessionId;
	int totLength;
	unsigned char *bufLocation;

	struct iphdr *iph = (struct iphdr *) (Buffer + sizeof(struct ethhdr)); //set ip header
	iphdrlen = iph->ihl * 4; //set ip header length
	struct tcphdr *tcph = (struct tcphdr*) (Buffer + iphdrlen
			+ sizeof(struct ethhdr)); //set tcp header
	tcphdrlen = tcph->doff * 4; //set tcp header length
	header_size = sizeof(struct ethhdr) + iphdrlen + tcphdrlen; //total header size

	fprintf(logfile,
			"\n\n***********************TCP Packet*************************\n");

	print_ip_header(Buffer, Size, logfile);

	fprintf(logfile, "\n");
	fprintf(logfile, "\nTCP Header\n");
	fprintf(logfile, "   |-Source Port      : %u\n", ntohs(tcph->source));
	fprintf(logfile, "   |-Destination Port : %u\n", ntohs(tcph->dest));
	fprintf(logfile, "   |-Sequence Number    : %u\n", ntohl(tcph->seq));
	fprintf(logfile, "   |-Acknowledge Number : %u\n", ntohl(tcph->ack_seq));
	fprintf(logfile, "   |-Header Length      : %d DWORDS or %d BYTES\n",
			(unsigned int) tcph->doff, (unsigned int) tcph->doff * 4);
	//fprintf(logfile , "   |-CWR Flag : %d\n",(unsigned int)tcph->cwr);
	//fprintf(logfile , "   |-ECN Flag : %d\n",(unsigned int)tcph->ece);
	fprintf(logfile, "   |-Urgent Flag          : %d\n",
			(unsigned int) tcph->urg);
	fprintf(logfile, "   |-Acknowledgement Flag : %d\n",
			(unsigned int) tcph->ack);
	fprintf(logfile, "   |-Push Flag            : %d\n",
			(unsigned int) tcph->psh);
	fprintf(logfile, "   |-Reset Flag           : %d\n",
			(unsigned int) tcph->rst);
	fprintf(logfile, "   |-Synchronise Flag     : %d\n",
			(unsigned int) tcph->syn);
	fprintf(logfile, "   |-Finish Flag          : %d\n",
			(unsigned int) tcph->fin);
	fprintf(logfile, "   |-Window         : %d\n", ntohs(tcph->window));
	fprintf(logfile, "   |-Checksum       : %d\n", ntohs(tcph->check));
	fprintf(logfile, "   |-Urgent Pointer : %d\n", tcph->urg_ptr);
	fprintf(logfile, "\n");
	fprintf(logfile,
			"                        DATA Dump                         ");
	fprintf(logfile, "\n");

	fprintf(logfile, "\nIP Header\n");
	logHexData(Buffer, iphdrlen, logfile);

	fprintf(logfile, "\nTCP Header\n");
	logHexData(Buffer + iphdrlen, tcph->doff * 4, logfile);

	fprintf(logfile, "\nData Payload\n");
	totLength = ntohs(iph->tot_len);
	logHexData(Buffer + header_size, totLength - (iphdrlen + tcphdrlen),
			logfile);

	fprintf(logfile,
			"\n###########################################################");
}

void print_ip_header(unsigned char* Buffer, int Size, FILE *logfile) {
	print_ethernet_header(Buffer, Size, logfile);

	unsigned short iphdrlen;

	struct iphdr *iph = (struct iphdr *) (Buffer + sizeof(struct ethhdr));
	iphdrlen = iph->ihl * 4;

	memset(&source, 0, sizeof(source));
	source.sin_addr.s_addr = iph->saddr;

	memset(&dest, 0, sizeof(dest));
	dest.sin_addr.s_addr = iph->daddr;

	fprintf(logfile, "\n");
	fprintf(logfile, "IP Header\n");
	fprintf(logfile, "   |-IP Version        : %d\n",
			(unsigned int) iph->version);
	fprintf(logfile, "   |-IP Header Length  : %d DWORDS or %d Bytes\n",
			(unsigned int) iph->ihl, ((unsigned int) (iph->ihl)) * 4);
	fprintf(logfile, "   |-Type Of Service   : %d\n", (unsigned int) iph->tos);
	fprintf(logfile, "   |-IP Total Length   : %d  Bytes(Size of Packet)\n",
			ntohs(iph->tot_len));
	fprintf(logfile, "   |-Identification    : %d\n", ntohs(iph->id));
	//fprintf(logfile , "   |-Reserved ZERO Field   : %d\n",(unsigned int)iphdr->ip_reserved_zero);
	//fprintf(logfile , "   |-Dont Fragment Field   : %d\n",(unsigned int)iphdr->ip_dont_fragment);
	//fprintf(logfile , "   |-More Fragment Field   : %d\n",(unsigned int)iphdr->ip_more_fragment);
	fprintf(logfile, "   |-TTL      : %d\n", (unsigned int) iph->ttl);
	fprintf(logfile, "   |-Protocol : %d\n", (unsigned int) iph->protocol);
	fprintf(logfile, "   |-Checksum : %d\n", ntohs(iph->check));
	fprintf(logfile, "   |-Source IP        : %s\n",
			inet_ntoa(source.sin_addr));
	fprintf(logfile, "   |-Destination IP   : %s\n", inet_ntoa(dest.sin_addr));
}

void print_ethernet_header(unsigned char* Buffer, int Size, FILE *logfile) {
	struct ethhdr *eth = (struct ethhdr *) Buffer;

	fprintf(logfile, "isARP(unsigned char* buffer, int size)\n");
	fprintf(logfile, "Ethernet Header\n");
	fprintf(logfile,
			"   |-Destination Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n",
			eth->h_dest[0], eth->h_dest[1], eth->h_dest[2], eth->h_dest[3],
			eth->h_dest[4], eth->h_dest[5]);
	fprintf(logfile,
			"   |-Source Address      : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n",
			eth->h_source[0], eth->h_source[1], eth->h_source[2],
			eth->h_source[3], eth->h_source[4], eth->h_source[5]);
	fprintf(logfile, "   |-Protocol            : %u \n",
			(unsigned short) eth->h_proto);
}

void logHexData(unsigned char* data, int Size, FILE *logfile) {

	fprintf(logfile, "\n\n");
	int i, j;
	for (i = 0; i < Size; i++) {

		if (i != 0 && i % 16 == 0)  //if one line of hex printing is complete...
				{
			fprintf(logfile, "         ");
			fprintf(logfile, "\n");

		}

		if (i % 16 == 0)
			fprintf(logfile, "   ");
		fprintf(logfile, " %02X", (unsigned int) data[i]);

	}
	fprintf(logfile, "\n");

}

void logRawData(unsigned char* data, int Size, FILE *logfile) {

	int i, j;
	for (i = 0; i < Size; i++) {
		fprintf(logfile, "%c", data[i]);
	}
}

void printHexData(unsigned char* data, int Size) {

	int i, j;
	for (i = 0; i < Size; i++) {

		if (i != 0 && i % 16 == 0)  //if one line of hex printing is complete...
				{
			printf("         ");
			printf("\n");

		}

		if (i % 16 == 0)
			printf("   ");
		printf(" %02X", (unsigned int) data[i]);

	}

}

void printRawData(unsigned char* data, int Size) {

	int i, j;
	for (i = 0; i < Size; i++) {

		/*if( i!=0 && i%16==0)   //if one line of hex printing is complete...
		 {
		 printf("         ");
		 printf("\n");

		 }

		 if(i%16==0) printf("   ");*/
		printf("%c", data[i]);

	}

}

void print_arp(FILE *logfile, unsigned char *Buffer) {
	//ethernet part
	struct ethhdr *ethr = (struct ethhdr *) Buffer;
	fprintf(logfile,
			"*************************arp packet********************\n\n");
	fprintf(logfile,
			"***********************ETHERNET HEADER*****************\n\n");
	fprintf(logfile, "Sender MAC:%02X:%02X:%02X:%02X:%02X:%02X \n",
			ethr->h_source[0], ethr->h_source[1], ethr->h_source[2],
			ethr->h_source[3], ethr->h_source[4], ethr->h_source[5]);
	fprintf(logfile, "Target MAC: %02X:%02X:%02X:%02X:%02X:%02X \n",
			ethr->h_dest[0], ethr->h_dest[1], ethr->h_dest[2], ethr->h_dest[3],
			ethr->h_dest[4], ethr->h_dest[5]);
	fprintf(logfile, "protocol: %u \n", ethr->h_proto);
	fprintf(logfile,
			"*******************************************************\n\n");

	struct arp_header *ah = (struct arp_header *) (Buffer + ETH2_HEADER_LEN);

	fprintf(logfile,
			"***************ARP INFO********************************\n");
	fprintf(logfile, "Hardware type: %u \n", ntohs(ah->hardware_type));
	fprintf(logfile, "Protocol type :%u \n", ah->protocol_type);
	fprintf(logfile, "OPcode : %u \n", ntohs(ah->opcode));

	if (ntohs(ah->opcode) == ARP_REQUEST) {
		printf("ARP REQUEST \n");
		fprintf(logfile, "ARP REQUEST \n");
	} else if (ntohs(ah->opcode) == ARP_REPLY) {
		printf("ARP REPLY \n");
		fprintf(logfile, "ARP REPLY \n");
	}

	fprintf(logfile, "Sender IP: %u.%u.%u.%u \n", ah->sender_ip[0],
			ah->sender_ip[1], ah->sender_ip[2], ah->sender_ip[3]);
	fprintf(logfile, "Sender MAC: %02X:%02X:%02X:%02X:%02X:%02X \n",
			ah->sender_mac[0], ah->sender_mac[1], ah->sender_mac[2],
			ah->sender_mac[3], ah->sender_mac[4], ah->sender_mac[5]);
	fprintf(logfile, "target IP: %u.%u.%u.%u \n", ah->target_ip[0],
			ah->target_ip[1], ah->target_ip[2], ah->target_ip[3]);
	//fprintf(logfile,"%S\n",inet_ntoa(target_a));
	if (ah->target_mac) {
		fprintf(logfile, "target MAC: %02X:%02X:%02X:%02X:%02X:%02X \n",
				ah->target_mac[0], ah->target_mac[1], ah->target_mac[2],
				ah->target_mac[3], ah->target_mac[4], ah->target_mac[5]);
	}
	fprintf(logfile,
			"*******************************************************\n\n");

}

