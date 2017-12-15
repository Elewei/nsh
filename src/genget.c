/* From: $OpenBSD: genget.c,v 1.5 2001/05/25 10:23:06 hin Exp $  */

#include <ctype.h>
#include <stdio.h>
#include "externs.h"

#define	LOWER(x) (isupper((int)x) ? tolower((int)x) : (x))

int isprefix(char *, char*);

/*
 * The prefix function returns 0 if *s1 is not a prefix
 * of *s2.  If *s1 exactly matches *s2, the negative of
 * the length is returned.  If *s1 is a prefix of *s2,
 * the length of *s1 is returned.
 */
int
isprefix(char *s1, char *s2)
{
    char *os1;
    char c1, c2;

    if (*s1 == '\0')
	return(-1);
    os1 = s1;
    c1 = *s1;
    c2 = *s2;
    while (LOWER(c1) == LOWER(c2)) {
	if (c1 == '\0')
	    break;
	c1 = *++s1;
	c2 = *++s2;
    }
    return(*s1 ? 0 : (*s2 ? (s1 - os1) : (os1 - s1)));
}

static char *ambiguous;		/* special return value for command routines */

char **
genget(char *name, char **table, int stlen)
     /* name to match */
     /* name entry in table */
	   	      
{
    char **c, **found;
    int n;

    if (name == 0)
	return 0;

    found = 0;
    for (c = table; *c != 0; c = (char **)((char *)c + stlen)) {
	if ((n = isprefix(name, *c)) == 0)
	    continue;
	if (n < 0)		/* exact match */
	    return(c);
	if (found)
	    return(&ambiguous);
	found = c;
    }
    return(found);
}

/*
 * Function call version of Ambiguous()
 */
int
Ambiguous(void *s)
{
    return((char **)s == &ambiguous);
}
