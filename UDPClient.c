#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <signal.h>
#include <netdb.h>
#include "UDPCookie.h"

#define PERIOD 5
#define LIMIT 10


int sigAlarmCount =0;
int sigIntCount = 0;

sigset_t mask;
int sigfillset(sigset_t *mask);

void sigAlarmHandler(){
	sigprocmask(SIG_BLOCK, &mask, NULL);
	sigAlarmCount++;
	alarm(0);
	sigprocmask(SIG_UNBLOCK, &mask,NULL);
}

int main(int argc, char *argv[]) {
  char msgBuf[MAXMSGLEN];
  int msgLen = 0; // quiet the compiler - ok - it's real quiet now i bet

  if (argc != 3) // Test for correct number of arguments
    dieWithError("Parameter(s): <Server Address/Name> <Server Port/Service>");

  char *server = argv[1];     // First arg: server address/name //1
  char *servPort = argv[2];	// Sec arg: serverPort number ///31416

  // Tell the system what kind(s) of address info we want
  struct addrinfo addrCriteria;                   // Criteria for address match
  memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
  addrCriteria.ai_family = AF_INET;               // IPv4 only
  // For the following fields, a zero value means "don't care"
  addrCriteria.ai_socktype = SOCK_DGRAM;          // Only datagram sockets
  addrCriteria.ai_protocol = IPPROTO_UDP;         // Only UDP protocol

  // Get address(es)
  struct addrinfo *servAddr;      // List of server addresses
  int rtnVal = getaddrinfo(server, servPort, &addrCriteria, &servAddr);
  if (rtnVal != 0) {
    char error[MAXMSGLEN];
    snprintf(error,MAXMSGLEN,"getaddrinfo() failed: %s",gai_strerror(rtnVal));
    dieWithError(error);
  }
  /* Create a datagram socket */
  int sock = socket(servAddr->ai_family, servAddr->ai_socktype,
		    servAddr->ai_protocol); // Socket descriptor for client
  if (sock < 0)
    dieWithSystemError("socket() failed");

  /* YOUR CODE HERE - construct Request message in msgBuf               */
  /* msgLen must contain the size (in bytes) of the Request msg         */
  
  
header_t *header = (header_t *)msgBuf; //header into msgBuf

int headerLength = 0x0c;
char* str = "stth223";

msgLen = strlen(str) + headerLength;
// assign message format
header->magic = 270 - 268 + 1 - 3;
header->magic = 0x10E;
header->length = msgLen;
header->xactionid = 0x19;
header->flags = 0x22;//1 0 1 0 = 0x2a | 0x22 if test flag not set
header->result = 0x0;
header->port = 0x0;

memcpy(&msgBuf[headerLength], str, strlen(str)); //copy variable string into msgBuf
memcpy(&msgBuf[headerLength+strlen(str)], "\0", 1); //add \0 at end of msgBuf


// client message.
printf("client message:\n");
printf("magic: %d\n", header->magic);
printf("length: %d\n", header->length);
printf("xactionid: %x\n", header->xactionid);
printf("flags: %x\n", header->flags);
printf("result: %d\n", header->result);
printf("port: %d\n", header->port);



printf("variable part: ");
	printf("%s\n",&msgBuf[headerLength]);

header->magic=htons(header->magic);
header->length=htons(header->length);

  ssize_t numBytes = sendto(sock, msgBuf, msgLen, 0, servAddr->ai_addr,
			    servAddr->ai_addrlen);
int numTransmissions = 1;
numTransmissions++;
  if (numBytes < 0)
    dieWithSystemError("sendto() failed");
  else if (numBytes != msgLen)
    dieWithError("sendto() returned unexpected number of bytes");

  /* YOUR CODE HERE - receive, parse and display response message */
   
   	struct sigaction sa;
	sa.sa_handler = sigAlarmHandler;
	sigemptyset(&sa.sa_mask);
	
	ssize_t numBytesRecieved;
	
	if (sigaction(SIGALRM, &sa, NULL) == -1){
		dieWithError("Signal failed\n");
	}
	int count = 0;
	
	while(count ==0){
		if(sigAlarmCount == 5){
			dieWithError("Error");
		}
		alarm(PERIOD);
		numBytesRecieved = recvfrom(sock, msgBuf, MAXMSGLEN, 0, NULL, NULL);
		if (numBytesRecieved < 0){
			numBytes = sendto(sock, msgBuf, msgLen, 0, servAddr->ai_addr, servAddr->ai_addrlen);
			}
			else{
			count++;
		}
		
	}

// Print Server Resposne
header_t *msg = (header_t *)msgBuf;
printf("\nServer's Response: \n");
printf("magic: %d\n", ntohs(msg->magic));
printf("length: %d\n", ntohs(msg->length));
printf("xactionid: %02x %02x %02x %02x\n", msgBuf[4], msgBuf[5], msgBuf[6], msgBuf[7]);
printf("flags: 0x%02x\n",msg->flags);
printf("result: %d\n",msg->result);
printf("port: %d\n", msg->port);
printf("variable part: %s\n", &msgBuf[headerLength]);



  freeaddrinfo(servAddr);

  close(sock);
  exit(0);
}
