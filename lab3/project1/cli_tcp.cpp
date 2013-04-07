// #include <stdio.h>
#include "UDP.h"
#include <iostream>
#pragma comment( linker, "/defaultlib:ws2_32.lib" )
using namespace std;

#pragma region Message
char* serializeMSG(MESSAGE_FRAME msg)
{
	const int MaxDataSize = 1024;

	char n[32];
	sprintf(n, "%d", msg.number);

	char message[MaxDataSize+5] = "";
	strcat(message, n);
	for (unsigned i=0;i<MaxDataSize;i++)
		message[i+2] = msg.data[i];
	
	return message;
}

MESSAGE_FRAME deserializeMSG(char *message)
{
	int MaxDataSize = 1024;
	MESSAGE_FRAME frame;

	frame.number = message[0];
	for (unsigned i=0;i<MaxDataSize;i++)
		frame.data[i] = message[i+2];

	return frame;
}
#pragma endregion Message

#pragma region usingSockets
void sendData(char *sendbuf, int port)
{
	sa_in.sin_port = htons(port);
	sendto(SOCK1, sendbuf, strlen(sendbuf), 0, (SOCKADDR*)&sa_in_router, sizeof(sa_in_router));
}

//overload to be able to force length of sent buffer
void sendData(char *sendbuf, int size, int port)
{
	sa_in.sin_port = htons(port);
	sendto(SOCK1, sendbuf, size, 0, (SOCKADDR*)&sa_in_router, sizeof(sa_in_router));
}

char recvData(char *recvbuf,int size)
{
	return recvfrom(SOCK1,recvbuf,size,0,(SOCKADDR*)&sa_in_router,&len);
}

void clear(char *str){
	for(int i=0;i<sizeof(str);++i){
		str[i] = '\0';
	}
}

#pragma endregion usingSockets

#pragma region Frames
void sendFrame(queue<MESSAGE_FRAME> q){
	for(int i = 0; i<q.size(); i++){
		char* data;
		data = serializeMSG(q.front());
		sendData(data, 1026);
		q.pop();
	}
}

queue<MESSAGE_FRAME> recvQueue(){
	queue<MESSAGE_FRAME> queueRcv, tempQ;
	char rec[1026] = "";
	MESSAGE_FRAME frame;
	++totalFramesReceived;
	clear(rec);
	for(int i = 0; i<5; i++){
		recvData(rec,1026);
		frame = deserializeMSG(rec);
		tempQ.push(frame);
		totalBytesReceived += 1024;
	}
	return queueRcv;
}

void sendFrame(MESSAGE_FRAME frame)
{
	char* data;
	++totalFramesSent;
	data = serializeMSG(frame);
	sendData(data,1026);
	totalBytesSent += 1024;
}

MESSAGE_FRAME recvFrame()
{
    char rec[1026] = "";
	MESSAGE_FRAME frame;
	++totalFramesReceived;
	clear(rec);
	recvData(rec,1026);
	frame = deserializeMSG(rec);
	totalBytesReceived += 1024;
	return frame;
}
#pragma endregion Frames

#pragma region SocketMethods
//Get Router information

int routerAddr(char * r, char * c){
	choiceSC = (int)c;
	if(strcmp(c, "1\0") == 0)
	{
		if((hp=gethostbyname(r)) == NULL) 
				return -1;
			memset(&sa_in_router,0,sizeof(sa_in_router));
			memcpy(&sa_in_router.sin_addr,hp->h_addr,hp->h_length);
			sa_in_router.sin_family = hp->h_addrtype;   
			sa_in_router.sin_port = htons(ROUTERSERVER);
	}
	else
	{
		if((hp=gethostbyname(r)) == NULL) 
				return -1;
			memset(&sa_in_router,0,sizeof(sa_in_router));
			memcpy(&sa_in_router.sin_addr,hp->h_addr,hp->h_length);
			sa_in_router.sin_family = hp->h_addrtype;   
			sa_in_router.sin_port = htons(ROUTERCLIENT);
	}
	return 0;
}

