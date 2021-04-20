#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "UDPCookie.h"

#define ERRLEN 128

int main(int argc, char *argv[]) {

  if (argc != 2) // Test for correct number of arguments
    dieWithError("Parameter(s): <Server Port #>");

  char *service = argv[1]; // First arg:  local port/service

  // Construct the server address structure
  struct addrinfo addrCriteria;                   // Criteria for address
  memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
  addrCriteria.ai_family = AF_INET;               // We want IPv4 only
  addrCriteria.ai_flags = AI_PASSIVE;             // Accept on any address/port
  addrCriteria.ai_socktype = SOCK_DGRAM;          // Only datagram socket
  addrCriteria.ai_protocol = IPPROTO_UDP;         // UDP socket

  struct addrinfo *servAddr; // List of server addresses
  int rtnVal = getaddrinfo(NULL, service, &addrCriteria, &servAddr);
  if (rtnVal != 0) {
    char error[ERRLEN];
    if (snprintf(error,ERRLEN,"getaddrinfo() failed: %s",
		 gai_strerror(rtnVal)) < 0) // recursive?!
      dieWithSystemError("snprintf() failed");
    dieWithError(error);
  }

  // Create socket for incoming connections
  int sock = socket(servAddr->ai_family, servAddr->ai_socktype,
      servAddr->ai_protocol);
  if (sock < 0)
    dieWithSystemError("socket() failed");

  // Bind to the local address/port
  if (bind(sock, servAddr->ai_addr, servAddr->ai_addrlen) < 0)
    dieWithSystemError("bind() failed");

  // Free address list allocated by getaddrinfo()
  freeaddrinfo(servAddr);

  for (;;) { // Run forever
    struct sockaddr_storage clntAddr; // Client address
    // Set Length of client address structure (in-out parameter)
    socklen_t clntAddrLen = sizeof(clntAddr);

    // Block until receive message from a client
    char buffer[MAXMSGLEN]; // I/O buffer
    // Size of received message
    ssize_t numBytesRcvd = recvfrom(sock, buffer, MAXMSGLEN, 0,
        (struct sockaddr *) &clntAddr, &clntAddrLen);
    if (numBytesRcvd < 0)
      dieWithSystemError("recvfrom() failed");

    /* YOUR CODE HERE:  parse & display incoming request message */
    // client message.
    	int headerLength = 0x0c;

	header_t *msg = (header_t *)buffer;
	printf("\nClient Message: \n");
	printf("magic: %d\n", ntohs(msg->magic)); //flip
	printf("length: %d\n", ntohs(msg->length));
	printf("xactionid: %02x %02x %02x %02x\n", buffer[4], buffer[5], buffer[6], buffer[7]);
	printf("flags: 0x%02x\n",msg->flags);
	printf("result: %d\n",msg->result);
	printf("port: %d\n", msg->port);
	printf("variable part: %s\n", &buffer[headerLength]);

    /* YOUR CODE HERE:  construct Response message in buffer, display it */
    
    //create respone message to send back
    
    header_t *header = (header_t*)buffer;
    char* message;
    int result = 0;
    int eFlag = 1;
    if (ntohs(msg->magic) !=270){
    	message = "Magic Num Incorrect.";
    	}
    else if(ntohs(msg->length) > 512 || ntohs(msg->length) <16){
    	message = "Message Length Incorrect.";
    	}
    
    else if(((msg->flags) & 0xF0) != 0x20){
    	message = "Version incorrect.";
    	}
    else if(msg->flags & 1){
    	message = "Bit 0 Incorrect.";
    }
    else if(!(msg->flags >>1) &1){
    	message = "Response Flag Wrong.";
    }
    else if( (msg->flags >> 2) & 1){
    	message = "Type Incorrect";
    	}
    else if (msg->result){
    	message = "Result should be 0";
    	}
    else if (msg->port){
    	message = "Port should be set to 0.";
    }
    else if(msg->flags == 0x2a){ //testing
    	message = &buffer[headerLength];
    	eFlag = 0;
    	}
    else if(msg->flags == 0x22){ //the real deal
    	message = &buffer[headerLength];
    	
    	eFlag = 0 ;
    	result = 15 ;
    	//
    	}    
    	
    // create header for response
    int msgLen = strlen(message)+headerLength;
    header->magic = 270;
    header->length = msgLen;
    header->xactionid = msg->xactionid ;
    int testFlag = ((msg->flags>>3)&1) <<3;
    header->flags = 0x20+testFlag+eFlag;
    header->result = result;
    header->port = 0;
    
    //add variable length to buffer;
    memcpy(&buffer[headerLength], message, strlen(message));
    
    //print response
    	printf("\nServer's Response: \n");
	printf("magic: %d\n", (header->magic));
	printf("length: %d\n", (header->length));
	printf("xactionid: %x\n", header->xactionid);
	printf("flags: %x\n",header->flags);
	printf("result: %d\n",header->result);
	printf("port: %d\n", header->port);
	printf("variable part: %s\n", &buffer[headerLength]);
	
	//convert to correct format before sending back
	header->magic = htons(header->magic);
	header->length = htons(header->length);

    ssize_t numBytesSent = sendto(sock, buffer, numBytesRcvd, 0,
        (struct sockaddr *) &clntAddr, sizeof(clntAddr));
    if (numBytesSent < 0){
      dieWithSystemError("sendto() failed)");
      }
  }
  // NOT REACHED
}
