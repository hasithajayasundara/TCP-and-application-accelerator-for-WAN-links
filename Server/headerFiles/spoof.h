/*
 * spoof.h
 *
 *  Created on: Dec 19, 2017
 *      Author: root
 */

#ifndef SPOOF_H_
#define SPOOF_H_

#endif /* SPOOF_H_ */

#ifndef _NETINET_TCP_H_
#include <netinet/tcp.h>
#endif

#ifndef _NETINET_IP_H_
#include <netinet/ip.h>
#endif

#ifndef _LINUX_IF_ETHER_H
#include <linux/if_ether.h>
#endif

#ifndef NETINET_IN_H
#include<netinet/in.h>
#endif

#ifndef	_SYS_SOCKET_H
#include <sys/socket.h>
#endif



u_int32_t source_address = 168333752;
u_int32_t destination_address = 170524674;


unsigned short csum(unsigned short* ,int);
unsigned short compute_tcp_checksum(struct iphdr *, unsigned short *);
void compute_ip_checksum(struct iphdr* );
static unsigned short compute_checksum(unsigned short *, unsigned int );
char * syn(char* source_mac,char* target,SESSION* session);
void stringCopy(unsigned char* dest, unsigned char* src, int size);
char * dataPkt(char* source_mac,char* target,SESSION* session,char* payload,int size);
char * fin(char* source_mac,char* target,SESSION* session);

char * dataPkt(char* source_mac,char* target,SESSION* session,char* payload,int size){
	char* datagram=(char*)malloc(1518);
	int hdrLen=0;
	struct ethhdr *eths = (struct ethhdr *)datagram;

	struct iphdr *iphs = (struct iphdr *) (datagram + sizeof(struct ethhdr ));
	// ip header of the spoofed packet
	struct tcphdr *tcphs =  (struct tcphdr *)(datagram + sizeof(struct iphdr)+sizeof(struct ethhdr )); // ip header of spoofed packet
	memset(datagram,0,1518);


	iphs->ihl = sizeof(struct iphdr)/4;;//sizeof(struct iphdr )/4;
	iphs->version = 4;
	iphs->tos = 0;
	iphs->tot_len = htons(sizeof(struct iphdr )+sizeof(struct tcphdr )+size);
	iphs->id=htonl(10000 + rand () % 9000);
	iphs->frag_off= 0;//htons(16384);
	iphs->ttl = 255;
	iphs->protocol = IPPROTO_TCP;
	iphs->check = 0;		//set to 0 before calculating checksum
	iphs->saddr = session->sourceIP;//htonl(source_address);
	iphs->daddr = session->destIP;//htonl(destination_address);


	compute_ip_checksum(iphs);

	tcphs->source = htons(session->sourcePort);
	tcphs->dest = htons(session->destPort);
	tcphs->seq = htonl(session->seqNum);
	tcphs->ack_seq = htonl(session->ackNum);
	tcphs->res1 = 0;
	tcphs->doff = sizeof(struct tcphdr)/4;
	tcphs->fin = 0;
    tcphs->syn = 0;
    tcphs->rst = 0;
    tcphs->psh = 0;
    tcphs->ack = 1;
    tcphs->urg =0;
    tcphs->window = htons(2000); // maximum allowed window size;
    tcphs->check  =0 ;
    tcphs->urg_ptr =0 ;
    tcphs->res2 = 0;


    hdrLen =  sizeof(struct ethhdr) + (iphs->ihl*4) + (tcphs->doff*4);//total header size
    strconcat(datagram+hdrLen,payload,size);

    tcphs->check = compute_tcp_checksum(iphs,(unsigned short*)tcphs);


	eths->h_source[0] = source_mac[0];
	eths->h_source[1] = source_mac[1];
	eths->h_source[2] = source_mac[2];
	eths->h_source[3] = source_mac[3];
	eths->h_source[4] = source_mac[4];
	eths->h_source[5] = source_mac[5];

	eths->h_dest[0] = target[0];
	eths->h_dest[1] = target[1];
	eths->h_dest[2] = target[2];
	eths->h_dest[3] = target[3];
	eths->h_dest[4] = target[4];
	eths->h_dest[5] = target[5];

	eths->h_proto = 8;

    //update axk_seq and seq number
    session->ackNum=ntohl(tcphs->ack_seq);
    session->seqNum=ntohl(tcphs->seq);

	return datagram;
}