/*Socket for server*/
int socketS(int port) {
	WSADATA wsaData;
	int Ret = WSAStartup(MAKEWORD(2,2),&wsaData);//version Winsock 2.2
	printf("Initialize winsock . . .\n");
	if(Ret != 0) {
		printf("Unable to initialize winsock.\n");
		WSACleanup();//unable to open a socket, then cleanup
		return -1;  //error
	}
	else {
		printf("Done.\n");
	}
	    
	/*Create a UDP socket and bind it.*/
	printf("Creating UDP server socket . . .\n");
	SOCK1 = ::socket(AF_INET,SOCK_DGRAM,0);

	memset(&sa_in, 0, sizeof(sa_in));
	sa_in.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//define the server's address
	sa_in.sin_family = AF_INET;
	sa_in.sin_port = htons(port);
	    
	if(SOCK1 == INVALID_SOCKET) {
		printf("Socket Error!\n");
		WSACleanup();
		return -1;  //error
	}
	printf("Done.\n");
	printf("Binding UDP socket . . .\n");
	if (bind(SOCK1, (LPSOCKADDR) &sa_in, sizeof(sa_in)) == SOCKET_ERROR) {
		printf("Bind socket 1 failed.\n");
		return -1;  //error
		WSACleanup();
	}
	printf("Done.\n");
	return 0; //return success
}

/*Socket for client*/
int socketC(int port) {
	WSADATA wsaData;
	int Ret = WSAStartup(MAKEWORD(2,2),&wsaData);//version Winsock 2.2
	printf("Initializing winsock . . .\n");
	if (Ret != 0) {
		printf("Unable to initialize winsock.\n");
		WSACleanup();//unable to open a socket, then cleanup
		return -1;  //error
	}
	else {
		printf("Done.\n");
	}
	    
	/*Create a UDP socket and bind it.*/
	printf("Creating UDP client socket . . .\n");
	SOCK1 = ::socket(AF_INET,SOCK_DGRAM,0); //create UDP socket for client
	SOCK2 = ::socket(AF_INET,SOCK_DGRAM,0);

	memset(&sa_in, 0, sizeof(sa_in));
	sa_in.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//define the server's address
	sa_in.sin_family = AF_INET;
	sa_in.sin_port = htons(port);

	if(SOCK1 == INVALID_SOCKET) {
		printf("Socket Error!\n");
		WSACleanup();
		return -1;  //error
	}
	printf("Done.\n");
	printf("Binding UDP socket . . .\n");

	if (bind(SOCK1, (LPSOCKADDR) &sa_in, sizeof(sa_in)) == SOCKET_ERROR) {
		printf("Bind socket 1 failed.\n");
		return -1;  //error
		WSACleanup();
	}
	printf("Done.\n");
	return 0; //return success
}

#pragma endregion SocketMethods

#pragma region TWHS
void threeWay(){
	char rec[50] = "";
	bool ack = false;
	char data[4] = "";

	while(!ack){

		itoa(three_way_hs.client_number, data,10);
		if(choiceSC == 1)
			sendData(data, PORTSERVER);
		else
			sendData(data, PORTCLIENT);
		totalBytesSent += 3;
		fout<<"Sent Client Number: "<< three_way_hs.client_number << endl;

		FD_SET(SOCK1,&readfds); 
		if(!(outfds=select(infds,&readfds,NULL,NULL,tp))) {}
		else if (outfds == SOCKET_ERROR) throw "failure in Select";
		if(outfds > 0){
				recvData(rec, 7);
				totalBytesReceived += 7;

				for (unsigned j=0;j<strlen(rec);++j)
					previous_buffer[j] = rec[j];

				int d = 0;
				char d1[4] = "",
					 d2[4] = "";
				for (int i = 0; i<strlen(rec);++i){
					if (rec[i] == '-')
						++d;
					else if (d == 0)
						d1[i] = rec[i];
					else
						d2[i-strlen(d1)-1]=rec[i];
				}
				if (three_way_hs.client_number == atoi(d1)){
					three_way_hs.server_number = atoi(d2);
					ack = true;
					fout << "Received Server number:  " << three_way_hs.server_number << endl; 
				}
		}
	}

	itoa(three_way_hs.server_number, data,10);
	if(choiceSC == 1)
		sendData(data, PORTSERVER);
	else
		sendData(data, PORTCLIENT);
	totalBytesSent += 1024;
	fout << "Sent ACK for Server Number" << endl;

	fout << "Sending direction" << endl; 
	MESSAGE_FRAME m;
	strcpy(m.data, three_way_hs.direction);
	sendFrame(m);

	fout << "Sending file name" << endl; 
	strcpy(m.data, three_way_hs.file_name);
	sendFrame(m);
}

