/*-----------------------------------------------------------------------*
 * Introduction:                                                         *
 *                                                                       *
 *   Realize the transmit protocol of ftp client.                        *
 *   It contains:                                                        *
 *   --> 1. request command to ftp server;                               * 
 *   --> 2. handle the reply code from ftp server.                       *
 *                                                                       *
 * ----------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#include "ftp_client.h"

#define SIZE(a) sizeof(a)/sizeof(a[0])

int cmd_type = -1;

ftp_current_state_s_type ftpClientState;
char filename[100];
/*--------------------------------------------------------------*/

static void do_pwd(int sockfd, char *buf);
static void do_cwd(int sockfd, char *buf);
static void do_ls(int sockfd, char *buf);
static void do_get(int sockfd, char *buf);
static void do_put(int sockfd, char *buf);
static void do_quit(int sockfd, char *buf);


typedef struct ftpreqcmd
{
  const char *reqcmd;
  void (*req_cmd_handler)(int sockfd, char *buf);
} ftpreqcmd_t;

static ftpreqcmd_t req_cmds[] = { 
  { "PWD",	    do_pwd  },
  { "CD",	      do_cwd  },
  { "LS",	      do_ls   },
  { "GET",	    do_get  },
  { "PUT",	    do_put  },
  { "QUIT",	    do_quit },
};

static void do_pwd(int sockfd, char *buf)
{
  ftp_request(sockfd, "PWD\n");
  return;
}

static void do_cwd(int sockfd, char *buf)
{
  char str[MAXBUF] = "cwd";
  char *sub_buf = strstr(buf, " ");
  
  strcat(str, sub_buf);
  ftp_request(sockfd, str);
  return;
}

static void do_ls(int sockfd, char *buf)
{
  ftp_request(sockfd, "PASV\n");
  cmd_type = 0;
  return;
}

static void do_get(int sockfd, char *buf)
{
  ftp_request(sockfd, "TYPE I\n");
  sscanf(buf, "get %s", filename);
  cmd_type = 1;
  return;
}

static void do_put(int sockfd, char *buf)
{
  ftp_request(sockfd, "TYPE I\n");
  sscanf(buf, "put %s", filename);
  cmd_type = 2;
  return;
}

static void do_quit(int sockfd, char *buf)
{
	write(sockfd, "QUIT\n", 5);
  if(close(sockfd) <0)
	{
	  printf("close error\n");
	}
  exit(0);
}

/*-------------------------------------------------------------*/

static void input_username(int sockfd, char *buf);
static void input_passwd(int sockfd, char *buf);
static void login_success(int sockfd, char *buf);
static void show_ftp_symbol(int sockfd, char *buf);
static void type_I_rsp_handler(int sockfd, char *buf);
static void pasv_rsp_handler(int sockfd, char *buf);

typedef struct ftp_server_reply_code
{
  const char *replycode;
  void (*reply_code_handler)(int sockfd, char *buf);
} ftpreplycode_t;

static ftpreplycode_t reply_codes[] = {
  { "220", input_username      },
  { "331", input_passwd        },
  { "230", login_success       },
  { "257", show_ftp_symbol     }, /* pwd */
  { "250", show_ftp_symbol     }, /* cd/cwd */
  { "226", show_ftp_symbol     }, /* ls */
  
  { "200", type_I_rsp_handler  }, /* send command 'TYPE I' */
  { "227", pasv_rsp_handler    }, /* send 'PASV\n' */
};

/*------------------------------------------------------------
 * Description:
 *   Input a username by stdin.
 *------------------------------------------------------------*/
static void input_username(int sockfd, char *buf)
{
  ftpClientState = USERNAME;
  if (write(STDOUT_FILENO, "USERNAME:", strlen("USERNAME:")) < 0)
  {
    PRINT("[FTP] Write error.");
	  exit(EXIT_FAILURE);
  }
  return;
}

/*------------------------------------------------------------
 * Description:
 *   Input a password by stdin.
 *------------------------------------------------------------*/
static void input_passwd(int sockfd, char *buf)
{
  ftpClientState = PASSWORD;
  if (write(STDOUT_FILENO, "PASSWD:", strlen("PASSWD:")) < 0)
  {
    PRINT("[FTP] Write error.");
	  exit(EXIT_FAILURE);
  }
  return;
}