char * syn_ack(char * pkbuff,u_int32_t seq){
	char* datagram=(char*)malloc(1518);
	struct ethhdr *ethr = (struct ethhdr *)pkbuff;
	struct ethhdr *eths = (struct ethhdr *)datagram;

	struct iphdr *iphr = (struct iphdr *)(pkbuff+sizeof(struct ethhdr ));  //ip header of incoming packet
	struct iphdr *iphs = (struct iphdr *) (datagram + sizeof(struct ethhdr ));    // ip header of the spoofed packet

	struct tcphdr *tcphr = (struct tcphdr *)(pkbuff+sizeof(struct ethhdr )+sizeof(struct iphdr )); // TCP header of incomming packet
	struct tcphdr *tcphs =  (struct tcphdr *)(datagram + sizeof(struct iphdr)+sizeof(struct ethhdr )); // ip header of spoofed packet



	memset(datagram,0,1518); /*zero out the buffer */

	// Fill in the IP header
	iphs->ihl =sizeof(struct iphdr)/4;//5;
	iphs->version = 4;
	iphs->tos = 0;
	iphs->tot_len = htons(sizeof(struct iphdr )+sizeof(struct tcphdr));
	iphs->id = iphr->id;
	iphs->frag_off= 0x00;//htons(16384);
	iphs->ttl = 64;
	iphs->protocol = IPPROTO_TCP;
	iphs->check = 0;		//set to 0 before calculating checksum
	iphs->saddr = iphr->daddr;
	iphs->daddr = iphr->saddr;

    compute_ip_checksum(iphs);

	//TCP header
	tcphs->source = tcphr->dest;
	tcphs->dest = tcphr->source;
	tcphs->seq = htonl(seq);//htonl(1000000 + rand () % 90000);
	tcphs->ack_seq = htonl(ntohl(tcphr->seq) +1);
	tcphs->doff=sizeof(struct tcphdr)/4;
	tcphs->res1 = 0;
	tcphs->res2 = 0;
    tcphs->fin = 0;
    tcphs->syn = 1;
    tcphs->rst = 0;
    tcphs->psh = 0;
    tcphs->ack = 1;
    tcphs->urg =0;
    tcphs->window = htons(2000); // maximum allowed window size;
    tcphs->check  =0 ;
    tcphs->urg_ptr =0 ;





    tcphs->check = compute_tcp_checksum(iphs,(unsigned short*)tcphs);


    eths->h_dest[0] = ethr->h_source[0];
    eths->h_dest[1] = ethr->h_source[1];
    eths->h_dest[2] = ethr->h_source[2];
    eths->h_dest[3] = ethr->h_source[3];
    eths->h_dest[4] = ethr->h_source[4];
    eths->h_dest[5] = ethr->h_source[5];

    eths->h_source[0] = ethr->h_dest[0];
    eths->h_source[1] = ethr->h_dest[1];
    eths->h_source[2] = ethr->h_dest[2];
    eths->h_source[3] = ethr->h_dest[3];
    eths->h_source[4] = ethr->h_dest[4];
    eths->h_source[5] = ethr->h_dest[5];

    eths->h_proto = 8;

    return datagram;


}

