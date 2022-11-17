// SCTPServer.C To compile - gcc sctp_server.c - o server - lsctp
#include <errno.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#define MAX_BUFFER 1024
#define MY_PORT_NUM 62324
/* This can be changed to suit the need and should be same in server \
         and client */

void sendToClient(int connSock, struct sockaddr_in *cliaddr) {
  char sendbuffer[MAX_BUFFER + 1] = "HI !! This is Server";
  int sendbufferlen = 0;
  sendbuffer[strcspn(sendbuffer, "\r\n")] = 0;
  sendbufferlen = strlen(sendbuffer);

  int ret = sctp_sendmsg(connSock, (void *)sendbuffer, (size_t)sendbufferlen,
                         (struct sockaddr *)cliaddr, 0, 0, 0, 0, 0, 0);

  if (ret == -1) {
    printf("Error in sctp_sendmsg\n");
    perror("sctp_sendmsg()");
  } else
    printf("Successfully sent %d bytes data to client\n", ret);
}

void receiveFromClient(int connSock, struct sockaddr_in *cliaddr) {
  int flags;
  struct sctp_sndrcvinfo sndrcvinfo;
  char receivebuffer[MAX_BUFFER + 1];
  // Clear the buffer
  bzero(receivebuffer, MAX_BUFFER + 1);

  int in = sctp_recvmsg(connSock, receivebuffer, sizeof(receivebuffer),
                        (struct sockaddr *)cliaddr, 0, &sndrcvinfo, &flags);

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

int main() {
  int listenSock, connSock, ret, in, flags, i;
  struct sockaddr_in servaddr, cliaddr;
  struct sctp_initmsg initmsg;
  struct sctp_event_subscribe events;

  //   char buffer[MAX_BUFFER + 1];

  listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
  if (listenSock == -1) {
    printf("Failed to create socket\n");
    perror("socket()");
    exit(1);
  }

  bzero((void *)&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(MY_PORT_NUM);

  ret = bind(listenSock, (struct sockaddr *)&servaddr, sizeof(servaddr));

  if (ret == -1) {
    printf("Bind failed \n");
    perror("bind()");
    close(listenSock);
    exit(1);
  }

  /* Specify that a maximum of 5 streams will be available per socket */
  memset(&initmsg, 0, sizeof(initmsg));
  initmsg.sinit_num_ostreams = 5;
  initmsg.sinit_max_instreams = 5;
  initmsg.sinit_max_attempts = 4;
  ret = setsockopt(listenSock, IPPROTO_SCTP, SCTP_INITMSG, &initmsg,
                   sizeof(initmsg));

  if (ret == -1) {
    printf("setsockopt() failed \n");
    perror("setsockopt()");
    close(listenSock);
    exit(1);
  }

  ret = listen(listenSock, 5);
  if (ret == -1) {
    printf("listen() failed \n");
    perror("listen()");
    close(listenSock);
    exit(1);
  }

  printf("Awaiting a new connection\n");

  connSock = accept(listenSock, (struct sockaddr *)NULL, (int *)NULL);
  if (connSock == -1) {
    printf("accept() failed %s\n", strerror(errno));
    perror("accept()");
    // close(connSock);
    exit(0);
  } else {
    printf("New client connected....= %d\n", connSock);
    // system ("cat /proc/net/sctp/assocs");
  }

  while (1) {
    receiveFromClient(connSock, &cliaddr);
    sleep(1);
    sendToClient(connSock, &cliaddr);
  }

  close(connSock);

  return 0;
}