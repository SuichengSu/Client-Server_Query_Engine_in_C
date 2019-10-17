#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "QueryProtocol.h"
// Edited by Suicheng Su
// CS5007 Summer 2019 Final Project
// Reference:
// https://github.com/angrave/SystemProgramming/wiki#8-networking

char *port_string = "1500";
unsigned short int port;
char *ip = "127.0.0.1";

#define BUFFER_SIZE 1000

void RunQuery(char *query) {
  struct addrinfo hints, *result;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET; // IPV4 only addresses
  hints.ai_socktype = SOCK_STREAM; // TCP
  // Find the address
  int s;
  s = getaddrinfo(ip, port_string, &hints, &result);
  if (s != 0) {
	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
	exit(1);
  }
  // Create the socket
  int sock_fd;
  sock_fd = socket(result->ai_family,result->ai_socktype,result->ai_protocol);
  if (sock_fd == -1) {
	perror("socket");
	exit(1);
  }
  // Connect to the server
  if (connect(sock_fd, result->ai_addr, result->ai_addrlen) == -1) {
	perror("connect (runquery)");
	close(sock_fd);
	exit(2);
  }
  freeaddrinfo(result);
  // Do the query-protocol
  char buffer[BUFFER_SIZE];
  int recvTemp = read(sock_fd, buffer, BUFFER_SIZE-1); //  Receive ack
  if (recvTemp == -1) {
	perror("receive ack");
	exit(1);
  }
  buffer[recvTemp] = '\0';
  recvTemp = CheckAck(buffer); //  Check if ack received
  if (recvTemp == -1) {
	perror("check ack received signal");
	exit(1);
  }
  if (strlen(query) > 100) {
	printf("Exceeding query maximum length of 100 characters!");
	exit(1);
  }
  send(sock_fd, query, strlen(query), 0); //  Send query
  recvTemp = recv(sock_fd, buffer, BUFFER_SIZE-1, 0); //  Receive # responses
  if (recvTemp == -1) {
	perror("receive # response");
	exit(1);
  }
  buffer[recvTemp] = '\0';
  int num_response = atoi(buffer);
  printf("Received %i response(s)\n", num_response); // Send client # reponses
  SendAck(sock_fd); //  Send server ACK

  for (int i = 0; i < 9999;/*num_response;*/ i++) {
	recvTemp = recv(sock_fd, buffer, BUFFER_SIZE-1, 0); //  Receive response
	if (recvTemp == -1) {
		perror("receive response");
		exit(1);
	}
	buffer[recvTemp] = '\0';
	if (CheckGoodbye(buffer) == 0) {
		break;
	}
	printf("%i: %s\n", i+1, buffer); // Print out response
	SendAck(sock_fd); // Send ACK to server
  }
  recvTemp = recv(sock_fd, buffer, BUFFER_SIZE-1, 0); // Check for receiveGOODBYE
  if (recvTemp == -1) {
	perror("GOODBYE receive");
	exit(1);
  }
  buffer[recvTemp] = '\0';
  
  // Close the connection
  if (CheckGoodbye(buffer) == -1) {
	perror("Goodbye check");
  }
  close(sock_fd);
}

void RunPrompt() {
  char input[BUFFER_SIZE];

  while (1) {
    printf("Enter a term to search for, or q to quit: ");
    scanf("%s", input);

    printf("input was: %s\n", input);

    if (strlen(input) == 1) {
      if (input[0] == 'q') {
        printf("Thanks for playing! \n");
        return;
      }
    }
    printf("\n\n");
    RunQuery(input);
  }
}

// This function connects to the given IP/port to ensure 
// that it is up and running, before accepting queries from users.
// Returns 0 if can't connect; 1 if can. 

int CheckIpAddress(char *ip, char *port) {
  // Connect to the server
  struct addrinfo hints, *result;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  int s;
  s = getaddrinfo(ip, port, &hints, &result);
  if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(1);
  }
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd == -1) {
	  perror("socket");
	  exit(1);
  }
  if (connect(sock_fd, result->ai_addr, result->ai_addrlen) == -1) {
	perror("connect (checkIP)");
	exit(2);
  }
  // Listen for an ACK
  char buffer[BUFFER_SIZE];
  int recvTemp = recv(sock_fd, buffer, BUFFER_SIZE-1, 0); //  Receive ack
  if (recvTemp == -1) {
        perror("receive ack");
        exit(1);
   }
  buffer[recvTemp] = '\0';
  recvTemp = CheckAck(buffer); //  Check if ack received
  if (recvTemp == -1) {
       perror("check ack received signal");
       exit(1);
  }

  // Send a goodbyei
  if (SendGoodbye(sock_fd) == -1) {
	perror("send goodbye");
	exit(1);
  }
  // Close the connection
  freeaddrinfo(result);
  close(sock_fd);
  return 1;
}

int main(int argc, char **argv) {
  if (argc != 3) {
    printf("Incorrect number of arguments. \n");
    printf("Correct usage: ./queryclient [IP] [port]\n");
  } else {
    ip = argv[1];
    port_string = argv[2];
  }
  if (CheckIpAddress(ip, port_string)) {
	RunPrompt();
  }
  return 0;
}
