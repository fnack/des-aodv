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

#ifndef AODV_CONFIG
#define AODV_CONFIG

#include <dessert.h>

enum aodv_bool {TRUE = 1, FALSE = 0};

#define RREQ_RETRIES				4
#define RREQ_RATELIMIT				16
#define TTL_START					2
#define TTL_INCREMENT				2
#define TTL_THRESHOLD				7

#define ACTIVE_ROUTE_TIMEOUT		3000 	// milliseconds
#define ALLOWED_HELLO_LOST			7
#define NODE_TRAVERSAL_TIME			2 		// milliseconds
#define NET_DIAMETER				6
#define NET_TRAVERSAL_TIME			(2 * NODE_TRAVERSAL_TIME * NET_DIAMETER)
#define BLACKLIST_TIMEOUT			(RREQ_RETRIES * NET_TRAVERSAL_TIME)
#define MY_ROUTE_TIMEOUT			(2 * ACTIVE_ROUTE_TIMEOUT)
#define PATH_DESCOVERY_TIME			(2 * NET_TRAVERSAL_TIME)
#define RERR_RATELIMIT				16

#define RREQ_EXT_TYPE				DESSERT_EXT_USER
#define RREP_EXT_TYPE				(DESSERT_EXT_USER + 1)
#define RERR_EXT_TYPE				(DESSERT_EXT_USER + 2)
#define RERRDL_EXT_TYPE				(DESSERT_EXT_USER + 3)
#define HELLO_EXT_TYPE				(DESSERT_EXT_USER + 4)
#define BROADCAST_EXT_TYPE			(DESSERT_EXT_USER + 5)

#define FIFO_BUFFER_MAX_ENTRY_SIZE	128 	// maximal packet count that can be stored in FIFO for one destination
#define DB_CLEANUP_INTERVAL			NET_TRAVERSAL_TIME
#define BUFFER_SENDOUT_DELAY		10
#define SCHEDULE_CHECK_INTERVAL		30 		// milliseconds

/**
 * Schedule type = send out packets from FIFO puffer for
 * destination with ether_addr
 */
#define AODV_SC_SEND_OUT_PACKET		1

/**
 * Schedule type = repeat RREQ with ttl *=2
 */
#define AODV_SC_REPEAT_RREQ			2

/**
 * Schedule type = send out route error for given next hop
 */
#define AODV_SC_SEND_OUT_RERR		3
#define AODV_SC_SEND_OUT_RWARN		4
#define HELLO_SIZE					128
#define HELLO_INTERVAL				2000 	// milliseconds
#define RREQ_SIZE					128

#define SIGNAL_STRENGTH_BLACK_ZONE            -90 //dbm
#define SIGNAL_STRENGTH_GREY_ZONE             -80 //dbm
#define SIGNAL_STRENGTH_THRESHOLD              20 //dbm

// --- Database Flags
#define AODV_FLAGS_ROUTE_INVALID 	1
#define AODV_FLAGS_NEXT_HOP_UNKNOWN	(1 << 1)
#define MAX_MESH_IFACES_COUNT		8

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

extern dessert_periodic_t *			periodic_send_hello;
extern char*						routing_log_file;
extern int 							hello_size;
extern int 							hello_interval;
extern int 							rreq_size;

#endif
