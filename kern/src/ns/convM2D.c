// INFERNO
#include <vfs.h>
#include <kfs.h>
#include <slab.h>
#include <kmalloc.h>
#include <kref.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <error.h>
#include <cpio.h>
#include <pmap.h>
#include <smp.h>
#include <ip.h>

int statcheck(uint8_t * buf, unsigned int nbuf)
{
	uint8_t *ebuf;
	int i;

	ebuf = buf + nbuf;

	if (nbuf < STATFIXLEN || nbuf != BIT16SZ + GBIT16(buf)){
		printk("nbuf %d, STATFIXLEN %d ", nbuf, STATFIXLEN);
		printk("BIT16SZ %d, GBIT16(buf) %d ", 
			BIT16SZ, GBIT16(buf));
		printk("This is bad!\n");
		return -1;
	}

	buf += STATFIXLEN - 4 * BIT16SZ;

	for (i = 0; i < 4; i++) {
		if (buf + BIT16SZ > ebuf)
			return -1;
		buf += BIT16SZ + GBIT16(buf);
	}

	if (buf != ebuf){
		return -1;
	}

	return 0;
}

static char nullstring[] = "";

unsigned int
convM2D(uint8_t * buf, unsigned int nbuf, struct dir *d, char *strs)
{
	uint8_t *p, *ebuf;
	char *sv[4];
	int i, ns;

	if (nbuf < STATFIXLEN)
		return 0;

	p = buf;
	ebuf = buf + nbuf;

	p += BIT16SZ;	/* ignore size */
	d->type = GBIT16(p);
	p += BIT16SZ;
	d->dev = GBIT32(p);
	p += BIT32SZ;
	d->qid.type = GBIT8(p);
	p += BIT8SZ;
	d->qid.vers = GBIT32(p);
	p += BIT32SZ;
	d->qid.path = GBIT64(p);
	p += BIT64SZ;
	d->mode = GBIT32(p);
	p += BIT32SZ;
	d->atime = GBIT32(p);
	p += BIT32SZ;
	d->mtime = GBIT32(p);
	p += BIT32SZ;
	d->length = GBIT64(p);
	p += BIT64SZ;

	for (i = 0; i < 4; i++) {
		if (p + BIT16SZ > ebuf)
			return 0;
		ns = GBIT16(p);
		p += BIT16SZ;
		if (p + ns > ebuf)
			return 0;
		if (strs) {
			sv[i] = strs;
			memmove(strs, p, ns);
			strs += ns;
			*strs++ = '\0';
		}
		p += ns;
	}

	if (strs) {
		d->name = sv[0];
		d->uid = sv[1];
		d->gid = sv[2];
		d->muid = sv[3];
	} else {
		d->name = nullstring;
		d->uid = nullstring;
		d->gid = nullstring;
		d->muid = nullstring;
	}

	return p - buf;
}
