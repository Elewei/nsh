
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/limits.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <net/if_trunk.h>
#include "externs.h"

struct trunk_protos tpr[] = TRUNK_PROTOS;

int
inttrunkport(char *ifname, int ifs, int argc, char **argv)
{
	struct trunk_reqport rp;
	int set, i;

	if (NO_ARG(argv[0])) {
		set = 0;
		argc--;
		argv++;
	} else
		set = 1;

	argc--;
	argv++;

	if ((!set && argc < 1) || (set && argc < 1)) {
		printf("%% trunkport <ifname> [ifname ...]\n");
		printf("%% no trunkport <ifname> [ifname ...]\n");
		return(0);   
	}

	bzero(&rp, sizeof(rp));
	strlcpy(rp.rp_ifname, ifname, sizeof(rp.rp_ifname));

	for (i = 0; i < argc; i++) {
		if (set) {  
			strlcpy(rp.rp_portname, argv[i],
			    sizeof(rp.rp_portname));
			if (ioctl(ifs, SIOCSTRUNKPORT, &rp) < 0) {
				switch(errno) {
				case ENOTTY:
					printf("%% %s not trunk\n",ifname);
					break;
				case EBUSY:
					printf("%% Failed (port %s already"
					    " assigned to a trunk group)\n",
					    argv[i]);
					break;
				case ENETDOWN:
					printf("%% Failed (port %s is "
					    "shutdown)\n", argv[i]);
					break;
				default:
					printf("%% inttrunkport:"
					    " SIOCSTRUNKPORT: %s\n",
				    strerror(errno));
				}
			}
		} else {
			strlcpy(rp.rp_portname, argv[i],
			    sizeof(rp.rp_portname));
			if (ioctl(ifs, SIOCSTRUNKDELPORT, &rp) < 0) {
				switch(errno) {
				case ENOTTY:
					printf("%% %s not trunk\n",ifname);
					break;
				case ENOENT:
					printf("%% Port %s not part of %s\n",
					    argv[i], ifname);
					break;
				default:
					printf("%% inttrunkport:"
					    " SIOCSTRUNKDELPORT: %s\n,",
				    strerror(errno));
				}
				return 1;
			}
		}
	}
	return 0;
}

int
inttrunkproto(char *ifname, int ifs, int argc, char **argv)
{
	int i, set;
	struct trunk_reqall ra;

	if (NO_ARG(argv[0])) {
		set = 0;
		argc--;
		argv++;
	} else
		set = 1;

	argc--;
	argv++;

	if ((!set && argc != 1) || (set && argc != 1)) {
                printf("%% trunkproto <proto>\n");
                printf("%% no trunkproto\n");
                return(0);
        }

	bzero(&ra, sizeof(ra));
	strlcpy(ra.ra_ifname, ifname, sizeof(ra.ra_ifname));

	if (set) {
		for (i = 0; i < TRUNK_PROTO_MAX; ++i) {
			if (isprefix(argv[0], (char *)tpr[i].tpr_name)) {
				ra.ra_proto = tpr[i].tpr_proto;
				break;
			}
		}
		if (i == TRUNK_PROTO_MAX) {
			printf("%% trunkproto <");
			for (i = 0; i < TRUNK_PROTO_MAX; ++i)
				printf("%s%s", i == 0 ? "" : "|", tpr[i].tpr_name);
			printf(">\n");
			return(0);
		}
	} else
		ra.ra_proto = TRUNK_PROTO_DEFAULT;

	if (ioctl(ifs, SIOCSTRUNK, &ra) != 0) {
		switch(errno) {
		case ENOTTY:
			printf("%% %s not trunk\n", ifname);
			break;
		default:
			printf("%% inttrunkproto: SIOCSTRUNK: %s\n", strerror(errno));
		}
		return 1;
	}

	return 0;
}

int
conf_trunk(FILE *output, int ifs, char *ifname)
{
	struct trunk_reqport rpbuf[TRUNK_MAX_PORTS];
        struct trunk_reqall ra;
        int i;

        bzero(&ra, sizeof(ra));
	for (i = 0; i < TRUNK_MAX_PORTS; i++)
		bzero(&rpbuf[i], sizeof(struct trunk_reqport));

        strlcpy(ra.ra_ifname, ifname, sizeof(ra.ra_ifname));
        ra.ra_size = sizeof(rpbuf);
	ra.ra_port = rpbuf;

        if (ioctl(ifs, SIOCGTRUNK, (caddr_t)&ra) == 0) {
		int pntd = 0;

		for (i = 0; i < nitems(tpr); i++)
			if (ra.ra_proto == tpr[i].tpr_proto)
				break;
		if (tpr[i].tpr_proto != TRUNK_PROTO_DEFAULT)
			fprintf(output," trunkproto %s\n", tpr[i].tpr_name);

                for (i = 0; i < ra.ra_ports; ++i)
			if(rpbuf[i].rp_portname[0] != '\0') {
				fprintf(output, " %s%s", pntd ? "" : "trunkport ",
				    rpbuf[i].rp_portname);
				pntd = 1;
			}
		if (pntd) {
			fprintf(output, "\n");
		}
        } else return (1);

	return (0);
}

void
show_trunk(int ifs, char *ifname)
{
	struct trunk_reqport rp, rpbuf[TRUNK_MAX_PORTS];
	struct trunk_reqall ra;
	int i;

	bzero(&ra, sizeof(ra));
	bzero(&rp, sizeof(rp));
	strlcpy(rp.rp_ifname, ifname, sizeof(rp.rp_ifname));
	strlcpy(rp.rp_portname, ifname, sizeof(rp.rp_portname));

	if (ioctl(ifs, SIOCGTRUNKPORT, (caddr_t)&rp) == 0) {
		printf("  Trunk parent %s ", rp.rp_ifname);
		bprintf(stdout, rp.rp_flags, TRUNK_PORT_BITS);
		printf("\n");
	}

	for (i = 0; i < TRUNK_MAX_PORTS; i++)
		bzero(&rpbuf[i], sizeof(struct trunk_reqport));

	strlcpy(ra.ra_ifname, ifname, sizeof(ra.ra_ifname));
	ra.ra_size = sizeof(rpbuf);
	ra.ra_port = rpbuf;

	if (ioctl(ifs, SIOCGTRUNK, (caddr_t)&ra) == 0) {
		int pntd = 0;

		for (i = 0; i < TRUNK_PROTO_MAX; i++)
			if (ra.ra_proto == tpr[i].tpr_proto) {
				printf("  Trunkproto %s", tpr[i].tpr_name);
				pntd = 1;
				break;
			}
		for (i = 0; i < ra.ra_ports; i++)
			if(rpbuf[i].rp_portname[0] != '\0') {
				printf(" (%s ", rpbuf[i].rp_portname);
				bprintf(stdout, rpbuf[i].rp_flags, TRUNK_PORT_BITS);
				printf(")");
				pntd = 1;
			}
		if (pntd)
			printf("\n");
	}
}
