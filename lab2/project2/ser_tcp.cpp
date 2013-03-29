//    SERVER TCP PROGRAM
// revised and tidied up by
// J.W. Atwood
// 1999 June 30
// There is still some leftover trash in this code.

/*******************

Cherlyn Quan (9769536)
Chao Yang (5682061)

COMP 445: Programming Assignment 1
February 11, 2013

*******************/

/* send and receive codes between client and server */
/* This is your basic WINSOCK shell */
#pragma once
#pragma comment( linker, "/defaultlib:ws2_32.lib" )
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>
#include <winsock.h>
#include <iostream>
#include <stdio.h>
#include <windows.h>

#include <fstream>
#include <ctime>
//#include <string>
#include "..\Project3\Router.h"

using namespace std;

//port data types

#define REQUEST_PORT 5001;
int port=REQUEST_PORT;

#define TRACE 1

//socket data types
SOCKET s;
SOCKADDR_IN sa;      // filled by bind; server info, IP, port 5001
SOCKADDR_IN sa_in;     // fill with server info, IP, port; router info, IP, port 7001
union {struct sockaddr generic;
	struct sockaddr_in ca_in;}ca;

int calen=sizeof(ca); 

int const MAX_SIZE = 128;
int const MAX = 260;

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

//variables for the timer
#define UTIMER 300000
#define STIMER 0
struct timeval timeouts;

int bitsread;
int bitsleft;

int clientnb;
int servernb;
int clientseqnb;
int serverseqnb;

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

//THREE-WAY HANDSHAKE
struct MESSAGE_FRAME 
{
	unsigned char header;		//ERROR, DATA, LASTPACKET, etc.
	unsigned int snwseq;
	char data [MAX_SIZE];		//data or error message
} message_frame;

struct THREE_WAY_HS
{
	int client_nb;
	int server_nb;
	char direction;
	char file_name[MAX];
} three_way_hs;

