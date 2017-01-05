/*
 * Copyright (C) 2015-2016 Firetools Authors
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

#if 0
//https://gist.github.com/edufelipe/6108057
int check_wireless(const char* ifname, char* protocol) {
	int sock = -1;
	struct iwreq pwrq;
	memset(&pwrq, 0, sizeof(pwrq));
	strncpy(pwrq.ifr_name, ifname, IFNAMSIZ);

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		return 0;
	}

	if (ioctl(sock, SIOCGIWNAME, &pwrq) != -1) {
		if (protocol) strncpy(protocol, pwrq.u.name, IFNAMSIZ);
		close(sock);
		return 1;
	}

	close(sock);
	return 0;
}
#endif

// detect network
void detect_network() {
	struct ifaddrs *ifaddr, *ifa;
	int n;

	if (getifaddrs(&ifaddr) == -1)
		errExit("getifaddrs");

	/* Walk through linked list, maintaining head pointer so we
	   can free list later */

	for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
		if (ifa->ifa_addr == NULL)
			continue;

		int family = ifa->ifa_addr->sa_family;
		if (family != AF_INET)
			continue;
		
		// no loopback
		if (ifa->ifa_flags & IFF_LOOPBACK)
			continue;

		char *addr = inet_ntoa(((struct sockaddr_in *)ifa->ifa_addr)->sin_addr);

		printf("%-8s  %s\n", ifa->ifa_name, addr);
		// check /sys/class/net/NETDEVICE/wireless
	}

	freeifaddrs(ifaddr);

}
