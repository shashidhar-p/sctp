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

int main() {
  int connSock, ret, in, flags, i;
  struct sockaddr_in servaddr, cliaddr;
  struct sctp_sndrcvinfo sri;
  // struct sctp_initmsg initmsg;
  struct sctp_event_subscribe events;
  socklen_t len;
  int rd_sz;

  //   char buffer[MAX_BUFFER + 1];

  connSock = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
  if (connSock == -1) {
    printf("Failed to create socket\n");
    perror("socket()");
    exit(1);
  }

  bzero((void *)&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(MY_PORT_NUM);

  ret = bind(connSock, (struct sockaddr *)&servaddr, sizeof(servaddr));

  if (ret == -1) {
    printf("Bind failed \n");
    perror("bind()");
    close(connSock);
    exit(1);
  }

  bzero(&events, sizeof(events));
  events.sctp_data_io_event = 1;
  ret =
      setsockopt(connSock, IPPROTO_SCTP, SCTP_EVENTS, &events, sizeof(events));

  if (ret == -1) {
    printf("setsockopt() failed \n");
    perror("setsockopt()");
    close(connSock);
    exit(1);
  }

  ret = listen(connSock, 5);
  if (ret == -1) {
    printf("listen() failed \n");
    perror("listen()");
    close(connSock);
    exit(1);
  }
  char receivebuffer[MAX_BUFFER + 1];
  // Clear the buffer
  bzero(receivebuffer, MAX_BUFFER + 1);
  int count = 0;
  while (1) {
    len = sizeof(struct sockaddr_in);
    rd_sz = sctp_recvmsg(connSock, receivebuffer, sizeof(receivebuffer),
                         (struct sockaddr *)&cliaddr, &len, &sri, &flags);

    if (rd_sz == -1) {
      printf("Error in sctp_recvmsg\n");
      perror("sctp_recvmsg()");
      close(connSock);
    } else {
      printf("From str:%d seq:%d (assoc:0x%x):", sri.sinfo_stream,
             sri.sinfo_ssn, (u_int)sri.sinfo_assoc_id);
      printf("%.*s", rd_sz, receivebuffer);
      printf("\n");
    }

    char sendbuffer[MAX_BUFFER + 1] = "HI !! This is Server";
    int sendbufferlen = 0;
    sendbuffer[strcspn(sendbuffer, "\r\n")] = 0;
    sendbufferlen = strlen(sendbuffer);

    sctp_sendmsg(connSock, sendbuffer, sendbufferlen,
                 (struct sockaddr *)&cliaddr, len, sri.sinfo_ppid,
                 sri.sinfo_flags, sri.sinfo_stream, 0, 0);
    sleep(1);
    count++;
  }
  close(connSock);
  return 0;
}