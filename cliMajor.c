//	CSCE 3600.001 - Major Assignment
//	group:	Alfredo J De Leon
//		Caleb Ebers
//		Aaron Johnson


/*
Socket Client Example (https://www.tutorialspoint.com/unix_sockets/socket_client_example.htm)
*/

#include <stdio.h> //printf
#include <stdlib.h>
#include <string.h> //strlen
#include <sys/socket.h> // socket
#include <arpa/inet.h> //inet_addr

#define BUF_SIZE 2147483

void * receiveMessage(void * socket);

int main(int argc, char const *argv[]) {
  int sock, ret;
  struct sockaddr_in server;
  char message[1000], server_reply[2000];
  char command[50]; // Either 'message' or 'put'
  char username[100];
  char temp[1000];
  pthread_t rThread;

	if(argc != 3){
		printf("Usage: ./client [PORT] [IP ADDRESS]\n");
		printf("Example: ./client 5001 129.120.151.95\n");
		return 0;
	}

  printf("Enter a username: ");
  scanf("%s", username);

  // Create socekt
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock == 1){
    printf("Could not create socket\n");
  }
  puts("Socket created");

  server.sin_addr.s_addr = inet_addr(argv[2]);
  server.sin_family = AF_INET;
  server.sin_port = htons(atoi(argv[1]));

  //Connect to remote server
  if(connect(sock, (struct sockaddr *)&server, sizeof(server))<0){
    perror("connect failed. Error");
    return 1;
  }

  puts("Connected\n");

  //creating a new thread for receiving messages from the server
  ret = pthread_create(&rThread, NULL, receiveMessage, (void *) sock);
  if (ret) {
   printf("ERROR: Return Code from pthread_create() is %d\n", ret);
   return(1);
  }

  printf("Enter command \n");
	printf("Usage: \n\tmessage [message content] \n\tput [filename]\n\t\"exit 1\" to disconnect \n");

  //Keep communcation with server
  for(;;){
    scanf("%s ", command);
    fgets(message, 1000, stdin);

    // Command message: Send message to server for other clients
    if(!strcmp(command,"message")){
      strcpy(temp, username);
      strcat(temp, ": ");
      strcat(temp, message);
      strcpy(message, temp);
      memset(temp, 0, 1000);
      //Send some data
      if(send(sock, message, strlen(message), 0) < 0){
        puts("Send failed");
        return 1;
      }
    }
    // Command put: Send file to server for other clients
    else if(!strcmp(command,"put")){
      printf("Command:%s\n------\n", command);
      message[strlen(message) - 1] = '\0'; // Remove endline after file name
      printf("Filename:\"%s\" \n", message);

      // Variables
      FILE * pFile;         //File pointer
      long lSize;           //Size of file
      char * file_content, *file_temp;  //Contents of file
      size_t result;        //Check for reading error

      // Open file to read
      printf("Opening:%s\n", message);
      pFile = fopen ( message , "rb" );
      if (pFile==NULL) {
        printf("Error: file does not exist!\n");
        return (1);
      }

      // obtain file size:
      fseek (pFile , 0 , SEEK_END);
      lSize = ftell (pFile);
      rewind (pFile);

      // allocate memory to contain the whole file:
      file_content = (char*) malloc (2*sizeof(char)*lSize);
      file_temp = (char*) malloc (2*sizeof(char)*lSize);
      if (file_content == NULL) {fputs ("Memory error\n",stderr); return (2);}

      // copy the file into the buffer:
      result = fread (file_content,1,lSize,pFile);
      if (result != lSize) {fputs ("Reading error\n",stderr); return (3);}

      // Add file indicator and file name before file contents
      strcpy(file_temp, "~");
      strcat(file_temp, message);
      strcat(file_temp, " ");
      strcat(file_temp, file_content);
      strcpy(file_content, file_temp);
      /* the whole file is now loaded in the memory buffer. */
      if(send(sock, file_content, strlen(file_content), 0) < 0){
        puts("Send failed");
        return 1;
      }
    }
    // Command exit: Leave the chat, broadcast that the client has left
    else if(!strcmp(command,"exit")){
      strcpy(temp, username);
      strcat(temp, " ");
      strcat(temp, "Has Left The Chat\n");
      strcpy(message, temp);
      memset(temp, 0, 1000);
      //Send some data
      if(send(sock, message, strlen(message), 0) < 0){
        puts("Send failed");
        return 1;
      }
      break;
    }
    // Unknown command: Error
    else{
            printf("Error: Invalid command!\n");
            printf("Usage: \n\tmessage [message content] \n\tput [filename]\n\t \"exit 1\" to disconnect\n");
    }
  }

  close(sock);
  return 0;
}

void * receiveMessage(void * socket) {
  int sockfd, ret;
  char buffer[BUF_SIZE];
  char file_name[100];
  sockfd = (int) socket;
  memset(buffer, 0, BUF_SIZE);
  for (;;) {
    ret = recvfrom(sockfd, buffer, BUF_SIZE, 0, NULL, NULL);
    if (ret < 0) {
      printf("Error receiving data!\n");
    }
    else {
      /* FILE HANDLER: Get file name and save contents */
      if(buffer[0] == '~'){
        // Get file name
        int i = 1;
        while(buffer[i] != ' '){
          file_name[i-1] = buffer[i];
          i++;
        }
        file_name[i+1] = '\0';
        printf("File recieved: \"%s\"\n", file_name);
        FILE * saveFile;         //File pointer
        printf("Saving:%s\n", file_name);
        saveFile = fopen ( file_name , "w" );
        if (saveFile==NULL) {
          fputs ("File error\n",stderr);
          exit(1);
        }
        fprintf(saveFile,"%s", buffer);
        fclose (saveFile);
        printf("Filename \"%s\" saved. \n", file_name);
      }
      /* MESSAGE HANDLER: Just print message */
      else{
        printf("%s\n", buffer);
      }
      memset(buffer, 0, BUF_SIZE);
      memset(file_name, 0, 100);
    }
  }
}
