#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>

#include "ftp_client.h"

/*--------------------------------------------------------------------------------*
 * Description:                                                                   *
 *   List all file under the current directory.                                   *
 *--------------------------------------------------------------------------------*/
void ftp_list(int sockfd)
{
  int nread;

  for(;;)
  {
    if((nread = recv(sockfd, rbuf_from_stdin, MAXBUF,0)) < 0)
    {
        printf("[ftp] recv error.\n");
    }
    else if(nread == 0) /* Data recv over. */
    {
        printf("[ftp] over.\n");
        break;
    }

    /*---------------------------------------
     * List the file content on the screen.
     ----------------------------------------*/
    if(write(STDOUT_FILENO, rbuf_from_stdin, nread) != nread)
    {
        printf("[ftp] send error to stdout.\n");
    }
  }
  
  if(close(sockfd) < 0)
  {
      printf("[ftp] close error\n");
      exit(1);
  }
  return;
}

/*--------------------------------------------------------------------------------*
 * Description:                                                                   *
 *   Upload file from client to server.                                           *
 *--------------------------------------------------------------------------------*/
int ftp_put_file(int sck, char *pUploadFileName_s)
{
  int  nread;
  char send_buf[MAXBUF*4];
	 
  int handle = open(pUploadFileName_s, O_RDWR);
  if(handle < 0)
  {
  	ERR_EXIT("Failed to open the upload file!");
  }
    
  /* ------------------------------------------------
   * For loop until all data has been read. */
  for(;;)
  {
    nread = read(handle, send_buf, MAXBUF);
    printf("+++++++++++++++++nread : %d\n", nread);
    if(nread < 0)
    {
      ERR_EXIT("Read error!");
    }
    else if(nread == 0)
    {
  	  printf("[ftp] All data has been read.\n");
      break;
    }
      
    /* Send data. */
    if(write(sck, send_buf, nread) < 0)
    {
      ERR_EXIT("Failed to upload file to server!");
    }
  }

  if(close(sck) < 0)
  {
    ERR_EXIT("close error\n");
  }
    
  return 0;
}

/*---------------------------------------------------------------------------------------*
 * Description:                                                                          *
 *   Download file from server to client.                                                *
 *---------------------------------------------------------------------------------------*/
int ftp_get_file(int sck, char *pDownloadFileName)
{
	int  nread, ntotal = 0;
  char recv_buf[MAXBUF*4]; /* Support 4M file transmit. */
  
  int handle = open(pDownloadFileName, O_WRONLY | O_CREAT | O_TRUNC, S_IREAD| S_IWRITE);
  if(handle < 0)
  {
  	 ERR_EXIT("[ftp] Open file error when get file."); 
  }

  for(;;)
  {
    nread = recv(sck, recv_buf, MAXBUF, 0);
    printf("+++++++++++++++nread : %d\n", nread);
    if(nread < 0)
    {
       ERR_EXIT("Failed to recv data from server!");
    }
    else if(nread == 0)
    {
       printf("[ftp] recv over.\n");
       break;
    }
 
    if(write(handle, recv_buf, nread) < 0)
    {
        ERR_EXIT("Failed to write data!");
    }

    /*-------------------------------------------------
     * Beyond the limit bytes, end up data transmit.
     */
    ntotal += nread;
    if (ntotal > MAXBUF*4)
      break;
  }

  if(close(sck) < 0)
  {
    ERR_EXIT("close error\n");
  }

   return 0;
}
