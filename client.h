#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#ifndef client_h
#define client_h

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

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

void prompt();

#endif