all: DNS_client

clean:
	rm -f ./DNS_client


nsclient: nsclient.c
	gcc -o DNS_client DNS_client.c
