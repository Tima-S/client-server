#include <stdio.h>
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


void get_data_from_server (int clientSocket) {
	char buffer[1024];

	send (clientSocket, "CONNECT", 8, 0);	

    read (clientSocket, buffer, 1024);
    printf ("Data received: %s \n",buffer); 
	memset (buffer, '\0', 1024);

	send (clientSocket, "GET DATA", 9, 0);	

    read (clientSocket, buffer, 1024);
	printf ("Data received: %s \n",buffer); 
	memset (buffer, '\0', 1024);

	send (clientSocket, "CLOSE", 6, 0);	

    read (clientSocket, buffer, 1024);
    printf ("Data received: %s \n",buffer); 
	memset (buffer, '\0', 1024);

}

int main() {
	int sock = create_socket_and_connect ();
	get_data_from_server (sock);
		
	
	close (sock);
  	return 0;
}