char * syn(char* source_mac,char* target,SESSION* session){
	char* datagram=(char*)malloc(1518);
	struct ethhdr *eths = (struct ethhdr *)datagram;

	struct iphdr *iphs = (struct iphdr *) (datagram + sizeof(struct ethhdr ));
	// ip header of the spoofed packet
	struct tcphdr *tcphs =  (struct tcphdr *)(datagram + sizeof(struct iphdr)+sizeof(struct ethhdr )); // ip header of spoofed packet
	memset(datagram,0,1518);


	eths->h_source[0] = source_mac[0];
	eths->h_source[1] = source_mac[1];
	eths->h_source[2] = source_mac[2];
	eths->h_source[3] = source_mac[3];
	eths->h_source[4] = source_mac[4];
	eths->h_source[5] = source_mac[5];

	eths->h_dest[0] = target[0];
	eths->h_dest[1] = target[1];
	eths->h_dest[2] = target[2];
	eths->h_dest[3] = target[3];
	eths->h_dest[4] = target[4];
	eths->h_dest[5] = target[5];

	eths->h_proto = 8;


	iphs->ihl = sizeof(struct iphdr)/4;;//sizeof(struct iphdr )/4;
	iphs->version = 4;
	iphs->tos = 0;
	iphs->tot_len = htons(sizeof(struct iphdr )+sizeof(struct tcphdr ));
	iphs->id=htons (54321);
	iphs->frag_off= 0;//htons(16384);
	iphs->ttl = 255;
	iphs->protocol = IPPROTO_TCP;
	iphs->check = 0;		//set to 0 before calculating checksum
	iphs->saddr = session->sourceIP;//htonl(source_address);
	iphs->daddr = session->destIP;//htonl(destination_address);

	compute_ip_checksum(iphs);

	tcphs->source = htons(session->sourcePort);
	tcphs->dest = htons(session->destPort);
	tcphs->seq = htonl(1000000 + rand () % 90000);
	tcphs->ack_seq = 0;
	tcphs->res1 = 0;
	tcphs->doff = sizeof(struct tcphdr)/4;
	tcphs->fin = 0;
    tcphs->syn = 1;
    tcphs->rst = 0;
    tcphs->psh = 0;
    tcphs->ack = 0;
    tcphs->urg =0;
    tcphs->window = htons(250); // maximum allowed window size;
    tcphs->check  =0 ;
    tcphs->urg_ptr =0 ;
    tcphs->res2 = 0;

    tcphs->check = compute_tcp_checksum(iphs,(unsigned short*)tcphs);

    //update axk_seq and seq number
    session->ackNum=ntohl(tcphs->ack_seq);
    session->seqNum=ntohl(tcphs->seq);

	return datagram;
}
char * send_ack(char * pkbuff,u_int32_t seqNum,u_int32_t ackNum){
	char* datagram=(char*)malloc(1518);
	struct ethhdr *ethr = (struct ethhdr *)pkbuff;
	struct ethhdr *eths = (struct ethhdr *)datagram;

	struct iphdr *iphr = (struct iphdr *)(pkbuff+sizeof(struct ethhdr ));  //ip header of incoming packet
	struct iphdr *iphs = (struct iphdr *) (datagram + sizeof(struct ethhdr ));    // ip header of the spoofed packet

	struct tcphdr *tcphr = (struct tcphdr *)(pkbuff+sizeof(struct ethhdr )+sizeof(struct iphdr )); // TCP header of incomming packet
	struct tcphdr *tcphs =  (struct tcphdr *)(datagram + sizeof(struct iphdr)+sizeof(struct ethhdr )); // ip header of spoofed packet



	memset(datagram,0,1518); /*zero out the buffer */

	// Fill in the IP header
	iphs->ihl =sizeof(struct iphdr)/4;//5;
	iphs->version = 4;
	iphs->tos = 0;
	iphs->tot_len = htons(sizeof(struct iphdr )+sizeof(struct tcphdr));
	iphs->id = iphr->id;
	iphs->frag_off= 0x00;//htons(16384);
	iphs->ttl = 64;
	iphs->protocol = IPPROTO_TCP;
	iphs->check = 0;		//set to 0 before calculating checksum
	iphs->saddr = iphr->daddr;
	iphs->daddr = iphr->saddr;

    compute_ip_checksum(iphs);

	//TCP header
	tcphs->source = tcphr->dest;
	tcphs->dest = tcphr->source;
	tcphs->seq = htonl(seqNum);//htonl(ntohl(tcphr->ack_seq)+1);
	tcphs->ack_seq = htonl(ackNum);//htonl(ntohl(tcphr->seq) +1);
	tcphs->doff=sizeof(struct tcphdr)/4;
	tcphs->res1 = 0;
	tcphs->res2 = 0;
    tcphs->fin = 0;
    tcphs->syn = 0;
    tcphs->rst = 0;
    tcphs->psh = 0;
    tcphs->ack = 1;
    tcphs->urg =0;
    tcphs->window = htons(2000); // maximum allowed window size;
    tcphs->check  =0 ;
    tcphs->urg_ptr =0 ;





    tcphs->check = compute_tcp_checksum(iphs,(unsigned short*)tcphs);


    eths->h_dest[0] = ethr->h_source[0];
    eths->h_dest[1] = ethr->h_source[1];
    eths->h_dest[2] = ethr->h_source[2];
    eths->h_dest[3] = ethr->h_source[3];
    eths->h_dest[4] = ethr->h_source[4];
    eths->h_dest[5] = ethr->h_source[5];

    eths->h_source[0] = ethr->h_dest[0];
    eths->h_source[1] = ethr->h_dest[1];
    eths->h_source[2] = ethr->h_dest[2];
    eths->h_source[3] = ethr->h_dest[3];
    eths->h_source[4] = ethr->h_dest[4];
    eths->h_source[5] = ethr->h_dest[5];

    eths->h_proto = 8;

    return datagram;


}

