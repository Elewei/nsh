/*
 * 
 *  网络Shell主程序
 *  
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>
#include <sys/socket.h>
#include <sys/syslimits.h>
#include <sys/ttycom.h>
#include <sys/signal.h>
#include "editing.h"
#include "stringlist.h"
#include "externs.h"

void usage(void);

jmp_buf toplevel;

char *vers = "0.2";
int bridge = 0;		      /* 桥接模式 interface() */
int verbose = 0;	      /* DEBUG 模式 */
int priv = 0, cli_rtable = 0;
int editing = 1;
pid_t pid;

History *histi = NULL;
History *histc = NULL;
HistEvent ev;
EditLine *elc = NULL;
EditLine *eli = NULL;
char *cursor_pos = NULL;

void intr(void);

int
main(int argc, char *argv[])
{
	int top, ch, iflag = 0, cflag = 0;
	char rc[PATH_MAX];

	if(getuid() != 0) 
		printf("%% 权限不够.\n");

	pid = getpid();

	while ((ch = getopt(argc, argv, "c:i:v")) != -1)
		switch (ch) {
		case 'c':
			cflag = 1;
			strlcpy(rc, optarg, PATH_MAX);
			break;
		case 'i':
			iflag = 1;
			strlcpy(rc, optarg, PATH_MAX);
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			usage();
	}

	argc -= optind;
	argv += optind;

	if (cflag && iflag)
		usage();
	if (argc > 0)
		usage();
	if (iflag)
		rmtemp(SQ3DBFILE);

	printf("%% 防火墙Shell程序 -- v%s\n", vers);

	/* 创建sqlite数据库表 */
	if (db_create_table_rtables() < 0)
		printf("%% database rtables creation failed\n");
	if (db_create_table_flag_x("ctl") < 0)
		printf("%% database ctl creation failed\n");
	if (db_create_table_flag_x("dhcprelay") < 0)
		printf("%% database dhcrelay creation failed\n");
	if (db_create_table_flag_x("ipv6linklocal") < 0)
		printf("%% database ipv6linklocal creation failed\n");
	if (db_create_table_flag_x("lladdr") < 0)
		printf("%% database lladdr creation failed\n");
	if (db_create_table_flag_x("rtadvd") < 0)
		printf("%% database rtadvd creation failed\n");
	if (db_create_table_flag_x("authkey") < 0)
		printf("%% database authkey creation failed\n");
	if (db_create_table_flag_x("peerkey") < 0)
		printf("%% database peerkey creation failed\n");

	if (iflag) {
		char *argv_demote[] = { "group", "carp", "carpdemote", "128" };
		char *argv_restore[] = { "no", "group", "carp", "carpdemote", "128" };
		priv = 1;
		group(nitems(argv_demote), argv_demote);
		cmdrc(rc);
		group(nitems(argv_restore), argv_restore);
		exit(0);
	}
	if (cflag) {
		priv = 1;
		cmdrc(rc);
		exit(0);
	}

	top = setjmp(toplevel) == 0;   // setjmp 第一次保存当前AR到toplevel中，返回0	
	if (top) {
		(void)signal(SIGWINCH, setwinsize);   // 窗口大小变化
		(void)signal(SIGINT, (sig_t)intr);    // 收到一个交互提示信号，执行intr
		(void)setwinsize(0);				  // 窗口大小默认
	} else
		putchar('\n');                   

	/* 循环执行command */
	for (;;) {
		command();
		top = 1;
	}

	/* NOTREACHED */
	return 0;
}

void
usage(void)
{
	fprintf(stderr, "usage: %s [-v] [-i rcfile | -c rcfile]\n", __progname);
	fprintf(stderr, "           -v indicates verbose operation\n");
	fprintf(stderr, "           -i rcfile loads initial system" \
		    " configuration from rcfile\n");
	fprintf(stderr, "           -c rcfile loads commands from rcfile\n");
	exit(1);
}

void
intr(void)
{
	longjmp(toplevel, 1);
}
