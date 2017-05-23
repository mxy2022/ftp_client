/*--------------------------------------------------------------------- *
 * Description:                                                         *
 *  Main function of ftp client application processor.                  *
 *----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "ftp_client.h"

int main(int argc, char *argv[])
{
  int  fd;
	char *host_name;
	  
  if(argc != 2)
  {
    exit(1);
  }
	host_name = argv[1];
    
  fd = client_open(host_name, CONTROL_PORT);
  if(fd < 0)
  {
    PRINT("[ftp] client_open failed.");
    return 0;
  }
    
  ftp_client_select(fd);

  return 0;
}


/*--------------------------------------------------------------------------------*
 * Description:                                                                   *
 *   Creat socket for the command transmit                                        *
 *--------------------------------------------------------------------------------*/
int client_open(char *host_name, int port)
{
  int control_sock; //??§制socket连接
  
  if ((control_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    PRINT("[FTP] Creat socket error.");
    return -1;
  }
  
	struct sockaddr_in servaddr;
  memset(&servaddr,0,sizeof(struct sockaddr_in));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
	/*--------------------------------------------------------------------------
	 * 结构hostent记录主机信息，包括主机名、别名、地址类型、地址长度和地址列表
	 * struct hostent{
   *   char * h_name; //地址的正式名称
   *   char ** h_aliases; //地址预备名称的指针
   *   short h_addrtype; //地址类型
   *   short h_length; //地址长度
   *   char ** h_addr_list; //主机网络地址指针
   *   #define h_addr h_addr_list[0]; //h_addr_list中的第一个地址
   * };
	 *-------------------------------------------------------------------------*/
  struct hostent *ht = NULL;
  /*
   * inet_aton() - convert the string of network address(xx.xx.xx.xx) to 
   * binary digit, and save the result in the sin_addr structure.
   * 
   * success - nonzero value, fail - 0.
   */
  if (inet_aton(host_name, &servaddr.sin_addr) == 0)
  {
    ht = gethostbyname(host_name);
    if (ht == NULL)
    {
      perror("gethostbyname");
      exit(EXIT_FAILURE);
    }
    servaddr.sin_addr = *(struct in_addr*)ht->h_addr;
    //memcpy(&servaddr.sin_addr.s_addr,ht->h_addr,ht->h_length);
  }
    
	/* connect用于将参数socket连接至参数servaddr指定的网络地址
	 */
  if(connect(control_sock,(struct sockaddr*)&servaddr,sizeof(struct sockaddr)) == -1)
  {
    return -1;
  }

  return control_sock;
}

