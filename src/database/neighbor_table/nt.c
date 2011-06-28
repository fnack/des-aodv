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

#include "nt.h"
#include "../timeslot.h"
#include "../../config.h"
#include "../schedule_table/aodv_st.h"
#include "../../pipeline/aodv_pipeline.h"

typedef struct neighbor_entry {
	struct __attribute__ ((__packed__)) { // key
		uint8_t 				ether_neighbor[ETH_ALEN];
		const dessert_meshif_t*	iface;
		uint8_t 				mobility;
		int8_t				max_rssi;
	};
	UT_hash_handle				hh;
} neighbor_entry_t;

typedef struct neighbor_table {
	neighbor_entry_t* 		entrys;
	timeslot_t*				ts;
} neighbor_table_t;

neighbor_table_t nt;

/** returns the hello_interval in ms */
int calc_hello_interval(uint8_t mobility) {
	if(mobility == 0)
		return HELLO_INTERVAL;
	if(mobility < 10)
		return (HELLO_INTERVAL/2);
	if(mobility < 100)
		return (HELLO_INTERVAL/4);
	if(mobility < 200)
		return (HELLO_INTERVAL/6);
	return (HELLO_INTERVAL/8);
}

void set_hello_interval(int new_interval) {
	hello_interval = new_interval;

	dessert_periodic_del(periodic_send_hello);
	struct timeval hello_interval_t;
	hello_interval_t.tv_sec = hello_interval / 1000;
	hello_interval_t.tv_usec = (hello_interval % 1000) * 1000;
	periodic_send_hello = dessert_periodic_add(aodv_periodic_send_hello, NULL, NULL, &hello_interval_t);

	dessert_notice("setting HELLO interval to [%d]ms", hello_interval);
}

/** increments the hello_interval, if the new mobility requers it */
int db_nt_increment_hello_interval(uint8_t mobility) {
	int new_interval = calc_hello_interval(mobility);
	if(new_interval < hello_interval) {
		set_hello_interval(new_interval);
	}
	return hello_interval;
}

int db_nt_decrement_hello_interval(neighbor_entry_t* entry) {
	int entry_interval = calc_hello_interval(entry->mobility);
	if(entry_interval <= hello_interval) {
		set_hello_interval(calc_hello_interval(0));
	}
	return hello_interval;
}

int8_t db_neighbor_entry_rssi(uint8_t ether_neighbor_addr[ETH_ALEN], const dessert_meshif_t* iface) {
	
	int8_t out = -120;
	
	avg_node_result_t neigh_result = dessert_rssi_avg(ether_neighbor_addr, iface->if_name);
	if(neigh_result.avg_rssi != 0) {
		out = neigh_result.avg_rssi;
	}
	return out;
}

neighbor_entry_t* db_neighbor_entry_create(uint8_t ether_neighbor_addr[ETH_ALEN], const dessert_meshif_t* iface, uint8_t mobility) {
	neighbor_entry_t* new_entry;
	new_entry = malloc(sizeof(neighbor_entry_t));
	if (new_entry == NULL) return NULL;
	memcpy(new_entry->ether_neighbor, ether_neighbor_addr, ETH_ALEN);
	new_entry->iface = iface;

	new_entry->mobility = mobility;
	new_entry->max_rssi = db_neighbor_entry_rssi(ether_neighbor_addr, iface);

	return new_entry;
}

void db_nt_on_neigbor_timeout(struct timeval* timestamp, void* src_object, void* object) {
	neighbor_entry_t* curr_entry = object;
	dessert_debug("%s <= x => " MAC, curr_entry->iface->if_name, EXPLODE_ARRAY6(curr_entry->ether_neighbor));
	db_nt_decrement_hello_interval(curr_entry);
	HASH_DEL(nt.entrys, curr_entry);

	// add schedule
	struct timeval curr_time;
	gettimeofday(&curr_time, NULL);
	aodv_db_sc_addschedule(&curr_time, curr_entry->ether_neighbor, AODV_SC_SEND_OUT_RERR, 0);
	free(curr_entry);
}

