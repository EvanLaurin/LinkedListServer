#include "server.h"

void free_list(struct Node **head)
{
  struct Node *cur = *head;
  struct Node *next;
  while (cur != NULL)
  {
    next = cur->next;
    free(cur);
    cur = next;
  }
  *head = NULL;
}

int initialize_list(struct Node **head, const char *filename)
{
  FILE *fptr = fopen(filename, "r");
  if (fptr == NULL)
  {
    printf("The file %s does not exist\n", filename);
    return 1;
  }

  char line[128];
  struct Node *cur = NULL;

  while (fgets(line, sizeof(line), fptr))
  {
    struct Node *node = malloc(sizeof(struct Node));
    if (!node)
    {
      printf("Memory allocation failed.\n");
      fclose(fptr);
      return 1;
    }
    node->data = atoi(line);
    node->next = NULL;

    if (*head == NULL)
    {
      *head = node;
      cur = node;
    }
    else
    {
      cur->next = node;
      cur = cur->next;
    }
  }

  fclose(fptr);
  return 0;
}

int enqueue(struct Node **head, int data) {
  struct Node *cur = *head;
  struct Node *node = malloc(sizeof(struct Node));
  if (node == NULL) {
    printf("Malloc failed\n");
    return 1;
  }
  node->data = data;
  node->next = NULL;
  if (*head == NULL) {
    *head = node;
    return 0;
  }
  while (cur->next != NULL) {
    cur = cur->next; 
  }
  cur->next = node;
  return 0;
}

int dequeue(struct Node **head, int *data) {
  if (*head == NULL) {
    printf("Empty list\n");
    return 1;
  }
  struct Node *node = *head;
  *data = node->data;
  *head = node->next;
  free(node);
  return 0;
}



int __cdecl main(int argc, char **argv)
{
  struct Node *head = NULL;

  // Initialize linked list
  if (initialize_list(&head, FILENAME) != 0)
  {
    return 1;
  }

  WSADATA wsaData;
  SOCKET ListenSocket = INVALID_SOCKET, ClientSocket = INVALID_SOCKET;
  struct addrinfo *result = NULL, hints;
  enum Request request;
  enum Response response;
  int data, iResult;

  // Initialize Winsock
  if ((iResult = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0)
  {
    printf("WSAStartup failed with error: %d\n", iResult);
    goto cleanup;
  }

  ZeroMemory(&hints, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;

  // Resolve server address and port
  if ((iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result)) != 0)
  {
    printf("getaddrinfo failed with error: %d\n", iResult);
    goto cleanup_wsa;
  }

  // Create socket for resolving connections
  ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (ListenSocket == INVALID_SOCKET)
  {
    printf("Socket creation failed with error: %ld\n", WSAGetLastError());
    goto cleanup_addrinfo;
  }

  // Bind socket to port
  if (bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR)
  {
    printf("Bind failed with error: %d\n", WSAGetLastError());
    goto cleanup_socket;
  }

  // Listen for incoming connections
  if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR)
  {
    printf("Listen failed with error: %d\n", WSAGetLastError());
    goto cleanup_socket;
  }
  
  printf("Waiting for client connections...\n");

  // Create new socket for communicating with client
  ClientSocket = accept(ListenSocket, NULL, NULL);
  if (ClientSocket == INVALID_SOCKET)
  {
    printf("Accept failed with error: %d\n", WSAGetLastError());
    goto cleanup_socket;
  }
  printf("Client connected.\n");

  while (TRUE)
  {
    iResult = recv(ClientSocket, (char *)&request, sizeof(request), 0);
    if (iResult <= 0)
    {
        printf("Connection closed or error occurred.\n");
        break;
    }

    switch (request)
    {
    case ENQUEUE:
      iResult = recv(ClientSocket, (char *)&data, sizeof(data), 0);
      if (iResult <= 0 || enqueue(&head, data) != 0) {
        response = FAILURE;
      } else {
        response = SUCCESS;
      }
      send(ClientSocket, (char *)&response, sizeof(response), 0);
      break;

    case DEQUEUE:
      if (dequeue(&head, &data) != 0) {
        response = EMPTY_QUEUE;
      } else {
        response = SUCCESS;
      }
      send(ClientSocket, (char *)&response, sizeof(response), 0);
      if (response == SUCCESS) {
        send(ClientSocket, (char *)&data, sizeof(data), 0);
      }
      break;

    case QUIT:
      printf("Client disconnected.\n");
      goto cleanup_client;

    default:
      printf("Unknown request received.\n");
      response = FAILURE;
      send(ClientSocket, (char *)&response, sizeof(response), 0);
      break;
    }
  }

cleanup_client:
  closesocket(ClientSocket);
cleanup_socket:
  closesocket(ListenSocket);
cleanup_addrinfo:
  freeaddrinfo(result);
cleanup_wsa:
  WSACleanup();
cleanup:
  free_list(&head);
  return 0;
}