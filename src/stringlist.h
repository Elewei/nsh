/* From: $OpenBSD: /usr/src/usr.bin/ftp/stringlist.h,v 1.2 2002/02/16 21:27:46 millert Exp $	*/

#ifndef _STRINGLIST_H
#define _STRINGLIST_H 

#include <sys/cdefs.h>
#include <sys/types.h>

/*
 * Simple string list
 */
typedef struct _stringlist {
	char	**sl_str;
	size_t	  sl_max;
	size_t	  sl_cur;
} StringList;

__BEGIN_DECLS
StringList	*sl_init(void);
void		 sl_add(StringList *, char *);
void		 sl_free(StringList *, int);
char		*sl_find(StringList *, char *);
__END_DECLS

#endif /* _STRINGLIST_H */
