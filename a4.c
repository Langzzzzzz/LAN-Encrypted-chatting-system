#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MY_PORT 60002
#define MAX_LENGTH 80

unsigned char key = 101;
unsigned char counter = 87;

void CBC_CTR(char *msg, int len);
int init_server();
int accept_client(int myListenSocket);
int connect_server(char *ip);
int send_msg(int clientSocket, char *msg);
int receive_msg(int clientSocket, char *msg);
unsigned char encrypt(unsigned char c, unsigned char k);

int main(int argc, char **argv) {
	int myListenSocket, clientSocket;
	char msg[MAX_LENGTH];
	int len;
	if (argc == 2) {
		clientSocket = connect_server(argv[1]);
		len=send_msg(clientSocket, msg);
		while (len>0) {
			len=receive_msg(clientSocket,msg);
			if(len>0){
				len=send_msg(clientSocket, msg);
			}
		}
		close(clientSocket);
	} else {
		myListenSocket = init_server();
		clientSocket = accept_client(myListenSocket);
		len=receive_msg(clientSocket,msg);
		while (len>0) {
			len=send_msg(clientSocket, msg);
			if(len>0){
				len=receive_msg(clientSocket,msg);
			}
		}
		close(myListenSocket);
		close(clientSocket);
	}
	return 0;
}

/*
 Function: CBC_CTR
 Purpose: encrypt or decrypt message
 In: message and len
 Out: void
 */
void CBC_CTR(char *msg, int len) {
	// loop over every character of the plain text message for every character
	for (int i = 0; i < len; i++) {
		// the global counter is encrypted with the global key
		unsigned char enc_counter = encrypt(counter, key);
		// the encrypted counter is xor’d with the plain text character to produce the corresponding ciphertext character
		msg[i] = msg[i] ^ enc_counter;
		// the global counter is incremented by 1
		counter++;
	}
}

/*
 Function: init_server
 Purpose: creat a server
 In: 
 Out: Reture an integer, which is listen socket 
 */
int init_server() {
	int myListenSocket, i;
	struct sockaddr_in myAddr;
	myListenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (myListenSocket < 0) {
		printf("eek! couldn't open socket\n");
		exit(-1);
	}
	/* setup my server address */
	memset(&myAddr, 0, sizeof(myAddr));
	myAddr.sin_family = AF_INET;
	myAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myAddr.sin_port = htons((unsigned short) MY_PORT);

	/* bind my listen socket */

	i = bind(myListenSocket, (struct sockaddr *) &myAddr, sizeof(myAddr));
	if (i < 0) {
		printf("eek! couldn't bind socket\n");
		exit(-1);
	}

	/* listen */
	i = listen(myListenSocket, 5);
	if (i < 0) {
		printf("eek! couldn't listen\n");
		exit(-1);
	}
	return myListenSocket;
}

/*
 Function: accept_client
 Purpose: server accept client.
 In: listen socket
 Out: Reture an integer, which is client socket 
 */
int accept_client(int myListenSocket) {
	int clientSocket;
	socklen_t addrSize;
	struct sockaddr_in clientAddr;
	addrSize = sizeof(clientAddr);

	printf("\nWaiting for connection request...\n");
	clientSocket = accept(myListenSocket, (struct sockaddr *) &clientAddr,
			&addrSize);
	if (clientSocket < 0) {
		printf("eek! couldn't accept the connection\n");
		exit(-1);
	}
	printf("... connection accepted\n");
	return clientSocket;
}

/*
 Function: connect_server
 Purpose: make a connection with server.
 In: ip address
 Out: Reture an integer, which is my socket 
 */
int connect_server(char *ip) {
	int mySocket, i;
	struct sockaddr_in addr;
	/* create socket */
	mySocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (mySocket < 0) {
		printf("eek! couldn't open socket\n");
		exit(-1);
	}
	/* setup address */
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip);
	addr.sin_port = htons((unsigned short) MY_PORT);

	/* connect to server */
	printf("\nConnecting to server...\n");
	i = connect(mySocket, (struct sockaddr *) &addr, sizeof(addr));
	if (i < 0) {
		printf("client could not connect!\n");
		exit(-1);
	}
	printf("... connected\n");
	return mySocket;
}

/*
 Function: send_msg
 Purpose: send encrypt message
 In: clientSocket and message
 Out: Reture an integer, which is rlen 
 */
int send_msg(int clientSocket, char *msg){
	int len,rlen;
	printf("\nYour msg-> ");
	fgets(msg, MAX_LENGTH, stdin);
	len=strlen(msg);
	msg[len-1]=0;
	if(strcmp(msg,"quit")==0){
		rlen=0;
	}else{
		rlen=len-1;
	}
	CBC_CTR(msg, len-1);
	send(clientSocket, msg, len-1, 0);
	return rlen;
}

/*
 Function: receive_msg
 Purpose: receive message 
 In: client socket and message
 Out: Reture an integer, which is len 
 */
int receive_msg(int clientSocket, char *msg){
	printf("\n... waiting to receive ...\n");
	int len = recv(clientSocket, msg, MAX_LENGTH, 0);
	if(len>0){
		printf("Received-> ");
		for(int i=0;i<len;i++){
			putchar(msg[i]);
		}
		putchar('\n');
		CBC_CTR(msg, len);
		msg[len]=0;
		printf("Received-> %s\n", msg);
		if(strcmp(msg,"quit")==0){
			return 0;
		}
	}
	return len;
}