void recvThreeWay()
{
    char rec[50] = "";
    bool ack = false;
	char data[50] = "";
	char data2[4] = "";

    recvData(rec,3);
	totalBytesReceived += 3;
    three_way_hs.client_number = atoi(rec);
	fout<<"Received Client Number: "<< three_way_hs.client_number << endl;
	for (unsigned i=0;i<sizeof(rec);i++)
		previous_buffer[i] = rec[i];

	itoa(three_way_hs.client_number, data,10);
	itoa(three_way_hs.server_number, data2,10);
	strcat(data, "-");
	strcat(data, data2);

    while(!ack){
           
        if(choiceSC == 1)
			sendData(data, PORTSERVER);
		else
			sendData(data, PORTCLIENT);
		totalBytesSent += 7;
		fout << "Sent ACK of Client Number and Server Number (" << data <<")" << endl;

        FD_SET(SOCK1,&readfds);  //always check the listener
        if(!(outfds=select(infds,&readfds,NULL,NULL,tp))) {}
        else if (outfds == SOCKET_ERROR) throw "failure in Select";
        if(outfds > 0){
				recvData(rec, 3);
				totalBytesReceived += 1024;
				if (strcmp(previous_buffer,rec) != 0){
					for (unsigned i=0;i<sizeof(rec);i++)
						previous_buffer[i] = rec[i];
					ack = true;
					fout << "Received ACK for Server Number" << endl;
				}
        }
    }

	fout << "Received Direction" << endl;
	MESSAGE_FRAME m;
	m = recvFrame();
	strcpy(three_way_hs.direction, m.data);

	fout << "Received file name" << endl;
	m = recvFrame();
	strcpy(three_way_hs.file_name, m.data);

}

#pragma endregion TWHS

#pragma region BinaryFileSend
void fileSend(char * file){


	// Extract filename
	char filename[50];
	int i=strlen(file);
	for(;i>0;i--)
		if(file[i-1]=='\\')
			break;
	for(int j=0;i<=(int)strlen(file);i++)
		filename[j++]=file[i];

	ifstream myFile (file, ios::in|ios::binary|ios::ate);
	int size = (int)myFile.tellg();
	myFile.close();

	char filesize[10];itoa(size,filesize,10);

	char rec[32] = "";
	MESSAGE_FRAME m;

	totalFrames = size / 1024;
	if(size%1024 > 0)
		totalFrames++;

	strcpy(m.data, filesize);
	fout << "Sending file size (" << filesize <<")"<<endl;
	sendFrame(m);

	FILE *fr = fopen(file, "rb");

	bool initial = true;

	while(p<totalFrames)
	{
		while(w < windowSize)
		{
			// Create Queue of 5 frames
			char buffer[1024]; 
			fread(buffer, 1024, 1, fr);
			for (unsigned i=0;i<1024;i++)
				m.data[i] = buffer[i];
			m.number = p + w;
			queueFrames.push(m);
			w++;
			p++;
		}
		p=queueFrames.back().number;
		sendFrame(queueFrames);

		queueAcks = recvQueue();

		int popCounter = 0;
		for(int i = 0; i < queueAcks.size(); i++)
		{
			if(queueAcks.front().data)
			{
				queueAcks.pop();
				popCounter++;
				if(&queueFrames.front() == &queueFrames.back())
					w = 0;
			}
			else
			{
				p -= popCounter;
				w -= popCounter;
				break;
			}
		}
	}
	fclose(fr);
}

