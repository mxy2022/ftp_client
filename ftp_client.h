#ifndef _FTPCLIENT_H_
#define _FTPCLINET_H_

#include <stdio.h>
#include <stdlib.h>

#define MAXBUF       1024
#define CONTROL_PORT 21

#define PRINT(s) printf("[%s:(%d)], %s\n", __func__, __LINE__, s)

#define ERR_EXIT(m) \
  do \
  { \
    perror(m); \
	exit(EXIT_FAILURE); \
  } \
  while (0)


/* Global variable */
char rbuf_from_server[MAXBUF];
char rbuf_from_stdin[MAXBUF];

typedef enum ftp_current_state {
	USERNAME,
	PASSWORD,
	LOGIN,
	PATHNAME,
	CLOSEDATA,
	ACTIONOK
}ftp_current_state_s_type;

void ftp_request(int sockd, const char *buf);


int client_open(char *host_name, int port);
void ftp_client_select(int sockfd);

void ftp_list(int sockfd);
int ftp_put_file(int sck, char *pUploadFileName_s);
int ftp_get_file(int sck, char *pDownloadFileName_s);

void ftp_requset_cmd(int sockfd, char *buf);
void ftp_reply_code(int sockfd, char *buf);

#endif
