#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#include "crc16.h"

#define PORT 7891

int create_socket_and_connect () {
    int clientSocket;

    clientSocket = socket (PF_INET, SOCK_STREAM, 0);
  
	if (clientSocket > 0) {
	    struct sockaddr_in serverAddr;
	    socklen_t addr_size;
		
	    serverAddr.sin_family = AF_INET;
	    serverAddr.sin_port = htons (PORT);
	    serverAddr.sin_addr.s_addr = inet_addr ("127.0.0.1");
	    memset (serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

	    addr_size = sizeof serverAddr;
		int result = connect (clientSocket, (struct sockaddr *) &serverAddr, addr_size);
		if (result < 0) {
			perror ("connect");
			clientSocket = -1;
		}
	}
	return clientSocket;
}

void get_partial_data (int clientSocket, char *buf, int size, int maxSize) {
	int result = 0;
	if (size > 0) {
		char *end = buf;
		int red = 0;
		while (red < size) {
			result = read (clientSocket, end, size - red);
			if (result > 0) {
				red += result;
				end += result;
			} 
			else {
				perror ("read data");
				exit (EXIT_FAILURE);
			}
		}
	}
	else {
		result = read (clientSocket, buf, maxSize);
		if (result <= 0)  {
			/*TODO Error */
			printf ("Error while data is reading \n");
			exit (EXIT_FAILURE);
		}	
	}
}

void convert_char_to_short (unsigned char* pchar, uint16_t* pshort) { 
	/* For little-endian only */
	*pshort = (uint16_t)(((unsigned int)pchar[0]) |
	                    (((unsigned int)pchar[1])<<8));
}

int get_signed_package (int clientSocket) {
	char buffer[2];
	get_partial_data (clientSocket, buffer, 2, 2);
	uint16_t size = 0;
	convert_char_to_short ( (unsigned char*) buffer, &size);
	
	printf ("size = %d\n", size);
	
	int result = -1;
	
	if (size > 0) {
		char *pack = calloc (size - 3, sizeof (char));
		get_partial_data (clientSocket, pack, size - 4, size - 4);
		printf ("package: %s\n", pack);
		
		char crc[2];
		get_partial_data (clientSocket, crc, 2, 2);
		uint16_t crc16 = 0;
		convert_char_to_short ( (unsigned char*) crc, &crc16);
		printf ("crc: %d \n", crc16);
		
		if (gen_crc16 ((uint8_t*) pack, size - 4) == crc16) {
			result = 0;
			printf ("CRC is correct\n");
		}	
		free (pack);
	}	
	return result;
}

void get_data_from_server (int clientSocket) {
	char buffer[1024];

	send (clientSocket, "CONNECT", 8, 0);
	get_partial_data (clientSocket, buffer, 0, 1024);
    printf ("Data received: %s \n",buffer);
	
	if (strcmp (buffer, "CONNECT ACK\n") == 0) {
		memset (buffer, '\0', 1024);

		do {
			send (clientSocket, "GET DATA", 9, 0);
		} while (get_signed_package (clientSocket) < 0);
		
		send (clientSocket, "CLOSE", 6, 0);	

		get_partial_data (clientSocket, buffer, 0, 1024);	
	    printf ("Data received: %s \n",buffer); 
		memset (buffer, '\0', 1024);
	}
}

int main() {
	int sock = create_socket_and_connect ();
	
	if (sock > 0)
		while (1) 
			get_data_from_server (sock);		
	
	close (sock);
  	return 0;
}