void fileRecv(){

	char rec[50] = "";

	//Get message containing file size
	MESSAGE_FRAME m;
	m = recvFrame();
	int size = atoi(m.data);

	totalFrames = size / 1024;
	if(size%1024 > 0)
		totalFrames++;
	
	if(size == -1){
		fout << "File does not exist on server!" << endl << endl;
		cout << "File does not exist on server!" << endl;
		return;
	}
	else{
		FILE *fw = fopen(three_way_hs.file_name, "a+");
		while(p<totalFrames && w<windowSize){
				m = recvFrame();
				if (m.number == i){
					m.ack = 1;
					queueFrames.push(m);
					p++;
				}
				else{
					m.ack = 0;
					clear(m.data);
					m.number = i;
					queueFrames.push(m);
				}
				w++;
		}
		MESSAGE_FRAME a;
		for(int i = 0; i < queueFrames.size() ; i++){
			a.ack = queueFrames.front().ack;
			a.data[0] =  queueFrames.front().ack;
			a.number = queueFrames.front().number;
			queueAcks.push(a);
		}
		sendFrame(queueAcks);
		for(int i = 0; i< queueFrames.size(); i++){
			bool good = true;
			if(good){
				if (queueFrames.front().ack){
					fwrite(queueFrames.front().data, 1024, 1, fw);
					queueFrames.pop();
				}
				else
				{
					queueFrames.pop();
					good = false;
				}
			}
			else
				queueFrames.pop();
		}
		fclose(fw);
	}
	
}
#pragma endregion BinaryFileSend

void clearSocket(){
	//Empty socket buffer.
	bool empty = false;
	while(!empty){
		char rec[2] = "";
		FD_SET(SOCK1,&readfds);  //always check the listener
		if(!(outfds=select(infds,&readfds,NULL,NULL,tp))) {}
		else if (outfds == SOCKET_ERROR) throw "failure in Select";
		if(outfds > 0){
				recvData(rec, 1);
		}else{
			empty = true;
		}
	}

}

