#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef server_h
#define server_h

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512
#define FILENAME "listdata.txt"

enum Request {
  NONE,
  ENQUEUE,
  DEQUEUE,
  SORT,
  PRINT,
  QUIT
};

enum Response {
  SUCCESS,
  FAILURE,
  EMPTY_QUEUE
};

struct Node {
  int data;
  struct Node *next;
};

#endif