int db_nt_init() {
	timeslot_t* new_ts;
	struct timeval timeout;
	uint32_t hello_int_msek = hello_interval * (ALLOWED_HELLO_LOST + 1);
	timeout.tv_sec = hello_int_msek / 1000;
	timeout.tv_usec = (hello_int_msek % 1000) * 1000;
	if (timeslot_create(&new_ts, &timeout, NULL, db_nt_on_neigbor_timeout) != TRUE) return FALSE;
	nt.entrys = NULL;
	nt.ts = new_ts;
	return TRUE;
}

void create_purge_timeout(struct timeval* purge_timeout, uint8_t mobility) {
	int remote_hello_interval = calc_hello_interval(mobility);
	uint32_t hello_int_msek = remote_hello_interval * (ALLOWED_HELLO_LOST + 1);
	purge_timeout->tv_sec = hello_int_msek / 1000;
	purge_timeout->tv_usec = (hello_int_msek % 1000) * 1000;
}

int db_nt_cap2Dneigh(uint8_t ether_neighbor_addr[ETH_ALEN], const dessert_meshif_t* iface, struct timeval* timestamp, uint8_t mobility) {
	neighbor_entry_t* curr_entry = NULL;
	uint8_t addr_sum[ETH_ALEN + sizeof(void*)];
	memcpy(addr_sum, ether_neighbor_addr, ETH_ALEN);
	memcpy(addr_sum + ETH_ALEN, &iface, sizeof(void*));
	HASH_FIND(hh, nt.entrys, addr_sum, ETH_ALEN + sizeof(void*), curr_entry);
	if (curr_entry == NULL) {
		//this neigbor is new, so create an entry
		curr_entry = db_neighbor_entry_create(ether_neighbor_addr, iface, mobility);
		if (curr_entry == NULL) return FALSE;
		HASH_ADD_KEYPTR(hh, nt.entrys, curr_entry->ether_neighbor, ETH_ALEN + sizeof(void*), curr_entry);
		dessert_debug("%s <=====> " MAC, iface->if_name, EXPLODE_ARRAY6(ether_neighbor_addr));
		db_nt_increment_hello_interval(mobility);
	} else {
		int8_t new = db_neighbor_entry_rssi(curr_entry->ether_neighbor, iface);
		int8_t max = curr_entry->max_rssi;

		if(max < new) {
			//walking to the ap
			dessert_debug("%s <=====> " MAC " rssi from %d to %d", iface->if_name, EXPLODE_ARRAY6(curr_entry->ether_neighbor), max, new);
			curr_entry->max_rssi = new;
		} else {
			//walking away
			if((max - SIGNAL_STRENGTH_THRESHOLD) > new) {
			  //-30 - 15                         > -30
				//we need to send a new warn
				struct timeval curr_time;
				gettimeofday(&curr_time, NULL);
				aodv_db_sc_addschedule(&curr_time, curr_entry->ether_neighbor, AODV_SC_SEND_OUT_RERR, AODV_FLAGS_RERR_W);
			}
		}
	}
	struct timeval* purge_timeout = malloc(sizeof(struct timeval));
	create_purge_timeout(purge_timeout, mobility);
	timeslot_addobject_with_timeout(nt.ts, timestamp, curr_entry, purge_timeout);
	free(purge_timeout);
	return TRUE;
}

int db_nt_check2Dneigh(uint8_t ether_neighbor_addr[ETH_ALEN], const dessert_meshif_t* iface, struct timeval* timestamp) {
	timeslot_purgeobjects(nt.ts, timestamp);
	neighbor_entry_t* curr_entry;
	uint8_t addr_sum[ETH_ALEN + sizeof(void*)];
	memcpy(addr_sum, ether_neighbor_addr, ETH_ALEN);
	memcpy(addr_sum + ETH_ALEN, &iface, sizeof(void*));
	HASH_FIND(hh, nt.entrys, addr_sum, ETH_ALEN + sizeof(void*), curr_entry);
	if (curr_entry == NULL) {
		return FALSE;
	}
	return TRUE;
}

int db_nt_cleanup(struct timeval* timestamp) {
	return timeslot_purgeobjects(nt.ts, timestamp);
}
