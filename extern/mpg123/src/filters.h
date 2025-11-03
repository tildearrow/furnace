/*
	filters: parse filter specifications

	A specification of a set of filters is a string of coefficients
	of each filter (including a_0=1 as sanity check, which is verfied
	by syn123 on filter createion), separated with : from the coefficiets
	of the next filter:

	b_0,...,b_N,a_0,...,a_N:b_0,...,b_N,a_0,...,a_N

	This code verifies such a string and prepares the data to hand over
	to the syn123 library.

	copyright 2020 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Thomas Orgis
*/

#ifndef MPG123_FILTERS_H
#define MPG123_FILTERS_H

#include "compat/compat.h"

struct filter
{
	unsigned int order;
	double *b;
	double *a;
};

struct filterlist
{
	size_t count;
	size_t coeff_total;
	double *coeff; // all coefficients in one block of memory
	struct filter *f;
};


// Parse given spec string and allocate filters data structure.
struct filterlist* parse_filterspec(const char *spec);
// Free all memory in the filter list and the list struct itself.
void free_filterlist(struct filterlist *filters);

#endif
