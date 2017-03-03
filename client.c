#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include "crc16.h"

#define PORT 7891

int create_socket_and_connect () {
    int clientSocket;
    struct sockaddr_in serverAddr;
    socklen_t addr_size;

    clientSocket = socket (PF_INET, SOCK_STREAM, 0);
  
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset (serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

    addr_size = sizeof serverAddr;
    connect (clientSocket, (struct sockaddr *) &serverAddr, addr_size);
	
	return clientSocket;
}

void get_partial_data (int clientSocket, char *buf, int size, int maxSize) {
	int result = 0;
	if (size > 0) {
		char *end = buf;
		int readed = 0;
		while (readed < size) {
			result = read (clientSocket, end, size - readed);
			if (result > 0) {
				readed += result;
				end += result;
			} 
			else {
				/*TODO Error */
				//printf ("Error while data is reading \n");
			}
		}
	}
	else {
		result = read (clientSocket, buf, maxSize);
		if (result <= 0)  {
			/*TODO Error */
			printf ("Error while data is reading \n");
		}
		
	}
	
}

void get_signed_package (int clientSocket) {
	char buffer[2];
	get_partial_data (clientSocket, buffer, 2, 2);
	uint16_t size = (uint16_t) *buffer;
	
	printf ("size = %d\n", size);
	
	
	if (size > 0) {
		char *pack = calloc (size - 3, sizeof (char));
		get_partial_data (clientSocket, pack, size - 4, size - 4);
		printf ("package: %s\n", pack);
		
		char crc[2];
		get_partial_data (clientSocket, crc, 2, 2);
		uint16_t crc16 = (uint16_t) *crc;
		printf ("crc: %d \n", crc16);
		
		//char trailing;
		//get_partial_data (clientSocket, &trailing, 1, 1);
		
		free (pack);
	}
}


void get_data_from_server (int clientSocket) {
	char buffer[1024];

	send (clientSocket, "CONNECT", 8, 0);	
	get_partial_data (clientSocket, buffer, 0, 1024);	
    printf ("Data received: %s \n",buffer); 
	memset (buffer, '\0', 1024);

	send (clientSocket, "GET DATA", 9, 0);
	get_signed_package (clientSocket);

	send (clientSocket, "CLOSE", 6, 0);	

	get_partial_data (clientSocket, buffer, 0, 1024);	
    printf ("Data received: %s \n",buffer); 
	memset (buffer, '\0', 1024);

}

int main() {
	int sock = create_socket_and_connect ();
	get_data_from_server (sock);
		
	
	close (sock);
  	return 0;
}