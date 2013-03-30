// CLIENT TCP PROGRAM
// Revised and tidied up by
// J.W. Atwood
// 1999 June 30

/*******************

Cherlyn Quan (9769536)
Chao Yang (5682061)

COMP 445: Programming Assignment 1
February 11, 2013

*******************/

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
#include <ctime>
#include <string>
#include "..\Project3\Router.h"

using namespace std;

//user defined port number
#define REQUEST_PORT 5000;
int port=REQUEST_PORT;

#define TRACE 1

//socket data types
SOCKET s;
SOCKADDR_IN sa;         // filled by bind
SOCKADDR_IN sa_in;      // fill with server info, IP, port

int const MAX_SIZE = 128;
int const MAX = 260;

//buffer data types
char szbuffer[128];

char *buffer;

int ibytesrecv=0;

//host data types
HOSTENT *hp;
HOSTENT *rp;

char localhost[11],
     remotehost[11];

//other
HANDLE test;
DWORD dwtest;

int infds = 1;
int outfds = 0;

struct timeval timeout;
const struct timeval *tp=&timeout;
fd_set readfds;

//variables for timer
#define UTIMER 300000
#define STIMER 0
struct timeval timeouts;

string choice;
char direction[4];
int bitsread;
int bitsleft;

int clientnb;
int servernb;
int clientseqnb;
int serverseqnb;

char http_action; // Get, Put, Lst, or Del
char filename[MAX];

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

//THREE-WAY HANDSHAKE
struct THREE_WAY_HS
{
    int client_nb;
    int server_nb;
    char direction;
    char file_name[MAX];
} three_way_hs;

struct MESSAGE_FRAME 
{
    unsigned char header;		// ERROR, DATA, LASTPACKET, etc.
    unsigned int snwseq;
    char data [MAX_SIZE];		// data or error message
} message_frame;

int len = sizeof(struct sockaddr);

void action_list(void);
void action_delete(void);

