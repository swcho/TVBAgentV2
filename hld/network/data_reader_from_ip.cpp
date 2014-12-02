//*************************************************************************
//	
// Author: HuyLe, huy@teleview.com
//	Copyright (C) 
//	Teleview Corporation
//	
//
//*************************************************************************

#if defined(WIN32)
#pragma comment(lib, "ws2_32.lib")
#include <Windows.h>

#else 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "hldplay.h"
#include "data_reader_from_ip.h"
#include "media_interface.h"

#if defined(WIN32)
int udp_server_open(tlv_socket_t *socket_id, int _port_number)
{
	WSADATA w; 
	struct sockaddr_in server;          /* Information about the  */
	struct hostent *hp;                 /* Information about this */
	char host_name[256];

	/* Open windows connection */
	//if (WSAStartup(0x0101, &w) != 0)
	if (WSAStartup(0x0202, &w) != 0)
	{
		fprintf(stderr, "Could not open Windows connection.\n");
		//exit(0);
	}

	/* Open a datagram socket */

	*socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	if (*socket_id == INVALID_SOCKET)
	{
		fprintf(stderr, "Could not create socket.\n");
		WSACleanup();
		//exit(0);
	}

	memset((void *)&server, '\0', sizeof(struct sockaddr_in));

	/* Set family and port */ 
	server.sin_family = AF_INET;
	server.sin_port = htons(_port_number);

	/* Get host name of this computer */
	gethostname(host_name, sizeof(host_name));
	hp = gethostbyname(host_name);

	/* Check for NULL pointer */
	if (hp == NULL)
	{
		fprintf(stderr, "Could not get host name.\n");
		closesocket(*socket_id);
		WSACleanup();
		//exit(0);
	}

	/* Assign the address */
	server.sin_addr.S_un.S_un_b.s_b1 = hp->h_addr_list[0][0];
	server.sin_addr.S_un.S_un_b.s_b2 = hp->h_addr_list[0][1];
	server.sin_addr.S_un.S_un_b.s_b3 = hp->h_addr_list[0][2];
	server.sin_addr.S_un.S_un_b.s_b4 = hp->h_addr_list[0][3];
	//server.sin_addr.S_un.

	if (bind(*socket_id, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1)
	{
		fprintf(stderr, "Could not bind name to socket.\n");
		closesocket(*socket_id);
		WSACleanup();
		//exit(0);
	}

	return 0;
}

// return value: 
// > 0: number of bytes is received 
// < 0: error
//    -1: "waiting resource" is released when this socket is closed ....

int udp_server_receive(tlv_socket_t sd, char *buff)
{

	int client_length = (int)sizeof(struct sockaddr_in);
	struct sockaddr_in client_addr;
	int bytes_received = recvfrom(sd, buff,  UDP_DATA_SIZE_MAX, 0, (struct sockaddr *)&client_addr, &client_length);

	if (bytes_received <  0)
	{
		int erro = WSAGetLastError();
//		printf("assert error code %d,  === %s, %d \n", erro, __FUNCTION__, __LINE__);
		if (erro == WSAEINTR)
			return -1;
		return -1;
	}
	return bytes_received; 
}


int rtp_server_receive(tlv_socket_t sd, char *buff)
{

	int client_length = (int)sizeof(struct sockaddr_in);
	struct sockaddr_in client_addr;
	int bytes_received = recvfrom(sd, buff,  UDP_DATA_SIZE_MAX, 0, (struct sockaddr *)&client_addr, &client_length);

	if (bytes_received <  0)
	{
		int erro = WSAGetLastError();
//		printf("assert error code %d,  === %s, %d \n", erro, __FUNCTION__, __LINE__);
		if (erro == WSAEINTR)
			return -1;
		return -1;
	}

	// check sequency number 

	memcpy(buff, buff + 12, bytes_received - 12);
	return bytes_received - 12; 
}





int udp_server_close(tlv_socket_t sd)
{
	closesocket(sd);
	WSACleanup();
	return 0;
}

#else 
int udp_server_open(tlv_socket_t *socket_id, int _udp_port)
{
	int sock;
	struct sockaddr_in server_addr;

	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
	{
		printf("assert: can not open socket function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return -1;
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(_udp_port);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(server_addr.sin_zero),8);

	if (bind(sock,(struct sockaddr *)&server_addr,
				sizeof(struct sockaddr)) == -1)
	{
		printf("assert: can not bind socket function: %s, line: %d \n", __FUNCTION__, __LINE__);
		return -1;
	}
	struct timeval tv;

	// timeout is about 300 milisecond 
	tv.tv_sec = 0;  
	tv.tv_usec = 300*1000;  
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
	
	*socket_id = sock;
	return 0;
}

int rtp_server_receive(tlv_socket_t socket_id, char *buff)
{
	struct sockaddr_in client_addr;
	socklen_t addr_len; 
	int bytes_received = recvfrom(socket_id, buff, UDP_DATA_SIZE_MAX, 0, (struct sockaddr *)&client_addr, &addr_len);
	if (bytes_received < 0) // time-out 
	{
		return -1;
	}

//	RTP_HEADER rtp_header;
//	rtp_header.m1_tp7 
	

		// check sequency number 

	memcpy(buff, buff + 12, bytes_received - 12);
	return bytes_received - 12; 
}

// return number of received bytes 
int udp_server_receive(tlv_socket_t socket_id, char *buff)
{
	struct sockaddr_in client_addr;
	socklen_t addr_len; 
	int num_byte = recvfrom(socket_id, buff, UDP_DATA_SIZE_MAX, 0, (struct sockaddr *)&client_addr, &addr_len);
	if (num_byte < 0)
		return -1;
	return num_byte; 
}
// todo write some code for this function 
int udp_server_close(tlv_socket_t sd)
{
	close(sd);
	return 0;
}

#endif 
