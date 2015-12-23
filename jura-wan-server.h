/******************************************************************************
Filename:     jura-wan-server.h

add by shenhz 20151210


******************************************************************************/

#ifndef _JURA_WAN_SERVSER_
#define _JURA_WAN_SERVSER_


/* -------------------------------------------------------------------------- */
/* --- DEPENDANCIES --------------------------------------------------------- */

/* fix an issue between POSIX and C99 */
#if __STDC_VERSION__ >= 199901L
	#define _XOPEN_SOURCE 600
#else
	#define _XOPEN_SOURCE 500
#endif

#include <stdint.h>		/* C99 types */
#include <stdio.h>		/* printf, fprintf, sprintf, fopen, fputs */
#include <unistd.h>		/* usleep */

#include <string.h>		/* memset */
#include <time.h>		/* time, clock_gettime, strftime, gmtime, clock_nanosleep*/
#include <stdlib.h>		/* atoi, exit */
#include <errno.h>		/* error messages */

#include <sys/socket.h> /* socket specific definitions */
#include <netinet/in.h> /* INET constants and stuff */
#include <arpa/inet.h>  /* IP address conversion stuff */
#include <netdb.h>		/* gai_strerror */
#include <pthread.h>


#include "base64.h"
#include "parson.h"

/* -------------------------------------------------------------------------- */
/* --- PRIVATE MACROS ------------------------------------------------------- */

#define ARRAY_SIZE(a)	(sizeof(a) / sizeof((a)[0]))
#define STRINGIFY(x)	#x
#define STR(x)		STRINGIFY(x)
#define MSG(args...)	fprintf(stderr, args) /* message that is destined to the user */

/* -------------------------------------------------------------------------- */
/* --- PRIVATE CONSTANTS ---------------------------------------------------- */

#define	PROTOCOL_VERSION	1

#define PKT_PUSH_DATA	0
#define PKT_PUSH_ACK	1
#define PKT_PULL_DATA	2
#define PKT_PULL_RESP	3
#define PKT_PULL_ACK	4


#define TCP_SOCKET
#define MAX_CONNUM	100

#define MAX_DATALEN	4096

#define Ser_Version	"V1.0.20151210.Test..."



int tcpserver(char* port);
int udpserver(char* port);

int parse_pkt_push_data(uint8_t* databuf);

#endif
