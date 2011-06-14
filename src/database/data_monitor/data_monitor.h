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

#include <linux/if_ether.h>

int aodv_db_data_monitor_signal_strength_update(struct timeval ts);

aodv_dm_t* aodv_db_data_monitor_pop();

int aodv_db_data_monitor_capture_packet(u_int8_t l2_source[ETH_ALEN], u_int8_t l25_source[ETH_ALEN], const dessert_meshif_t* iface, struct timeval ts);

#endif
