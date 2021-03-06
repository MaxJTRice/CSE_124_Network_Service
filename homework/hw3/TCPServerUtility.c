#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include "Practical.h"

static const int MAXPENDING = 5; // Maximum outstanding connection requests

int SetupTCPServerSocket(const char *service) {
  // Construct the server address structure
  struct addrinfo addrCriteria;                   // Criteria for address match
  memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
  addrCriteria.ai_family = AF_UNSPEC;             // Any address family
  addrCriteria.ai_flags = AI_PASSIVE;             // Accept on any address/port
  addrCriteria.ai_socktype = SOCK_STREAM;         // Only stream sockets
  addrCriteria.ai_protocol = IPPROTO_TCP;         // Only TCP protocol

  struct addrinfo *servAddr; // List of server addresses
  int rtnVal = getaddrinfo(NULL, service, &addrCriteria, &servAddr);
  if (rtnVal != 0)
    DieWithUserMessage("getaddrinfo() failed", gai_strerror(rtnVal));

  int servSock = -1;
  struct addrinfo *addr = servAddr;
  for (addr = servAddr; addr != NULL; addr = addr->ai_next) {
    // Create a TCP socket
    servSock = socket(addr->ai_family, addr->ai_socktype,
        addr->ai_protocol);
    if (servSock < 0)
      continue;       // Socket creation failed; try next address

    // Bind to the local address and set socket to listen
    if ((bind(servSock, addr->ai_addr, addr->ai_addrlen) == 0) &&
        (listen(servSock, MAXPENDING) == 0)) {
      // Print local address of socket
      struct sockaddr_storage localAddr;
      socklen_t addrSize = sizeof(localAddr);
      if (getsockname(servSock, (struct sockaddr *) &localAddr, &addrSize) < 0)
        DieWithSystemMessage("getsockname() failed");
      fputs("Binding to ", stdout);
      PrintSocketAddress((struct sockaddr *) &localAddr, stdout);
      fputc('\n', stdout);
      break;       // Bind and listen successful
    }

    close(servSock);  // Close and try again
    servSock = -1;
  }

  // Free address list allocated by getaddrinfo()
  freeaddrinfo(servAddr);

  return servSock;
}

int AcceptTCPConnection(int servSock) {
  struct sockaddr_storage clntAddr; // Client address
  // Set length of client address structure (in-out parameter)
  socklen_t clntAddrLen = sizeof(clntAddr);

  // Wait for a client to connect
  int clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
  if (clntSock < 0)
    DieWithSystemMessage("accept() failed");

  // clntSock is connected to a client!

  fputs("Handling client ", stdout);
  PrintSocketAddress((struct sockaddr *) &clntAddr, stdout);
  fputc('\n', stdout);

  return clntSock;
}

void HandleTCPClient(int clntSocket) {
//  char buffer[BUFSIZE]; // Buffer for echo string

//  // Receive message from client
//  ssize_t numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);
//  if (numBytesRcvd < 0)
//    DieWithSystemMessage("recv() failed");

//  // Send received string and receive again until end of stream
//  while (numBytesRcvd > 0) { // 0 indicates end of stream
//    // Echo message back to client
//    ssize_t numBytesSent = send(clntSocket, buffer, numBytesRcvd, 0);
//    if (numBytesSent < 0)
//      DieWithSystemMessage("send() failed");
//    else if (numBytesSent != numBytesRcvd)
//      DieWithUserMessage("send()", "sent unexpected number of bytes");

//    // See if there is more data to receive
//    numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);
//    if (numBytesRcvd < 0)
//      DieWithSystemMessage("recv() failed");
//  }

  char buffer[BUFSIZE];
  ssize_t totalBytes=0;
  char times[1000]="time";
  char dates[1000]="date";
  char result[1000]="";
  char tosend[1000];
  while(totalBytes<4){
      ssize_t numBytesRcvd=recv(clntSocket,buffer,BUFSIZE,0);
      if (numBytesRcvd < 0) {
        if (errno == EWOULDBLOCK) { // a timeout occured
              printf("closing socket\n");
              close(clntSocket);
              return;
            }
        else {
          DieWithSystemMessage("recv() failed");
            }
      }
      totalBytes+=numBytesRcvd;
      //printf("the current number length is:%d\n",numBytesRcvd);
      //printf("the current string is:%s\n",buffer);
      buffer[numBytesRcvd]='\0';
      strcat(result,buffer);
  }
  //printf("the final result is:%s\n",result);
  time_t t;
  struct tm *curr;
  t=time(NULL);
  curr=localtime(&t);
  if(strncmp(result,times,4)==0){
      strftime(tosend,sizeof(tosend),"%X",curr);
  }
  else if(strncmp(result,dates,4)==0){
      strftime(tosend,sizeof(tosend),"%Y-%m-%d",curr);
  }
  ssize_t numBytesSent=send(clntSocket,tosend,sizeof(tosend),0);
  //else{ssize_t numBytesSent=send(clntSocket,result,sizeof(result),0);}

  close(clntSocket); // Close client socket
}