char * send_fin_ack(char * pkbuff, u_int32_t seq, u_int32_t ack_seq){
	char* datagram=(char*)malloc(1518);
	struct ethhdr *ethr = (struct ethhdr *)pkbuff;
	struct ethhdr *eths = (struct ethhdr *)datagram;

	struct iphdr *iphr = (struct iphdr *)(pkbuff+sizeof(struct ethhdr ));  //ip header of incoming packet
	struct iphdr *iphs = (struct iphdr *) (datagram + sizeof(struct ethhdr ));    // ip header of the spoofed packet

	struct tcphdr *tcphr = (struct tcphdr *)(pkbuff+sizeof(struct ethhdr )+sizeof(struct iphdr )); // TCP header of incomming packet
	struct tcphdr *tcphs =  (struct tcphdr *)(datagram + sizeof(struct iphdr)+sizeof(struct ethhdr )); // ip header of spoofed packet



	memset(datagram,0,1518); /*zero out the buffer */

	// Fill in the IP header
	iphs->ihl =sizeof(struct iphdr)/4;//5;
	iphs->version = 4;
	iphs->tos = 0;
	iphs->tot_len = htons(sizeof(struct iphdr )+sizeof(struct tcphdr));
	iphs->id = iphr->id;
	iphs->frag_off= 0x00;//htons(16384);
	iphs->ttl = 64;
	iphs->protocol = IPPROTO_TCP;
	iphs->check = 0;		//set to 0 before calculating checksum
	iphs->saddr = iphr->daddr;
	iphs->daddr = iphr->saddr;

    compute_ip_checksum(iphs);

	//TCP header
	tcphs->source = tcphr->dest;
	tcphs->dest = tcphr->source;
	tcphs->seq =   htonl(seq); // htonl(ntohl(tcphr->ack_seq)+1);
	tcphs->ack_seq = htonl(ack_seq); // htonl(ntohl(tcphr->seq) +1);
	tcphs->doff=sizeof(struct tcphdr)/4;
	tcphs->res1 = 0;
	tcphs->res2 = 0;
    tcphs->fin = 1;
    tcphs->syn = 0;
    tcphs->rst = 0;
    tcphs->psh = 0;
    tcphs->ack = 1;
    tcphs->urg =0;
    tcphs->window = htons(2000); // maximum allowed window size;
    tcphs->check  =0 ;
    tcphs->urg_ptr =0 ;





    tcphs->check = compute_tcp_checksum(iphs,(unsigned short*)tcphs);


    eths->h_dest[0] = ethr->h_source[0];
    eths->h_dest[1] = ethr->h_source[1];
    eths->h_dest[2] = ethr->h_source[2];
    eths->h_dest[3] = ethr->h_source[3];
    eths->h_dest[4] = ethr->h_source[4];
    eths->h_dest[5] = ethr->h_source[5];

    eths->h_source[0] = ethr->h_dest[0];
    eths->h_source[1] = ethr->h_dest[1];
    eths->h_source[2] = ethr->h_dest[2];
    eths->h_source[3] = ethr->h_dest[3];
    eths->h_source[4] = ethr->h_dest[4];
    eths->h_source[5] = ethr->h_dest[5];

    eths->h_proto = 8;

    return datagram;


}


