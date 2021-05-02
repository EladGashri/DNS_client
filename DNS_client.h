#ifndef DNSCLIENT_H
#define DNSCLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <stdbool.h>
#include <sys/time.h>

#define MAX_COMMAND_LENGTH 256
#define MAX_MESSAGE_LENGTH 65536
#define ALLOWED_CHAR(c) ((c>=45 && c<=57) || (c>=65 && c<=90)|| (c>=97 && c<=122))

#pragma pack(push, 1)

typedef struct{
    unsigned short
            ID:16,
            QR:1,
            OPCODE:4,
            AA:1,
            TC:1,
            RD:1,
            RA:1,
            Z:3,
            RCODE:4,
            QDCOUNT:16,
            ANCOUND:16,
            NSCOUNT:16,
            ARCOUNT:16;
    unsigned char
            QNAME[0];
    unsigned short
            QTYPE:16,
            QCLASS:16;
} DNS_header_question;

typedef struct {
    unsigned short
            NAME:16,
            TYPE:16,
            CLASS:16;
    unsigned int
            TTL:32;
    unsigned short
            RDLENGTH:16;
    unsigned char
            RDATA[0];
} DNS_answer;

#pragma pack(pop)

bool check_domain_name(unsigned char* domain_name);
struct hostent* dns_query(unsigned char* domain_name);
void initialize_message(DNS_header_question *message);
void set_formatted_domain_name(unsigned char* formatted_domain_name, unsigned char* domain_name);

int socket_fd;
struct sockaddr_in serv_addr;
short id=0;
struct hostent *hostent_response;

#endif
