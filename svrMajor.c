// CSCE 3600.001 - Major Assignment
// group:	Alfredo J Deleon
//		Caleb Ebers
//		Aaron Johnson


#include <stdio.h>
#include <string.h> //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet addr
#include <unistd.h> //write
#include <pthread.h>
#include <stdlib.h>

#define BUF_SIZE 10000

int num_of_connections=0;
int connection_arr[4];

void *connection_handler(void *);

int main(int argc, char *argv[]){
  int socket_desc, client_sock, c;
  int *new_sock;
  struct sockaddr_in server, client;

  if(argc != 2){
    printf("Usage: ./server [PORT]\n");
    printf("Example: ./server 5001\n");
    return 0;
  }

  //Create socket
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);
  if(socket_desc == -1){
    printf("Could not create socket\n");
  }
  puts("Socket created");

  // Prepare the sockaddr in structure
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(atoi(argv[1])); // Get port number from argument

  /* Now bind the host address using bind() call.*/
  if (bind(socket_desc, (struct sockaddr *) &server, sizeof(server)) < 0) {
     perror("ERROR on binding");
     return(1);
  }
  puts("bind done");

  //listen
  listen(socket_desc, 3);

  //Accept and incoming connections
  puts("Waiting for incoming connections...");
  c = sizeof(struct sockaddr_in);

  // accept connection from an incoming clients
  while(client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)){
    puts("Connection accepted");
    pthread_t sniff;
    new_sock=malloc(1);
    *new_sock=client_sock;
    if(pthread_create(&sniff, NULL, connection_handler, (void *) new_sock) < 0){
      perror("could not create thread");
      return(1);
    }
    puts("Handler assigned");
  }

  if (client_sock < 0) {
     perror("accept failed");
     return(1);
  }

  return 0;
}

void *connection_handler(void *socket_desc){
  int sock=*(int *)socket_desc;
  int read_size;
  char client_message[BUF_SIZE], file_name[100];
  int i = 0;
  //printf("From %s %d  SocketID= %d\n", inet_ntoa(client.sin_addr), client.sin_port, client_sock);
  connection_arr[num_of_connections] = sock;
  num_of_connections++;
  printf("Number of connections: %d\n", num_of_connections);

  //Recieve a message from client
  while( (read_size = recv(sock, client_message, BUF_SIZE,0)) > 0){
    /* FILE HANDLER: Get file name, then send to contents to other clients */
    if(client_message[0] == '~'){
      // Get file name
      int i = 1;
      while(client_message[i] != ' '){
        file_name[i-1] = client_message[i];
        i++;
      }
      file_name[i+1] = '\0';
      printf("Sending: \"%s\"\n", file_name);

      // Send file contents to other clients
      for(i = 0; i < num_of_connections; i++){
        write(connection_arr[i], client_message, strlen(client_message));
      }
    }
    /* MESSAGE HANDLER: Send the message back to the client */
    else{
      for(i = 0; i < num_of_connections; i++){
        write(connection_arr[i], client_message, strlen(client_message));
      }
    }

    // Clear buffers
    memset(file_name, 0, 100);
    memset(client_message, 0, BUF_SIZE);
  }

  if(read_size == 0){
    puts("Client disconnected");
    // Remove the socket that disconnected
    for( i=0; i < num_of_connections; i++){
      if(connection_arr[i] == sock)
              connection_arr[i] = -1;
    }

    // Put -1 at end of array to show it's not a socket
    int c, d, swap;
    for (c = 0 ; c < num_of_connections - 1; c++)
    {
      for (d = 0 ; d < num_of_connections - c - 1; d++)
      {
        if (connection_arr[d] < connection_arr[d+1]) /* For decreasing order use < */
        {
              swap     = connection_arr[d];
              connection_arr[d]   = connection_arr[d+1];
              connection_arr[d+1] = swap;
        }
      }
    }

    // Decriment # of connections
    num_of_connections--;

    fflush(stdout);
  }
  else if(read_size == -1){
    perror("recv failed");
  }
}