char * fin(char* source_mac,char* target,SESSION* session){
	char* datagram=(char*)malloc(1518);
	struct ethhdr *eths = (struct ethhdr *)datagram;
	struct iphdr *iphs = (struct iphdr *) (datagram + sizeof(struct ethhdr ));    // ip header of the spoofed packet
	struct tcphdr *tcphs =  (struct tcphdr *)(datagram + sizeof(struct iphdr)+sizeof(struct ethhdr )); // ip header of spoofed packet



	memset(datagram,0,1518); /*zero out the buffer */

	// Fill in the IP header
	iphs->ihl =sizeof(struct iphdr)/4;//5;
	iphs->version = 4;
	iphs->tos = 0;
	iphs->tot_len = htons(sizeof(struct iphdr )+sizeof(struct tcphdr));
	iphs->id = htons (10000 + rand () % 9000);
	iphs->frag_off= 0x00;//htons(16384);
	iphs->ttl = 64;
	iphs->protocol = IPPROTO_TCP;
	iphs->check = 0;		//set to 0 before calculating checksum
	iphs->saddr = session->sourceIP;
	iphs->daddr = session->destIP;

    compute_ip_checksum(iphs);

	//TCP header
	tcphs->source = htons(session->sourcePort);
	tcphs->dest = htons(session->destPort);
	tcphs->seq = htonl(session->seqNum);
	tcphs->ack_seq = htonl(session->ackNum);
	tcphs->doff=sizeof(struct tcphdr)/4;
	tcphs->res1 = 0;
	tcphs->res2 = 0;
    tcphs->fin = 1;
    tcphs->syn = 0;
    tcphs->rst = 0;
    tcphs->psh = 0;
    tcphs->ack = 0;
    tcphs->urg =0;
    tcphs->window = htons(250); // maximum allowed window size;
    tcphs->check  =0 ;
    tcphs->urg_ptr =0 ;





    tcphs->check = compute_tcp_checksum(iphs,(unsigned short*)tcphs);


    eths->h_source[0] = source_mac[0];
	eths->h_source[1] = source_mac[1];
	eths->h_source[2] = source_mac[2];
	eths->h_source[3] = source_mac[3];
	eths->h_source[4] = source_mac[4];
	eths->h_source[5] = source_mac[5];

	eths->h_dest[0] = target[0];
	eths->h_dest[1] = target[1];
	eths->h_dest[2] = target[2];
	eths->h_dest[3] = target[3];
	eths->h_dest[4] = target[4];
	eths->h_dest[5] = target[5];

    eths->h_proto = 8;

    return datagram;


}








////////// checksum check :P :P :P LOL /////////////////////////

