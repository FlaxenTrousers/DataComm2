/*
	Alex Palacio and Josh Reid
	anp344			 jmr744

	server.cpp - server for PA2
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "packet.cpp"
#include <time.h>
#include <signal.h>
#include <fstream>

#define MAXBUFLEN 100

int main(int argc, char *argv[])
{
	int sockfdReceive;
	int sockfdSend;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];
	int expectedSeqnum = 0;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;  // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;  // use my IP

	if ((rv = getaddrinfo(NULL, argv[2], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfdReceive = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("receiver: socket");
			continue;
		}

		if (bind(sockfdReceive, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfdReceive);
			perror("receiver: bind");
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "receiver: failed to bind socket\n");
		return 2;
	}

	if ((rv = getaddrinfo("127.0.0.1", argv[3], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfdSend = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("receiver: socket");
			continue;
		}

		if (connect(sockfdSend, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfdSend);
			perror("server: connect");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo);
	ofstream output, audit;
	output.open(argv[4]);
	audit.open("arrival.log");
	char spacketACK[38];
	char* blank = new char[0];
	char* finalBlank = new char[0];

	while(1) {
		bzero(buf, MAXBUFLEN);
		addr_len = sizeof their_addr;
		if ((numbytes = recvfrom(sockfdReceive, buf, MAXBUFLEN - 1, 0,
			(struct sockaddr *)&their_addr, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}

		packet recvPacket = packet(0, 0, 0, blank);
		recvPacket.deserialize(buf);

		if (expectedSeqnum == recvPacket.getSeqNum() && recvPacket.getType() == 3) {
			bzero(spacketACK, 38);
			audit << recvPacket.getSeqNum();
			audit << "\n";
			packet finalACK = packet(2, expectedSeqnum, 0, finalBlank);
			finalACK.serialize(spacketACK);
			if ((numbytes = sendto(sockfdSend, spacketACK, strlen(spacketACK), 0,
				p->ai_addr, p->ai_addrlen)) == -1) {
				perror("ACK: sendto");
				exit(1);
			}
			output.close();
			audit.close();
			close(sockfdReceive);
			close(sockfdSend);
			break;
		} 

		if (expectedSeqnum == recvPacket.getSeqNum() && recvPacket.getType() == 1) {
			bzero(spacketACK, 38);
			output << recvPacket.getData();
			audit << recvPacket.getSeqNum();
			audit << "\n";
			packet ACKpack = packet(0, expectedSeqnum, 0, blank);
			ACKpack.serialize(spacketACK);
			if ((numbytes = sendto(sockfdSend, spacketACK, strlen(spacketACK), 0,
				p->ai_addr, p->ai_addrlen)) == -1) {
				perror("ACK: sendto");
				exit(1);
			}
			expectedSeqnum = (expectedSeqnum + 1) % 8;
		}
		else {
			bzero(spacketACK, 38);
			packet ACKbadPack = packet(0, ((expectedSeqnum + 7) % 8), 0, blank);
			ACKbadPack.serialize(spacketACK);
			if ((numbytes = sendto(sockfdSend, spacketACK, strlen(spacketACK), 0,
				p->ai_addr, p->ai_addrlen)) == -1) {
				perror("ACK: sendto");
				exit(1);
			}
		}
	}

	return 0;
}