// Those select the specific syn123 functions, without
// this, the native wrapper is used, hopefully matching off_t.
//#define _FILE_OFFSET_BITS 64
//#define _FILE_OFFSET_BITS 32
#define SYN123_PORTABLE_API
#include "config.h"
#include <syn123.h>

#include <inttypes.h>


#ifdef SYN123_PORTABLE_API
typedef int64_t synoff;
typedef int64_t synprint;
#define SYNPRINT PRIi64
#else
typedef off_t synoff;
typedef intmax_t synprint;
#define SYNPRINT PRIiMAX
#endif

#include <stdio.h>

const long arate = 8000;
const long brate = 48000;
const synoff ins = 12637;
const synoff outs = 75822;

int main()
{
#ifdef SYN123_PORTABLE_API
	synoff outs2 = syn123_resample_total64(arate,brate,ins);
	synoff ins2  = syn123_resample_intotal64(arate,brate,outs);
#else
	synoff outs2 = syn123_resample_total(arate,brate,ins);
	synoff ins2  = syn123_resample_intotal(arate,brate,outs);
#endif
	int err = 0;
	if(outs2 != outs && ++err)
		fprintf(stderr, "total mismatch: %" SYNPRINT " != %" SYNPRINT "\n"
		,	(synprint)outs2, (synprint)outs );
	if(ins2 != ins && ++err)
		fprintf(stderr, "intotal mismatch: %" SYNPRINT " != %" SYNPRINT "\n"
		,	(synprint)ins2, (synprint)ins );
	printf("%s\n", err ? "FAIL" : "PASS");
	return err;
}
