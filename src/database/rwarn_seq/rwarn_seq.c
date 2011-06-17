/******************************************************************************
Copyright 2009, Freie Universitaet Berlin (FUB). All rights reserved.

These sources were developed at the Freie Universitaet Berlin,
Computer Systems and Telematics / Distributed, embedded Systems (DES) group
(http://cst.mi.fu-berlin.de, http://www.des-testbed.net)
-------------------------------------------------------------------------------
This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see http://www.gnu.org/licenses/ .
--------------------------------------------------------------------------------
For further information and questions please use the web site
       http://www.des-testbed.net
*******************************************************************************/

#include <uthash.h>
#include <stdlib.h>
#include <stdio.h>
#include "rwarn_seq.h"
#include "../../config.h"

typedef struct rwan_packet_id {
	u_int8_t src_addr[ETH_ALEN]; // key
	u_int16_t seq_num;
	UT_hash_handle hh;
} rwarn_packet_id_t;

rwarn_packet_id_t* rwarn_entrys = NULL;

//returns TRUE if input entry is newer
//        FALSE if input entry is older
//        -1 if error
int aodv_db_rwarn_capt_rwarn_seq(u_int8_t shost_ether[ETH_ALEN], u_int16_t shost_seq_num) {

	rwarn_packet_id_t* entry = NULL;
	HASH_FIND(hh, rwarn_entrys, shost_ether, ETH_ALEN, entry);
	if (entry == NULL) {
		//never got rwarn from this host
		entry = malloc(sizeof(rwarn_packet_id_t));
		if (entry == NULL) {
			return -1;
		}
		memcpy(entry->src_addr, shost_ether, ETH_ALEN);
		HASH_ADD_KEYPTR(hh, rwarn_entrys, entry->src_addr, ETH_ALEN, entry);
		entry->seq_num = shost_seq_num;
		return TRUE;
	}
	
	//rwarn source is known
	if ((entry->seq_num - shost_seq_num > (1<<15)) || (entry->seq_num < shost_seq_num)) {
		//rwarn packet is newer
		entry->seq_num = shost_seq_num;
		return TRUE;
	}
	
	//rwarn packet is old
	return FALSE;
}
