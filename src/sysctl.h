
struct ipsysctl {
	char           *name;
	int             mib[6];
	int32_t         def_larg;	/* default value, or 0 for on/off
					 * sysctls */
	int             enable;	/* if on/off sysctl, 0 default is enabled, 1
				 * default is disabled, 2 always show ena ble
				 * or disable */
};

struct sysctltab {
	char           *name;
	int             pf;
	Menu           *table;
	struct ipsysctl *sysctl;
};

extern struct sysctltab sysctls[];
extern struct ipsysctl ipsysctls[];
extern struct ipsysctl ip6sysctls[];
extern struct ipsysctl mplssysctls[];
extern struct ipsysctl ddbsysctls[];
extern struct ipsysctl pipexsysctls[];
extern Menu iptab[];
extern Menu ip6tab[];
extern Menu mplstab[];
extern Menu ddbtab[];
extern Menu pipextab[];

#define DEFAULT_MAXIFPREFIXES	16	/* net.inet6.ip6.maxifprefixes */
#define DEFAULT_MAXIFDEFROUTERS	16	/* net.inet6.ip6.maxifdefrouters */
#define DEFAULT_MAXDYNROUTES	4096	/* net.inet6.ip6.maxdynroutes */
