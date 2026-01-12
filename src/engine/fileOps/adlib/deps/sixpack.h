/********************************************/
/*  SIXPACK.H -- Data compression program   */
/*  Written by Philip G. Gage, April 1991   */
/********************************************/

#ifndef _SIXDEPACK_H_
#define _SIXDEPACK_H_

#ifdef __cplusplus
extern "C" {
#endif

unsigned short sixdepak(unsigned short *source, unsigned char *dest,
				    unsigned short size, unsigned int destsize);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _SIXDEPACK_H_ */
