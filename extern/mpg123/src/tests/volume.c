#include "config.h"
#include "syn123.h"
#include <math.h>
#include <stdio.h>
#include <stdint.h>

#define AMPTEST(name, bits, wbits, enc) \
int name(syn123_handle *sh) \
{ \
	int err = 0; \
	float factor = 0.5; \
	fprintf(stderr, "amplification %d bits\n", bits); \
	int##bits##_t *audio = malloc(bits/8*1<<bits); \
	size_t clp = 0; \
	for(int##wbits##_t i=0; i<1<<bits; ++i) \
		audio[i] = (int##bits##_t)(i-(1<<(bits-1))); \
	if((err = syn123_amp(audio, enc, 1<<bits, factor, 0., &clp, sh))) \
	{ \
		fprintf(stderr, "cannot amp: %s\n", syn123_strerror(err)); \
		++err; \
	} \
	if(clp) \
		++err; \
	if(!err) \
		for(int##wbits##_t i=0; i<1<<bits; ++i) \
			if(labs((int##wbits##_t)audio[i] - ((i-(1<<(bits-1)))/2)) > 1) \
			{ \
				++err; \
				break; \
			} \
	free(audio); \
	return err ? 1 : 0; \
}

AMPTEST(amptest_8, 8, 16, MPG123_ENC_SIGNED_8)
AMPTEST(amptest_16, 16, 32, MPG123_ENC_SIGNED_16)


int main()
{
	int ret = 0;
	fprintf(stderr, "create handle\n");
	syn123_handle *sh = syn123_new(1,1,MPG123_ENC_FLOAT_32,0,NULL);
	if(!sh)
	{
		fprintf(stderr, "cannot create syn123 handle\n");
		++ret;
	}
	if(ret)
		goto end;

	fprintf(stderr, "dB conversion\n");
	double lin = 0.37283;
	double db  = syn123_lin2db(lin);
	double ldb = syn123_db2lin(db);
	if(fabs(ldb-lin) > 1e-15)
	{
		fprintf(stderr, "db2lin(lin2db) off by %g\n", ldb-lin);
		++ret;
	}
	db = syn123_lin2db(2);
	if(fabs(db-6.)>0.05)
	{
		fprintf(stderr, "lin2db of factor 2 off by %g", db-6.);
		++ret;
	}
	if(!ret)
		ret += amptest_8(sh);
	if(!ret)
		ret += amptest_16(sh);

end:
	syn123_del(sh);
	printf(ret ? "FAIL\n" : "PASS\n");
	return 0;
}
