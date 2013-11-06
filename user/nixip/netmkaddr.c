#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

/* truncate functionality for now. No lib/ndb.
#include <libnet.h>
 */

/*
 *  make an address, add the defaults
 *
 *  if you pass in only one ! in linear, this assumes this ! was between the net
 *  and the host.  if you pass in no !s, we'll build one from defnet/defsrv.
 */
char *
netmkaddr(char *linear, char *defnet, char *defsrv)
{
	/* TODO: this isn't threadsafe */
	static char addr[256];
	char *cp;

	/*
	 *  dump network name
	 */
	cp = strchr(linear, '!');
	if(cp == 0){
		if(defnet==0){
			if(defsrv)
				snprintf(addr, sizeof(addr), "net!%s!%s",
					linear, defsrv);
			else
				snprintf(addr, sizeof(addr), "net!%s", linear);
		}
		else {
			if(defsrv)
				snprintf(addr, sizeof(addr), "%s!%s!%s", defnet,
					linear, defsrv);
			else
				snprintf(addr, sizeof(addr), "%s!%s", defnet,
					linear);
		}
		return addr;
	}

	/*
	 *  if there is already a service, use it
	 */
	cp = strchr(cp+1, '!');
	if(cp)
		return linear;

	/*
	 *  add default service
	 */
	if(defsrv == 0)
		return linear;
	snprintf(addr, sizeof(addr), "%s!%s", linear, defsrv);

	return addr;
}
