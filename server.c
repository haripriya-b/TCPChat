
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#define PORT "3925"

int TCPListenSocket(){
	struct sockaddr_in serveraddr, clientaddr;
	int clientaddrlen;
	int request_sock;
	int i;

	/* Create the request socket. */
	request_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (request_sock < 0){
		printf("Creation of a socket failed.\n");
		exit(1);
	}
	
	/* Fill in the address structure */
	bzero((void *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = INADDR_ANY;
	serveraddr.sin_port = htons(atoi(PORT));
	
	/* Allow that the socket to reuse a port that the server
	* has also used when it was started before. Otherwise
	* TCP waits for a few minutes before allowing reuse. *	/
	i = 1;
	setsockopt( request_sock, SOL_SOCKET,
	SO_REUSEADDR, &i, sizeof(i));
	
	/* Bind the address to the socket. */
	if (bind(request_sock, (struct sockaddr *)&serveraddr,sizeof serveraddr) < 0) {
		perror("bind failed. Error");
		//printf("Binding address to socket failed\n");
		exit(1);
	}

	/* Start listening to the socket */
	if (listen(request_sock, SOMAXCONN) < 0) {
		printf("Can't listen to the socket\n");
		exit(1);
	}
	return request_sock;
}

/* Reads exactly one byte from a socket connection */
char safereadbyte(int so){
	int bytes;
	char buf[1];
	bytes = read(so, buf, 1);
	/* Check whether the read worked */
	if (bytes<0){
		perror("Error in saferead");
		if (close(so)) perror("close");
		exit(1);
	}
	return buf[0];
}

/* Reads exactly l bytes from the socket */
int saferead(int so, char buf[], int l){
	int i;
	for (i=0; i<l; i++){
		buf[i]=safereadbyte(so);
	}
	return l;
}

/* Write to a socket in the same way as write, but returns an
error message if the socket has been closed in the meantime */
int safewrite(int so, char buf[], int l){
	int i;
	if (i=write(so, buf, l)==0){
		printf("Can't write to socket, connection is closed" );
		exit(1);
	}
	return i;
}


int connect_two_clients(int listen_socket) { 
	int client_socket1, client_socket2; 
	struct sockaddr_in clientaddr1, clientaddr2; 
	int clientaddrlen1, clientaddrlen2; 

	char client_name1[80], client_name2[80]; 
	char client_port1[5], client_port2[5]; 
	char name_length1[3], name_length2[3]; 

	memset( client_name1, 0, 80 ); 
	memset( client_name2, 0, 80 ); 
	memset( client_port1, 0, 5 ); 
	memset( client_port2, 0, 5 ); 
	memset( name_length1, 0, 3 ); 
	memset( name_length2, 0, 3 ); 

	printf("server: waiting for connections...\n");

	/* Accept a connection from a first client */
	clientaddrlen1 = sizeof(clientaddr1);
	client_socket1 = accept(listen_socket,
	(struct sockaddr *)&clientaddr1, &clientaddrlen1);
	if (client_socket1 < 0)
		perror("Can't accept connection from a first chat client\n" );


	/* Read machine name and port number that client sends
	* as its contact information */
	saferead(client_socket1, name_length1, 2);
	saferead(client_socket1, client_name1, atoi(name_length1));
	saferead(client_socket1, client_port1, 4);

	/* Tell the first client that it is the server in the chat
	* connection, and to wait for connection by a chat partner */
	safewrite(client_socket1, "S", 1);

	close(client_socket1);

	printf("server: got connection from client 1\n");


	printf("server: waiting for connections...\n");

	/* Accept a connection from second client */
	clientaddrlen2 = sizeof(clientaddr2);
	client_socket2 = accept(listen_socket,
	(struct sockaddr *)&clientaddr2,
	&clientaddrlen2);

	if (client_socket2 < 0) perror("Can't accept connection from a second chat client\n" );

	printf("server: got connection from client 2\n");

	/* Read machine name and port number that client sends
	* as its contact information */
	saferead(client_socket2, name_length2, 2);
	saferead(client_socket2, client_name2, atoi(name_length2));
	saferead(client_socket2, client_port2, 4);

	
	
	/* Tell the second client that it is the client in the chat
	* connection, and to connect to the given name and port */
	safewrite(client_socket2, "C", 1);
	safewrite(client_socket2, name_length1, 2);
	safewrite(client_socket2, client_name1, atoi(name_length1));
	safewrite(client_socket2, client_port1, 4);

	/* Close the sockets for both clients */
	close(client_socket2);
	return 0;
}

int main(int argc, char *argv[]) {
	int listen_socket;
	
	/* Start listening to the port number from the command line */
	
	listen_socket = TCPListenSocket();

	/* Connect pairs of clients forever */

	while (1) connect_two_clients(listen_socket);
	/* We don't ever come here */
		close(listen_socket);
}

