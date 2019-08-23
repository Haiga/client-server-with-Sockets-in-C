#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include <vector>
/* Server port */
#define PORT 4242

/* Buffer length */
#define BUFFER_LENGTH 4096

struct args {
    int clientfd;
    int id;
};
//GLOBAL
//std::vector<char[BUFFER_LENGTH]> buffer;//
char buffer[BUFFER_LENGTH];
void *client_function(void *arg) {
	int clientfd = ((struct args*)arg)->clientfd;
	/* Copies into buffer our welcome messaage */
	
	//buffer.push_back("Hello, client!\n\0");
	strcpy(buffer, "Hello, client!\n\0");
	
	/* Sends the message to the client */
	if (send(clientfd, buffer, strlen(buffer), 0)) {
		fprintf(stdout, "Client connected.\nWaiting for client message ...\n");
		/* Communicates with the client until bye message come */
		do {
			/* Zeroing buffers */
			memset(buffer, 0x0, BUFFER_LENGTH);
			/* Receives client message */
			int message_len;
			if((message_len = recv(clientfd, buffer, BUFFER_LENGTH, 0)) > 0) {
				buffer[message_len-1] = '\0';
				printf("Client says: %s\n", buffer);
			}
			/* 'bye' message finishes the connection */
			if(strcmp(buffer, "bye") == 0) {
				send(clientfd, "bye", 3, 0);
			}else {
				send(clientfd, "yep\n", 4, 0);
			}
		}while(strcmp(buffer, "bye"));}
	pthread_exit(NULL);
}
//using namespace std;
/*
* Main execution of the server program of the simple protocol
*/
int main(void) {

	/* Client and Server socket structures */
	struct sockaddr_in client, server;
	
	/* File descriptors of client and server */
	int serverfd, clientfd;
	
	std::vector<int> clientfd_vector;//
	std::vector<int> client_vector;
	std::vector<pthread_t> client_threads_vector;//
	
	int num_clients_connected = 0;//
	
	//char buffer[BUFFER_LENGTH];

	fprintf(stdout, "Starting server\n");

	/* Creates a IPv4 socket */
	serverfd = socket(AF_INET, SOCK_STREAM, 0);
	if(serverfd ==-1) {
		perror("Can't create the server socket:");
		return EXIT_FAILURE;
	}

	fprintf(stdout, "Server socket created with fd: %d\n", serverfd);

	/* Defines the server socket properties */
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	memset(server.sin_zero, 0x0, 8);

	/* Handle the error of the port already in use */
	int yes = 1;
	if(setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR,&yes, sizeof(int)) ==-1){
		perror("Socket options error:");
		return EXIT_FAILURE;
	}


	/* bind the socket to a port */
	if(bind(serverfd, (struct sockaddr*)&server, sizeof(server)) ==-1 ) {
		perror("Socket bind error:");
		return EXIT_FAILURE;
	}


	/* Starts to wait connections from clients */
	if(listen(serverfd, 1) ==-1) {
		perror("Listen error:");
		return EXIT_FAILURE;
	}

	fprintf(stdout, "Listening on port %d\n", PORT);

	socklen_t client_len = sizeof(client);
	while(true){
		if ((clientfd=accept(serverfd,(struct sockaddr *) &client, &client_len )) ==-1) {
			perror("Accept error:");
			return EXIT_FAILURE;
		}
		//Push in array clientfd
		clientfd_vector.push_back(clientfd);
		
		//Create new Thread
		pthread_t new_thread;
		client_threads_vector.push_back(new_thread);
		num_clients_connected = num_clients_connected + 1;
		struct args *client_info = (struct args *)malloc(sizeof(struct args));
		client_info->clientfd = clientfd;
		client_info->id = num_clients_connected-1;
		pthread_create(&client_threads_vector[num_clients_connected - 1], NULL, client_function, client_info);
		

		/* Client connection Close */
		close(clientfd);
	}
	

	/* Close the local socket */
	close(serverfd);

	printf("Connection closed\n\n");

	return EXIT_SUCCESS;
}
