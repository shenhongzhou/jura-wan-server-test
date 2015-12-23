/******************************************************************************
Filename:     parse_pkt_fwd.c

add by shenhz 20151210


******************************************************************************/

#include "jura-wan-server.h"


/**************************************************************************
 * Function:parse_pkt_push_data
 * 
 *
*/
int parse_pkt_push_data(uint8_t* databuf)
{
	/* JSON parsing variables */
	JSON_Value *root_val = NULL;
	JSON_Object *rxpk_obj = NULL;
	JSON_Array *rxpk_arr = NULL;
	JSON_Value *val = NULL; /* needed to detect the absence of some fields */
	const char *str; /* pointer to sub-strings in the JSON data */
	int rxpk_arr_count = 0, no = 0;

	uint32_t size = -1, i;
	uint8_t payload_bin[256] = {0};
	
	printf("JSON up: %s ", (char *)(databuf + 12)); /* DEBUG: display JSON payload */


	/* try to parse JSON */
	root_val = json_parse_string_with_comments((const char *)(databuf + 12)); /* JSON offset */
	if (root_val == NULL) {
		MSG("WARNING: [down] invalid JSON, RX aborted\n");
		return;
	}

	/* look for JSON sub-array 'rxpk' */
	rxpk_arr = json_object_get_array(json_value_get_object(root_val), "rxpk");
	if (rxpk_arr == NULL) {
		MSG("WARNING: [up] no \"rxpk\" array in JSON, RX aborted\n");
		json_value_free(root_val);
		return;
	}

	rxpk_arr_count = json_array_get_count(rxpk_arr);

	for(no=0; no<rxpk_arr_count; no++)
	{

		/* look for JSON sub-object 'rxpk_arr[no]' */
		rxpk_obj = json_array_get_object(rxpk_arr, no);
		if (rxpk_obj == NULL) {
			MSG("WARNING: [up] no \"rxpk\" object in JSON, TX aborted\n");
			json_value_free(root_val);
			return;
		}

		/* Parse payload length (mandatory) */
		val = json_object_get_value(rxpk_obj,"size");
		if (val == NULL) {
			MSG("WARNING: [up] no mandatory \"rxpk.size\" object in JSON, RX aborted\n");
			json_value_free(root_val);
			return;
		}
		size = (uint16_t)json_value_get_number(val);
			
		/* Parse payload data (mandatory) */
		str = json_object_get_string(rxpk_obj, "data");
		if (str == NULL) {
			MSG("WARNING: [up] no mandatory \"rxpk.data\" object in JSON, RX aborted\n");
			json_value_free(root_val);
			return;
		}
		i = b64_to_bin(str, strlen(str), payload_bin, sizeof payload_bin);
		if (i != size) {
			MSG("WARNING: [up] mismatch between .size and .data size once converter to binary\n");
		}
			
		/* free the JSON parse tree from memory */
		json_value_free(root_val);

#if 1  
		int ret = -1;
		printf("\nNodes rxpk_arr[%d] playout_bin[%d]:", no, size);
		for (ret = 0; ret < size; ret++) {
			printf("%.2X ", payload_bin[ret]);
		}
		printf("\n");
#endif
	}
}

#if 0
/**************************************************************************
 * Function:fwd_pkt_pull_resp
 * 
 *
*/
int fwd_pkt_pull_resp(int flag)
{

	uint8_t databuf[500];
	int buff_index;
	int byte_nb;

	/* application parameters */
	float f_target = 471.0; /* target frequency */
	int sf = 7; /* SF7 by default */
	int bw = 250; /* 125kHz bandwidth by default */
	int pow = 20; /* 20 dBm by default */

	/* packet payload variables */
	uint8_t payload_bin[20] = "TEST**##############"; /* # is for padding */
	char payload_b64[30];
	int payload_index;

	/* PKT_PULL_RESP datagrams header */
	databuf[0] = PROTOCOL_VERSION;
	databuf[1] = 0; /* no token */
	databuf[2] = 0; /* no token */
	databuf[3] = PKT_PULL_RESP;
	buff_index = 4;
	
	/* start of JSON structure */
	memcpy((void *)(databuf + buff_index), (void *)"{\"txpk\":{\"imme\":true", 20);
	buff_index += 20;
	
	/* TX frequency */
	i = snprintf((char *)(databuf + buff_index), 20, ",\"freq\":%.6f", f_target);
	if ((i>=0) && (i < 20)) {
		buff_index += i;
	} else {
		MSG("ERROR: snprintf failed line %u\n", (__LINE__ - 4));
		exit(EXIT_FAILURE);
	}
	
	/* RF channel */
	memcpy((void *)(databuf + buff_index), (void *)",\"rfch\":0", 9);
	buff_index += 9;
	
	/* TX power */
	i = snprintf((char *)(databuf + buff_index), 12, ",\"powe\":%i", pow);
	if ((i>=0) && (i < 12)) {
		buff_index += i;
	} else {
		MSG("ERROR: snprintf failed line %u\n", (__LINE__ - 4));
		exit(EXIT_FAILURE);
	}
	
	/* modulation, datarate and ECC coding rate */
	i = snprintf((char *)(databuf + buff_index), 50, ",\"modu\":\"LORA\",\"datr\":\"SF%iBW%i\",\"codr\":\"4/5\"", sf, bw);
	if ((i>=0) && (i < 50)) {
		buff_index += i;
	} else {
		MSG("ERROR: snprintf failed line %u\n", (__LINE__ - 4));
		exit(EXIT_FAILURE);
	}
	
#if 0
	/* signal polarity */
	if (invert) {
		memcpy((void *)(databuf + buff_index), (void *)",\"ipol\":true", 12);
		buff_index += 12;
	} else {
		memcpy((void *)(databuf + buff_index), (void *)",\"ipol\":false", 13);
		buff_index += 13;
	}
	
	/* Preamble size */
	memcpy((void *)(databuf + buff_index), (void *)",\"prea\":8", 9);
	buff_index += 9;
	
#endif
	/* payload size */
	memcpy((void *)(databuf + buff_index), (void *)",\"size\":20", 10);
	buff_index += 10;
	
	/* payload JSON object */
	memcpy((void *)(databuf + buff_index), (void *)",\"data\":\"", 9);
	buff_index += 9;
	payload_index = buff_index; /* keep the value where the payload content start */
	
	/* payload place-holder & end of JSON structure */
	memcpy((void *)(databuf + buff_index), (void *)"###########################", 27);
	buff_index += 27;
	memcpy((void *)(databuf + buff_index), (void *)"\"}}", 3);
	buff_index += 3; /* ends up being the total length of payload */
	
	
	
		
	/* encode the payload in Base64 */
	bin_to_b64_nopad(payload_bin, sizeof payload_bin, payload_b64, sizeof payload_b64);
	memcpy((void *)(databuf + payload_index), (void *)payload_b64, 27);
		
	/* send packet to the gateway */
	if(flag == 0)//tcp
	{
		byte_nb = send(sock, (void *)databuf, buff_index, 0);
		if (byte_nb ==  buff_index) {
			MSG("INFO: packet sent successfully\n");
		} else {
			MSG("WARNING: sendto returned an error %s\n", strerror(errno));
		}
	}
	esle if(flag == 1)//udp
	{
		byte_nb = sendto(sock, (void *)databuf, buff_index, 0, (struct sockaddr *)&dist_addr, addr_len);
		if (byte_nb == -1) {
			MSG("WARNING: sendto returned an error %s\n", strerror(errno));
		} else {
			MSG("INFO: packet #%i sent successfully\n", i);
		}
	}	
	

}
#endif
