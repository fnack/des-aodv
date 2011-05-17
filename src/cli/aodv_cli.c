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

#include <dessert.h>
#include <string.h>
#include <time.h>

#include "aodv_cli.h"
#include "../database/aodv_database.h"
#include "../database/neighbor_table/nt.h"
#include "../config.h"
#include "../pipeline/aodv_pipeline.h"

// -------------------- config ------------------------------------------------------------


// -------------------- Testing ------------------------------------------------------------

int cli_set_hello_size(struct cli_def *cli, char *command, char *argv[], int argc) {
    uint16_t min_size = sizeof(dessert_msg_t) + sizeof(struct ether_header) + 2;

    if(argc != 1) {
        label_out_usage:
        cli_print(cli, "usage %s [%d..1500]\n", command, min_size);
        return CLI_ERROR;
    }

    uint16_t psize = (uint16_t) strtoul(argv[0], NULL, 10);
    if(psize < min_size || psize > 1500) goto label_out_usage;
    hello_size = psize;
    dessert_notice("setting HELLO size to %d", hello_size);
    return CLI_OK;
}

int cli_set_hello_interval(struct cli_def *cli, char *command, char *argv[], int argc) {
	if(argc != 1) {
		cli_print(cli, "usage %s [interval]\n", command);
		return CLI_ERROR;
	}

	hello_interval = (uint16_t) strtoul(argv[0], NULL, 10);
	int db_nt_init();
	dessert_periodic_del(periodic_send_hello);
	struct timeval hello_interval_t;
	hello_interval_t.tv_sec = hello_interval / 1000;
	hello_interval_t.tv_usec = (hello_interval % 1000) * 1000;
	periodic_send_hello = dessert_periodic_add(aodv_periodic_send_hello, NULL, NULL, &hello_interval_t);
	dessert_notice("setting HELLO interval to %d", hello_interval);
    return CLI_OK;
}

int cli_set_rreq_size(struct cli_def *cli, char *command, char *argv[], int argc) {
    uint16_t min_size = sizeof(dessert_msg_t) + sizeof(struct ether_header) + 2;

    if(argc != 1) {
        label_out_usage:
        cli_print(cli, "usage %s [%d..1500]\n", command, min_size);
        return CLI_ERROR;
    }

    uint16_t psize = (uint16_t) strtoul(argv[0], NULL, 10);
    if(psize < min_size || psize > 1500) goto label_out_usage;
    rreq_size = psize;
    dessert_notice("setting RREQ size to %d", rreq_size);
    return CLI_OK;
}

int cli_send_rreq(struct cli_def* cli, char* command, char* argv[], int argc) {
	u_int8_t dhost_hwaddr[ETHER_ADDR_LEN];
	if (argc >= 1) {
		if(sscanf(argv[0], MAC, EXPLODE_ARRAY6(&dhost_hwaddr)) == 6) {
			// args are correct -> send rreq
			struct timeval ts;
			gettimeofday(&ts, NULL);
			aodv_send_rreq(dhost_hwaddr, &ts, TTL_START);
			return CLI_OK;
		}
	}
	// args are not correct
	cli_print(cli, "usage of %s command [hardware address as XX:XX:XX:XX:XX:XX]\n", command);
	return CLI_ERROR_ARG;
}

int cli_show_hello_size(struct cli_def *cli, char *command, char *argv[], int argc) {
    cli_print(cli, "HELLO size = %d bytes\n", hello_size);
    return CLI_OK;
}

int cli_show_hello_interval(struct cli_def *cli, char *command, char *argv[], int argc) {
	cli_print(cli, "HELLO interval = %d millisec\n", hello_interval);
    return CLI_OK;
}

int cli_show_rreq_size(struct cli_def *cli, char *command, char *argv[], int argc) {
    cli_print(cli, "RREQ size = %d bytes\n", rreq_size);
    return CLI_OK;
}

int cli_show_rt(struct cli_def* cli, char* command, char* argv[], int argc){
	char* rt_report;
	aodv_db_view_routing_table(&rt_report);
	cli_print(cli, "\n%s\n", rt_report);
	free(rt_report);
	return CLI_OK;
}

// -------------------- common cli functions ----------------------------------------------

int cli_set_routing_log(struct cli_def *cli, char *command, char *argv[], int argc) {
	routing_log_file = malloc(strlen(argv[0]));
	strcpy(routing_log_file, argv[0]);
	FILE* f = fopen(routing_log_file, "a+");
	time_t lt;
	lt = time(NULL);
	fprintf(f, "\n--- %s\n", ctime(&lt));
	fclose(f);
	dessert_info("logging routing data at file %s", routing_log_file);
	return CLI_OK;
}
