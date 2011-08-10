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

#include "../config.h"
#include "aodv_cli.h"
#include "../database/aodv_database.h"
#include "../pipeline/aodv_pipeline.h"

// -------------------- Testing ------------------------------------------------------------

int cli_set_shortcut(struct cli_def* cli, char* command, char* argv[], int argc) {
    uint32_t mode;

    if(argc != 1 || sscanf(argv[0], "%" PRIu32 "", &mode) != 1 || (mode != 0 && mode != 1)) {
        cli_print(cli, "usage of %s command [0, 1]\n", command);
        return CLI_ERROR_ARG;
    }

    if(mode == 1) {
        dessert_info("use dest_only = true");
        dest_only = true;
    }
    else {
        dessert_info("use dest_only = false");
        dest_only = false;
    }

    return CLI_OK;
}


int cli_set_hello_size(struct cli_def* cli, char* command, char* argv[], int argc) {
    uint16_t min_size = sizeof(dessert_msg_t) + sizeof(struct ether_header) + 2;

    if(argc != 1) {
    label_out_usage:
        cli_print(cli, "usage %s [%" PRIu16 "..1500]\n", command, min_size);
        return CLI_ERROR;
    }

    uint16_t psize = (uint16_t) strtoul(argv[0], NULL, 10);

    if(psize < min_size || psize > 1500) {
        goto label_out_usage;
    }

    hello_size = psize;
    dessert_notice("setting HELLO size to %" PRIu16 "", hello_size);
    return CLI_OK;
}

int cli_set_hello_interval(struct cli_def* cli, char* command, char* argv[], int argc) {
    if(argc != 1) {
        cli_print(cli, "usage %s [interval]\n", command);
        return CLI_ERROR;
    }

    hello_interval = (uint16_t) strtoul(argv[0], NULL, 10);
    aodv_db_neighbor_table_reset();
    dessert_periodic_del(periodic_send_hello);
    struct timeval hello_interval_t;
    hello_interval_t.tv_sec = hello_interval / 1000;
    hello_interval_t.tv_usec = (hello_interval % 1000) * 1000;
    periodic_send_hello = dessert_periodic_add(aodv_periodic_send_hello, NULL, NULL, &hello_interval_t);
    dessert_notice("setting HELLO interval to %" PRIu16 "", hello_interval);
    return CLI_OK;
}

int cli_set_rreq_size(struct cli_def* cli, char* command, char* argv[], int argc) {
    uint16_t min_size = sizeof(dessert_msg_t) + sizeof(struct ether_header) + 2;

    if(argc != 1) {
    label_out_usage:
        cli_print(cli, "usage %s [%" PRIu16 "..1500]\n", command, min_size);
        return CLI_ERROR;
    }

    uint16_t psize = (uint16_t) strtoul(argv[0], NULL, 10);

    if(psize < min_size || psize > 1500) {
        goto label_out_usage;
    }

    rreq_size = psize;
    dessert_notice("setting RREQ size to %" PRIu16 "", rreq_size);
    return CLI_OK;
}

int cli_set_gossipp(struct cli_def* cli, char* command, char* argv[], int argc) {

    if(argc != 1) {
    label_out_usage:
        cli_print(cli, "usage %s [0.0..1.0]\n", command);
        return CLI_ERROR;
    }

    double psize = strtod(argv[0], NULL);

    if(psize < 0 || psize > 1) {
        goto label_out_usage;
    }

    gossipp = psize;
    dessert_notice("setting p for gossip to %lf", gossipp);
    return CLI_OK;
}

int cli_send_rreq(struct cli_def* cli, char* command, char* argv[], int argc) {

    if(argc != 2) {
        cli_print(cli, "usage of %s command [hardware address as XX:XX:XX:XX:XX:XX] [initial_metric]\n", command);
        return CLI_ERROR_ARG;
    }

    mac_addr host;
    int ok = dessert_parse_mac(argv[0], &host);

    if(ok != 0) {
        cli_print(cli, "usage of %s command [hardware address as XX:XX:XX:XX:XX:XX] [initial_metric]\n", command);
        return CLI_ERROR_ARG;
    }

    metric_t initial_metric = 0;
    initial_metric = atoi(argv[1]);

    cli_print(cli, MAC " -> using %" AODV_PRI_METRIC " as initial_metric\n", EXPLODE_ARRAY6(host), initial_metric);

    struct timeval ts;

    gettimeofday(&ts, NULL);

    aodv_send_rreq(host, &ts, NULL, initial_metric);

    return CLI_OK;
}

