//    SERVER TCP PROGRAM
// revised and tidied up by
// J.W. Atwood
// 1999 June 30
// There is still some leftover trash in this code.

/* send and receive codes between client and server */
/* This is your basic WINSOCK shell */
#pragma once
#pragma comment( linker, "/defaultlib:ws2_32.lib" )
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>
#include <winsock.h>
#include <iostream>
#include <windows.h>

#include <fstream>
//#include <string>

using namespace std;

//port data types

#define REQUEST_PORT 0x7070

int port=REQUEST_PORT;

//socket data types
SOCKET s;

SOCKET s1;
SOCKADDR_IN sa;      // filled by bind
SOCKADDR_IN sa1;     // fill with server info, IP, port
union {struct sockaddr generic;
	struct sockaddr_in ca_in;}ca;

	int calen=sizeof(ca);

	int const MAX_SIZE = 128;
	int const MAX = 30;

	//buffer data types
	char szbuffer[128];

	char *buffer;
	int ibufferlen;
	int ibytesrecv;

	int ibytessent;

	//host data types
	char localhost[11];

	HOSTENT *hp;

	//wait variables
	int nsa1;
	int r,infds=1, outfds=0;
	struct timeval timeout;
	const struct timeval *tp=&timeout;

	fd_set readfds;

	//others
	HANDLE test;
	DWORD dwtest;

	//reference for used structures

	/*  * Host structure

	    struct  hostent {
	    char    FAR * h_name;             official name of host *
	    char    FAR * FAR * h_aliases;    alias list *
	    short   h_addrtype;               host address type *
	    short   h_length;                 length of address *
	    char    FAR * FAR * h_addr_list;  list of addresses *
#define h_addr  h_addr_list[0]            address, for backward compat *
};

	 * Socket address structure

	 struct sockaddr_in {
	 short   sin_family;
	 u_short sin_port;
	 struct  in_addr sin_addr;
	 char    sin_zero[8];
	 }; sa_in */

	// Transferring the data in and from the packet structure
	struct CONTROL_FRAME
	{
		unsigned char direction;	//GET or PUT
		char fname[MAX];			//filename
	} control_frame;

	struct MESSAGE_FRAME
	{
		unsigned char header;		//ERROR, DATA, LASTPACKET, etc.
		char data [MAX_SIZE];		//data or error message
	} message_frame;

	int main(void)
	{

		WSADATA wsadata;

		try{
			if (WSAStartup(0x0202,&wsadata)!=0)
			{
				cout<<"Error in starting WSAStartup()\n";
			}else
			{
				buffer="WSAStartup was suuccessful\n";
				WriteFile(test,buffer,sizeof(buffer),&dwtest,NULL);

				/* display the wsadata structure */
				cout<< endl
					<< "wsadata.wVersion "       << wsadata.wVersion       << endl
					<< "wsadata.wHighVersion "   << wsadata.wHighVersion   << endl
					<< "wsadata.szDescription "  << wsadata.szDescription  << endl
					<< "wsadata.szSystemStatus " << wsadata.szSystemStatus << endl
					<< "wsadata.iMaxSockets "    << wsadata.iMaxSockets    << endl
					<< "wsadata.iMaxUdpDg "      << wsadata.iMaxUdpDg      << endl << endl;
			}

			//Display info of local host

			gethostname(localhost,10);
			cout << "ftpd_tcp starting at host: [" << localhost << "]" << endl;
			cout << "waiting to be contacted for transferring files..." << endl;

			if((hp=gethostbyname(localhost)) == NULL)
			{
				cout << "gethostbyname() cannot get local host info?"
					<< WSAGetLastError() << endl;
				exit(1);
			}

			//Create the server socket
			if((s = socket(AF_INET,SOCK_STREAM,0))==INVALID_SOCKET)
				throw "can't initialize socket";
			// For UDP protocol replace SOCK_STREAM with SOCK_DGRAM


			//Fill-in Server Port and Address info.
			sa.sin_family = AF_INET;
			sa.sin_port = htons(port);
			sa.sin_addr.s_addr = htonl(INADDR_ANY);


			//Bind the server port

			if (bind(s,(LPSOCKADDR)&sa,sizeof(sa)) == SOCKET_ERROR)
				throw "can't bind the socket";
			cout << "Bind was successful" << endl;

			//Successfull bind, now listen for client requests.

			if(listen(s,10) == SOCKET_ERROR)
				throw "couldn't  set up listen on socket";
			else cout << "Listen was successful" << endl;

			FD_ZERO(&readfds);

			//wait loop

			while(1)
			{

				FD_SET(s,&readfds);  //always check the listener

				if(!(outfds=select(infds,&readfds,NULL,NULL,tp))) {}

				else if (outfds == SOCKET_ERROR) throw "failure in Select";

				else if (FD_ISSET(s,&readfds))  cout << "got a connection request" << endl;

				//Found a connection request, try to accept.

				if((s1=accept(s,&ca.generic,&calen))==INVALID_SOCKET)
					throw "Couldn't accept connection\n";

				//Connection request accepted.
				cout<<"accepted connection from "<<inet_ntoa(ca.ca_in.sin_addr)<<":"
					<<hex<<htons(ca.ca_in.sin_port)<<endl;

				//variable to hold client Hostname
				char clientHostName[128] = {0};

				//Fill in szbuffer from accepted request.
				if((ibytesrecv = recv(s1,szbuffer,128,0)) == SOCKET_ERROR)
					throw "Receive error in server program\n";
				else
				{
					for(int i=0; i<strlen(szbuffer); i++)
						clientHostName[i] = szbuffer[i];
					cout << "User " << clientHostName << " request file ";
				}

				//get control_frame from client
				ibytesrecv = recv(s1, (char*)&control_frame, sizeof(control_frame), 0);

				//check for error else display direction of transfer get (send) is char 0 and put (store) is char 1
				if(ibytesrecv == SOCKET_ERROR)
					throw "Receive error in server program\n";
				else
				{
					cout << control_frame.fname << " to be ";
					if (control_frame.direction == '0')
						cout << "sent." << endl;
					else
						cout << "stored." << endl;
				}

				//if direction is get, send file to client
				if (control_frame.direction == '0')
				{
					//message saying file is being sent
					cout << "Sending file to " << clientHostName << ", waiting... " << endl;

					//open file to be sent
					ifstream fileout;

					//open file to convert to binary
					fileout.open(control_frame.fname, ios_base::in | ios_base::binary);

					//check if the file exists
					if (!fileout.is_open())
					{
						//send packet to client showing that file doesn't exist
						message_frame.header = '9';		//char 9 indicates an error

						//send error message to client
						send(s1, (char*)&message_frame, sizeof(message_frame), 0);

						//if file doesn't exists, show error message
						cout << "ERROR: File does not exist" << endl;
						cout << "File " << control_frame.fname << " cannot be sent to client " << clientHostName << endl << endl;
					}
					else		//file exits
					{
						//get the file size
						fileout.seekg(0,ios::end);
						int filesize = fileout.tellg();
						fileout.seekg(0, ios::beg);

						//to keep track of the bits sent
						int bitsread = 0;

						//to keep track of the bits left to read
						int bitsleft = filesize;

						//flag when the file is completely received
						bool lastpacket = false;

						//keep sending packets until there are no more bits to read
						while(lastpacket == false)
						{
							//if data is at the beginning of middle of the file
							if (bitsleft > MAX_SIZE)
							{
								//packet in the middle is displayed with char 1
								message_frame.header = '1';

								//read data into packet until the given size has reached
								fileout.read(message_frame.data, MAX_SIZE);

								//change bits read and bits left accordingly
								bitsleft = bitsleft - MAX_SIZE;
								bitsread = bitsread + MAX_SIZE;

								//send packet to server
								send(s1, (char *)&message_frame, sizeof(message_frame), 0);

								//flag that this is not the last packet
								lastpacket = false;
							}
							else //if data is at the end of the file
							if(bitsleft <= MAX_SIZE)
							{
							//char 2 means this packet is the last
							message_frame.header = (unsigned char)bitsleft;

							//since we will not fill the entire message_frame.data, clear it
							memset(message_frame.data, 0, sizeof(message_frame.data));

							//read data into the packet until the alloted size is reached
							fileout.read(message_frame.data, bitsleft);

							//change bits read and bits left accordingly
							bitsleft = bitsleft - bitsleft;
							bitsread = bitsread + bitsleft;

							//send packet to server
							send(s1, (char *)&message_frame, sizeof(message_frame), 0);

							//flag that this is the last packet
							lastpacket = true;
							}
						}
					}

					//close the file
					fileout.close();

					//send message as long as there is no error
					if(message_frame.header != '9')
					{
						//message saying file has been sent
						cout << "File " << control_frame.fname << " sent to client " << clientHostName << "!" << endl << endl;
					}

				}
				else //direction is put i.e. char 1, store file sent from client
				{
					//message saying file is being received
					cout << "Receiving file from " << clientHostName << ", waiting..." << endl;

					//create file to write to as filein
					ofstream filein;

					//open for output in binary mode (overwrite file if it already exists)
					filein.open(control_frame.fname, ios::out | ios::binary );

					//bool to flag when the file as been completely received
					bool lastpacket = false;

					//continue recieving packets of file until none are left i.e. receive packet indicating end of file
					while(lastpacket == false)
					{
						//get message_frame from client
						ibytesrecv = recv(s1, (char *)&message_frame, sizeof(message_frame), 0);

						//check for error else recieve packets
						if(ibytesrecv == SOCKET_ERROR)
							throw "Receive error in server program\n";
						else //no error
						{
							switch (message_frame.header)
							{
								case '1': //beginning or middle of file
									//write packet to file
									filein.write (message_frame.data, sizeof(message_frame.data));

									//flag that this is not the last packet
									lastpacket = false;
									break;
								default: //end of file
									//write packet to file
									filein.write (message_frame.data, (int)message_frame.header);

									//flag that this is the last packet
									lastpacket = true;
									break;
							}
						}
					}

					//close the file
					filein.close();

					//message saying file has been received
					cout << "File " << control_frame.fname << " received from client " << clientHostName << "!" << endl << endl;
				}


/*				//Print reciept of successful message.
				cout << "This is message from client: " << szbuffer << endl;

				//Send to Client the received message (echo it back).
				ibufferlen = strlen(szbuffer);

				if((ibytessent = send(s1,szbuffer,ibufferlen,0))==SOCKET_ERROR)
					throw "error in send in server program\n";
				else cout << "Echo message:" << szbuffer << endl;
*/

			}//wait loop

		} //try loop

		//Display needed error message.

		catch(char* str) { cerr<<str<<WSAGetLastError()<<endl;}

		//close Client socket
		closesocket(s1);

		//close server socket
		closesocket(s);

		/* When done uninstall winsock.dll (WSACleanup()) and exit */
		WSACleanup();
		return 0;
	}




