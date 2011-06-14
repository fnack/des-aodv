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

#ifndef AODV_PIPELINE
#define AODV_PIPELINE

#ifdef ANDROID
#include <linux/if_ether.h>
#endif

#include <dessert.h>
#include "../config.h"

extern pthread_rwlock_t pp_rwlock;
extern u_int32_t broadcast_id;

/**
 * Unknown sequence number
 */
#define AODV_FLAGS_RREQ_U 			1 << 11

/**
 * Destination only flag
 */
#define AODV_FLAGS_RREQ_D			1 << 12
/**
 * Aknowledgement required
 */
#define AODV_FLAGS_RREP_A			1 << 6

/**
 * Not delete flag of RERR
 */
#define AODV_FLAGS_RERR_N			1 << 7


/** RREQ - Route Request Message */
struct aodv_msg_rreq {
	/**
	 * flags format: J R G D U 0 0 0   0 0 0 0 0 0 0 0
	 * J - Join flag; reserved for multicast //outdated
	 * R - Repair flag; reserved for multicast //outdated
	 * G - Gratuitous RREP flag; indicates whether a gratuitous
	 * 	   RREP should be unicast to the node specified in the ether_dhost
	 * D - Destination only flag; indicates only the destiantion may respond to this RREQ
	 * U - Unknown sequence number; indicates the destination sequence number is unknown
	 */
	u_int16_t		flags;
	/** The number of hops from the originator to the node habdling the request */
	u_int8_t		hop_count;
	/**
	 * Destination Sequence Number;
	 * The latest sequence number received in the past by the originator for any
	 * route towards the destination.
	 */
	u_int32_t		seq_num_dest;
	/**
	 * Originator Sequence Number;
	 * The current sequence number to be used in the route entry pointing towards
	 * the originator of the route request.
	 */
	u_int32_t		seq_num_src;
} __attribute__ ((__packed__));

/** RREP - Route Reply Message */
struct aodv_msg_rrep {
	/**
	 * flags format: R A 0 0 0 0 0 0
	 * R - repair flag;
	 * A - acknowledgement required;
	 */
	u_int8_t		flags;
	/** not used */
	u_int8_t		prefix_size;
	/**  Hop Count: The number of hops from the originator to destination */
	u_int8_t		hop_count;
	/** Destination sequence number */
	u_int32_t		seq_num_dest;
	/**
	 * LifeTime:
	 * The time in millisecond for which nodes receiving the RREP consider the
	 * route to be valid.
	 */
	time_t			lifetime;
} __attribute__ ((__packed__));

/** RERR - Route Error Message */
struct aodv_msg_rerr {
	/**
	 * flags format: N 0 0 0 0 0 0 0
	 * N - No delete flag; set when a node has performed a local repair of a link
	 */
	u_int8_t		flags;
	/** The number of interfaces of the RERR last hop */
	u_int8_t 		iface_addr_count;
	/** all of mesh interfaces of current host listed i this message */
	u_int8_t		ifaces[ETH_ALEN * MAX_MESH_IFACES_COUNT];
} __attribute__ ((__packed__));


/** RWARN - Route Warn Message */
struct aodv_msg_rwarn {
	u_int8_t		source_mobility;
	u_int16_t		seq_num;
} __attribute__ ((__packed__));

/** HELLO - Hello Message */
struct aodv_msg_hello {
} __attribute__ ((__packed__));

struct aodv_msg_broadcast {
	/**
	 * Broadcast ID;
	 * A sequence number uniqiely identifying the broadcast packet (RREQ or simple packet)
	 * in combination with ether_shost
	 */
	u_int32_t		id;
} __attribute__ ((__packed__));

/**
 * Struct for routing log sequence number
 */
struct rl_seq {
	u_int32_t 	seq_num;
	u_int8_t	hop_count;
} __attribute__ ((__packed__));

typedef struct _onlb_dest_list_element {
	u_int8_t 							dhost_ether[ETH_ALEN];
	struct _onlb_dest_list_element		*prev, *next;
} _onlb_element_t;

// ------------- helper -------------------------------------------------------

extern pthread_rwlock_t rlflock;
extern pthread_rwlock_t rlseqlock;

void rlfile_log(const u_int8_t src_addr[ETH_ALEN], const u_int8_t dest_addr[ETH_ALEN],
		const u_int32_t seq_num, const u_int8_t hop_count, const u_int8_t in_iface[ETH_ALEN],
		const u_int8_t out_iface[ETH_ALEN], const u_int8_t next_hop_addr[ETH_ALEN]);

// ------------- pipeline -----------------------------------------------------

int aodv_handle_hello(dessert_msg_t* msg, size_t len,
		dessert_msg_proc_t *proc, const dessert_meshif_t *iface, dessert_frameid_t id);

int aodv_handle_rreq(dessert_msg_t* msg, size_t len,
		dessert_msg_proc_t *proc, const dessert_meshif_t *iface, dessert_frameid_t id);

int aodv_handle_rwarn(dessert_msg_t* msg, size_t len,
		dessert_msg_proc_t *proc, const dessert_meshif_t *iface, dessert_frameid_t id);

int aodv_handle_rerr(dessert_msg_t* msg, size_t len,
		dessert_msg_proc_t *proc, const dessert_meshif_t *iface, dessert_frameid_t id);

int aodv_handle_rrep(dessert_msg_t* msg, size_t len,
		dessert_msg_proc_t *proc, const dessert_meshif_t *iface, dessert_frameid_t id);

int aodv_fwd2dest(dessert_msg_t* msg, size_t len,
		dessert_msg_proc_t *proc, const dessert_meshif_t *iface, dessert_frameid_t id);

int aodv_monitor_last_hops(dessert_msg_t* msg, size_t len,
		dessert_msg_proc_t *proc, const dessert_meshif_t *iface, dessert_frameid_t id);

/**
 * Encapsulate packets as dessert_msg,
 * set NEXT HOP if known and send via AODV routing protocol
 */
int aodv_sys2rp (dessert_msg_t *msg, size_t len, dessert_msg_proc_t *proc,
		dessert_sysif_t *sysif, dessert_frameid_t id);

/** forward packets received via AODV to tun interface */
int rp2sys(dessert_msg_t* msg, size_t len,
		dessert_msg_proc_t *proc, const dessert_meshif_t *iface, dessert_frameid_t id);

/** drop errors (drop corrupt packets, packets from myself and etc...)*/
int aodv_drop_errors(dessert_msg_t* msg, size_t len,
		dessert_msg_proc_t *proc, const dessert_meshif_t *iface, dessert_frameid_t id);

// ------------------------------ periodic ----------------------------------------------------

int aodv_periodic_send_hello(void *data, struct timeval *scheduled, struct timeval *interval);

/** clean up database from old entrys */
int aodv_periodic_cleanup_database(void *data, struct timeval *scheduled, struct timeval *interval);

dessert_msg_t* aodv_create_rerr(_onlb_element_t** head, u_int16_t count);

int aodv_periodic_scexecute(void *data, struct timeval *scheduled, struct timeval *interval);

int aodv_schedule_monitor_signal_strength(void *data, struct timeval *scheduled, struct timeval *interval);
// ------------------------------ helper ------------------------------------------------------

void aodv_send_rreq(u_int8_t dhost_ether[ETH_ALEN], struct timeval* ts, u_int8_t ttl);

void aodv_send_rwarn(u_int8_t rwarn_dest[ETH_ALEN], u_int8_t rwarn_next_hop[ETH_ALEN], dessert_meshif_t *iface);

#endif
