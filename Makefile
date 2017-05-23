#################################################
# Description: The Makefile of ftp client       # 
#                                               #
# Author: Matthew mei                           #
# Date: May. 24th 2017                          #
#################################################

OBJECTS = ftpclient_main.o ftp_file_transmit.o ftp_client_select.o ftp_client_proto.o 

CC := gcc
#CFLAGS += -Wall
CFLAGS += -Werror
CFLAGS += -Wextra
CFLAGS += -Wno-format-zero-length

ftp_client:$(OBJECTS) 
	$(CC) $(CFLAGS) -o $@ $^ 

$^:ftp_client.h

.PHONY:clean
clean:
	-rm ftp_client $(OBJECTS)
