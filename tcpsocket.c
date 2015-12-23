/******************************************************************************
Filename:     tcpsocket.c

add by shenhz 20151210


******************************************************************************/

#include "jura-wan-server.h"



typedef	struct TcpClient {

	int sock;
	char host_name[64];
	char port_name[64];

}Tcpclient_t;

Tcpclient_t tcpc;

uint8_t data_resp[MAX_DATALEN];
volatile int flag_resp = 0, byte_resp;

/**************************************************************************
 * Function:Thread_tcp
 * 
 *
*/
void* Thread_tcp(void* env)
{
	Tcpclient_t tcpcc = *(Tcpclient_t*)env;

	uint8_t databuf[MAX_DATALEN];
	int byte_nb;
	
	
	/* variables for protocol management */
	uint32_t raw_mac_h; /* Most Significant Nibble, network order */
	uint32_t raw_mac_l; /* Least Significant Nibble, network order */
	uint64_t gw_mac; /* MAC address of the client (gateway) */
	uint8_t ack_command;

	
	while (1) {
		memset(databuf,0,MAX_DATALEN);
		/* wait to receive a packet */
		byte_nb = read(tcpcc.sock,databuf,MAX_DATALEN);
		if (byte_nb == -1) {
			MSG("ERROR: recvfrom returned %s \n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		else if(byte_nb == 0) {
			continue;
		}
		databuf[byte_nb] = '\0';
		printf("\n-> pkt in , host %s (port %s), %i bytes", tcpcc.host_name, tcpcc.port_name, byte_nb);
		
		/* check and parse the payload */
		if (byte_nb < 12) { /* not enough bytes for packet from gateway */
			printf(" (too short for GW <-> MAC protocol)\n");
			continue;
		}
		/* don't touch the token in position 1-2, it will be sent back "as is" for acknowledgement */
		if (databuf[0] != PROTOCOL_VERSION) { /* check protocol version number */
			printf(", invalid version %u\n", databuf[0]);
			continue;
		}
		raw_mac_h = *((uint32_t *)(databuf+4));
		raw_mac_l = *((uint32_t *)(databuf+8));
		gw_mac = ((uint64_t)ntohl(raw_mac_h) << 32) + (uint64_t)ntohl(raw_mac_l);
		
		/* interpret gateway command */
		switch (databuf[3]) {
			case PKT_PUSH_DATA:
				printf(", PUSH_DATA from gateway 0x%08X%08X\n", (uint32_t)(gw_mac >> 32), (uint32_t)(gw_mac & 0xFFFFFFFF));
				parse_pkt_push_data(databuf);//dbg
				ack_command = PKT_PUSH_ACK;
				printf("\n<- pkt out, PUSH_ACK for host %s (port %s)", tcpcc.host_name, tcpcc.port_name);
				break;
			case PKT_PULL_DATA:
				printf(", PULL_DATA from gateway 0x%08X%08X\n", (uint32_t)(gw_mac >> 32), (uint32_t)(gw_mac & 0xFFFFFFFF));
				ack_command = PKT_PULL_ACK;
				printf("\n<- pkt out, PULL_ACK for host %s (port %s)", tcpcc.host_name, tcpcc.port_name);

				if(flag_resp == 1)
					flag_resp = 2;

				break;
			case PKT_PULL_RESP://the data from a tcp client tx test, then fwd to a online gateway
				printf(", PULL_RESP pkt, then fwd to online gateway...\n");
				flag_resp = 1;
				byte_resp = byte_nb;
				memset(data_resp,0,MAX_DATALEN);
				memcpy(data_resp,databuf,byte_nb);
				break;
			default:
				printf(", unexpected command %u\n", databuf[3]);
				continue;
		}
		
		/* add some artificial latency */
		usleep(30000); /* 30 ms */
		

		switch(flag_resp) {
			case 0:
				/* send acknowledge and check return value */
				databuf[3] = ack_command;
				byte_nb = send(tcpcc.sock, (void *)databuf, 4, 0);
				if (byte_nb == 4) {
					printf(", %i bytes sent\n", byte_nb);
				} else {
					printf(", send error:%s\n", strerror(errno));
				}
				break;
			case 1:
				break;
			case 2:
				//fwd PULL_RESP to gateway
				printf("\n<- pkt out, PULL_RESP to gateway 0x%08X%08X\n", (uint32_t)(gw_mac >> 32), (uint32_t)(gw_mac & 0xFFFFFFFF));
				byte_nb = send(tcpcc.sock, (void *)data_resp, byte_resp, 0);
				if (byte_nb == byte_resp) {
					printf(", %i bytes sent\n", byte_nb);
				} else {
					printf(", send error:%s\n", strerror(errno));
				}
				flag_resp = 0;
				break;
		}

		
	}

	return 0;
}


/**************************************************************************
 * Function:
 * 
 *
*/
int tcpserver(char* port)
{
	int i; /* loop variable and temporary variable for return value */
	
	/* server socket creation */
	int sock; /* socket file descriptor */
	struct addrinfo hints;
	struct addrinfo *result; /* store result of getaddrinfo */
	struct addrinfo *q; /* pointer to move into *result data */
	char host_name[64];
	char port_name[64];
	
	/* variables for receiving and sending packets */
	struct sockaddr_storage dist_addr;
	socklen_t addr_len = sizeof dist_addr;
	
	
	/* check if port number was passed as parameter */
	if (port <= 0) { 
		MSG("Error:ser_port <=0!\n");
		exit(EXIT_FAILURE);
	}
	
	/* prepare hints to open network sockets */
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; /* should handle IP v4 or v6 automatically */

	hints.ai_socktype = SOCK_STREAM;

	hints.ai_flags = AI_PASSIVE; /* will assign local IP automatically */
	
	/* look for address */
	i = getaddrinfo(NULL, port, &hints, &result);
	if (i != 0) {
		MSG("ERROR: getaddrinfo returned %s\n", gai_strerror(i));
		exit(EXIT_FAILURE);
	}
	
	/* try to open socket and bind it */
	for (q=result; q!=NULL; q=q->ai_next) {
		sock = socket(q->ai_family, q->ai_socktype,q->ai_protocol);
		if (sock == -1) {
			continue; /* socket failed, try next field */
		} else {
			i = bind(sock, q->ai_addr, q->ai_addrlen);
			if (i == -1) {
				shutdown(sock, SHUT_RDWR);
				continue; /* bind failed, try next field */
			} else {
				break; /* success, get out of loop */
			}
		}
	}
	if (q == NULL) {
		MSG("ERROR: failed to open socket or to bind to it\n");
		i = 1;
		for (q=result; q!=NULL; q=q->ai_next) {
			getnameinfo(q->ai_addr, q->ai_addrlen, host_name, sizeof host_name, port_name, sizeof port_name, NI_NUMERICHOST);
			MSG("INFO: result %i host:%s service:%s\n", i, host_name, port_name);
			++i;
		}
		exit(EXIT_FAILURE);
	}
	MSG("INFO: jura-wan-server listening on port %s\n", port);

	
    	if (listen(sock,MAX_CONNUM) == -1)
    	{
       	 	perror("listen:");
       	 	exit(1);
    	}


	freeaddrinfo(result);
	
#ifdef TCP_SOCKET
	int new_fd = -1;
	pthread_t pid;
	pthread_attr_t 	p_attr;

	pthread_attr_init(&(p_attr));
	pthread_attr_setdetachstate(&(p_attr), PTHREAD_CREATE_DETACHED);//detached

	
	while(1)
	{
	 	if ((new_fd = accept(sock,(struct sockaddr *)&dist_addr, &addr_len)) == -1)
        	{
           	 	perror("accept:");
            		exit(1);
        	}
        	else if(new_fd > 0)
        	{

			/* display info about the sender */
			i = getnameinfo((struct sockaddr *)&dist_addr, addr_len, host_name, sizeof host_name, port_name, sizeof port_name, NI_NUMERICHOST);
			if (i == -1) {
				MSG("ERROR: getnameinfo returned %s \n", gai_strerror(i));
				exit(EXIT_FAILURE);
			}

			tcpc.sock = new_fd;
			memset(tcpc.host_name,0,64);
			memset(tcpc.port_name,0,64);
			memcpy(tcpc.host_name,host_name,strlen(host_name));
			memcpy(tcpc.port_name,port_name,strlen(port_name));

        		printf("server get connection from host_name:%s port_name:%s\n\r",tcpc.host_name,tcpc.port_name);
        	
			pthread_create(&pid, &p_attr, Thread_tcp, (void*)&tcpc);      

					
		}
        
    	}

#endif

}
