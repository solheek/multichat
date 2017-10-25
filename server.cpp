#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define BUFSIZE 1024

using namespace std;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;//imutex object 

/* shared resource */
char recvbuf[BUFSIZE]; //receive from client
char endbuf[2];
int cNum; //count client
int socks[10]; //max client is 10

int listensd = -1;

void *chat(void *sd) {	
	int *sock = (int *)&sd; 
	char sendbuf[BUFSIZE];

	sprintf(sendbuf, "%s %d %s", "\n=> You're < client ", (*sock)+1, ">. ");	
	cout << "New < client " << (*sock)+1 << " > is accessed" << endl;
	send(socks[*sock], sendbuf, sizeof(sendbuf), 0);
	
	while(1){
		if(*endbuf == '#')
			break; 

		memset(sendbuf, 0, sizeof(sendbuf));
	
		pthread_mutex_trylock(&mutex);
		read(socks[*sock], recvbuf, sizeof(recvbuf));

		cout << "** Client " << (*sock)+1 << ": " << recvbuf << endl;
	
		sprintf(sendbuf, "%s %d %s", "Client",(*sock)+1, ": ");
		strncat(sendbuf, recvbuf, 1000);
		for(int i=0 ; i<sizeof(socks) ; i++) {
			if(i!=(*sock))
				send(socks[i], sendbuf, sizeof(sendbuf),0);
		}
		pthread_mutex_unlock(&mutex);
	
	}
}

void *end_chat(void *sd) {
	int *sock = (int *)&sd;

	memset(endbuf, 0, sizeof(endbuf));
	while(1) {
		gets(endbuf);

		if(*endbuf == '#'){
			for(int i=0 ; i<cNum; i++){
				send(socks[i], endbuf, sizeof(endbuf), 0);
				close(socks[i]);
			}
		}
		break;
	}
	close(listensd);
	cout << "=> Close Chatting." << endl;
	exit(0);
}

int main() { 
	struct sockaddr_in serverAddr; 
	socklen_t size; 
	pthread_t thread[10];
	pthread_t end_thread;

	int i=0;

	memset(thread, 0, sizeof(thread));

	listensd = socket(PF_INET, SOCK_STREAM, 0); //IPv4 protocol

	if(listensd == -1) {
		cout << "\nFailed to create socket server." << endl;
		exit(1);
	}

	cout << "\n=> Socket server is created.." << endl;

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET; 
	serverAddr.sin_port = htons(6743); 
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(listensd, (struct sockaddr*)&serverAddr, sizeof(serverAddr))){
		cout << "\nFailed to bind server Address." << endl;
		exit(1);
	}

	cout << "=> Looking for clients..." << endl; 

	if(listen(listensd, 10) == -1) {
		cout << "\nListen Error." <<endl;
		exit(1);
	}
			
	cout << "=> Server is running on port " << ntohs(serverAddr.sin_port) <<endl;

	/******* Chat ******/
	while(1) {
		int connectsd;
		char *ip_addr;
		
		struct sockaddr_in clientAddr;
		size = sizeof(clientAddr);

		connectsd = accept(listensd, (struct sockaddr*)&clientAddr, &size);
		ip_addr = inet_ntoa(clientAddr.sin_addr);

		if(cNum < 10){ //MAX: 2 people
			cout << "\n=> New connection from " << ip_addr << endl;
			socks[cNum++] = connectsd;
			cout << "=> Total client is " << cNum  << ". "<< endl;
			pthread_create(&thread[cNum-1], NULL, chat, (void *)(cNum-1));
			pthread_create(&end_thread, NULL, end_chat, (void *)(listensd));
		}
		else {
			cout << "\n=> Total client is already 10. Can't accept more client.\n" << endl;
			send(connectsd, "#", sizeof(endbuf),0);
		}

	}	

	//for(i=0 ; i<sizeof(thread) ; i++){
	//	if(thread[i] != 0)
	//		pthread_join(thread[i], NULL);
	//}

	//pthread_join(end_thread, NULL);
	
	cout << "=> Ended chatting... Bye!" << endl;
	
	close(listensd);

	return 0;
}
