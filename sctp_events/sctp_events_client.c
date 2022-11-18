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

int main(int argc, char *argv[]) {
  int connSock, i, ret, flags, rd_sz;
  struct sockaddr_in servaddr, peeraddr;
  // struct sctp_status status;
  struct sctp_event_subscribe events;
  struct sctp_sndrcvinfo sri;
  socklen_t len;

  // Establish connection
  connSock = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
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

  bzero(&events, sizeof(events));
  events.sctp_data_io_event = 1;
  setsockopt(connSock, IPPROTO_SCTP, SCTP_EVENTS, &events, sizeof(events));

  //----------------------------------------------
  bzero(&sri, sizeof(sri));
  int count = 0;
  while (count <= 5) {
    sri.sinfo_stream = 0;
    char sendbuffer[MAX_BUFFER + 1] = "HI !! This is client";
    int sendbufferlen = 0;
    sendbuffer[strcspn(sendbuffer, "\r\n")] = 0;
    sendbufferlen = strlen(sendbuffer);

    sctp_sendmsg(connSock, sendbuffer, sendbufferlen,
                 (struct sockaddr *)&servaddr, sizeof(servaddr), 0, 0,
                 sri.sinfo_stream, 0, 0);

    char receivebuffer[MAX_BUFFER + 1];
    bzero(receivebuffer, MAX_BUFFER + 1);
    sleep(1);
    len = sizeof(peeraddr);
    rd_sz = sctp_recvmsg(connSock, receivebuffer, sizeof(receivebuffer),
                         (struct sockaddr *)&peeraddr, &len, &sri, &flags);
    if (rd_sz == -1) {
      printf("Error in sctp_recvmsg\n");
      perror("sctp_recvmsg()");
      close(connSock);
      //   continue;
    } else {
      printf("From str:%d seq:%d (assoc:0x%x):", sri.sinfo_stream,
             sri.sinfo_ssn, (u_int)sri.sinfo_assoc_id);
      printf("%.*s", rd_sz, receivebuffer);
      printf("\n");
      count++;
    }
  }
  close(connSock);
  // close (connSock);
  // while (1);
  return 0;
}