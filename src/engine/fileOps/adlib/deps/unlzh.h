/* unlzh.h -- decompress files in SCO compress -H (LZH) format.
 * The code in this file is directly derived from the public domain 'ar002'
 * written by Haruhiko Okumura.
 */

#ifndef _UNLZH_H_
#define _UNLZH_H_

#ifdef __cplusplus
extern "C" {
#endif

int LZH_decompress(char *source, char *dest, int source_size, int dest_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _UNLZH_H_ */
