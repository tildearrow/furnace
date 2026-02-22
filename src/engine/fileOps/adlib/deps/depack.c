/*
 * aPLib compression library  -  the smaller the better :)
 *
 * C depacker
 *
 * Copyright (c) 1998-2014 Joergen Ibsen
 * All Rights Reserved
 *
 * http://www.ibsensoftware.com/
 */
        // aPLib v0.26b decompressor
        // incompatible with current releases
        // changes up to v1.1.0 have been commented out
        // aP_depack_asm(const void *source, void *destination)

#include "depack.h"

/* internal data structure */
struct APDSTATE {
	const unsigned char *source;
	unsigned char *destination;
	unsigned int destlen;
	unsigned int tag;
	unsigned int bitcount;
};

static unsigned int aP_getbit(struct APDSTATE *ud)
{
	unsigned int bit;

	/* check if tag is empty */
	if (!ud->bitcount--) {
		/* load next tag */
		ud->tag = *ud->source++;
		ud->bitcount = 7;
	}

	/* shift bit out of tag */
	bit = (ud->tag >> 7) & 0x01;
	ud->tag <<= 1;

	return bit;
}

static unsigned int aP_getgamma(struct APDSTATE *ud)
{
	unsigned int result = 1;

	/* input gamma2-encoded bits */
	do {
		result = (result << 1) + aP_getbit(ud);
	} while (aP_getbit(ud));

	return result;
}

unsigned int aP_depack(const void *source, void *destination, int srcsize, int dstsize)
{
	struct APDSTATE ud;
	unsigned int offs, len, R0/*, LWM*/;
	int done;
	int i;

	ud.source = (const unsigned char *) source;
	ud.destination = (unsigned char *) destination;
	ud.destlen = 0;
	ud.bitcount = 0;

	R0 = (unsigned int) -1;
	//LWM = 0;
	done = 0;

	/* first byte verbatim */
	if (!srcsize) return ud.destlen;
	if (!dstsize) return ud.destlen;
	*ud.destination++ = *ud.source++;
	srcsize--;
	dstsize--; ud.destlen++;

	/* main decompression loop */
	while (!done) {
		if (aP_getbit(&ud)) {
			if (aP_getbit(&ud)) {
				if (aP_getbit(&ud)) {
					offs = 0;

					for (i = 4; i; i--) {
						offs = (offs << 1) + aP_getbit(&ud);
					}

					if (offs) {
						if (!dstsize) return ud.destlen;
						if (offs > ud.destlen) return ud.destlen;
						*ud.destination = *(ud.destination - offs);
						ud.destination++;
						dstsize--; ud.destlen++;
					}
					else {
						if (!dstsize) return ud.destlen;
						*ud.destination++ = 0x00;
						dstsize--; ud.destlen++;
					}

					//LWM = 0;
				} else {
					if (!srcsize) return ud.destlen;
					offs = *ud.source++;
					srcsize--;

					len = 2 + (offs & 0x0001);

					offs >>= 1;

					if (offs) {
						for (; len; len--) {
							if (!dstsize) return ud.destlen;
							if (offs > ud.destlen) return ud.destlen;
							*ud.destination = *(ud.destination - offs);
							ud.destination++;
							dstsize--; ud.destlen++;
						}
					}
					else {
						done = 1;
					}

					R0 = offs;
					//LWM = 1;
				}
			} else {
				offs = aP_getgamma(&ud);

				//if ((LWM == 0) && (offs == 2)) {
				if (offs == 2) {
					//-------------------
					offs = R0;

					len = aP_getgamma(&ud);

					for (; len; len--) {
						if (!dstsize) return ud.destlen;
						if (offs > ud.destlen) return ud.destlen;
						*ud.destination = *(ud.destination - offs);
						ud.destination++;
						dstsize--; ud.destlen++;
					}
				} else {
					/*
					if (LWM == 0) {
						offs -= 2;
					}
					else {
						offs -= 2;
					}
					*/
					offs -= 3;
					//-------------------

					offs <<= 8;
					if (!srcsize) return ud.destlen;
					offs += *ud.source++;
					srcsize--;

					len = aP_getgamma(&ud);

					if (offs >= 32000) {
						len++;
					}
					if (offs >= 1280) {
						len++;
					}
					if (offs < 128) {
						len += 2;
					}

					for (; len; len--) {
						if (!dstsize) return ud.destlen;
						if (offs > ud.destlen) return ud.destlen;
						*ud.destination = *(ud.destination - offs);
						ud.destination++;
						dstsize--; ud.destlen++;
					}

					R0 = offs;
				}

				//LWM = 1;
			}
		}
		else {
			if (!srcsize) return ud.destlen;
			if (!dstsize) return ud.destlen;
			*ud.destination++ = *ud.source++;
			srcsize--;
			dstsize--; ud.destlen++;
			//LWM = 0;
		}
	}

	return ud.destlen;
}
