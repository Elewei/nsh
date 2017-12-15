
#include <stdio.h>
#include <stdlib.h>
#include "externs.h"

int noptind;
char *nopterr;

int
nopt(int argc, char **argv, struct nopts *tokens)
{
	struct nopts *op;

	if (argc < 1)
		return(-1);
	op = (struct nopts *)genget(argv[noptind], (char **)tokens,
	    sizeof(struct nopts));
	if (op == 0) {
		nopterr = "Invalid argument";
		return(-1);
	}
	if (Ambiguous(op)) {
		nopterr = "Ambiguous argument";
		return(-1);
	}
	if (op->type == req_arg) {
		if ((argc - noptind - 1) == 0) {
			nopterr = "Missing required argument";
			return(-1);
		}
		noptind += 2;
	}
	if (op->type == no_arg)
		noptind += 1;

	return (op->arg);
}
