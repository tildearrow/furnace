/*
	term_none: no-op terminal, nothing at all

	copyright 2008-2022 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Thomas Orgis and Jonathan Yong
*/

int term_have_fun(int fd, int want_visuals)
{
	return 0;
}

int term_width(int fd)
{
	return -1;
}

int term_setup(void)
{
	return -1;
}

void term_restore(void)
{
}

int term_get_key(int stopped, int do_delay, char *val)
{
	return 0;
}
