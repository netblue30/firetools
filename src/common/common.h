/*
 * Copyright (C) 2015-2018 Firetools Authors
 *
 * This file is part of firetools project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef COMMON_H
#define COMMON_H
//#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define errExit(msg)    do { char msgout[500]; sprintf(msgout, "Error %s:%s(%d)", msg, __FUNCTION__, __LINE__); perror(msgout); exit(1);} while (0)

// macro to print ip addresses in a printf statement
#define PRINT_IP(A) \
((int) (((A) >> 24) & 0xFF)),  ((int) (((A) >> 16) & 0xFF)), ((int) (((A) >> 8) & 0xFF)), ((int) ( (A) & 0xFF))

// macro to print a mac addresses in a printf statement
#define PRINT_MAC(A) \
((unsigned) (*(A)) & 0xff), ((unsigned) (*((A) + 1) & 0xff)), ((unsigned) (*((A) + 2) & 0xff)), \
((unsigned) (*((A) + 3)) & 0xff), ((unsigned) (*((A) + 4) & 0xff)), ((unsigned) (*((A) + 5)) & 0xff)

// convert a network mask to the number of bits
static inline uint8_t mask2bits(uint32_t mask) {
	uint32_t tmp = 0x80000000;
	int i;
	uint8_t rv = 0;

	for (i = 0; i < 32; i++, tmp >>= 1) {
		if (tmp & mask)
			rv++;
		else
			break;
	}
	return rv;
}

// read an IPv4 address and convert it to uint32_t
static inline int atoip(const char *str, uint32_t *ip) {
	unsigned a, b, c, d;

	// allow only numbers
	const char *ptr = str;
	while (*ptr != '\0') {
		if (isdigit(*ptr) || *ptr == '.')
			;
		else
			return 1;
		ptr++;
	}

	if (sscanf(str, "%u.%u.%u.%u", &a, &b, &c, &d) != 4 || a > 255 || b > 255 || c > 255 || d > 255)
		return 1;
		
	*ip = a * 0x1000000 + b * 0x10000 + c * 0x100 + d;
	return 0;
}

// verify ip address is in the ifip/ifmask network range
static inline char *in_netrange(uint32_t ip, uint32_t ifip, uint32_t ifmask) {
	if ((ip & ifmask) != (ifip & ifmask))
		return (char *) "Error: the IP address is not in the interface range\n";
	else if ((ip & ifmask) == ip)
		return (char *) "Error: the IP address is a network address\n";
	else if ((ip | ~ifmask) == ip)
		return (char *) "Error: the IP address is a network address\n";
	return NULL;
}

#endif
