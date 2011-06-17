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
#include <pthread.h>
#include <linux/if_ether.h>
#include "../../config.h"

typedef struct aodv_dm_source {
	struct aodv_dm_source     *next;
	const u_int8_t            l25_source[ETH_ALEN];
	struct timeval            *last_seen;
	struct timeval            *last_warn;
	struct aodv_dm_source     *prev;
} aodv_dm_source_t;

typedef struct aodv_dm {
	struct aodv_dm            *next;
	const u_int8_t            l2_source[ETH_ALEN]; // ID
	u_int8_t                  max_rssi;
	u_int8_t                  last_rssi;
	const dessert_meshif_t*   iface;
	aodv_dm_source_t*   l25_list;
	struct aodv_dm            *prev;
} aodv_dm_t;


int aodv_db_data_monitor_signal_strength_update(struct timeval *ts);

aodv_dm_t* aodv_db_data_monitor_pop();

int aodv_db_data_monitor_capture_packet(u_int8_t l2_source[ETH_ALEN], u_int8_t l25_source[ETH_ALEN], const dessert_meshif_t* iface, struct timeval *ts);
