The program contacts a DNS server using UDP protocol. The Program receives as a command line argument an IP address of a DNS server in a dotted-decimal notation. 
It prompts the user to enter a domain name. If the domain name exists in the DNS server, it's IP is printed in a dotted-decimal notation.
DNS queries are constructed according to RFC 1035.
The program is written in C.

In order to build and run the program write these 2 following lines in the shell:

make

./DNS_client <DNS server IP address>

There are 2 important data structures in the code:
DNS_header_question - contains the header and the question sections fields of the DNS message.
DNS_answer - contains the answer section fields of the DNS message.

The program receives a domain name from the user and checks if it is a valid domain name by using the function check_domain_name. If it is, the domain name is sent to the function dnsQuery. 
In this function, the domain name is sent to the set_formatted_domain_name function in order to get the correct format for the DNS server. This format is entered to the DNS_header_question data structure. 
The rest of the feilds in DNS_header_question are initialized using the function initialize_message. The DNS_header_question data structure is send to the DNS server using UDP protocol. 
The package sent back from the DNS server is casted as a DNS_answer data structure. the RCODE field is checked and if it is set to a value different from 0, an error in printed. Otherwire, the DNS_answer data structure is used to fill a struct hostent which is returned from the function dnsQuery to the main function.
The IP address which is stored in the h_name field in the struct hostent is printed.
