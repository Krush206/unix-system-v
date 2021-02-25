/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:isheader.c	1.3.3.2"
#include "mail.h"
/*
 * isheader(lp, ctf) - check if lp is header line and return type
 *	lp	-> 	pointer to line
 *	ctfp	->	continuation flag (should be FALSE the first time
 *			isheader() is called on a message.  isheader() sets
 *			it for the remaining calls to that message)
 * returns
 *	FALSE	->	not header line
 *	H_*     ->	type of header line found.
 */
int
isheader(lp, ctfp)
char	*lp;
int	*ctfp;
{
	register char	*p, *q;
	register int	i;

	p = lp;
	while((*p) && (*p != '\n') && (isspace(*p))) {
		p++;
	}
	if((*p == NULL) || (*p == '\n')) {
		/* blank line */
		return (FALSE);
	}

	if ((*ctfp) && ((*lp == ' ') || (*lp == '\t'))) {
		return(H_CONT);
	}

	*ctfp = FALSE;
	for (i = 1; i < H_CONT; i++) {
		if (!isit(lp, i)) {
			continue;
		}
		if ((i == H_FROM) || (i == H_FROM1)) {
			/* From and >From must be case sensitive matches. */
			if (strncmp(lp, header[i].tag, strlen(header[i].tag)) != 0 )
				return(FALSE);
			/*
			 * Should NEVER get 'From ' or '>From ' line on stdin
			 * if invoked as mail (rather than rmail) since
			 * 'From ' and/or '>From ' lines are generated by
			 * program itself. Therefore, if it DOES match and
			 * ismail == TRUE, it must be part of the content.
			 */
			if (sending && ismail) {
				return (FALSE);
			}
		}
		/* the : must be followed by whitespace or the end of the line */
		q++;
		if (*q && (*q != ' ') && (*q != '\t') && (*q != '\n') && (*q != '\r'))
			return(FALSE);
		*ctfp = TRUE;
		return (i);
	}
	/*
	 * Check if name: value pair
 	 */
	if ((p = strpbrk(lp, ":")) != NULL ) {
		for(q = lp; q < p; q++)  {
			if ( (!isalnum(*q)) && (*q != '-') && (*q != '>'))  {
				return(FALSE);
			}
		}
		*ctfp = TRUE;
		return(H_NAMEVALUE);
	}
	return(FALSE);
}
