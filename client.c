#include "client.h"

void prompt() {
  printf("\n");
  printf("1. Add a new node to the end of list,\n");
  printf("2. Remove the first node in the list,\n");
  printf("2. Sort the list,\n");
  printf("4. Print all the lists,\n");
  printf("5. Quit.\n");
  printf("\n");
  printf("Enter your choice: ");
}

int __cdecl main(int argc, char **argv) 
{
  WSADATA wsaData;
  SOCKET ConnectSocket = INVALID_SOCKET;
  struct addrinfo *result = NULL, hints;
  enum Request request;
  enum Response response;
  int data, iResult;

  // Validate the parameters
  if (argc != 2) {
    printf("Usage: %s <server-name>\n", argv[0]);
    return 1;
  }

  // Initialize Winsock
  if ((iResult = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
    printf("WSAStartup failed with error: %d\n", iResult);
    return 1;
  }

  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  // Resolve the server address and port
  if ((iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result)) != 0) {
    printf("getaddrinfo failed with error: %d\n", iResult);
    goto cleanup_wsa;
  }

    // Create a socket for connecting to the server
  ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (ConnectSocket == INVALID_SOCKET) {
    printf("Socket creation failed with error: %d\n", WSAGetLastError());
    goto cleanup_addrinfo;
  }

  // Connect to server
  iResult = connect(ConnectSocket, result->ai_addr, (int)result->ai_addrlen);
  if (iResult == SOCKET_ERROR) {
    printf("Unable to connect to server.\n");
    closesocket(ConnectSocket);
    ConnectSocket = INVALID_SOCKET;
    goto cleanup_addrinfo;
  }

  freeaddrinfo(result);

  if (ConnectSocket == INVALID_SOCKET) {
    printf("Failed to connect to server.\n");
    goto cleanup_wsa;
  }

  // Main client loop
  while (TRUE) {
    prompt();
        
    if (scanf("%d", (int *)&request) != 1) {
      printf("Invalid input. Please enter a number.\n");
      while (getchar() != '\n'); // Clear the input buffer
      continue;
    }

    switch (request) {
    case ENQUEUE:
      printf("Enter data to enqueue: ");
      if (scanf("%d", &data) != 1) {
        printf("Invalid input. Please enter a valid integer.\n");
        while (getchar() != '\n'); // Clear the input buffer
        continue;
      }

      // Send ENQUEUE request and data
      iResult = send(ConnectSocket, (char *)&request, sizeof(request), 0);
      if (iResult == SOCKET_ERROR) {
        printf("Send failed with error: %d\n", WSAGetLastError());
        goto cleanup_socket;
      }

      iResult = send(ConnectSocket, (char *)&data, sizeof(data), 0);
      if (iResult == SOCKET_ERROR) {
        printf("Send failed with error: %d\n", WSAGetLastError());
        goto cleanup_socket;
      }

      // Receive response
      iResult = recv(ConnectSocket, (char *)&response, sizeof(response), 0);
      if (iResult <= 0) {
        printf("Connection closed by server or error occurred.\n");
        goto cleanup_socket;
      }
      printf("Server response: %s\n", response == SUCCESS ? "Success" : "Failure");
      break;

    case DEQUEUE:
      // Send DEQUEUE request
      iResult = send(ConnectSocket, (char *)&request, sizeof(request), 0);
      if (iResult == SOCKET_ERROR) {
        printf("Send failed with error: %d\n", WSAGetLastError());
        goto cleanup_socket;
      }

      // Receive response and data
      iResult = recv(ConnectSocket, (char *)&response, sizeof(response), 0);
      if (iResult <= 0 || response == EMPTY_QUEUE) {
        printf("Queue is empty or connection error occurred.\n");
        continue;
      }
      iResult = recv(ConnectSocket, (char *)&data, sizeof(data), 0);
      if (iResult <= 0) {
        printf("Error receiving dequeued data.\n");
        goto cleanup_socket;
      }
      printf("Dequeued data: %d\n", data);
      break;

    case QUIT:
      // Send QUIT request and exit
      send(ConnectSocket, (char *)&request, sizeof(request), 0);
      goto cleanup_socket;
    default:
      printf("Invalid request. Please try again.\n");
      break;
    }
  }

cleanup_socket:
  closesocket(ConnectSocket);
cleanup_addrinfo:
  freeaddrinfo(result);
cleanup_wsa:
  WSACleanup();
  return 0;
}