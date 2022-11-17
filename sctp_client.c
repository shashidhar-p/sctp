// SCTPClient.C
// To compile - gcc sctp_client.c -o client -lsctp
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define MAX_BUFFER 1024
#define MY_PORT_NUM                                                          \
  62324 /* This can be changed to suit the need and should be same in server \
           and client */

void sendToServer(int connSock) {
  char sendbuffer[MAX_BUFFER + 1] = "HI !! This is client";
  int sendbufferlen = 0;
  sendbuffer[strcspn(sendbuffer, "\r\n")] = 0;
  sendbufferlen = strlen(sendbuffer);

  int ret = sctp_sendmsg(connSock, (void *)sendbuffer, (size_t)sendbufferlen,
                         NULL, 0, 0, 0, 0, 0, 0);
  if (ret == -1) {
    printf("Error in sctp_sendmsg\n");
    perror("sctp_sendmsg()");
  } else
    printf("Successfully sent %d bytes data to server\n", ret);
}

void receiveFromServer(int connSock) {
  int flags;
  struct sctp_sndrcvinfo sndrcvinfo;
  char receivebuffer[MAX_BUFFER + 1];
  bzero(receivebuffer, MAX_BUFFER + 1);

  int in = sctp_recvmsg(connSock, receivebuffer, sizeof(receivebuffer),
                        (struct sockaddr *)NULL, 0, &sndrcvinfo, &flags);

  if (in == -1) {
    printf("Error in sctp_recvmsg\n");
    perror("sctp_recvmsg()");
    close(connSock);
    //   continue;
  } else {
    // Add '\0' in case of text data
    receivebuffer[in] = '\0';

    printf(" Length of Data received: %d\n", in);
    printf(" Data : %s\n", (char *)receivebuffer);
  }
}

int main(int argc, char *argv[]) {
  int connSock, i, ret, flags;
  struct sockaddr_in servaddr;
  struct sctp_status status;

  // Establish connection
  connSock = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);

  if (connSock == -1) {
    printf("Socket creation failed\n");
    perror("socket()");
    exit(1);
  }

  bzero((void *)&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(MY_PORT_NUM);
  servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

  ret = connect(connSock, (struct sockaddr *)&servaddr, sizeof(servaddr));

  if (ret == -1) {
    printf("Connection failed\n");
    perror("connect()");
    close(connSock);
    exit(1);
  }

  //----------------------------------------------
  while (1) {
    sendToServer(connSock);
    // sleep(5);
    receiveFromServer(connSock);

    sleep(1);
  }

  close(connSock);
  // close (connSock);
  // while (1);
  return 0;
}