int main(void)
{
    // Handshake
    srand((unsigned)time(NULL));

    timeouts.tv_sec = STIMER;
    timeouts.tv_usec = UTIMER;

    //Create log file and open for writting
    FILE* filelog;
    filelog = fopen("clientlog.txt", "w");

    WSADATA wsadata;

    try 
    {
        if (WSAStartup(0x0202, &wsadata) != 0)
        {
            cout << "Error in starting up WSAStartup()" << endl;
        }
        else
        {
            buffer = "WSAStartup succesfully.\n";
            WriteFile(test, buffer, sizeof(buffer), &dwtest, NULL);
        }

        // Display name of local host
        gethostname(localhost, 10);

        // Start the client
        cout << "FTP UDP CLIENT Started at host \"" << localhost << "\"" << endl;
        cout << "======================================================" << endl;
        cout << "Attempting to connect to socket." << endl;

        if ((hp = gethostbyname(localhost)) == NULL) 
        {
            cerr << "Error: Client cannot connect to socket." << endl;
        }

        //Ask for name of remote server
        cout << "Enter remote server name: " << flush ;   
        cin >> remotehost ;

        //Display remote host name
        cout << "Remote host name is: \"" << remotehost << "\"" << endl;

        if ((rp = gethostbyname(remotehost)) == NULL) 
        {
            cerr << "ERROR: Failed getting remote host by name." << endl;
        }

        // Create the socket
        if ((s = socket(AF_INET,SOCK_DGRAM,0)) == INVALID_SOCKET) 
        {
            cerr << "ERROR: Failed connecting to local socket." << endl;
        }
        /* For UDP protocol replace SOCK_STREAM with SOCK_DGRAM */

        //Fill-in Client Port and Address Info
        memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;   
        sa.sin_port = htons(PEER_PORT1);
        sa.sin_addr.s_addr = htonl(INADDR_ANY);		//host to network
        //bind the port to the socket
        if (bind(s, (LPSOCKADDR)&sa, sizeof(sa)) == SOCKET_ERROR)
        {
            cerr << "ERROR: Failed binding to local host socket." << endl;
        }

        //Specify server address for client to connect to server.
        memset(&sa_in, 0, sizeof(sa_in));
        memcpy(&sa_in.sin_addr, rp->h_addr, rp->h_length);
        sa_in.sin_family = rp->h_addrtype;   
        sa_in.sin_port = htons(ROUTER_PORT1);

        //Display the host machine internet address
        cout << "Connecting to remote host \""
             << inet_ntoa(sa_in.sin_addr) << "\"..." << endl;

        //Connect Client to the server
        if (connect(s, (LPSOCKADDR)&sa_in, sizeof(sa_in)) == SOCKET_ERROR)
        {
            cout << "ERROR: Failed connecting to the remote host." << endl;
        }

        // Length of sockaddr struct
        int len = sizeof(struct sockaddr);
        
        // * **** * **** * //
        // REPEAT LOOP
        // * **** * **** * //
        while (true) {
        //make sure input from user is get, put or quit or else keep asking
        do
        {
            cout << "What would you like to do (get/put/del/lst): ";
            //get direction of transfer from user
            cin >> direction;

        } while (!strcmp(direction, "get") == 0 && !strcmp(direction, "put") == 0
            && !strcmp(direction, "del") == 0 && !strcmp(direction, "lst") == 0);

        // In three_way_hs, list is char l, delete is char d, get is char g and put is char p
        if (strcmp(direction, "lst") == 0)
            http_action = 'l';
        else if (strcmp(direction, "del") == 0)
            http_action = 'd';
        else if(strcmp(direction, "get") == 0)
            http_action = 'g';
        else if (strcmp(direction, "put") == 0)
            http_action = 'p';

        // Ask for the filename if not listing
        if (http_action != 'l')
        {
            cout << "FILENAME: ";
            cin >> filename;
        }

        // Create random client number for the 3-way handshake
        clientnb = rand() % 256;
        cout << "*********************" << endl;
        cout << "Client Number: " << clientnb << endl;
        cout << "*********************" << endl;

        // three_way_hs.client_nb = clientnb;
        // to clear the direction transfer
        // memset(direction, 0, sizeof(direction));
        // START OF THREE-WAY HANDSHAKE
        // timeout loop to receive server number in three way handshake
        int i = 1; // Packet tracker in TWH
        
        /**************/
        /* SYN        */
        /**************/
        while (true)
        {
            FD_ZERO(&readfds);
            FD_SET(s, &readfds);

            // Clean the three way hs struct
            memset(&three_way_hs, 0, sizeof(three_way_hs));

            three_way_hs.client_nb = clientnb;
            three_way_hs.direction = http_action;
			strcpy(three_way_hs.file_name, filename);

            // sending three_way_hs with client number to server
            // sendto(socket to send result, datagram result to send, datagram length, flags: no options, addr, addr lenth);
            // SYN
            cout << "[TWH]: SENDING SYN TO SERVER" << endl;
            sendto(s, (char*)&three_way_hs, sizeof(three_way_hs), 0, (struct sockaddr*)&sa_in, sizeof(sa_in)); 
                
            //timeout is 300ms
            // WAITING FOR SYN,ACK
            outfds = select(1, &readfds, NULL, NULL, &timeouts);

            if (outfds) // if outfds is 1, data have been received
            {
                //receive server number in three way handshake
                //recvform(socket, buffer, length of buffer, flags, socket address, length of socket address);
                ibytesrecv = recvfrom(s, (char*)&three_way_hs, sizeof(three_way_hs), 0, (struct sockaddr*)&sa_in, &len);
            
                //to check for error
                if (ibytesrecv == SOCKET_ERROR)
                {
                    cerr << "ERROR: Client socket SYN error." << endl;
                }

                //to check if the received client number is the same as the first created
                if (three_way_hs.client_nb != clientnb)
                {
                    cerr << "[TWH]: WRONG SYN,ACK NUMBER " << three_way_hs.client_nb
                            << "FROM SERVER PACKET (" << i++ << ")" << endl;
                    continue;
                }

                break;
            }
        }

        // CLIENT NUMBER MUST BE CORRECT NOW
        /*******/
        /* ACK
        /*******/
        cout << "[TWH]: SYN, ACK RECEIVED" << endl;
        sendto(s, (char*)&three_way_hs, sizeof(three_way_hs), 0, (struct sockaddr*)&sa_in, sizeof(sa_in));
        cout << "[TWH]: ACK SENT." << endl;
        cout << "[TWH]: THE FATE OF THIS CONNECTION WILL BE DETERMINED BY THIS PACKET." << endl;

        //THREE WAY HANDSHAKE COMPLETE

        //to determine initial sequence number
        //0 if number is even; 1 if number is odd
        clientseqnb = three_way_hs.server_nb % 2;  // Sequence number expected by client
        serverseqnb = three_way_hs.client_nb % 2;  // Sequence number expected by server

        //if direction is list (i.e. char 'l'), list the files in the directory
        switch (three_way_hs.direction)
        {
        case 'l':
            action_list();
            break;
        case 'd':
            action_delete();
            break;
        }

    } // END OF REPEAT LOOP
    } // END OF TRY LOOP
    catch (char *str) 
    { 
        fprintf(filelog, "%s: %s %s", str, dec, WSAGetLastError());
        fflush(filelog);
        cerr << str << ":" << dec << WSAGetLastError() << endl;
    }

    fprintf(filelog, "DONE.");
    fflush(filelog);

    //close the client socket
    closesocket(s);

    /* When done uninstall winsock.dll (WSACleanup()) and exit */ 
    WSACleanup();  

    //close log file
    fclose(filelog);

    return 0;
}

void action_put(void)
{

}

