/*
 * From: $OpenBSD: /usr/src/sbin/ifconfig/ifconfig.c,v 1.311 2015/12/10
 * 17:26:59 mmcc Exp $
 */


#include <sys/types.h>
#include <sys/ioctl.h>

#include <sys/socket.h>
#include <sys/syslimits.h>
#include <net/route.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet6/in6_var.h>
#include <netinet6/nd6.h>
#include <arpa/inet.h>
#include <net/if_pflow.h>

#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "externs.h"

int
pflow_addr(const char *val, struct sockaddr_storage *ss)
{
	struct addrinfo hints, *res0;
	int error, flag;
	char *cp, *ip, *port, buf[HOST_NAME_MAX + 1 + sizeof(":65535")];

	if (strlcpy(buf, val, sizeof(buf)) >= sizeof(buf)) {
		printf("%% pflow_addr: bad value: (%s)\n", val);
		return(1);
	}
	port = NULL;
	cp = buf;
	if (*cp == '[')
		flag = 1;
	else
		flag = 0;

	for (; *cp; ++cp) {
		if (*cp == ']' && *(cp + 1) == ':' && flag) {
			*cp = '\0';
			*(cp + 1) = '\0';
			port = cp + 2;
			break;
		}
		if (*cp == ']' && *(cp + 1) == '\0' && flag) {
			*cp = '\0';
			port = NULL;
			break;
		}
		if (*cp == ':' && !flag) {
			*cp = '\0';
			port = cp + 1;
			break;
		}
	}

	ip = buf;
	if (flag)
		ip++;

	bzero(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;	/* dummy */

	if ((error = getaddrinfo(ip, port, &hints, &res0)) != 0) {
		printf("%% pflow_addr: error in parsing address string: %s\n",
		       gai_strerror(error));
		return(1);
	}
	memcpy(ss, res0->ai_addr, res0->ai_addr->sa_len);
	freeaddrinfo(res0);
	return(0);
}

int
pflow_status(int type, int ifs, char *ifname, char *result)
{
	struct ifreq ifr;
	struct pflowreq preq;
	struct sockaddr_in *sin;
	struct sockaddr_in6 *sin6;
	int error;
	char buf[INET6_ADDRSTRLEN];

	bzero(&ifr, sizeof(struct ifreq));
	bzero(&preq, sizeof(struct pflowreq));
	ifr.ifr_data = (caddr_t) & preq;
	strlcpy(ifr.ifr_name, ifname, IFNAMSIZ);

	if (ioctl(ifs, SIOCGETPFLOW, (caddr_t) & ifr) == -1)
		return(1);

	if (type == PFLOW_SENDER) {
		if (preq.flowsrc.ss_family == AF_INET ||
		    preq.flowsrc.ss_family == AF_INET6) {
			error = getnameinfo((struct sockaddr *) & preq.flowsrc,
			    preq.flowsrc.ss_len, buf, sizeof(buf), NULL, 0,
			    NI_NUMERICHOST);
			if (error) {
				printf("%% pflow_status: getnameinfo/0: %s\n",
				    gai_strerror(error));
				return(1);
			}
		}
		switch (preq.flowsrc.ss_family) {
		case AF_INET:
			sin = (struct sockaddr_in *) & preq.flowsrc;
			if (sin->sin_addr.s_addr == INADDR_ANY)
				return(1);
			strlcpy(result, buf, INET6_ADDRSTRLEN);
			if (sin->sin_port != 0) {
				snprintf(buf, INET6_ADDRSTRLEN, ":%u",
				    ntohs(sin->sin_port));
				strlcat(result, buf, INET6_ADDRSTRLEN);
			}
			break;
		case AF_INET6:
			sin6 = (struct sockaddr_in6 *) & preq.flowsrc;
			if (IN6_IS_ADDR_UNSPECIFIED(&sin6->sin6_addr))
				return(1);
			strlcpy(result, buf, INET6_ADDRSTRLEN);
			if (sin6->sin6_port != 0) {
				snprintf(buf, INET6_ADDRSTRLEN, ":%u",
				    ntohs(sin6->sin6_port));
				strlcat(result, buf, INET6_ADDRSTRLEN);
			}
			break;
		default:
			return(1);
		}
		return(0);
	}
	if (type == PFLOW_RECEIVER) {
		if (preq.flowdst.ss_family == AF_INET ||
		    preq.flowdst.ss_family == AF_INET6) {
			error = getnameinfo((struct sockaddr *) & preq.flowdst,
			    preq.flowdst.ss_len, buf, sizeof(buf), NULL, 0,
			    NI_NUMERICHOST);
			if (error) {
				printf("%% pflow_status: getnameinfo/1: %s\n",
				    gai_strerror(error));
				return(1);
			}
		}
		switch (preq.flowdst.ss_family) {
		case AF_INET:
			sin = (struct sockaddr_in *) & preq.flowdst;
			if (sin->sin_addr.s_addr == INADDR_ANY)
				return(1);
			snprintf(result, INET6_ADDRSTRLEN, "%s", buf);
			if (sin->sin_port != 0) {
				snprintf(buf, INET6_ADDRSTRLEN, ":%u",
				    ntohs(sin->sin_port));
				strlcat(result, buf, INET6_ADDRSTRLEN);
			}
			break;
		case AF_INET6:
			sin6 = (struct sockaddr_in6 *) & preq.flowdst;
			if (IN6_IS_ADDR_UNSPECIFIED(&sin6->sin6_addr))
				return(1);
			snprintf(result, INET6_ADDRSTRLEN, "%s", buf);
			if (sin6->sin6_port != 0) {
				snprintf(buf, sizeof(buf), ":%u",
				    ntohs(sin6->sin6_port));
				strlcat(result, buf, INET6_ADDRSTRLEN);
			}
			break;
		default:
			return(1);
		}	
		return(0);
	}
	if (type == PFLOW_VERSION) {
		snprintf(result, INET6_ADDRSTRLEN, "%d", preq.version);
		return(0);
	}
	return(1);
}
