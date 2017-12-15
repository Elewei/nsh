

#include <stdio.h>
#include <string.h>
#include <err.h>
#include <stdlib.h>

#include "stringlist.h"

#define _SL_CHUNKSIZE	20

/*
 * sl_init(): Initialize a string list
 */
StringList *
sl_init(void)
{
	StringList *sl = malloc(sizeof(StringList));
	if (sl == NULL)
		err(1, "stringlist");

	sl->sl_cur = 0;
	sl->sl_max = _SL_CHUNKSIZE;
	sl->sl_str = calloc(sl->sl_max, sizeof(char *));
	if (sl->sl_str == NULL)
		err(1, "stringlist");
	return sl;
}


/*
 * sl_add(): Add an item to the string list
 */
void
sl_add(StringList *sl, char *name)
{
	if (sl->sl_cur == sl->sl_max - 1) {
		sl->sl_max += _SL_CHUNKSIZE;
		sl->sl_str = reallocarray(sl->sl_str, sl->sl_max,
		    sizeof(char *));
		if (sl->sl_str == NULL)
			err(1, "stringlist");
	}
	sl->sl_str[sl->sl_cur++] = name;
}


/*
 * sl_free(): Free a stringlist
 */
void
sl_free(StringList *sl, int all)
{
	size_t i;

	if (sl == NULL)
		return;
	if (sl->sl_str) {
		if (all)
			for (i = 0; i < sl->sl_cur; i++)
				free(sl->sl_str[i]);
		free(sl->sl_str);
	}
	free(sl);
}