int main(void)
{
	//handshake
	THREE_WAY_HS three_way_hs;
	srand((unsigned)time(NULL));

	timeouts.tv_sec = STIMER;
	timeouts.tv_usec = UTIMER;

	//create log file and open for writting
	FILE* filelog;
	filelog = fopen("serverlog.txt", "w");

	WSADATA wsadata;

	try
	{        		 
		if (WSAStartup(0x0202,&wsadata)!=0)
		{  
			cout<<"Error in starting WSAStartup()\n";
		}
		else
		{
			buffer="WSAStartup was suuccessful\n";   
			WriteFile(test, buffer, sizeof(buffer), &dwtest, NULL); 
		}  

		gethostname(localhost, 10);

		// Start the server
		cout << "FTP UDP SERVER Started at host \"" << localhost << "\"" << endl;
		cout << "======================================================" << endl;
		cout << "Attempting to connect to socket." << endl;

		if ((hp = gethostbyname(localhost)) == NULL) 
		{
			cerr << "Error: gethostbyname() cannot get local host info: "
				 << WSAGetLastError() << endl; 
		}

		//Create the server socket
		if((s = socket(AF_INET,SOCK_DGRAM,0))==INVALID_SOCKET) 
		{
			cerr << "Error: Client cannot connect to socket." << endl;
		}
		// For UDP protocol replace SOCK_STREAM with SOCK_DGRAM 

		//Fill-in Server Port and Address info.
		sa.sin_family = AF_INET;
		sa.sin_port = htons(PEER_PORT2);
		sa.sin_addr.s_addr = htonl(INADDR_ANY);

		//Bind the server port
		if (bind(s,(LPSOCKADDR)&sa,sizeof(sa)) == SOCKET_ERROR)
		{
			cerr << "ERROR: Cannot bind to socket." << endl;
		}

		cout << "Socket connection established. BLAB ready." << endl;

		FD_ZERO(&readfds);
		FD_SET(s, &readfds);

		//variable to hold cliendhostname
		char clientHostName[128] = {0};

		//length of sockaddr struct
		int len = sizeof(struct sockaddr);

		/******************************/
		/* LISTENING MODE
		/******************************/
		while(true)
		{
			

			/*******************************/
			/* START OF THREE WAY HANDSHAKE */
			/******************************/
			//timeout loop to receieve client number in three way handshake
			while(true)
			{
				FD_ZERO(&readfds);
				FD_SET(s, &readfds);

				//notify client action
				cout << "Waiting for incoming client connection..." << endl;
				
				// timeout is 0 -> wait forever until it receives something
				outfds = select(1, &readfds, NULL, NULL,  0);

				//if outfds is 1, data has been received
				if (outfds)
				{
					cout << "[WARNING]: Incoming client connection detected..." << endl;
					cout << "Attempting to establish connection through THREEWAY-HANDSHAKE [TWH] protocol." << endl;
					break;
				}
				//if outfds is 0, data has not been received. Then, does nothing and keep waiting
			}

			//receive client number in three way handshake
			if (outfds == 1)
				ibytesrecv = recvfrom(s, (char*)&three_way_hs, sizeof(three_way_hs), 0, (struct sockaddr*)&sa, &len);

			//to check for error else display direction of transfer
			if (ibytesrecv == SOCKET_ERROR)
			{
				cerr << "Error: Socket error from 1st THREEWAY-HANDSHAKE packet." << endl;
			}

			//create server number for three qay handshake
			servernb = rand() * 11 % 256;
			three_way_hs.server_nb = servernb;
			clientnb = three_way_hs.client_nb;

			cout << "*********************" << endl;
			cout << "Server Number: " << servernb << endl;
			cout << "Client Number: " << clientnb << endl;
			cout << "*********************" << endl;

			//quit program if direction is q is received
			switch (three_way_hs.direction)
			{
			case 'l':
				cout << "ACTION [LST]" << endl;
				break;
			case 'd':
				cout << "ACTION [DEL]" << endl;
				break;
			case 'g':
				cout << "ACTION [GET]" << endl;
				break;
			case 'p':
				cout << "ACTION [PUT]" << endl;
			default:
				cout << "ACTION [ERR]" << endl;
			}

			// timeout loop to receive client ACK in three way handshake
			int i = 1; // Packet tracker in TWH

			while(true)
			{
				FD_ZERO(&readfds);
				FD_SET(s, &readfds);

				//send server number in three way handshake
				cout << "[TWH]: SENT SYN,ACK TO CLIENT (" << clientnb << ")" << endl;
				sendto(s, (char*)&three_way_hs, sizeof(three_way_hs), 0, (struct sockaddr*)&sa, sizeof(sa));

				//timeout is 300ms
				cout << "[TWH]: WAITING ACK FROM CLIEMT" << endl;
				outfds = select(1, &readfds, NULL, NULL, &timeouts);

				//if outfds is 1, data has been received
				if (outfds)
				{
					//receive client ACK to finish three way handshake
					ibytesrecv = recvfrom(s, (char*)&three_way_hs, sizeof(three_way_hs), 0, (struct sockaddr*)&sa, &len);

					//check for error 
					if (ibytesrecv == SOCKET_ERROR)
					{
						cerr << "ERROR: Socket error from receiving [TWH] 3rd ack." << endl;
					}
			
					//check if the receiver server number is the same as the first one created
					if (three_way_hs.server_nb != servernb)
					{
						cout << "[TWH]: WRONG SERVER NUMBER " << three_way_hs.server_nb
							 << " FROM CLIENT PACKET (" << i++ << ")" << endl;
						continue;
					}
					else if (three_way_hs.server_nb == servernb)
					{
						cout << "[TWH]: ACK RECEIVED. CONNECTION ESTABLISHED" << endl;
						break;
					}
				}
			}

			/**********************************************************/
			/* END OF THREE WAY HANDSHAKE 
			/**********************************************************/

			//determine initial sequence numbers: 0 if number is even; 1 if it is odd
			serverseqnb = three_way_hs.client_nb % 2; // Sequence number expected by client
			clientseqnb = three_way_hs.server_nb % 2; // Sequence number expected by server

			// If direction is list (i.e. char l), list all files on server
			
			/**********************************************************/
			/* START OF LISTING
			/**********************************************************/
			if (three_way_hs.direction == 'l')
			{
				//directory search variables
				WIN32_FIND_DATA fd;
				HANDLE h = FindFirstFile("*.*", &fd);

				while(true)
				{

					while(true)
					{
						FD_ZERO(&readfds);
						FD_SET(s, &readfds);

						// seqnb
						message_frame.snwseq = serverseqnb;

						// if there are files on the server
						// char f indicates first or middle file on server
						message_frame.header = 'f';

						// Clear data_frame.data
						memset(message_frame.data, 0, sizeof(message_frame.data));

						// Place file name into packet
						strcpy_s(message_frame.data, sizeof(message_frame.data), fd.cFileName);

						// Send file name into packet
						sendto(s, (char*)&message_frame, sizeof(message_frame), 0, (struct sockaddr*)&sa, sizeof(sa));

						// Output
						cout << "[LST]: SENT PACKET SEQ (" << message_frame.snwseq << ")" << endl;

						//timeout is 300ms
						outfds = select(1, &readfds, NULL, NULL, &timeouts);

						//if outfds is 1, data has been received
						if (outfds)
						{

							// Receive client ACK to finish three way handshake
							ibytesrecv = recvfrom(s, (char*)&message_frame, sizeof(message_frame), 0, (struct sockaddr*)&sa, &len);

							// Sequence number counter for received ACKS
							cout << "[LST]: RECEIVED PACKET SEQ (" << message_frame.snwseq << ")" << endl;
							

							//check for error
							if (ibytesrecv == SOCKET_ERROR)
							{
								cerr << "ERROR: Socket connection error for receiving LST acks";
							}

							//check if seqnb are the same
							if ((serverseqnb+1) == message_frame.snwseq
								&& message_frame.header == 'a')
							{
								cout << "[LST]: RECEIVED PACKET IS ACK FOR SEQ (" << message_frame.snwseq << ")" << endl;
								
								// Add 1 to seqnb	
								serverseqnb++;
								break;
							}

							//else resend packet
						}

						//if outfds is 0, data has not been received. Then restart loop and resend message
					}

					// If no more files on server left to display
					if (FindNextFile(h, &fd) == FALSE)
					{
						while(true)
						{
							FD_ZERO(&readfds);
							FD_SET(s, &readfds);

							//seqnb
							message_frame.snwseq = serverseqnb;

							//char l indicates no more files left on server
							message_frame.header = 'l';

							//clear data_frame.data
							memset(message_frame.data, 0, sizeof(message_frame.data));

							//send packet indicating no more files left on the server
							sendto(s, (char*)&message_frame, sizeof(message_frame), 0, (struct sockaddr*)&sa, sizeof(sa));

							//print info to log
							cout << "[LST]: LAST PACKET. SEQ (" << message_frame.snwseq << ")" << endl;

							//timeout is 300ms
							outfds = select(1, &readfds, NULL, NULL, &timeouts);

							//if outfds is 1, data has been received
							if (outfds)
							{

								// Receive client ACK to finish three way handshake
								ibytesrecv = recvfrom(s, (char*)&message_frame, sizeof(message_frame), 0, (struct sockaddr*)&sa, &len);

								// Sequence number counter for received ACKS
								cout << "[LST]: RECEIVED PACKET SEQ (" << message_frame.snwseq << ")" << endl;
							

								//check for error
								if (ibytesrecv == SOCKET_ERROR)
								{
									cerr << "ERROR: Socket connection error for receiving LST acks";
								}

								//check if seqnb are the same
								if ((serverseqnb+1) == message_frame.snwseq
									&& message_frame.header == 'a')
								{
									// This SEQ should be serverseqnb + 1
									// That is the correct SEQ number that the client is expecting
									cout << "[LST]: RECEIVED PACKET IS ACK FOR SEQ (" << message_frame.snwseq << ")" << endl;
								
									// Add 1 to seqnb	
									serverseqnb = serverseqnb + 1;
									break;
								}
							}

							//if outfds is 0, data has not been received. Then restart loop and resend message
						}

						//print info to log
						cout << "[LST]: COMPLETED" << endl;

						break;
					}
				}
			} // END OF [LST]


			/**********************************************************/
			/* START OF DELETING
			/**********************************************************/
			if (three_way_hs.direction == 'd')
			{
				// Open the file to be deleted
				ifstream filedel;

				filedel.open(three_way_hs.file_name, ios_base::in | ios_base::binary);

				if (!filedel.is_open()) // IF ITS NOT OPEN, THEN FILE DONT EXISTS
				{
					cout << "[DEL]: FILE DOES NOT EXISTS ON SERVER!" << endl;
					while (true)
					{
						// Attach message frame
						message_frame.header = 'e';

						// Set sequence number
						message_frame.snwseq = serverseqnb;

						// Send file name into packet
						sendto(s, (char*)&message_frame, sizeof(message_frame), 0, (struct sockaddr*)&sa, sizeof(sa));

						cout << "[DEL]: SENT ERROR PACKET SEQ (" << message_frame.snwseq << ")" << endl;

						// timeout is 300ms
						outfds = select(1, &readfds, NULL, NULL, &timeouts);

						// WAITING FOR ACK
						if (outfds)
						{
							// Waiting for the ACK
							// Receive client ACK to finish three way handshake
							ibytesrecv = recvfrom(s, (char*)&message_frame, sizeof(message_frame), 0, (struct sockaddr*)&sa, &len);

							// Sequence number counter for received ACKS
							cout << "[DEL]: RECEIVED PACKET SEQ (" << message_frame.snwseq << ")" << endl;
							

							//check for error
							if (ibytesrecv == SOCKET_ERROR)
							{
								cerr << "ERROR: Socket connection error for receiving DEL acks";
							}

							//check if seqnb are the same
							if ((serverseqnb+1) == message_frame.snwseq
								&& message_frame.header == 'a')
							{
								// This SEQ should be serverseqnb + 1
								// That is the correct SEQ number that the client is expecting
								cout << "[DEL]: RECEIVED PACKET IS ACK FOR SEQ (" << message_frame.snwseq << ")" << endl;
								// Add 1 to seqnb	
								serverseqnb = serverseqnb + 1;
								
								cout << "[DEL]: COMPLETED" << endl;

								break;
							}
						}
					} // END OF FILE DOES NOT EXISTS WHILE LOOP
				} // END OF IF FILE DOES NOT EXISTS
			} // END OF DELETE ACTION
		} // END OF WAIT LISTENING


	}  // END OF TRY
	catch(char* str) 
	{ 
		fprintf(filelog, "%s: %s %s", str, dec, WSAGetLastError());
		fflush(filelog);
		cerr << str << WSAGetLastError() << endl;
	}

	//close Client socket
	closesocket(s);		

	//close server socket
	closesocket(s);

	/* When done uninstall winsock.dll (WSACleanup()) and exit */ 
	WSACleanup();

	//message that program is exiting
	cout << "System is quitting... " << endl;

	//close file log
	fclose(filelog);

	return 0;
}




