#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

#include "DirectoryParser_MT.h"
#include "QueryProtocol.h"
#include "MovieSet.h"
#include "MovieIndex.h"
#include "MovieTitleIndex.h"
#include "DocIdMap.h"
#include "Hashtable.h"
#include "QueryProcessor.h"
#include "FileParser.h"
#include "FileCrawler.h"
#include "DirectoryParser.h"
#include "Util.h"

DocIdMap docs;
MovieTitleIndex docIndex;

#define BUFFER_SIZE 1000

#define SEARCH_RESULT_LENGTH 1500
char movieSearchResult[SEARCH_RESULT_LENGTH];

int Cleanup();

void sigint_handler(int sig) {
  write(0, "Exit signal sent. Cleaning up...\n", 34);
    Cleanup();
  exit(0);
}

int HandleClient(int sock_fd) {
  printf("Handling Client"); // Delete later
  struct sockaddr_storage client_addr;
  int new_fd;
  socklen_t sin_size;
  char buffer[BUFFER_SIZE];
  sin_size = sizeof client_addr;
  while (1) {
  // Step 5: Accept connection
	new_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &sin_size);
	if (new_fd == -1) {
		perror("accept");
		exit(1);
	}
  // Step 6: Read, then write if you want
	int len = read(new_fd, buffer, BUFFER_SIZE-1);
	buffer[len] = '\0';
	// Send ACK
	SendAck(new_fd);
	// If query is GOODBYE, quit connection
	if (strcmp(buffer, "GOODBYE") == 0) {
		SendGoodbye(new_fd);
		close(new_fd);
		return -1;
	}
	// Run Query and get response
	SearchResultIter result = FindMovies(docIndex, buffer);
	if (result == NULL) {
		send(new_fd, "0", strlen("0"), 0);
		SendGoodbye(new_fd);
		close(new_fd);
	} else {
		// Send number of responses
		printf("%i entries are found. \n", result->num_results);
		sprintf(buffer, "%i", result ->num_results);
		send(new_fd, buffer, strlen(buffer), 0);
		// Wait for ack
		int templen;
		templen = recv(new_fd, buffer, BUFFER_SIZE-1, 0);
		if (templen == -1) {
			perror("receive ack");
			exit(1);
		} else {
			buffer[templen] = '\0';
			CheckAck(buffer);
		}
		SearchResult searchResults = (SearchResult)malloc(sizeof(*searchResults));
		if (searchResults == NULL) {
			perror("allocate search results");
			exit(1);
		}

		SearchResultGet(result, searchResults);
		CopyRowFromFile(searchResults, docs, movieSearchResult);
		send(new_fd, movieSearchResult, strlen(movieSearchResult), 0);
		templen = recv(new_fd, buffer, BUFFER_SIZE-1, 0);
		if (templen == -1) {
                        perror("receive ack");
                        exit(1);
                } else {
                        buffer[templen] = '\0';
                        CheckAck(buffer);
		}
		// For each response
		int nextResult;
		while (SearchResultIterHasMore(result) != 0) {
			nextResult = SearchResultNext(result);
			SearchResultGet(result, searchResults);
			// Send response
			send(new_fd, movieSearchResult, strlen(movieSearchResult), 0);
			// Wait for ACK
			templen = recv(new_fd, buffer, BUFFER_SIZE-1,0);
			if (templen  == -1) {
                        	perror("receive ack");
                        	exit(1);
                	} else {
                        	buffer[templen] = '\0';
                        	CheckAck(buffer);
			}
		}
	}
	// Cleanup	
	DestroySearchResultIter(result);
	// Send GOODBYE
	SendGoodbye(new_fd);
	// Close connection
	close(new_fd);
  }

  return 0;
}

int Setup(char *dir) {
  printf("Crawling directory tree starting at: %s\n", dir);
  // Create a DocIdMap
  docs = CreateDocIdMap();
  CrawlFilesToMap(dir, docs);
  printf("Crawled %d files.\n", NumElemsInHashtable(docs));

  // Create the index
  docIndex = CreateMovieTitleIndex();

  if (NumDocsInMap(docs) < 1) {
    printf("No documents found.\n");
    return 0;
  }

  // Index the files
  printf("Parsing and indexing files...\n");
  ParseTheFiles(docs, docIndex);
  printf("%d entries in the index.\n", NumElemsInHashtable(docIndex->ht));
  return NumElemsInHashtable(docIndex->ht);
}

int Cleanup() {
  DestroyMovieTitleIndex(docIndex);
  DestroyDocIdMap(docs);

  return 0;
}

int main(int argc, char **argv) {
  char *port = NULL;
  char *dir_to_crawl = NULL;

  int debug_flag = 0;
  int c;

  while ((c = getopt (argc, argv, "dp:f:")) != -1) {
    switch (c)
      {
      case 'd':
        debug_flag = 1;
        break;
      case 'p':
        port = optarg;
        break;
      case 'f':
        dir_to_crawl = optarg;
        break;
      }
  }

  if (port == NULL) {
    printf("No port provided; please include with a -p flag.\n");
    exit(0);
  }

  if (dir_to_crawl == NULL) {
    printf("No directory provided; please include with a -f flag.\n");
    exit(0);
  }

  // Setup graceful exit
  struct sigaction kill;

  kill.sa_handler = sigint_handler;
  kill.sa_flags = 0;  // or SA_RESTART
  sigemptyset(&kill.sa_mask);

  if (sigaction(SIGINT, &kill, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  int num_entries = Setup(dir_to_crawl);
  if (num_entries == 0) {
    printf("No entries in index. Quitting. \n");
    exit(0);
  }

  int s;
  // Step 1: Get address stuff
  struct addrinfo hints, *results;
  int yes = 1;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  s = getaddrinfo(NULL, argv[2], &hints, &results);
  if (s != 0) {
	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
	return 1;
  }
  // Step 2: Open socket
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd == -1) {
	perror("socket (server)");
	exit(1);
  }
  if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
	perror("set socket");
	exit(1);
  }

  // Step 3: Bind socket
  if (bind(sock_fd, results->ai_addr, results->ai_addrlen) != 0) {
	  perror("bind()");
	  exit(1);
  }

  // Step 4: Listen on the socket
  if (listen(sock_fd, 1) != 0) {
	  perror("listen()");
	  exit(1);
  }
  printf("Waiting for connections...\n");
  HandleClient(sock_fd);
  // Got Kill signal
  close(sock_fd);

  Cleanup();
  
  return 0;
}
