#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ctype.h>

#include "ftp_client.h"

extern ftp_current_state_s_type ftpClientState;

void ftp_request(int sockd, const char *buf)
{
  size_t  nleft = strlen(buf);
  printf("----nleft : %d.\n", nleft);
  if (write(sockd, buf, nleft) < 0)
  {
    ERR_EXIT("[FTP] Write error.");
  }
  return;
#if 0
  ssize_t nwritten;
  char    *bufp = (char *)buf;

  while (nleft > 0)
  {
    if ((nwritten = write(sockd, bufp, nleft)) < 0)
    {
      PRINT("[FTP] Write error.");
    }
    else if(nwritten == 0)
      continue;

    bufp += nwritten;
    nleft -= nwritten; 
  }
  return;
#endif
}

/*-----------------------------------------------------------------------*
 * Description:                                                          *
 *   Utilities select() listening the change of fd(stdin or ftp_server). *
 *-----------------------------------------------------------------------*/
void ftp_client_select(int sockfd)
{
  int  maxfdp1;
  char wbuf[MAXBUF] = {0};

  fd_set rset;
  FD_ZERO(&rset);
  maxfdp1 = sockfd + 1;
	
  for(;;)
  {	 
	/*--------------------------------------------- 
	 * FD_SET��һ���ļ������������ļ�����������,
	 * STDIN_FILENOΪ��׼�����豸���ļ�������.
	 *---------------------------------------------*/
    FD_SET(STDIN_FILENO,&rset);
    FD_SET(sockfd,&rset);
		
	/* select���ڼ����Ǵӱ�׼���루���̣���������˷���
	 */
    if (select(maxfdp1, &rset, NULL, NULL, NULL) < 0)
    {
      PRINT("select error.");
    }
		
	/* -----------------------------------------------------------------------------------------------
	 * FD_ISSET:���fdΪfd_set���е�һ��Ԫ�أ��򷵻ط���ֵ����Ҫ���ڵ���select֮������ļ�����������
	 * ���ļ��������Ƿ��б仯.
	 *------------------------------------------------------------------------------------------------*/
	 
	/* 1. Check the changed from stdin */
    if(FD_ISSET(STDIN_FILENO, &rset)) 
    {
      bzero(wbuf, MAXBUF);          
      bzero(rbuf_from_stdin, MAXBUF);
            
      /* Input a command on the keyboard. */
      if((read(STDIN_FILENO, rbuf_from_stdin, MAXBUF)) < 0)
      {
      	PRINT("[ftp] read error from stdin.");
		    exit(EXIT_FAILURE);
      }  

  	  switch(ftpClientState)
  	  {
  	    case USERNAME:
    		  sprintf(wbuf,"USER %s", rbuf_from_stdin);
          ftp_request(sockfd, wbuf);
    		  break;
  		  case PASSWORD:
          sprintf(wbuf,"PASS %s", rbuf_from_stdin);
          ftp_request(sockfd, wbuf);
  		    break;
  		  default:
          ftp_requset_cmd(sockfd, rbuf_from_stdin);
  		    break;
  	  }
    }/* if(FD_ISSET(STDIN_FILENO, &rset)) */
		
		
	/* 2. 'sockfd' descriptor has changed, the response is from server. */
    if (FD_ISSET(sockfd, &rset))
    {	
      int nread;
      bzero(rbuf_from_server, strlen(rbuf_from_server));
      if((nread = recv(sockfd, rbuf_from_server, MAXBUF, 0)) < 0)
  	  {
  	    PRINT("[FTP] recv error.");
  	  }
      else if(nread == 0)
  	  {
  	    PRINT("[FTP] Tcp connected is break.");
  		  break;
  	  }
      ftp_reply_code(sockfd, rbuf_from_server);
    }
  }

	return;
}