unsigned short check_csum(char *pkbuff){
	struct ethhdr *ethr = (struct ethhdr *)pkbuff;
	struct iphdr *iphr = (struct iphdr *)(pkbuff+sizeof(struct ethhdr ));
	struct tcphdr *tcphr = (struct tcphdr *)(pkbuff+sizeof(struct ethhdr )+sizeof(struct iphdr ));

	u_int16_t temp = ntohs(tcphr->check);
	unsigned short calculated = ntohs(compute_tcp_checksum(iphr,(unsigned short*)tcphr));
	//printf("temp:%u\tcalculated:%d",temp,calculated);
	//return 0;
	if(calculated == temp){
		return 1;
	}
	else{
		return 0;
	}

}

//*************************8ALTERNATE IMPLEMENTATION ************************************************




/* set tcp checksum: given IP header and tcp segment */

unsigned short compute_tcp_checksum(struct iphdr *pIph, unsigned short *ipPayload) {

    register unsigned long sum = 0;

    unsigned short tcpLen = ntohs(pIph->tot_len) - (pIph->ihl<<2);

    struct tcphdr *tcphdrp = (struct tcphdr*)(ipPayload);

    //add the pseudo header

    //the source ip

    sum += (pIph->saddr>>16)&0xFFFF;

    sum += (pIph->saddr)&0xFFFF;

    //the dest ip

    sum += (pIph->daddr>>16)&0xFFFF;

    sum += (pIph->daddr)&0xFFFF;

    //protocol and reserved: 6

    sum += htons(IPPROTO_TCP);

    //the length

    sum += htons(tcpLen);



    //add the IP payload

    //initialize checksum to 0

    tcphdrp->check = 0;

    while (tcpLen > 1) {

        sum += * ipPayload++;

        tcpLen -= 2;

    }

    //if any bytes left, pad the bytes and add

    if(tcpLen > 0) {

        //printf("+++++++++++padding, %dn", tcpLen);

        sum += ((*ipPayload)&htons(0xFF00));

    }

      //Fold 32-bit sum to 16 bits: add carrier to result

      while (sum>>16) {

          sum = (sum & 0xffff) + (sum >> 16);

      }

      sum = ~sum;

    //set computation result

    //tcphdrp->check = (unsigned short)sum;
      return (unsigned short)sum;

}




//##############################IP CHECKSUM CALCULATION #####################################

unsigned short verify_ip_checksum(struct iphdr* iphdrp){
	u_int16_t temp_check = ntohs(iphdrp->check);
	iphdrp->check = 0;
	if(ntohs(compute_checksum((unsigned short*)iphdrp,iphdrp->ihl<<2))==temp_check){
		return 1;
	}

	return 0;

}






/* set ip checksum of a given ip header*/

void compute_ip_checksum(struct iphdr* iphdrp){

  iphdrp->check = 0;

  iphdrp->check = compute_checksum((unsigned short*)iphdrp, iphdrp->ihl<<2);

}

/* Compute checksum for count bytes starting at addr, using one's complement of one's complement sum*/

static unsigned short compute_checksum(unsigned short *addr, unsigned int count) {

  register unsigned long sum = 0;

  while (count > 1) {

    sum += * addr++;

    count -= 2;

  }

  //if any bytes left, pad the bytes and add

  if(count > 0) {

    sum += ((*addr)&htons(0xFF00));

  }

  //Fold sum to 16 bits: add carrier to result

  while (sum>>16) {

      sum = (sum & 0xffff) + (sum >> 16);

  }

  //one's complement

  sum = ~sum;

  return ((unsigned short)sum);

}

void stringCopy(unsigned char* dest, unsigned char* src, int size)
{
	int i=0;
	for(;i<size;++i)
	{
		*(dest+i)=src[i];
	}

}

void strconcat(char* startPtr,char* sub,int size){
	int i=0;
	for(;i<size;++i){
		*(startPtr+i)=*(sub+i);
	}
}
