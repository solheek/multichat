#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>

#define BUFSIZE 1024

using namespace std;

char sendBuf[BUFSIZE];
char recvBuf[BUFSIZE];

void *recv_chat(void *sd) {
	int *sock = (int *)&sd;

	while(1) {
		read(*sock, recvBuf, sizeof(recvBuf));	
		cout << recvBuf << endl;

		if(*recvBuf == '#'){
			close(*sock);
			cout << "\n**Chatting is closed.**\n" << endl;
			exit(0);
		}
	}
}


void *send_chat(void *sd) {
	int *sock = (int*)&sd;

	while(1) {
		memset(sendBuf, 0, sizeof(sendBuf));		
		cin >> sendBuf;
		
		if(sizeof(sendBuf) > 1)
			send(*sock, sendBuf, BUFSIZE, 0);
	}

	cout << "=> Ended Connection...Bye" << endl;
	exit(1);
}

int main() {
	int clientsd;
	int portNum = 6743;

	struct sockaddr_in serverAddr;

	pthread_t recv_th, send_th;

	clientsd = socket(AF_INET, SOCK_STREAM, 0);

	if(clientsd == -1) {
		cout << "\nFailed to create socket client." << endl;
		exit(1);
	}

	cout << "\n=> Socket client is created..." << endl;

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(portNum);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(connect(clientsd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == 0){
		cout << "=> Connection to the server port: "<< portNum << endl;
	}
	
	memset(recvBuf, 0, sizeof(recvBuf));
	read(clientsd, recvBuf, sizeof(recvBuf));

	if(*recvBuf != '#'){
		cout << recvBuf << " Chatting started. " << endl;
	}
	else {
		close(clientsd);
		cout << "\n**Members are alreay 10. You can't enter the chat.**\n" << endl;
		return 0;	
	}

	pthread_create(&recv_th, NULL, recv_chat, (void *)clientsd);
	pthread_create(&send_th, NULL, send_chat, (void *)clientsd);

	pthread_join(recv_th, NULL);
	pthread_join(send_th, NULL);

	cout << "=> Ended connection.." << endl;
	close(clientsd);
	return 0;
}