void action_delete(void)
{
    cout << "[DEL]: DELETING SERVER FILE \"" << filename << "\"" << endl;
	bool ack = false;
    while (!ack)
    {
        // Just waiting for the reply, because the TWH contains the delete information
        // that the server should already have received
        cout << "[DEL]: WAITING FOR SERVER FILE STATUS REPLY" << endl;
        FD_ZERO(&readfds);
        FD_SET(s, &readfds);

        outfds = select(1, &readfds, NULL, NULL, &timeouts);

        if (outfds)
        {
            ibytesrecv = recvfrom(s, (char*)&message_frame, sizeof(message_frame), 0, (struct sockaddr*)&sa_in, &len);
            
            cout << "[DEL]: RECEIVED PACKET SEQ (" << message_frame.snwseq
                     << ") EXPECTING SEQ (" << serverseqnb << ")" << endl;

            if (ibytesrecv == SOCKET_ERROR) 
                cerr << "ERROR: Socket error from [DEL] packet." << endl;
            
            if (message_frame.snwseq == serverseqnb)
            {
                cout << "[DEL]: IN ORDER PACKET SEQUENCE DETECTED" << endl;
                switch (message_frame.header)
                {
                case 'e':
                    cout << "[DEL]: ERROR, NO FILE FOUND ON SERVER!" << endl;
                    break;
                case 's':
                    cout << "[DEL]: FILE DELETED FROM SERVER!" << endl;
                    break;
                }

                // Expecting next sequence packet
                serverseqnb++;

                // After receiving this packet send a single ACK
                memset(message_frame.data, 0, sizeof(message_frame.data));
                    
                message_frame.header = 'a'; // PACKET ACK
                message_frame.snwseq = serverseqnb;

                sendto(s, (char*)&message_frame, sizeof(message_frame), 0, (struct sockaddr*)&sa_in, sizeof(sa_in));

                cout << "[DEL]: ACK SEND FOR PACKET SEQ (" << message_frame.snwseq << ")" << endl;
                cout << "[DEL]: THIS PACKET IS SEND ONCE. FAILED TO RECEIVE BY SERVER WILL CRASH." << endl;
                ack = true;
            }
            else
            {
                cout << "[DEL]: OUT OF ORDER PACKET SEQ (" << message_frame.snwseq 
                     << ") EXPECTING (" << serverseqnb << ")" << endl;
                cout << "[DEL]: SENDING ACK FOR EXPECTED PACKET SEQ (" << serverseqnb << ")" << endl;

                memset(message_frame.data, 0, sizeof(message_frame.data));
                    
                message_frame.header = 'a'; // PACKET ACK
                message_frame.snwseq = serverseqnb;

                sendto(s, (char*)&message_frame, sizeof(message_frame), 0, (struct sockaddr*)&sa_in, sizeof(sa_in));
            }
        }
    }
}

void action_list(void)
{
    
    cout << "[LST]: LISTING SERVER FOLDER DIRECTORY." << endl;
	bool ack = false;

    do
    {
        // Clear the message frame
        memset(message_frame.data, 0, sizeof(message_frame.data));

        while (!ack)
        {
            // ACTION LIST					
            FD_ZERO(&readfds);
            FD_SET(s, &readfds);

            outfds = select(1, &readfds, NULL, NULL, &timeouts);

            if (outfds)
            {
                ibytesrecv = recvfrom(s, (char*)&message_frame, sizeof(message_frame), 0, (struct sockaddr*)&sa_in, &len);

                cout << "[LST]: RECEIVED PACKET SEQ (" << message_frame.snwseq
                     << ") EXPECTING SEQ (" << serverseqnb << ")" << endl;

                if (ibytesrecv == SOCKET_ERROR)
                {
                    cerr << "ERROR: Socket error from [LST] packet." << endl;
                }

                // Checking if this packet is in sequence
                if (message_frame.snwseq == serverseqnb)
                {
                    cout << "[LST]: IN ORDER PACKET" << endl;
                    switch (message_frame.header)
                    {
                    case 'e':
                        cout << "[LST]: NO FILE FOUND ON SERVER!" << endl;
                        break;
                    case 'f':
                        cout << message_frame.data << endl << endl;
                        break;
                    case 'l':
                        // PRINT INFO TO LOG
                        cout << "[LST]: TRANSFER COMPLETE" << endl;
						ack = true;
                        break;
                    }

                    // Expecting next sequence packet
                    serverseqnb++;

                    // After receiving this packet send a single ACK
                    memset(message_frame.data, 0, sizeof(message_frame.data));
                    
                    message_frame.header = 'a'; // PACKET ACK
                    message_frame.snwseq = serverseqnb;

                    sendto(s, (char*)&message_frame, sizeof(message_frame), 0, (struct sockaddr*)&sa_in, sizeof(sa_in));

                    cout << "[LST]: ACK SEND FOR PACKET SEQ (" << message_frame.snwseq << ")" << endl;
                }
                else // Send ACK for the latest sequence number
                {
                    cout << "[LST]: OUT OF ORDER PACKET SEQ (" << message_frame.snwseq 
                         << ") EXPECTING (" << serverseqnb << ")" << endl;
                    cout << "[LST]: SENDING ACK FOR EXPECTED PACKET SEQ (" << serverseqnb << ")" << endl;

                    memset(message_frame.data, 0, sizeof(message_frame.data));
                    
                    message_frame.header = 'a'; // PACKET ACK
                    message_frame.snwseq = serverseqnb;

                    sendto(s, (char*)&message_frame, sizeof(message_frame), 0, (struct sockaddr*)&sa_in, sizeof(sa_in));
                }
            }
        }
    } while (!ack);
}

