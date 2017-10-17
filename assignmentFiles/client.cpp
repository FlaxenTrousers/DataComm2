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

#define WINDOWSIZE 7  // Size of the sliding window. It will be the number of seq numbers minus 1.


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
		printf("Incomplete number of arguments. Arguments follow the form:\n 
				<emulatorName: host address of the emulator>,
				<sendToEmulator: UDP port number used by the emulator to receive data from the client>,
				<receiveFromEmulator: UDP port number used by the client to receive ACKs from the emulator>,
				<fileName: name of the file to be transferred> ");
		return 0;
	}


}