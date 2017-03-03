#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <netinet/in.h>
#include <string.h>

#include <pthread.h>
#include <sys/ioctl.h>

#include "crc16.h"

#define MAX_CONN 5
#define PORT 7891


int create_server_socket () {
    int serverSocket;
    struct sockaddr_in serverAddr;
 
    serverSocket = socket(PF_INET, SOCK_STREAM, 0);
  
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  


    bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

    if (listen (serverSocket, MAX_CONN) == 0)
    	return serverSocket;
    else
     	return EXIT_FAILURE;
}
	
void* worker (void* argument)
{
	int clientSocket = *((int*) argument);
 	
	int isSession = 0; /*0 - no logical session is started, 1 - session is started */
 
 	while (1) {
		int count;
		ioctl (clientSocket, FIONREAD, &count);
		
		if (count > 0) {
			count++; /* for trailing 0 */
			char *buf = calloc (count, sizeof (char));
			read (clientSocket, buf, count - 1);
			printf ("Data received: %s \n",buf); 	
			
			/*TODO think about another way to define command */
			/*TODO State machine is needed */
			if ((isSession == 0) && (strcmp (buf, "CONNECT") == 0)) {
				send (clientSocket, "CONNECT ACK\n", 13, 0);
				printf ("Connected\n");
				isSession = 1;
				/*TODO it might be some umique actions for logical session */
			}
			
			if (isSession == 1) {
				if (strcmp (buf, "GET DATA") == 0) {
					
					const char *dataArray = "123456789";
					uint16_t dataSize = strlen (dataArray) + 1; 
					uint16_t packageSize = dataSize + 4;
					uint16_t dataCrc16 = gen_crc16 (dataArray, dataSize);
					
					char *package [packageSize];
					char *cp = package;
					memcpy (cp, &packageSize, 2); cp +=2;
					memcpy (cp, dataArray, dataSize);
					cp += dataSize;
					memcpy (cp, &dataCrc16, 2);				

					send (clientSocket, package, packageSize, 0);

					
					
					/*TODO Generator of data */
				}
			
			
				if (strcmp (buf, "DATA RECEIVED") == 0) {
					/*TODO may be ack */
				}

				if (strcmp (buf, "CLOSE") == 0) {
					send (clientSocket, "CLOSE ACK\n", 11, 0);
					isSession = 0;
				
				}
				
				
			}
			
			free (buf);
		}
		else {
			/* Check the socket is readable */
		    fd_set rfd;
		    FD_ZERO (&rfd);
		    FD_SET (clientSocket, &rfd);
		    struct timeval tv = { 1 };
		    select (clientSocket + 1, &rfd, 0, 0, &tv);
		    if (!FD_ISSET (clientSocket, &rfd)) {
		        printf("Client disconnected.\n");
				close (clientSocket);
		      	break;
			}			
		} 
	}
  	return NULL;
}






int main() {
	int serverSocket = create_server_socket();
    if (serverSocket != EXIT_FAILURE) 
      printf("Listening...\n");
    else {
      printf("Error binding socket.\n");
	  return EXIT_FAILURE;
  	}

	int connection = 0;
	pthread_t threads[MAX_CONN];
	
	while (1) {
		int newSocket = accept(serverSocket, (struct sockaddr *) NULL, NULL);
		
	    pthread_create( &threads[connection], NULL, worker, &newSocket);
		/* TODO check result code for pthread_create */

		connection++;
		if (connection >= MAX_CONN) {
			connection = 0; 
		}

 	}
	return EXIT_SUCCESS;
}