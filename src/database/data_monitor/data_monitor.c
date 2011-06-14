/******************************************************************************
Copyright 2011, Freie Universitaet Berlin (FUB). All rights reserved.

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
#include "data_seq.h"
#include "../../config.h"

typedef struct aodv_dm_source {
	aodv_dm_t                 *next;
	u_int8_t                  l25_source[ETH_ALEN];
	timeslot_t*               last_seen;
	timeslot_t*               last_warn;
	aodv_dm_t                 *prev;
} aodv_dm_source_t;

typedef struct aodv_dm {
	aodv_dm_t                 *next;
	u_int8_t                  l2_source[ETH_ALEN]; // ID
	u_int8_t                  max_rssi;
	u_int8_t                  last_rssi;
	const dessert_meshif_t*   iface;
	aodv_dm_source_t*         l25_list;
	aodv_dm_t                 *prev;
} aodv_dm_t;

aodv_dm_t                         dm;

static int dm_cmp(aodv_dm_t *left, aodv_dm_t *right) {
	return memcmp(left->l2_source, right->l2_source, sizeof(left->l2_source));
}

static int dm_source_cmp(aodv_dm_source_t *left, aodv_dm_source_t *right) {
	return memcmp(left->l25_source, right->l25_source, sizeof(left->l25_source));
}

void purge_dm_entrys() {
	DL_FORALL_SAFE() {
		if(entry->ts < xxx)
			DL_DELETE(entry);
	}
	return;
}

int aodv_db_dm_init() {
	dm = NULL;

	struct timeval dmt;
	dmt.tv_sec = MY_ROUTE_TIMEOUT / 1000;
	dmt.tv_usec = (MY_ROUTE_TIMEOUT % 1000) * 1000;
	return timeslot_create(&dm.ts, &dmt, &dm, purge_dm_entry);
}


dm_result* create_dm(u_int8_t l2_source[ETH_ALEN], const dessert_meshif_t* iface, struct timeval ts) {
	aodv_dm_t *dm;
	malloc(&dm, sizeof(aodv_dm_t));

	memcpy(dm.l2_source, l2_source, sizeof(l2_source));
	dm.dessert_meshif_t = iface;
	memcpy(dm.ts, ts, sizeof(ts));

	return dm;
}

aodv_dm_source_t* create_dm_source(u_int8_t l25_source[ETH_ALEN]) {
	aodv_dm_sourcet *dm_source;
	malloc(&dm_source, sizeof(aodv_dm_source_t));

	memcpy(dm.l25_source, l25_source, sizeof(l25_source));

	return dm_source;
}

int aodv_db_data_monitor_signal_strength_update(struct timeval ts) {

	purge_dm_entrys();

	DL_FOREACH(dm) {
		avg_node_result_t result = dessert_rssi_avg(aodv_monitor_last_hops_rbuff[i].l2_source, aodv_monitor_last_hops_rbuff[i].iface->if_name);
		if(result.avg_rssi != 0) {
			dm->last_rssi = result.avg_rssi;
			dm->max_rssi = max(dm->max_rssi, result.avg_rssi);
		}
	}
	return 0;
}

aodv_dm_t* aodv_db_data_monitor_pop() {

	DL_FOREACH(dm) {
		DL_FOREACH(dm_source) {

			if((max_rssi - MONITOR_SIGNAL_STRENGTH_THRESHOLD) <= last_rssi) {
				//    -35    -               20                   <=      -50
				dessert_trace("RSSI VAL of %d from " MAC " threshold (%d) not reached...doing nothing",
					      result.avg_rssi,
					      EXPLODE_ARRAY6(aodv_monitor_last_hops_rbuff[i].l2_source),
					      max_rssi - MONITOR_SIGNAL_STRENGTH_THRESHOLD);
				continue;
			}
			
			if((last_warn + MONITOR_SIGNAL_STRENGTH_WARN_INTERVAL) > now)) {
				//send only every MONITOR_SIGNAL_STRENGTH_WARN_INTERVAL seconds
				dessert_trace("RSSI VAL of %d from " MAC " -> but SIGNAL_STRENGTH_MIN_WARN_INTERVAL*5 is not reached",
					      result.avg_rssi,
					      EXPLODE_ARRAY6(aodv_monitor_last_hops_rbuff[i].l2_source),
					      MONITOR_SIGNAL_STRENGTH_GREY_ZONE);
				continue;
			}

			return dm_source;
		}
	}
	return NULL;
}

int aodv_db_data_monitor_capture_packet(u_int8_t l2_source[ETH_ALEN], u_int8_t l25_source[ETH_ALEN], const dessert_meshif_t* iface, struct timeval ts)
	//lock
	aodv_rt_entry_t dm_needle, dm_result;
	memcpy(dm_needle.l2_source, l2_source, sizeof(l2_source));
	DL_SEARCH(dm, dm_needle, dm_result, dm_cmp);
	
	if(!dm_result) {
		dm_result = create_dm(l2_source, iface, ts);
		DL_APPEND(dm, dm_result);
		return TRUE;
	}

	aodv_rt_entry_t dm_source_needle, dm_source_result;
	memcpy(dm_source_needle.l25_source, l25_source, sizeof(l25_source));
	DL_SEARCH(dm, dm_source_needle, &dm_source_result, dm_source_cmp);
	
	if(!dm_source_result) {
		dm_source_t *dm_source_result = malloc(sizeof(dm_source_result));
		dm_source_result = create_source_dm(l25_source);
		DL_APPEND(dm->l25_list, dm_source_result);
		return TRUE;
	}

	DL_FIND(dm_result);
	dm_source_result->last_seen = ts;

	return TRUE;
}

