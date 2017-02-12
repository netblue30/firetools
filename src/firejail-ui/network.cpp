/*
 * Copyright (C) 2015-2017 Firetools Authors
 *
 * This file is part of firetools project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public  as published by
 * the Free Software Foundation; either version 2 of the , or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public  for more details.
 *
 * You should have received a copy of the GNU General Public  along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "firejail_ui.h"
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if.h>
#include <linux/if_link.h>
#include <linux/wireless.h>
#include <sys/ioctl.h>

#define BUFSIZE 1024

// return default gateway for the system in host format
uint32_t network_get_defaultgw() {
	FILE *fp = fopen("/proc/self/net/route", "r");
	if (!fp)
		// probably we are dealing with a GrSecurity system
		return 0; // attempt error recovery
	
	char buf[BUFSIZE];
	uint32_t retval = 0;
	while (fgets(buf, BUFSIZE, fp)) {
		if (strncmp(buf, "Iface", 5) == 0)
			continue;
		
		char *ptr = buf;
		while (*ptr != ' ' && *ptr != '\t')
			ptr++;
		while (*ptr == ' ' || *ptr == '\t')
			ptr++;
			
		unsigned dest;
		unsigned gw;
		int rv = sscanf(ptr, "%x %x", &dest, &gw);
		if (rv == 2 && dest == 0) {
			retval = ntohl(gw);
			break;
		}
	}

	fclose(fp);
	return retval;
}



// return 1 if the interface is a wireless interface
int check_wireless(const char* ifname, char* protocol) {
	int sock = -1;
	struct iwreq pwrq;
	memset(&pwrq, 0, sizeof(pwrq));
	strncpy(pwrq.ifr_name, ifname, IFNAMSIZ);

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("sockqet");
		return 0;
	}

	if (ioctl(sock, SIOCGIWNAME, &pwrq) != -1) {
		if (protocol)
			strncpy(protocol, pwrq.u.name, IFNAMSIZ);
		close(sock);
		return 1;
	}

	close(sock);
	return 0;
}

// detect network
const char *detect_network() {
	struct ifaddrs *ifaddr, *ifa;

	if (getifaddrs(&ifaddr) == -1)
		errExit("getifaddrs");
		
	// find the default gateway
	uint32_t gw = network_get_defaultgw();
	printf("default gateway detected: %d.%d.%d.%d\n", PRINT_IP(gw));
	if (gw == 0) {
		fprintf(stderr, "Warning: cannot find the default gateway. Networking namespace is disabled.\n");
		return "";
	}
	
	// Walk through linked list, maintaining head pointer so we can free list later
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL)
			continue;

		int family = ifa->ifa_addr->sa_family;
		if (family != AF_INET)
			continue;
		
		// no loopback
		if (ifa->ifa_flags & IFF_LOOPBACK)
			continue;

		// interface not running
		if ((ifa->ifa_flags & (IFF_UP | IFF_RUNNING)) != (IFF_UP | IFF_RUNNING))
			continue;
		
		// no wireless
		if (check_wireless(ifa->ifa_name, NULL))
			continue;

		uint32_t if_addr = ntohl(((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr);
		uint32_t if_mask = ntohl(((struct sockaddr_in *)ifa->ifa_netmask)->sin_addr.s_addr);
		printf("network interface: %s %d.%d.%d.%d %d.%d.%d.%d\n", 
			ifa->ifa_name, PRINT_IP(if_addr), PRINT_IP(if_mask));

		// check default gateway is resolved on this interface
		if (in_netrange(gw, if_addr, if_mask) == NULL) {
			char *ifname = strdup(ifa->ifa_name);
			if (!ifname)
				errExit("strdup");
			freeifaddrs(ifaddr);
			return ifname;
		}
	}
	fprintf(stderr, "Warning: no suitable interface detected for network namespace.\n");
	freeifaddrs(ifaddr);
	return "";
}
