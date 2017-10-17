/*
	Alex Palacio and Josh Reid
	anp344			 jmr744

	client.cpp - client for PA2
*/

#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <fstream>
#include <iostream>
#include "packet.cpp"

#define SEQNUM 8
#define WINDOWSIZE 7  // Size of the sliding window. It will be the number of seq numbers minus 1.

using namespace std;


int main(int argc, char* argv[])
{
/* 
	arg[1] = <emulatorName: host address of the emulator>,
	arg[2] = <sendToEmulator: UDP port number used by the emulator to receive
				data from the client>
	arg[3] = <receiveFromEmulator: UDP port number used by the client to receive
				ACKs from the emulator>,
	arg[4] = <fileName: name of the file to be transferred> 
*/

	if (argc < 5)
	{
		printf("Incomplete number of arguments. Arguments follow the form:\n <emulatorName: host address of the emulator>, <sendToEmulator: UDP port number used by the emulator to receive data from the client>, <receiveFromEmulator: UDP port number used by the client to receive ACKs from the emulator>, <fileName: name of the file to be transferred> ");
		return 0;
	}

	//Necessary variables for UDP connection.

	struct addrinfo hints;
	int sockfd;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_DGRAM;
	int numBytes;
	char buffer[4];
	struct addrinfo *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	// Get the server(emulator) address.
	if ((rv = getaddrinfo("127.0.0.1", argv[2], &hints, &servinfo)) != 0)
	{
		// Print to stderror if there is an error in getting the server address
		fprintf(stderr, "getaddrinfo client: %s\n", gai_strerror(rv)); 
		return 1;
	}

	// Connect to the server. Code from beej.us - Link at top of file.
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
		{
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}
	// If no connection was formed, throw an error.
	// Server may not be initialized
	if (p == NULL)
	{
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}
	// clear the linked list
	freeaddrinfo(servinfo);
	// Build Packets
	//
	char packetData[30];

	// Vars for file reading
	int actualRead = 0;
	int wholePacks = 0;
	int extraBytes = 0;
	int seqNum = 0;
	int packetNumber = 0;
	ifstream infile;
	infile.open(argv[4]);

	// Seek to proper space in file.
	infile.seekg(packetNumber * 30);
	infile.read(packetData, sizeof packetData);
	//Make packet and increase sequence number.
	packet pack = packet(1, seqNum, 30, packetData);
	seqNum = (seqNum + 1) % SEQNUM;

	// Serialize packet and send data.
	char spacket[37];
	pack.serialize(spacket); //SEGFAULT

	if ((numBytes = sendto(sockfd, spacket, 37, 0, p->ai_addr, p->ai_addrlen)) == -1) 
	    {
	   		perror("talker: sendto");
	    	exit(1);
		}
//*/
	infile.close();
	close(sockfd);
	return 0;
}