void closesocket() {
	printf("Closing sockets . . .\n");
	WSACleanup();
	closesocket(SOCK1);
	closesocket(SOCK1);
	printf("Done.");
}

 int main(void) {
	 char *fn="log.txt";
	fout.open(fn);

	tp->tv_sec=0;
	tp->tv_usec=30000;

	char choice[16];
	while(1){
	printf("\nPlease select the usage of this program:");
	printf("\n[1]Server or [2]Client or [3]Quit: ");
	scanf("%s", &choice);
	printf("\n");
	char router[255];
	if (strcmp(choice, "3\0") != 0){
		printf("Please enter the router's name:");
		scanf("%s", &router);
	}

	if (strcmp(choice, "1\0") == 0) {//act as server

		if (socketS(PORTSERVER) == -1) {
			printf("Failed to create UDP socket!\n");
			return 0;
		}
		char rbuf[32768] = "\0";		
		if(routerAddr(router, choice) == -1)
			printf("This is not working");

		cout << "Server ready. Waiting for client...\n" << endl;

		while(1){
			
			FD_ZERO(&readfds);
			FD_SET(SOCK1,&readfds);  //always check the listener
			if(!(outfds=select(infds,&readfds,NULL,NULL,tp))) {}
			else if (outfds == SOCKET_ERROR) throw "failure in Select";
			else if (FD_ISSET(SOCK1,&readfds)){
				cout << "Got a connection request" << endl; 
				fout << "Got a connection request" << endl; 
			}

			//Found a connection request, try to accept. 
			if(outfds > 0){

				totalBytesReceived = 0;
				totalBytesSent = 0;
				numFramesSent = 0;
				numFramesReceived = 0;
				totalFramesSent = 0;
				totalFramesReceived = 0;

				srand ( time(NULL) );
				three_way_hs.server_number = rand() % 255;
				recvThreeWay();

				cout << "Request from \"" << inet_ntoa(sa_in.sin_addr) << "\" for file: \"" << three_way_hs.file_name << "\". Direction: " << three_way_hs.direction <<endl;
				fout << "Request from \"" << inet_ntoa(sa_in.sin_addr) << "\" for file: \"" << three_way_hs.file_name << "\". Direction: " << three_way_hs.direction <<endl;

				char rec[50] = "";

				// PUT or GET branches ////////////////////////////////////////
				if(strcmp(three_way_hs.direction, "put") == 0 ){

					cout << "<< RECEIVING >>" << endl;
					fout << "Beginning to RECEIVE" << endl;
						
					fileRecv();
					clearSocket();
				}
				else if(strcmp(three_way_hs.direction, "get") ==0 ){
					cout << endl << "<< SENDING >>" << endl;
					fout << "Beginning to SEND" << endl;

					fileSend(three_way_hs.file_name);
					clearSocket();
				}

				cout << "Done" << endl << "----------\n" << endl;
				cout << "Server ready. Waiting for client...\n" << endl;
				fout << "Done" << endl;
				fout << "Bytes Sent: " << totalBytesSent << endl;
				fout << "Bytes Received: " << totalBytesReceived << endl;
				fout << "Frames Sent: " << totalFramesSent << endl;
				fout << "Frames Received: " << totalFramesReceived << endl << endl;
			}
		}//wait loop
		closesocket();
		printf("\n");
	}
	else if (strcmp(choice, "2\0") == 0) { //act as client
		if (socketC(PORTCLIENT) == -1) {
			printf("Failed to create UDP socket!\n");
			return 0;
		}
		if(routerAddr(router, choice) == -1){
			printf("This is not working");
			exit(0);
		}

		try{

			cout << "Do you want to \"put\" or \"get\"? ";
				cin >> choice;

				cout << "Please enter the path and filename: ";
				cin >> filename;

				bool filecheckloop = true;

				while(filecheckloop && strcmp(choice, "put") == 0){

					//Check if file exists
					ifstream ifile(filename);
					if(!ifile){
						cout << "File not found\n\n";
						cout << "Please enter the path and filename: ";
						cin >> filename;
					}
					else{
						filecheckloop = false;
					}
				}

				fout << "User selected to " << choice << " file " << filename <<endl;

				totalBytesReceived = 0;
				totalBytesSent = 0;
				numFramesSent = 0;
				numFramesReceived = 0;
				totalFramesSent = 0;
				totalFramesReceived = 0;

				sprintf(szbuffer,"%s", filename); 
				char rec[50] = "";

				//Build 3WHS message
				srand( time(NULL) );
				three_way_hs.client_number = rand() % 255;
				strcpy(three_way_hs.direction, choice);
				strcpy(three_way_hs.file_name,szbuffer);

				
				threeWay();

				// PUT or GET branches ////////////////////////////////////////
				if (strcmp(choice, "put") == 0){
					cout << endl << "<< SENDING >>" << endl;
					fout << "Beginning to SEND" << endl;

 					fileSend(filename);
					clearSocket();
				}
				else if(strcmp(choice, "get") ==0){
					cout << endl << "<< RECEIVING >>" << endl;
					fout << "Beginning to RECEIVE" << endl;

					fileRecv();
					clearSocket();
				}
			} // try loop
			
		//Display any needed error response.
		catch (char *str){ 
			cerr<<str<<":"<<dec<<WSAGetLastError()<<endl;
			fout<<str<<":"<<dec<<WSAGetLastError()<<endl;
		}

		cout << "Done" << endl << "----------\n" << endl;
		fout << "Done" << endl;
		fout << "Bytes Sent: " << totalBytesSent << endl;
		fout << "Bytes Received: " << totalBytesReceived << endl;
		fout << "Total Frames Sent: " << totalFramesSent << endl;
		fout << "Total Frames Received: " << totalFramesReceived << endl << endl;
		closesocket();
		}

	else if (strcmp(choice, "3\0") == 0){ //quit program
		fout << "- User terminated program - "<< endl;
		exit(0);
	}
	else
		printf("Please enter 1 or 2 or 3!\n\n");
	}
	return 0;
}