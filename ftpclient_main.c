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
  int control_sock; //??����socket����
  
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
	 * �ṹhostent��¼������Ϣ����������������������ַ���͡���ַ���Ⱥ͵�ַ�б�
	 * struct hostent{
   *   char * h_name; //��ַ����ʽ����
   *   char ** h_aliases; //��ַԤ�����Ƶ�ָ��
   *   short h_addrtype; //��ַ����
   *   short h_length; //��ַ����
   *   char ** h_addr_list; //���������ַָ��
   *   #define h_addr h_addr_list[0]; //h_addr_list�еĵ�һ����ַ
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
    
	/* connect���ڽ�����socket����������servaddrָ���������ַ
	 */
  if(connect(control_sock,(struct sockaddr*)&servaddr,sizeof(struct sockaddr)) == -1)
  {
    return -1;
  }

  return control_sock;
}

