/******************************************************************************
Filename:     jura-wan-server.c

add by shenhz 20151210


******************************************************************************/

#include "jura-wan-server.h"



/**************************************************************************
 * Function:
 * 
 *
*/
int main(int argc, char **argv)
{
	int opt;
	uint8_t flag = -1;
	char serv_port[8];

	uint8_t using[]={
"Usage:\n\tjura-wan-server [options]\n\
Options:\n\
\t-t\t--tcp\n\
\t-u\t--udp\n\
\t-p\t--port(10-20000)\n\
\t-h\t--help)\n"
};

	MSG("\n**************JURA WAN SERVER Version %s**************\n",Ser_Version);
	
	while ((opt = getopt_long(argc, argv, "tup:h", NULL, NULL)) != -1) 
	{
		
		switch(opt)
		{
			case 't':
				flag = 0;
				break;

			case 'u':
				flag = 1;
				break;

			case 'p':
				strncpy(serv_port, optarg, sizeof serv_port);
				break;

			case 'h':
				
				MSG("%s",using);
				break;
		}
	}

	

	if(flag == 0)
	{
		MSG("INFO:using tcp server...port %s\n",serv_port);
		tcpserver(serv_port);
	}
	else if(flag == 1)
	{

		MSG("INFO:using udp server...port %s\n",serv_port);
		udpserver(serv_port);
	}
	else
	{
		MSG("%s",using);
		exit(EXIT_FAILURE);

	}

	return 0;
	
}
