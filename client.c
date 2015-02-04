/*
 * client.c -- a stream socket client demo
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <arpa/inet.h>


//#define MYNAME "127.0.0.1"
#define MYPORT "3948"
#define PORT "3925" // the port client will be connecting to
#define MAX_BUFSIZE 500 // max number of bytes we can get at once
#define MAXDATASIZE 500
// get sockaddr, IPv4 or IPv6:


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
	serveraddr.sin_port = htons(atoi(MYPORT));
	
	/* Allow that the socket to reuse a port that the server
	* has also used when it was started before. Otherwise
	* TCP waits for a few minutes before allowing reuse. */
	i = 1;
	setsockopt( request_sock, SOL_SOCKET,
	SO_REUSEADDR, &i, sizeof(i));
	
	/* Bind the address to the socket. */
	if (bind(request_sock, (struct sockaddr *)&serveraddr,sizeof serveraddr) < 0) {
		perror("bind failed. Error");
		printf("Binding address to socket failed\n");
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
	/* Check whether the connection is still open */
	if (bytes==0){
		printf("server: end of file on %d\n",so);
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



void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int TCPClientSocket(char servername[], char port[]){
	//struct hostent *hostp;
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_in serveraddr;
	int sockfd, rv;
	char s[INET6_ADDRSTRLEN];
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(servername, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
		s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	return sockfd;
}




int getChatPartner(char mainserver[]) {
	int sockfd, listen_socket, partneraddresslength;
	struct sockaddr partneraddress;
	
	char name_length[3];
	char client_or_server[2];
	char chatpartnername[80];
	char chatpartnerport[5];
	char buf[80];


	memset( name_length, 0, 3 );
	memset( client_or_server, 0, 2 );
	memset( chatpartnername, 0, 80 );
	memset( chatpartnerport, 0, 5 );






	char hostn[400];
	char *ip; //placeholder for the hostname
	struct hostent *hostIP; //placeholder for the IP address

	//if the gethostname returns a name then the program will get the ip
	//address using gethostbyname
	if((gethostname(hostn, sizeof(hostn))) == 0)
	{
	hostIP = gethostbyname(hostn); //the netdb.h function gethostbyname
	
	ip = inet_ntoa(*(struct in_addr *)hostIP->h_addr);
	}
	else
	{
	printf("ERROR:FC4539 - IP Address not found."); //error if the
	//hostname is not found
	}

	





	sockfd = TCPClientSocket(mainserver, PORT);

	/* Tell the server how you can be contacted by a chat partner */
	sprintf(name_length, "%zu", strlen(ip));
	safewrite(sockfd, name_length, 2);
	safewrite(sockfd, ip, strlen(ip));
	safewrite(sockfd, MYPORT, 4);


	/* Receive from the server whether you will act as a client or as a server in the chat */
	saferead(sockfd, client_or_server, 1);
	
	
	//close(sockfd);
	if (client_or_server[0] == 'C') {
		/* You will be the client in the chat */

		/* Close the socket that you would have needed as a chat server */
		//close(listen_socket);

		/* Read the length of the chat partner's hostname */
		saferead(sockfd, name_length, 2);
		/* Read the chat partner's hostname */
		saferead(sockfd, chatpartnername, atoi(name_length));
		/* Read the chat partner's port number */
		saferead(sockfd, chatpartnerport, 4);

		/* Close the connection to the server */
		close(sockfd);

		/* Connect to the chat partner */
		sockfd = TCPClientSocket(chatpartnername, chatpartnerport);
	}

	else {
		/* Act as the server in the chat, no more information is coming from the server */
		close(sockfd);

		/* Create a socket that is read to accept connections from a chat partner */
		listen_socket = TCPListenSocket();


		/* Accept a connection from a chat partner */
		partneraddresslength = sizeof(partneraddress);
		sockfd = accept(listen_socket,(struct sockaddr *)&partneraddress, &partneraddresslength);
		if (sockfd < 0)
			perror("Error accepting connection from chat partner" );

	}
	printf("Connection create, chat along!!\n");
	return sockfd;
}

int sendFile(int socket, char text[]) {
	FILE *fp;
	int size =0, k, status, nsize = 0, temp_size, i, sent, numbytes, lala;
	char buf[20], file_buf[505], temp_buf[505];
	char ch;
	struct stat info;

	nsize = strlen(text);
	char name[nsize];
	lala = strlen(name);
	strncpy(name, text, nsize-1);
	name[nsize-1] = '\0';	
	lala = strlen(name);
	
	if((fp = fopen(name, "r")) == NULL) {
		perror("Can't open file");
		text[0] = '/';
		write(socket,text, 80);
		return 1;
	}
	write(socket,text, 80);    
	printf("Sending File: %s\n",text);

	status = stat( name,&info );

	if( status == -1 ){
		printf(" fstat failed \n");
		return 1;
	}
	size = info.st_size;
	temp_size = size;

	for( i=0; i<=9; i++ )	{
		buf[i] =  size%10;
		size = size/10;
	}

	if (send(socket, buf, 10, 0) == -1){
		perror("Cant send file size...\n\n");
		return 1;
	}


	i = 0;
	sent = 0;
	while( sent <= temp_size ){
		ch = fgetc(fp);
		if( sent == temp_size ){
			if (i>0){
				if (( numbytes = send(socket, file_buf, i, 0)) == -1)
		        	perror("Error in sending file.....\n\n\n");
			}
				break;
		}

		file_buf[i++] = ch;
		
		if(i == MAX_BUFSIZE){					
			if (( numbytes = send(socket, file_buf, MAX_BUFSIZE, 0)) == -1)
	        		perror("Error in sending file....\n\n\n");
				i = 0;
		}	
			sent++;
	}

	fclose(fp);
}

int receiveFile(int socket, char text[]) {
	FILE *fp;
	int nsize,k, i, size=0, temp_size, mul = 1, numbytes, lala, recvd; 
	int count_freq, repeat;
	char ch,buf[20],file_buf[505], temp_buf[505];
	saferead(socket, text,80);
	if(text[0] == '/')  
		return 1;
	

	nsize = strlen(text);
	char name[nsize];
	strncpy(name, text, nsize-1);
	name[nsize-1] = '\0';	
	
	if ((fp = fopen(name,"w")) == NULL) {
        	perror(" Can't open file");
		return 1;
	}

	if ((numbytes=recv(socket, buf, 10, 0)) == -1) {
		perror("Cant receive size of the file....\n\n");
		return 1;
	}

	for( i=0; i<=9; i++ ){
		size =  size + ( buf[i]*mul );
		mul = mul * 10;
	}


	recvd = 0;
	printf("Received File: %s\n", name);
	repeat = size/MAXDATASIZE;
		
	if ( repeat>50 )
		count_freq = repeat/50;

	i=0;	
	while( recvd < size ){
			
		if ((numbytes=recv(socket, temp_buf, MAXDATASIZE, 0)) == -1) {
			perror("Error receiving file from server....\n\n");
			return 1;
		}
			
		fwrite( temp_buf, sizeof(temp_buf[0]), numbytes, fp );
		recvd = recvd + numbytes;
	}
	fclose(fp);  

}


int chat(int socket) {
	int status;
	char text[10];


	if(fork() == 0) {
		
		if(text[0] == '#') {
			printf("Chat-Partner left.");
			exit(1);
		}

		while (text[0] != '#'){
			saferead(socket, text,80);
			if(text[0] == '$') {
				receiveFile(socket, text);
			}
			else
				printf("Chat-Partner: %s", text);
			if(text[0] == '#') {
				printf("Chat-Partner left.");
				exit (1);
		}



		}
			
	}

	else  {
	
		if(text[0] == '#') {
			write(socket,text,80);
			exit(1);
		}
		while (text[0] != '#'){
			fgets(text,80,stdin);
			write(socket,text, 80);
			
			if(text[0] == '#') {
				write(socket,text,80);
				exit(1);
			}			
			if(text[0] == '$') {
				printf("Enter the name of the file: ");
				fgets(text,80,stdin);
				sendFile(socket,text);
			}
		}
				
	} 



}



int main(int argc, char *argv[]) {
	int sock;

	sock = getChatPartner(argv[1]);

	chat(sock);

	close(sock);

}



