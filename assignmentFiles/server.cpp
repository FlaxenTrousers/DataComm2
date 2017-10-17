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

int randomPortNumber()
{
	srand ((unsigned int)time(NULL));
	int min = 1024;
	int max = 65535;
	int range = (max - min) + 1;
	int rnd = min + rand() % range;
}

packet depacketizeMeCaptain(char * spacket) {
	char blank[1];
	packet pack = packet(0, 0, 0, blank);
	pack.deserialize(spacket);
	return pack;
}

packet ACKnCrunch(int expectedSeqnum) {
	packet pack = packet(0, expectedSeqnum, 0, NULL);
	return pack;
}

packet finalCrunch(int expectedSeqnum) {
	packet pack = packet(2, expectedSeqnum, 0, NULL);
	return pack;
}

int main(int argc, char *argv[])
{
	int sockfd;
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
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("receiver: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("receiver: bind");
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "receiver: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	while(1) {
		addr_len = sizeof their_addr;
		if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,
			(struct sockaddr *)&their_addr, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}

		char spacket[numbytes];
		strncpy(spacket, buf, numbytes);
		packet pack = depacketizeMeCaptain(spacket);

		if (expectedSeqnum == pack.getSeqNum()) {
			char spacketACK[7];
			packet ACKpack = ACKnCrunch(expectedSeqnum);
			ACKpack.serialize(spacketACK);
			if ((numbytes = sendto(sockfd, spacket, strlen(spacket), 0,
				p->ai_addr, p->ai_addrlen)) == -1) {
				perror("ACK: sendto");
				exit(1);
			}
		}

		if (pack.getType() == 3) {
			// send final ACK
			char spacketFinalACK[7];
			packet finalACK = finalCrunch(expectedSeqnum);
			finalACK.serialize(spacketFinalACK);
			if ((numbytes = sendto(sockfd, spacketFinalACK, strlen(spacketFinalACK), 0,
				p->ai_addr, p->ai_addrlen)) == -1) {
				perror("ACK: sendto");
				exit(1);
			}
			break;
		} 
	}

	close(sockfd);

	return 0;
}