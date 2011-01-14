#include <uthash.h>
#include <stdio.h>

#include "aodv_broadcast_t.h"
#include "../../config.h"
#include "../timeslot.h"

typedef struct aodv_brcid_entry {
	u_int8_t 		shost_ether[ETH_ALEN];
	u_int32_t		brc_id;
	UT_hash_handle	hh;
} aodv_brcid_entry_t;

typedef struct aodv_brcid {
	aodv_brcid_entry_t*	entrys;
	timeslot_t*				ts;
} aodv_brcid_t;

aodv_brcid_t	brcid_table;

void purge_brcid_entry(struct timeval* timestamp, void* src_object, void* object) {
	aodv_brcid_entry_t* entry = object;
	HASH_DEL(brcid_table.entrys, entry);
	free(entry);
}

int aodv_db_brct_init() {
	brcid_table.entrys = NULL;
	struct timeval 	pdt; 								// PATH_DISCOVERY_TIME
	pdt.tv_sec = PATH_DESCOVERY_TIME / 1000;
	pdt.tv_usec = (PATH_DESCOVERY_TIME % 1000) * 1000;
	timeslot_create(&brcid_table.ts, &pdt, &brcid_table, purge_brcid_entry);
	return TRUE;
}


int aodv_db_brct_addid(u_int8_t shost_ether[ETH_ALEN], u_int32_t rreq_id, struct timeval* timestamp) {
	aodv_brcid_entry_t* entry;
	timeslot_purgeobjects(brcid_table.ts, timestamp);
	HASH_FIND(hh, brcid_table.entrys, shost_ether, ETH_ALEN, entry);
	if (entry == NULL) {
		entry = malloc(sizeof(aodv_brcid_entry_t));
		if (entry == NULL) return FALSE;
		memcpy(entry->shost_ether, shost_ether, ETH_ALEN);
		HASH_ADD_KEYPTR(hh, brcid_table.entrys, entry->shost_ether, ETH_ALEN, entry);
		entry->brc_id = rreq_id;
		timeslot_addobject(brcid_table.ts, timestamp, entry);
		return TRUE;
	}
	if (entry->brc_id < rreq_id) {
		entry->brc_id = rreq_id;
		timeslot_addobject(brcid_table.ts, timestamp, entry);
		return TRUE;
	}
	return FALSE;
}
