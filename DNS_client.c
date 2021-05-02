#include "DNS_client.h"


int main(int argc, char *argv[]){

    if (argc==1){
        perror("must insert a DNS server as a command line argument\n");
        exit(1);
    }else if (argc>2){
        perror("must insert a singular DNS server as a command line argument\n");
        exit(1);
    }
    const char* DNS_server=argv[1];

    WSADATA wsaData;
    int iResult= WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult!= NO_ERROR){
        perror("Error at WSAStartup()\n");
        exit(1);
    }

    if ((socket_fd = socket (AF_INET, SOCK_DGRAM, 0))<0){
        perror("Could not create a socket");
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(DNS_server);
    serv_addr.sin_port = htons(53);

    hostent_response=(struct hostent*) malloc(sizeof(struct hostent));
    hostent_response->h_aliases=(char**) {NULL};
    hostent_response->h_addrtype=AF_INET;

    unsigned char command[MAX_COMMAND_LENGTH];
    struct hostent* hostent_from_dns_query;

    char *ip_address;

    while (true) {
        printf("please enter a domain name>");
        scanf("%s", command);
        if (strcmp(command, "quit") == 0) {
            break;
        } else if (check_domain_name(command)) {
            hostent_from_dns_query = dns_query(command);
            if (hostent_from_dns_query!=NULL){
                ip_address=hostent_from_dns_query->h_name;
                printf("%s\n", inet_ntoa(*(struct in_addr*)ip_address));
            }
        }else{
            printf("ERROR: BAD NAME\n");
        }
    }
    free(hostent_response);
    return 0;
}


struct hostent* dns_query(unsigned char* domain_name){

    unsigned char temp_formatted_domain_name[MAX_COMMAND_LENGTH];
    set_formatted_domain_name(temp_formatted_domain_name, domain_name);

    int message_size=sizeof(DNS_header_question)+(sizeof(char)*(strlen(temp_formatted_domain_name)+1));
    DNS_header_question *message_to_send=(DNS_header_question*) malloc(message_size);
    set_formatted_domain_name(message_to_send->QNAME, domain_name);
    initialize_message(message_to_send);

    int nbytes;
    nbytes=sendto(socket_fd,(const char*) message_to_send, message_size,0,(const struct sockaddr*) &serv_addr,sizeof(serv_addr));
    if (nbytes<0){
        perror("Could not send message to DNS server\n");
        free(message_to_send);
        return NULL;
    }

    struct timeval tv;
    tv.tv_sec=2;
    tv.tv_usec=0;
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(socket_fd,&read_fds);
    int select_output=select(1,&read_fds,NULL,NULL,&tv);
    if (select_output<0){
        perror("socket is not read ready\n");
        free(message_to_send);
        return NULL;
    }else if(select_output==0){
        printf("TIMEOUT\n");
        free(message_to_send);
        return NULL;
    }

    int serv_addr_size=sizeof(serv_addr);
    char buffer[MAX_MESSAGE_LENGTH];
    nbytes = recvfrom(socket_fd, buffer, MAX_MESSAGE_LENGTH, 0, (struct sockaddr *) &serv_addr, &serv_addr_size);
    if (nbytes < 0) {
        perror("Could not receive a response from the DNS server\n");
        free(message_to_send);
        return NULL;
    }

    DNS_header_question *response_header=(DNS_header_question*) buffer;

    if ((ntohs(response_header->RCODE)&15)!=0){
        switch((ntohs(response_header->RCODE)&15)){
            case 2:
                printf("ERROR: SERVER FAILURE\n");
                break;
            case 3:
                printf("ERROR: NONEXISTENT\n");
                break;
            case 4:
                printf("ERROR: NOT IMPLEMENTED\n");
                break;
            case 5:
                printf("ERROR: REFUSED\n");
                break;
        }
        free(message_to_send);
        return NULL;
    }

    DNS_answer *response_answer=(DNS_answer*)(buffer + message_size);
    *((int*)hostent_response->h_name)=*((int*)response_answer->RDATA);
    hostent_response->h_length=(short) (ntohs(response_answer->RDLENGTH));
    char* addr_list[]= {(char*) ((int*)response_answer->RDATA), NULL};
    hostent_response->h_addr_list= addr_list;

    free(message_to_send);
    return hostent_response;
}


void initialize_message(DNS_header_question *message){
    message->ID = htons(++id);
    message->QR = htons(0);
    message->OPCODE = htons(0);
    message->AA = htons(0);
    message->TC = htons(0);
    message->RD = htons(0);
    message->RA = htons(0);
    message->Z = htons(0);
    message->RCODE = htons(0);
    message->QDCOUNT = htons(1);
    message->ANCOUND = htons(0);
    message->NSCOUNT = htons(0);
    message->ARCOUNT = htons(0);
    message->QTYPE = htons(1);
    message->QCLASS = htons(1);
}


bool check_domain_name(unsigned char* domain_name){
    int domain_name_length=strlen(domain_name);
    unsigned char current_char;
    int chars_since_dot=0;
    for (int string_index=0;string_index<domain_name_length;string_index++){

        current_char=*(domain_name+string_index);
        if (!ALLOWED_CHAR(current_char)){
            return false;
        }else if(current_char=='.') {
            if (chars_since_dot == 0 || chars_since_dot > 63) {
                return false;
            } else {
                chars_since_dot = 0;
            }
        }else{
            chars_since_dot++;
        }
    }return chars_since_dot==domain_name_length? false: true;
}


void set_formatted_domain_name(unsigned char* formatted_domain_name, unsigned char* domain_name) {
    size_t formatted_domain_name_size = strlen((char*) domain_name) + 2;
    formatted_domain_name[formatted_domain_name_size];

    int chars_since_dot = 0;
    for (int string_index = 0; string_index < strlen((char*) domain_name); string_index++) {
        if (domain_name[string_index] == '.') {
            formatted_domain_name[string_index - chars_since_dot] = chars_since_dot;
            chars_since_dot = 0;
        } else {
            formatted_domain_name[string_index + 1] = domain_name[string_index];
            chars_since_dot++;
        }
    }
    formatted_domain_name[formatted_domain_name_size - 2 - chars_since_dot] = chars_since_dot;
    formatted_domain_name[formatted_domain_name_size - 1] = '\0';
}
