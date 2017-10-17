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

#define MAXBUFLEN 100
#define PACKETSIZE 30
#define SEQNUM 8
#define WINDOWSIZE 7  // Size of the sliding window. It will be the number of seq numbers minus 1.

using namespace std;

int sendPackets(char * file, int sockfd, struct addrinfo *p)
{
	// Build Packets
	//
	char packetData[PACKETSIZE];
	int numBytes;
	// Vars for file reading
	int actualRead = 0;
	int seqNum = 0;
	int packetNumber = 0;
	ifstream infile;
	infile.open(file);
	bool endOfFile = false;

	//Log Files
	ofstream seqlog, acklog;
	seqlog.open("seqnum.log");
	acklog.open("ack.log");

	while(!infile.eof())
	{
		bzero(packetData, PACKETSIZE);
		// Seek to proper space in file.
		infile.seekg(packetNumber * PACKETSIZE);
		infile.read(packetData, sizeof packetData);
		
		actualRead = infile.gcount();

		printf("%d\n", actualRead);
		if (actualRead == 0) break;

		//Make packet and increase sequence number.
		packet pack = packet(1, seqNum, actualRead, packetData);

		seqlog << seqNum;

		seqNum = (seqNum + 1) % SEQNUM;
		packetNumber++;

		// Serialize packet and send data.
		char spacket[actualRead+8];
		pack.serialize(spacket);
		if ((numBytes = sendto(sockfd, spacket, strlen(spacket), 0, p->ai_addr, p->ai_addrlen)) == -1) 
	    {
	   		perror("Packettalker: sendto");
	    	exit(1);
		}
	}

	packet Qpack = packet(3, seqNum, 0, NULL);
	char Qpacket[PACKETSIZE+8];
	Qpack.serialize(Qpacket);

	seqlog << seqNum;

	if ((numBytes = sendto(sockfd, Qpacket, strlen(Qpacket), 0, p->ai_addr, p->ai_addrlen)) == -1) 
    {
   		perror("EOTtalker: sendto");
    	exit(1);
	}

	close(sockfd);
	infile.close();
	seqlog.close();
	acklog.close();
	return seqNum;
}

int main(int argc, char* argv[])
{
	if (argc < 5)
	{
		printf("Incomplete number of arguments. Arguments follow the form:\n <emulatorName: host address of the emulator>, <sendToEmulator: UDP port number used by the emulator to receive data from the client>, <receiveFromEmulator: UDP port number used by the client to receive ACKs from the emulator>, <fileName: name of the file to be transferred> ");
		return 0;
	}

	//Necessary variables for UDP connection.

	struct addrinfo hints;
	int sockfd, sockfdReceive;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_DGRAM;
	int numBytes;
	char buffer[4];
	struct addrinfo *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;

	if ((rv = getaddrinfo(NULL, argv[3], &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfdReceive = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) 
		{
			perror("receiver: socket");
			continue;
		}

		if (bind(sockfdReceive, p->ai_addr, p->ai_addrlen) == -1) 
		{
			close(sockfdReceive);
			perror("receiver: bind");
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "receiver: failed to bind socket\n");
		return 2;
	}


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

	sendPackets(argv[4], sockfd, p);

	// receive ACKS
	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sockfdReceive, buf, MAXBUFLEN - 1, 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) 
	{
		perror("recvfrom");
		exit(1);
	}
	printf("%s\n", buf);
	
	close(sockfdReceive);
	return 0;
}