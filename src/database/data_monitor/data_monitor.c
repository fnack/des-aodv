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
#include <utlist.h>
#include "data_monitor.h"
#include "../../config.h"

aodv_dm_t *dm = NULL;
pthread_rwlock_t data_monitor_lock = PTHREAD_RWLOCK_INITIALIZER;

static int dm_cmp(aodv_dm_t *left, aodv_dm_t *right) {
	return memcmp(left->l2_source, right->l2_source, sizeof(left->l2_source));
}

static int dm_source_cmp(aodv_dm_source_t *left, aodv_dm_source_t *right) {
	return memcmp(left->l25_source, right->l25_source, sizeof(left->l25_source));
}

void purge_dm_entrys(struct timeval ts) {

	aodv_dm_t *iter;
	DL_FOREACH(dm, iter) {
		aodv_dm_source_t *dm_source_iter;
		DL_FOREACH(iter->l25_list, dm_source_iter) {
			if((dm_source_iter->last_seen->tv_sec + MONITOR_SIGNAL_STRENGTH_TIMEOUT) < ts->tv_sec) {
				DL_DELETE(iter->l25_list, dm_source_iter);
			}
		}
	}
	return;
}

dm_result* create_dm(u_int8_t l2_source[ETH_ALEN], const dessert_meshif_t* iface, uint8_t last_rssi) {
	aodv_dm_t *dm;
	malloc(dm, sizeof(aodv_dm_t));

	memset(dm_source, 0x0, sizeof(aodv_dm_source_t));
	memcpy(dm.l2_source, l2_source, sizeof(l2_source));
	dm->dessert_meshif_t = iface;
	dm->max_rssi = last_rssi;
	dm->last_rssi = last_rssi;

	return dm;
}

aodv_dm_source_t* create_dm_source(u_int8_t l25_source[ETH_ALEN], struct timeval last_seen) {
	aodv_dm_sourcet *dm_source;
	malloc(dm_source, sizeof(aodv_dm_source_t));

	memset(dm_source, 0x0, sizeof(aodv_dm_source_t));
	memcpy(dm.l25_source, l25_source, sizeof(l25_source));
	memcpy(dm.last_seen, last_seen, sizeof(last_seen));

	return dm_source;
}

int aodv_db_data_monitor_signal_strength_update(struct timeval *ts) {
	pthread_rwlock_wrlock(&data_monitor_lock);
	purge_dm_entrys(ts);

	aodv_dm_t *sample;
	DL_FOREACH(dm, sample) {
		avg_node_result_t result = dessert_rssi_avg(sample->l2_source, sample->if_name);
		if(result.avg_rssi != 0) {
			sample->last_rssi = result.avg_rssi;
			sample->max_rssi = max(dm->max_rssi, result.avg_rssi);
		} else {
			dessert_debug("rssi of " MAC " is %d", EXPLODE_ARRAY6(sample->l2_source), result.avg_rssi);
		}
	}
	pthread_rwlock_unlock(&data_monitor_lock);
	return TRUE;
}

aodv_dm_t* aodv_db_data_monitor_pop(struct timeval *now) {
	pthread_rwlock_wrlock(&data_monitor_lock);
	aodv_dm_t *dm_sample;
	DL_FOREACH(dm, dm_sample) {
		aodv_dm_t *dm_source_sample;
		DL_FOREACH(dm_source, dm_source_sample) {

			//signal is lower than allowed
			int a = (dm_source_sample->max_rssi - MONITOR_SIGNAL_STRENGTH_THRESHOLD) <= dm_source_sample->last_rssi;

			//we need to send a new warn
			int b = (dm_source_sample->last_warn + MONITOR_SIGNAL_STRENGTH_WARN_INTERVAL) > now->tv_sec;

			if(a && b) {
				memcpy(dm_source_sample->last_warn, now, sizeof(struct timeval);
				return dm_sample;
			}
		}
	}
	pthread_rwlock_unlock(&data_monitor_lock);
	return NULL;
}

int aodv_db_data_monitor_capture_packet(u_int8_t l2_source[ETH_ALEN], u_int8_t l25_source[ETH_ALEN], const dessert_meshif_t* iface, struct timeval ts) {
	pthread_rwlock_wrlock(&data_monitor_lock);
	aodv_dm_t *dm_needle = NULL;
	aodv_dm_t *dm_result = NULL;
	memcpy(dm_needle->l2_source, l2_source, sizeof(l2_source));
	DL_SEARCH(dm, dm_needle, dm_result, dm_cmp);
	
	if(!dm_result) {
		dm_result = create_dm(l2_source, iface, ts);
		DL_APPEND(dm, dm_result);
		pthread_rwlock_unlock(&data_monitor_lock);
		return TRUE;
	}

	aodv_dm_source_t *dm_source_needle = NULL;
	aodv_dm_source_t *dm_source_result = NULL;
	memcpy(dm_source_needle.l25_source, l25_source, sizeof(l25_source));
	DL_SEARCH(dm->l25_list, dm_source_needle, dm_source_result, dm_source_cmp);

	if(!dm_source_result) {
		dm_source_result = create_dm_source(l25_source, ts);
		DL_APPEND(dm_source_source->l25_list, dm_source_result);
		pthread_rwlock_unlock(&data_monitor_lock);
		return TRUE;
	}

	memcpy(dm_source_result->last_seen, ts, sizeof(struct timeval));
	pthread_rwlock_unlock(&data_monitor_lock);
	return TRUE;
}

