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

#include "../config.h"
#include "aodv_pipeline.h"

int aodv_metric_do(metric_t* metric) {

    switch(metric_type) {
        case AODV_METRIC_HOP_COUNT: {
            (*metric)++;
            break;
        }
        case AODV_METRIC_RSSI: {
            dessert_crit("AODV_METRIC_RSSI -> not implemented! -> using AODV_METRIC_HOP_COUNT as fallback");
            (*metric)++; /* HOP_COUNT */
            break;
        }
        case AODV_METRIC_ETX: {
            dessert_crit("AODV_METRIC_ETX -> not implemented! -> using AODV_METRIC_HOP_COUNT as fallback");
            (*metric)++; /* HOP_COUNT */
            break;
        }
        case AODV_METRIC_ETT: {
            dessert_crit("AODV_METRIC_ETT -> not implemented! -> using AODV_METRIC_HOP_COUNT as fallback");
            (*metric)++; /* HOP_COUNT */
            break;
        }
        default: {
            dessert_crit("unknown metric=%" PRIu8 " -> using HOP_COUNT as fallback", metric_type);
            (*metric)++; /* HOP_COUNT */
            return false;
        }
    }

    return true;
}