static void login_success(int sockfd, char *buf)
{
  
  ftpClientState = LOGIN;
  if (write(STDOUT_FILENO, "ftp> ", strlen("ftp> ")) < 0)
  {
    PRINT("[FTP] Write error.");
	  exit(EXIT_FAILURE);
  }
  return;
}

/*------------------------------------------------------------------
 * When server received 'PASV' command, will return 227 ....... 
 *------------------------------------------------------------------*/
static void pasv_rsp_handler(int sockfd, char *buf)
{
  int  new_port, data_sock;
  int  addr[6];
  char host[100];
  char wbuf[100];

  printf("--------------buf : %s\n", buf);
 
  sscanf(buf, "%*[^(](%d,%d,%d,%d,%d,%d)", &addr[0],&addr[1],&addr[2],&addr[3],&addr[4],&addr[5]);
  bzero(host,strlen(host));
  sprintf(host,"%d.%d.%d.%d",addr[0],addr[1],addr[2],addr[3]);
  new_port = addr[4]*256 + addr[5];
  printf("New port <%d> connect to server, host <%s>.\n", new_port, host);
  data_sock = client_open(host, new_port);

  switch(cmd_type)
  {
    case 0: //ls
      write(sockfd, "LIST\n", strlen("LIST\n"));
      ftp_list(data_sock);
      break;
    case 1: //get
      sprintf(wbuf, "RETR %s\n", filename);
      printf("%s\n", wbuf);
      write(sockfd, wbuf, strlen(wbuf));
      ftp_get_file(data_sock, filename);
      break;
    case 2: //put
      sprintf(wbuf, "STOR %s\n", filename);
      printf("%s\n",wbuf);
      write(sockfd, wbuf, strlen(wbuf));
      ftp_put_file(data_sock, filename);
      break;
    default:
      printf("[FTP] Error cmd_type (%d).\n", cmd_type);
      break;
        
  }
}

/* Return code <200 Switching to Binary mode> for command "TYPE I".
 * */
static void type_I_rsp_handler(int sockfd, char *buf)
{
  printf("----------buf : %s\n", buf);
  ftp_request(sockfd, "PASV\n");
  return;
}

static void show_ftp_symbol(int sockfd, char *buf)
{
  printf("[ftp] buf : %s\n", buf);
  strcat(buf, "ftp> ");
  if(write(STDOUT_FILENO, buf, strlen(buf)) < 0)
	{
    printf("write error to stdout\n");
	}
  return;
}

/*------------------------------------------------------------------*/
void ftp_requset_cmd(int sockfd, char *buf)
{
  unsigned int i;
  char tmp_buf[MAXBUF];
  
  for(i = 0; i < strlen(buf); i++)
  {
    if ((buf[i] == ' ') ||
      (buf[i] == '\n'))
    {
      break;
    }
    tmp_buf[i] = toupper(buf[i]);
  }
  tmp_buf[i] = '\0';
  printf("[FTP] current req command : %s\n", tmp_buf);

  for(i = 0; i < SIZE(req_cmds); i++)
  {
    if (strncmp(req_cmds[i].reqcmd, tmp_buf, strlen(tmp_buf)) == 0)
    {
      if (req_cmds[i].req_cmd_handler != NULL)
      {
        req_cmds[i].req_cmd_handler(sockfd, buf);
        break;
      }
    }
  }
  
  if (i == SIZE(req_cmds))
  {
    printf("[ftp] Invalid req command(%s).\n", tmp_buf);
  	char buf[MAXBUF] = {0};
    show_ftp_symbol(sockfd, buf);
  }
  return;
}

void ftp_reply_code(int sockfd, char *buf)
{
  unsigned int i;
  for(i = 0; i < SIZE(reply_codes); i++)
  {
    if (strncmp(rbuf_from_server, reply_codes[i].replycode ,3) == 0)
    {
      if (reply_codes[i].reply_code_handler != NULL)
      {
        reply_codes[i].reply_code_handler(sockfd, rbuf_from_server);
      }
	    else
		  {
		    PRINT("[FTP] No handler function.");
		  }
	    break;
    }
  }
}

