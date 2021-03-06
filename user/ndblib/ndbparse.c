/* 
 * This file is part of the UCB release of Plan 9. It is subject to the license
 * terms in the LICENSE file found in the top-level directory of this
 * distribution and at http://akaros.cs.berkeley.edu/files/Plan9License. No
 * part of the UCB release of Plan 9, including this file, may be copied,
 * modified, propagated, or distributed except according to the terms contained
 * in the LICENSE file.
 */
#include <stdlib.h>

#include <stdio.h>
#include <parlib/parlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <iplib/iplib.h>
#include <ndblib/ndb.h>

/*
 *  Parse a data base entry.  Entries may span multiple
 *  lines.  An entry starts on a left margin.  All subsequent
 *  lines must be indented by white space.  An entry consists
 *  of tuples of the forms:
 *	attribute-name
 *	attribute-name=value
 *	attribute-name="value with white space"
 *
 *  The parsing returns a 2-dimensional structure.  The first
 *  dimension joins all tuples. All tuples on the same line
 *  form a ring along the second dimension.
 */

/*
 *  parse the next entry in the file
 */
struct ndbtuple*
ndbparse(struct ndb *db)
{
	char *line;
	struct ndbtuple *t;
	struct ndbtuple *first, *last;
	int len;

	first = last = 0;
	for(;;){
		if((line = fgets(db->buf, sizeof(db->buf),db->b)) == 0)
			break;
		len = strlen(db->buf);
		if(line[len-1] != '\n')
			break;
		if(first && !isspace(*line) && *line != '#'){
			fseek(db->b, -len, 1);
			break;
		}
		t = _ndbparseline(line);
		if(t == 0)
			continue;
		if(first)
			last->entry = t;
		else
			first = t;
		last = t;
		while(last->entry)
			last = last->entry;
	}
	ndbsetmalloctag(first, getcallerpc(&db));
	return first;
}
