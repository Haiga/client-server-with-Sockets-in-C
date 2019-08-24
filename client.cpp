#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Defines the server port */
//#define PORT 4242
#define PORT 4242
/* Sockets buffers length */
#define LEN 4096

/* Server address */
#define SERVER_ADDR "127.0.0.1"

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <vector>
#include <semaphore.h>

int sockfd;
sem_t semaforoConsumidor;

void *client_listen(void *arg) {
    int slen;
    /* Receive buffer */
    char buffer_in[LEN];

    while (true) {

		/* Zeroing the buffers */
		memset(buffer_in, 0x0, LEN);

		/* Receives an answer from the server */
		if(slen = recv(sockfd, buffer_in, LEN, 0)>0){
            printf("Alguem disse: %s\n", buffer_in);
        }

		/* 'bye' message finishes the connection */
		if(strcmp(buffer_in, "bye") == 0) break;
	}
    close(sockfd);

    fprintf(stdout, "\nConnection closed\n\n");
    sem_post(&semaforoConsumidor);
    pthread_exit(NULL);
}

void *client_send(void *arg) {
    //int slen;
    /* Receive buffer */
    //char buffer_in[LEN];

    /* Send buffer */
    char buffer_out[LEN];
    while (true) {

		/* Zeroing the buffers */
//		memset(buffer_in, 0x0, LEN);
		memset(buffer_out, 0x0, LEN);

		fprintf(stdout, "Say something to the server: ");
		fgets(buffer_out, LEN, stdin);

		/* Sends the read message to the server through the socket */
		send(sockfd, buffer_out, strlen(buffer_out), 0);

		/* Receives an answer from the server */
		//slen = recv(sockfd, buffer_in, LEN, 0);
		//printf("Server answer: %s\n", buffer_in);

		/* 'bye' message finishes the connection */
		//if(strcmp(buffer_in, "bye") == 0) break;
	}
    pthread_exit(NULL);
}
/*
* Main execution of the client program of our simple protocol
*/
int main(int argc, char **argv) {

	/* Server socket */
	struct sockaddr_in server;
	/* Client file descriptor for the local socket */


	int len = sizeof(server);
	int slen;

	/* Receive buffer */
	char buffer_in[LEN];
//
//	/* Send buffer */
//	char buffer_out[LEN];

	fprintf(stdout, "Starting Client ...\n");

	/*
	* Creates a socket for the client
	*/
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Error on client socket creation:");
		return EXIT_FAILURE;
	}
	fprintf(stdout, "Client socket created with fd: %d\n", sockfd);

	/* Defines the connection properties */
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = inet_addr(SERVER_ADDR);
	memset(server.sin_zero, 0x0, 8);

	/* Tries to connect to the server */
	if (connect(sockfd, (struct sockaddr*) &server, len) == -1) {
		perror("Can't connect to server");
		return EXIT_FAILURE;
	}

	/* Receives the presentation message from the server */
	if ((slen = recv(sockfd, buffer_in, LEN, 0)) > 0) {
		buffer_in[slen + 1] = '\0';
		fprintf(stdout, "Server says: %s\n", buffer_in);
	}

//	/*
//	* Commuicate with the server until the exit message come
//	*/
//	while (true) {
//
//		/* Zeroing the buffers */
//		memset(buffer_in, 0x0, LEN);
//		memset(buffer_out, 0x0, LEN);
//
//		fprintf(stdout, "Say something to the server: ");
//		fgets(buffer_out, LEN, stdin);
//
//		/* Sends the read message to the server through the socket */
//		send(sockfd, buffer_out, strlen(buffer_out), 0);
//
//		/* Receives an answer from the server */
//		slen = recv(sockfd, buffer_in, LEN, 0);
//		printf("Server answer: %s\n", buffer_in);
//
//		/* 'bye' message finishes the connection */
//		if(strcmp(buffer_in, "bye") == 0) break;
//	}
    pthread_t tid_listening, tid_sending;
    sem_init(&semaforoConsumidor, 0, 0);
    pthread_create(&tid_listening, nullptr, client_listen, nullptr);
    pthread_create(&tid_sending, nullptr, client_send, nullptr);
	/* Close the connection whith the server */
    sem_wait(&semaforoConsumidor);
	//close(sockfd);

	//fprintf(stdout, "\nConnection closed\n\n");

	return EXIT_SUCCESS;
}
