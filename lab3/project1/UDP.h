/*
 *Project properties -> Configuration Properties -> Linker -> Input -> Additional Dependencies -> ws2_32.lib
 */
#pragma comment( linker, "/defaultlib:ws2_32.lib" )

#include <WS2tcpip.h>
#include <WinSock2.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <queue>
#include <utility>
#include <stack>

using namespace std;
#define TIMEOUT 3000 //integer in ms
#define IP "127.0.0.1" //IP address, in format of "x.x.x.x"
#define LIMIT 50 //try out time limit
#define PORTSERVER 5001
#define PORTCLIENT 5000
#define ROUTERCLIENT 7000
#define ROUTERSERVER 7001
//Define message frame structure
#define STOPNWAIT 1 //Bits width for stop and wait

	struct MESSAGE_FRAME {
		unsigned char number;
		unsigned char ack:STOPNWAIT;
		char data[1024]; //or error message
	} message_frame;

		SOCKET SOCK1, SOCK2;
	sockaddr_in addr1, addr2;
	int len=sizeof(sockaddr);
	HOSTENT* hp;
	SOCKADDR from;
	SOCKADDR_IN sa_in_router;		// address structure for peer host 1 address
	SOCKADDR_IN sa_in;

	int totalBytesSent = 0;
	int totalBytesReceived = 0;
	int numFramesSent = 0;
	int numFramesReceived = 0;
	int totalFramesSent = 0;
	int totalFramesReceived = 0;
	int p=0, w=0;
	char szbuffer[128];
	int windowsize,totalFrames,framessend=0,i=0,j=0,k,buffer,l;
	int serversocket;
	sockaddr_in serveraddr,clientaddr;
	char req[50];
	queue<MESSAGE_FRAME> queueFrames;
	queue<MESSAGE_FRAME> queueAcks;


	ofstream fout;			//log file
	int choiceSC, windowSize;
	int infds=1, outfds=0;
	fd_set readfds;
	struct timeval timeout;
	struct timeval *tp=&timeout;
	char previous_buffer[1026] = "";
	char localhost[20], remotehost[20],filename[250],choice[3];

	//Define Three way handshake structure
	struct THREE_WAY_HS{
		int client_number;
		char direction[3];
		int server_number;
		char file_name[50];
		char windowSize[5];
	} three_way_hs;

int socketC(int port); //for server use
int socketS(int port); //for client use
int sendto(char * buf, int length, int a = 1);
int receiveFrom(char * buf, int c, int a = 1); //if c is set to 0, random drop/delay is enabled. 0 means disabled.
void closesocket();
int speed();
void delay(int n);
int routerAddr(char* r, char * c); //get router name and find ip
char* serializeMSG(MESSAGE_FRAME msg);
MESSAGE_FRAME deserializeMSG(char *message);
void sendData(char *sendbuf, int port);
void sendData(char *sendbuf, int size, int port);
char recvData(char *recvbuf,int size);
void clear(char *str);
void sendFrame(MESSAGE_FRAME frame);
MESSAGE_FRAME recvFrame();
queue<MESSAGE_FRAME> recvQueue();