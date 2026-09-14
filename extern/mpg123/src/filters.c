/*
	filters: parse filter specifications

	copyright 2020-2023 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Thomas Orgis
*/

#include "filters.h"
#include <ctype.h>

#include "common/debug.h"

// Validate syntax of filter specification, returning the number
// of configured filters.
// b_0,...,b_N,a_0,...,a_N:b_0,...,b_N,a_0,...,a_N
// 1. An even number, at least two of coefficients for each
// 2. Numbers (decimal, scientific notation ...) separated by comma.
// 3. Spacing optional.	
// If there's a non-empty spec, getting zero filters out of it is the error.
// Total number of coefficients of successfully parsed filters is optionally
// returned, too.
static size_t validate_filterspec(const char *spec, size_t *coeff_total)
{
	size_t count = 0;
	size_t ncoeff = 0;
	size_t totcoeff = 0;
	while(*spec)
	{
		char *nspec = (char*)spec;;
		errno = 0;
		strtod(spec, &nspec);
		if(errno)
		{
			merror("Bad number in filter spec, here: %s", spec);
			return 0;
		}
		if(nspec == spec)
		{
			merror("Parser did not advance on: %s", spec);
			return 0;
		}
		spec += nspec-spec;
		++ncoeff;
		++totcoeff;
		while(isspace(*spec)) ++spec;
		// Increment number of coefficients if one shall follow,
		// increment number of filters if one shall follow.
		if(*spec == ',')
			++spec;
		else if(*spec == ':')
		{
			++spec;
			if(!ncoeff || ncoeff % 2)
				break;
			ncoeff = 0;
			++count;
		}
	}
	if(coeff_total)
		*coeff_total = totcoeff;
	// If we collected enough coefficients until end of string, we got
	// another filter.
	if(!ncoeff || ncoeff%2)
	{
		merror("Bad number of coefficients in final filter spec: %zu", ncoeff);
		return 0;
	} else
		return ++count;
}

// Storage of the filters must succeed as we validated the spec.
// Still playing safe where things would massively go wrong.
static int store_filters(struct filterlist *fl, const char *spec)
{
	size_t ci = 0; // offset in global coefficient list
	errno = 0;
	for(size_t fi=0; fi<fl->count; ++fi)
	{
		size_t fcoeffs = 0; // coefficients for this filter, 2*(order+1)
		while(!errno && *spec && *spec != ':')
		{
			char *nspec;
			fl->coeff[ci+fcoeffs] = strtod(spec, &nspec);
			fcoeffs++;
			spec += nspec-spec;
			if(*spec == ',')
				++spec;
			while(isspace(*spec))
				++spec;
		}
		if(errno)
		{
			merror("Number parsing error on validated spec: %s", INT123_strerror(errno));
			return -1;
		}
		if(*spec) // Skip ":"
			++spec;
		if(fcoeffs < 2)
		{
			merror("Bad coefficient count for filter %zu on validated spec.", fi);
			fl->f[fi].order = 0;
			fl->f[fi].b = fl->f[fi].a = NULL;
			return -1;
		}
		fl->f[fi].order = fcoeffs/2-1;
		fl->f[fi].b = fl->coeff + ci;
		fl->f[fi].a = fl->coeff + ci+fl->f[fi].order + 1;
		ci+=fcoeffs;
	}
	if(ci != fl->coeff_total)
	{
		error("Mismatch in total coefficient count.");
		return -1;
	}
	return 0;
}

void free_filterlist(struct filterlist *fl)
{
	if(!fl)
		return;
	if(fl->coeff)
		free(fl->coeff);
	if(fl->f)
		free(fl->f);
}

struct filterlist* parse_filterspec(const char *spec)
{
	struct filterlist *fl = NULL;
	size_t count, coeffs;
	count = validate_filterspec(spec, &coeffs);
	if(count)
	{
		fl = malloc(sizeof(*fl));
		if(coeffs > UINT_MAX)
		{
			merror("Too many filter coefficients: %zu", coeffs);
			return NULL;
		}
	}
	mdebug("%zu filters, %zu coefficients, fl=%p", count, count ? coeffs : 0, (void*)fl);
	if(fl)
	{
		fl->count = count;
		fl->coeff_total = coeffs;
		fl->coeff = malloc(sizeof(*fl->coeff)*coeffs);
		fl->f = malloc(sizeof(*fl->f)*count);
		if(!fl->coeff || !fl->f)
		{
			if(fl->coeff)
				free(fl->coeff);
			if(fl->f)
				free(fl->f);
			free(fl);
			fl = NULL;
		}
	}
	if(fl)
	{
		if(store_filters(fl, spec))
		{
			error("Storage failed after validaton (impossible!)");
			free_filterlist(fl);
			fl = NULL;
		}
	}
	return fl;
}

