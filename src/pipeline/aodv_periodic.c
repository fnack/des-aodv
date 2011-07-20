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

#include "../database/aodv_database.h"
#include "aodv_pipeline.h"
#include "../config.h"
#include <string.h>
#include <pthread.h>
#include <utlist.h>

int aodv_periodic_send_hello(void *data, struct timeval *scheduled, struct timeval *interval) {

	// create new HELLO message with hello_ext.
	dessert_msg_t* hello_msg;
	dessert_msg_new(&hello_msg);
	hello_msg->ttl = 2;

	dessert_ext_t* ext;
	dessert_msg_addext(hello_msg, &ext, HELLO_EXT_TYPE, sizeof(struct aodv_msg_hello));

	void* payload;
	uint16_t size = max(hello_size - sizeof(dessert_msg_t) - sizeof(struct ether_header) - 2, 0);

	dessert_msg_addpayload(hello_msg, &payload, size);
	memset(payload, 0xA, size);

	dessert_meshsend(hello_msg, NULL);
	dessert_msg_destroy(hello_msg);
	return 0;
}

int aodv_periodic_cleanup_database(void *data, struct timeval *scheduled, struct timeval *interval) {
	struct timeval timestamp;
	gettimeofday(&timestamp, NULL);
	if (aodv_db_cleanup(&timestamp)) {
		return 0;
	} else {
		return 1;
	}
}

dessert_msg_t* aodv_create_rerr(nht_destlist_entry_t **destlist) {
	if (*head == NULL) return NULL;
	dessert_msg_t* msg;
	dessert_ext_t* ext;
	dessert_msg_new(&msg);

	// set ttl
	msg->ttl = MAX_TTL;

	// add l25h header
	dessert_msg_addext(msg, &ext, DESSERT_EXT_ETH, ETHER_HDR_LEN);
	struct ether_header* rreq_l25h = (struct ether_header*) ext->data;
	memcpy(rreq_l25h->ether_dhost, ether_broadcast, ETH_ALEN);

	// add RERR ext
	dessert_msg_addext(msg, &ext, RERR_EXT_TYPE, sizeof(struct aodv_msg_rerr));
	struct aodv_msg_rerr* rerr_msg = (struct aodv_msg_rerr*) ext->data;
	rerr_msg->flags = AODV_FLAGS_RERR_N;

	// write addresses of all my mesh interfaces
	void* ifaceaddr_pointer = rerr_msg->ifaces;
	uint8_t ifaces_count = 0;
	dessert_meshif_t *iface;
	MESHIFLIST_ITERATOR_START(iface)
		if(ifaces_count >= MAX_MESH_IFACES_COUNT)
			break;
		memcpy(ifaceaddr_pointer, iface->hwaddr, ETH_ALEN);
		ifaceaddr_pointer += ETH_ALEN;
		ifaces_count++;
	MESHIFLIST_ITERATOR_STOP;
	rerr_msg->iface_addr_count = ifaces_count;

	while(*destlist) {
		int dl_len = 0;

		//count the length of destlist up to MAX_MAC_SEQ_PER_EXT elements
		for(nht_destlist_entry_t *iter = *destlist;
		    (dl_len <= MAX_MAC_SEQ_PER_EXT) && iter;
		    ++dl_len, iter = iter->next) {
		}
		if(dessert_msg_addext(msg, &ext, RERRDL_EXT_TYPE, dl_len * sizeof(struct aodv_mac_seq)) != DESSERT_OK)
			break;

		struct aodv_mac_seq *start = (struct aodv_mac_seq*) ext->data, *iter;
		for(iter = start; iter < start + dl_len; ++iter) {
			nht_destlist_entry_t* el = *head;
			memcpy(iter->host, el->dhost_ether, ETH_ALEN);
			iter->sequence_number = el->destination_sequence_number;

			DL_DELETE(*head, el);
			free(el);
		}
	}

	return msg;
}

int aodv_periodic_scexecute(void *data, struct timeval *scheduled, struct timeval *interval) {
	uint8_t schedule_type;
	void* schedule_param = NULL;
	uint8_t ether_addr[ETH_ALEN];
	struct timeval timestamp;
	gettimeofday(&timestamp, NULL);

	if (aodv_db_popschedule(&timestamp, ether_addr, &schedule_type, &schedule_param) == FALSE) {
		return 0;
	}

	switch(schedule_type) {
	case AODV_SC_SEND_OUT_PACKET: {
			//do nothing
			break;
		}
	case AODV_SC_REPEAT_RREQ: {
			aodv_send_rreq(ether_addr, &timestamp, (dessert_msg_t*) (schedule_param));
			break;
		}
	case AODV_SC_SEND_OUT_RERR: {
			uint32_t rerr_count;
			aodv_db_getrerrcount(&timestamp, &rerr_count);
			if (rerr_count >= RERR_RATELIMIT) {
				return 0;
			}
			nht_destlist_entry_t *destlist;
			if(!aodv_db_rt_inv_over_nexthop(ether_addr)) {
				return 0; //nexthop not in nht
			}
			if(!aodv_db_rt_get_destlist(ether_addr, &destlist)) {
				return 0; //nexthop not in nht
			}

			while(TRUE) {
				dessert_msg_t* rerr_msg = aodv_create_rerr(&destlist);
				if (!rerr_msg) {
					break;
				}
				dessert_meshsend(rerr_msg, NULL);
				dessert_msg_destroy(rerr_msg);
				aodv_db_putrerr(&timestamp);
			}
			break;
		}
	case AODV_SC_SEND_OUT_RWARN: {
			nht_destlist_entry_t * head = NULL;
			aodv_db_get_warn_endpoints_from_neighbor_and_set_warn(ether_addr, &head);

			nht_destlist_entry_t *dest, *tmp;
			DL_FOREACH_SAFE(head, dest, tmp) {
				dessert_debug("AODV_SC_SEND_OUT_RWARN -> down: " MAC " dest: " MAC,
				              EXPLODE_ARRAY6(ether_addr),
				              EXPLODE_ARRAY6(dest->dhost_ether));
				aodv_send_rreq(dest->dhost_ether, &timestamp, NULL);
			}
			break;
		}
	case AODV_SC_UPDATE_RSSI: {
			dessert_meshif_t* iface = (dessert_meshif_t*) (schedule_param);
			int diff = aodv_db_update_rssi(ether_addr, iface, &timestamp);
			if(diff > AODV_SIGNAL_STRENGTH_THRESHOLD) {
				//walking away -> we need to send a new warn
				dessert_debug("%s <= W => " MAC, iface->if_name, EXPLODE_ARRAY6(ether_addr));
				aodv_db_addschedule(&timestamp, ether_addr, AODV_SC_SEND_OUT_RWARN, 0);
			}
			break;
		}
	default: {
			dessert_crit("unknown schedule type=%u", schedule_type);
		}
	}
	return 0;
}
