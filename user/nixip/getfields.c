#include <string.h>

/* currently only used in arp.c; so we don't do the full monty. */
int
getfields(char *str, char **args, int max, int mflag, char *set)
{
	int nr, intok, narg;

	if(max <= 0)
		return 0;

	narg = 0;
	args[narg] = str;
	if(!mflag)
		narg++;
	intok = 0;
	for(;; str++) {
		nr = *str;
		if(nr == 0)
			break;
		if(index(set, nr)){
			if(narg >= max)
				break;
			*str = 0;
			intok = 0;
			args[narg] = str + 1;
			if(!mflag)
				narg++;
		} else {
			if(!intok && mflag)
				narg++;
			intok = 1;
		}
	}
	return narg;
}