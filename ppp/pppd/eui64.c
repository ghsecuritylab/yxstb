/*
 * eui64.c - EUI64 routines for IPv6CP.
 *
 * Copyright (c) 1999 Tommi Komulainen.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The name(s) of the authors of this software must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission.
 *
 * 4. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Tommi Komulainen
 *     <Tommi.Komulainen@iki.fi>".
 *
 * THE AUTHORS OF THIS SOFTWARE DISCLAIM ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id: eui64.c,v 1.6 2002/12/04 23:03:32 paulus Exp $
 */

#define RCSID	"$Id: eui64.c,v 1.6 2002/12/04 23:03:32 paulus Exp $"

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if __GLIBC__ >= 2
#include <net/if.h>
#else
#include <linux/if.h>
#endif

#include "pppd.h"

static const char rcsid[] = RCSID;

/*
 * eui64_ntoa - Make an ascii representation of an interface identifier
 */
char *
eui64_ntoa(e)
    eui64_t e;
{
    static char buf[32];

    snprintf(buf, 32, "%02x%02x:%02x%02x:%02x%02x:%02x%02x",
	     e.e8[0], e.e8[1], e.e8[2], e.e8[3], 
	     e.e8[4], e.e8[5], e.e8[6], e.e8[7]);
    return buf;
}

#ifdef INET6
/*
 * ether_to_eui64 - Convert 48-bit Ethernet address into 64-bit EUI
 *
 * convert the 48-bit MAC address of eth0 into EUI 64. caller also assumes
 * that the system has a properly configured Ethernet interface for this
 * function to return non-zero.
 */
int
ether_to_eui64(eui64_t *p_eui64)
{
    struct ifreq ifr;
    int skfd;
    const unsigned char *ptr;

    skfd = socket(PF_INET6, SOCK_DGRAM, 0);
    if(skfd == -1)
    {
        warn("could not open IPv6 socket");
        return 0;
    }

    strcpy(ifr.ifr_name, "eth0");
    if(ioctl(skfd, SIOCGIFHWADDR, &ifr) < 0)
    {
        close(skfd);
        warn("could not obtain hardware address for eth0");
        return 0;
    }
    close(skfd);

    /*
     * And convert the EUI-48 into EUI-64, per RFC 2472 [sec 4.1]
     */
    ptr = ifr.ifr_hwaddr.sa_data;
    p_eui64->e8[0] = ptr[0] | 0x02;
    p_eui64->e8[1] = ptr[1];
    p_eui64->e8[2] = ptr[2];
    p_eui64->e8[3] = 0xFF;
    p_eui64->e8[4] = 0xFE;
    p_eui64->e8[5] = ptr[3];
    p_eui64->e8[6] = ptr[4];
    p_eui64->e8[7] = ptr[5];

    return 1;
}
#endif

