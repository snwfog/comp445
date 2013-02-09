// CLIENT TCP PROGRAM
// Revised and tidied up by
// J.W. Atwood
// 1999 June 30

//char* getmessage(char *);

/* send and receive codes between client and server */
/* This is your basic WINSOCK shell */
#pragma comment( linker, "/defaultlib:ws2_32.lib" )
#include <winsock2.h>
#include <ws2tcpip.h>

#include <winsock.h>
#include <stdio.h>
#include <iostream>

#include <string.h>
#include <windows.h>

#include <fstream>

using namespace std;

//user defined port number
#define REQUEST_PORT 0x7070;
int port=REQUEST_PORT;

//socket data types
SOCKET s;
SOCKADDR_IN sa;         // filled by bind
SOCKADDR_IN sa_in;      // fill with server info, IP, port

int const MAX_SIZE = 128;
int const MAX = 30;

//buffer data types
char szbuffer[128];

char *buffer;

int ibufferlen=0;
//int ibytessent;
int ibytesrecv=0;

int msgSent;
int msgReceived = 0;

//host data types
HOSTENT *hp;
HOSTENT *rp;

char localhost[11],
     remotehost[11];

//other
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
 }; */

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

	try
	{
		if (WSAStartup(0x0202,&wsadata)!=0)
		{
			cout<<"Error in starting WSAStartup()" << endl;
		}
		else
		{
			buffer="WSAStartup was successful\n";
			WriteFile(test,buffer,sizeof(buffer),&dwtest,NULL);

			/* Display the wsadata structure */
			cout<< endl
				<< "wsadata.wVersion "       << wsadata.wVersion       << endl
				<< "wsadata.wHighVersion "   << wsadata.wHighVersion   << endl
				<< "wsadata.szDescription "  << wsadata.szDescription  << endl
				<< "wsadata.szSystemStatus " << wsadata.szSystemStatus << endl
				<< "wsadata.iMaxSockets "    << wsadata.iMaxSockets    << endl
				<< "wsadata.iMaxUdpDg "      << wsadata.iMaxUdpDg      << endl << endl;
		}

		/*
		//Display name of local host.

		gethostname(localhost,10);
		cout<<"Local host name is \"" << localhost << "\"" << endl;

		if((hp=gethostbyname(localhost)) == NULL)
			throw "gethostbyname failed\n";

		//Ask for name of remote server

		cout << "please enter your remote server name :" << flush ;
		cin >> remotehost ;
		cout << "Remote host name is: \"" << remotehost << "\"" << endl;

		if((rp=gethostbyname(remotehost)) == NULL)
			throw "remote gethostbyname failed\n";

		//Create the socket
		if((s = socket(AF_INET,SOCK_STREAM,0))==INVALID_SOCKET)
			throw "Socket failed\n";
		/* For UDP protocol replace SOCK_STREAM with SOCK_DGRAM */
		/*
		//Specify server address for client to connect to server.
		memset(&sa_in,0,sizeof(sa_in));
		memcpy(&sa_in.sin_addr,rp->h_addr,rp->h_length);
		sa_in.sin_family = rp->h_addrtype;
		sa_in.sin_port = htons(port);

		//Display the host machine internet address

		cout << "Connecting to remote host:";
		cout << inet_ntoa(sa_in.sin_addr) << endl;

		//Connect Client to the server
		if (connect(s,(LPSOCKADDR)&sa_in,sizeof(sa_in)) == SOCKET_ERROR)
			throw "connect failed\n";

		/* Have an open connection, so, server is

		   - waiting for the client request message
		   - don't forget to append <carriage return>
		   - <line feed> characters after the send buffer to indicate end-of file */
		/*
		//append client message to szbuffer + send.

		sprintf(szbuffer,"hello world! ...\r\n");

		ibytessent=0;
		ibufferlen = strlen(szbuffer);
		ibytessent = send(s,szbuffer,ibufferlen,0);
		if (ibytessent == SOCKET_ERROR)
			throw "Send failed\n";
		else
			cout << "Message to server: " << szbuffer;

		//wait for reception of server response.
		ibytesrecv=0;
		if((ibytesrecv = recv(s,szbuffer,128,0)) == SOCKET_ERROR)
			throw "Receive failed\n";
		else
			cout << "hip hip hoorah!: Successful message replied from server: " << szbuffer;

		*/

		//Display name of local host.
		gethostname(localhost,10);
		cout << "ftp_tcp starting at host: [" << localhost << "]" << endl;

		if((hp=gethostbyname(localhost)) == NULL)
			throw "gethostbyname failed\n";

		//stays connected until user types "quit" to stop connection
		while(!strcmp(remotehost, "quit") == 0)
		{

			//Ask for name of remote server
			cout << "please enter your remote server name: " << flush ;
			cin >> remotehost ;

			//If remote host is "quit" then exit the program
			if (strcmp(remotehost, "quit") == 0)
				exit(0);

			//Display remote host name
			cout << "Remote host name is: \"" << remotehost << "\"" << endl;

			if((rp=gethostbyname(remotehost)) == NULL)
				throw "remote gethostbyname failed\n";

			//Create the socket
			if((s = socket(AF_INET,SOCK_STREAM,0))==INVALID_SOCKET)
				throw "Socket failed\n";
			/* For UDP protocol replace SOCK_STREAM with SOCK_DGRAM */

			//Specify server address for client to connect to server.
			memset(&sa_in,0,sizeof(sa_in));
			memcpy(&sa_in.sin_addr,rp->h_addr,rp->h_length);
			sa_in.sin_family = rp->h_addrtype;
			sa_in.sin_port = htons(port);

			//Display the host machine internet address
			cout << "Connecting to remote host: ";
			cout << inet_ntoa(sa_in.sin_addr) << endl;

			//Connect Client to the server
			if (connect(s,(LPSOCKADDR)&sa_in,sizeof(sa_in)) == SOCKET_ERROR)
				throw "connect failed\n";

			//sending client's hostname to server
			sprintf(szbuffer, localhost);
			msgSent = 0;
			ibufferlen = strlen(szbuffer);
			msgSent = send(s,szbuffer,ibufferlen,0);

			/* Have an open connection, for server to

			   - waiting for the client request message
			   - don't forget to append <carriage return>
			   - <line feed> characters after the send buffer to indicate end-of file

			*/

			//variable to hold direction of transfer i.e. get or put
			char direction[4];


			//to clear the direction transfer
			memset(direction, '\0', sizeof(direction));

			//make sure input from user is get, put or quit or else keep asking
			while(!strcmp(direction, "get") == 0 && !strcmp(direction, "put") == 0)
			{
				cout << "Type direction of transfer: ";
				//get direction of transfer from user
				cin >> direction;
			}

			//in control_frame get is char 0 and put is char 1
			if(strcmp(direction, "get") == 0)
				control_frame.direction = '0';
			else
				control_frame.direction = '1';

			//Ask for name of file
			cout << "Type name of file to be transferred: ";
			cin >> control_frame.fname;

			//if direction is put check the file exists on the clients computer
			if(control_frame.direction == '1')
			{
				//try to open requested file
				ifstream checkfile;
				checkfile.open(control_frame.fname, ios_base::in | ios_base::binary );

				//keep asking for filename until it exists
				while(!checkfile.is_open()) //checkfile doesn't exist
				{
					//close file
					checkfile.close();

					//clear control_frame.fname
					memset(control_frame.fname, 0, sizeof(control_frame.fname));

					//Ask for name of file
					cout << "Type name of file to be transferred: ";
					cin >> control_frame.fname;

					//try to open the file
					checkfile.open(control_frame.fname, ios_base::in | ios_base::binary );
				}

				//close file
				checkfile.close();
			}

			//send control_frame to server
			send(s, (char *)&control_frame, sizeof(control_frame), 0);

			//if direction is get i.e. char 0, retrieve file to server
			if(control_frame.direction == '0')
			{
				//message saying file is being received
				cout << "Receiving file from " << remotehost << ", waiting..." << endl;

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
					ibytesrecv = recv(s, (char *)&message_frame, sizeof(message_frame), 0);

					//check for error else recieve packets
					if(ibytesrecv == SOCKET_ERROR)
						throw "Receive error in server program\n";
					else //no error
					{
						switch (message_frame.header)
						{
							case '9':
								//close file
								filein.close();

								//delete file since it doesn't exist on the server
								remove(control_frame.fname);

								//display message that file doesn't exist on the server
								cout <<"ERROR: File Does Not Exist!" << endl;
								cout << "File " << control_frame.fname << " cannot be retrieved from the server ";
								cout << remotehost << "!" << endl << endl;

								//flag that this is the last packet
								lastpacket = true;
								break;
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


				//success unless recieved error
				if(message_frame.header != '9')
				{
					//close the file
					filein.close();

					//message saying file has been received
					cout << "File " << control_frame.fname << " received from host " << remotehost << "!" << endl << endl;
				}
			}
			else //direction is put i.e. char 1, send file to server to be stored
			{
				//message saying file is being sent
				cout << "Sending file to " << remotehost << ", waiting..." << endl;

				//open file to be sent as fileout
				ifstream fileout;

				//open for input in binary mode
				fileout.open(control_frame.fname, ios_base::in | ios_base::binary );

				//check if the requested file exists (this should never happen, just a double check)
				if (!fileout.is_open()) //file does not exit
				{
					//if file does not exist: error message
					cout <<"ERROR: File Does Not Exist!" << endl << endl;
				}
				else //file exists
				{
					//get the file size
					fileout.seekg (0, ios::end);
					int filesize = fileout.tellg();
					fileout.seekg (0, ios::beg);

					//keep track of the bits already read
					int bitsread = 0;

					//keep track of the bits left to read
					int bitsleft = filesize;

					//bool to flag when the file as been completely received
					bool lastpacket = false;

					//keep sending packets containing the file data until there are no more bits to read
					while(lastpacket == false)
					{
						//if data is at the begining or middle of the file
						if(bitsleft > MAX_SIZE)
						{
							//char 1 means this packet is in the middle
							message_frame.header = '1';

							//read data into the packet until the alloted size is reached
							fileout.read(message_frame.data, MAX_SIZE);

							//change bits read and bits left accordingly
							bitsleft = bitsleft - MAX_SIZE;
							bitsread = bitsread + MAX_SIZE;

							//send packet to server
							send(s, (char *)&message_frame, sizeof(message_frame), 0);

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
								send(s, (char *)&message_frame, sizeof(message_frame), 0);

								//flag that this is the last packet
								lastpacket = true;
							}
					}
				}

				//close the file
				fileout.close();

				//message saying file has been sent
				cout << "File " << control_frame.fname << " send to host " << remotehost << "!" << endl << endl;
			}
		} //end while of quit
	} // try loop

	//Display any needed error response.
	catch (char *str) { cerr<<str<<":"<<dec<<WSAGetLastError()<<endl;}

	//close the client socket
	closesocket(s);

	/* When done uninstall winsock.dll (WSACleanup()) and exit */
	WSACleanup();
	return 0;
}