int cli_set_metric(struct cli_def* cli, char* command, char* argv[], int argc) {

    if(argc != 1) {
        cli_print(cli, "usage of %s command [metric (1=hop_count | 2=rssi)]\n", command);
        return CLI_ERROR_ARG;
    }

    char* metric_string = argv[0];

    int hop = strcmp(metric_string, "AODV_METRIC_HOP_COUNT");

    if(hop == 0) {
        metric_type = AODV_METRIC_HOP_COUNT;
    }

    int rssi = strcmp(metric_string, "AODV_METRIC_RSSI");

    if(rssi == 0) {
        metric_string = "AODV_METRIC_RSSI -> not implemented!  -> using AODV_METRIC_HOP_COUNT as fallback";
        metric_type = AODV_METRIC_HOP_COUNT;
    }

    int etx = strcmp(metric_string, "AODV_METRIC_ETX");

    if(etx == 0) {
        metric_string = "AODV_METRIC_ETX -> not implemented! -> using AODV_METRIC_HOP_COUNT as fallback";
        metric_type = AODV_METRIC_HOP_COUNT;
    }

    int ett = strcmp(metric_string, "AODV_METRIC_ETT");

    if(ett == 0) {
        metric_string = "AODV_METRIC_ETT -> not implemented! -> using AODV_METRIC_HOP_COUNT as fallback";
        metric_type = AODV_METRIC_HOP_COUNT;
    }

    cli_print(cli, "metric set to %s", metric_string);

    return CLI_OK;
}

int cli_show_metric(struct cli_def* cli, char* command, char* argv[], int argc) {

    char* metric_string = NULL;

    switch(metric_type) {
        case AODV_METRIC_HOP_COUNT: {
            metric_string = "AODV_METRIC_HOP_COUNT";
            break;
        }
        case AODV_METRIC_RSSI: {
            metric_string = "AODV_METRIC_RSSI -> not implemented!  -> using AODV_METRIC_HOP_COUNT as fallback";
            break;
        }
        case AODV_METRIC_ETX: {
            metric_string = "AODV_METRIC_ETX -> not implemented! -> using AODV_METRIC_HOP_COUNT as fallback";
            break;
        }
        case AODV_METRIC_ETT: {
            metric_string = "AODV_METRIC_ETT -> not implemented! -> using AODV_METRIC_HOP_COUNT as fallback";
            break;
        }
        default: {
            metric_string = "UNKNOWN METRIC -> you have some serious problems -> using AODV_METRIC_HOP_COUNT as fallback";
        }
    }

    cli_print(cli, "metric is set to %s", metric_string);
    return CLI_OK;
}

int cli_show_hello_size(struct cli_def* cli, char* command, char* argv[], int argc) {
    cli_print(cli, "HELLO size = %" PRIu16 " bytes\n", hello_size);
    return CLI_OK;
}

int cli_show_hello_interval(struct cli_def* cli, char* command, char* argv[], int argc) {
    cli_print(cli, "HELLO interval = %" PRIu16 " millisec\n", hello_interval);
    return CLI_OK;
}

int cli_show_rreq_size(struct cli_def* cli, char* command, char* argv[], int argc) {
    cli_print(cli, "RREQ size = %" PRIu16 " bytes\n", rreq_size);
    return CLI_OK;
}

int cli_show_rt(struct cli_def* cli, char* command, char* argv[], int argc) {
    char* rt_report;
    aodv_db_view_routing_table(&rt_report);
    cli_print(cli, "\n%s\n", rt_report);
    free(rt_report);
    return CLI_OK;
}

int cli_show_neighbor_timeslot(struct cli_def* cli, char* command, char* argv[], int argc) {
    char* report;
    aodv_db_neighbor_timeslot_report(&report);
    cli_print(cli, "\n%s\n", report);
    free(report);
    return CLI_OK;
}

int cli_show_packet_buffer_timeslot(struct cli_def* cli, char* command, char* argv[], int argc) {
    char* report;
    aodv_db_packet_buffer_timeslot_report(&report);
    cli_print(cli, "\n%s\n", report);
    free(report);
    return CLI_OK;
}
