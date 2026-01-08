/*
	httpget: HTTP input routines (the header)

	copyright 2007 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Thomas Orgis

	Note about MIME types:
	You feed debunk_mime() a MIME string and it classifies it as it is relevant for mpg123.
	In httpget.c are the MIME class lists, which may be appended to to support more bogus MIME types.
*/

#ifndef _HTTPGET_H_
#define _HTTPGET_H_
#include "mpg123.h"

#include "net123.h"

/* Pulled in by mpg123app.h! */

struct httpdata
{
	mpg123_string content_type;
	mpg123_string icy_name;
	mpg123_string icy_url;
	off_t icy_interval;
	mpg123_string proxyhost;
	mpg123_string proxyport;
	/* Partly dummy for now... later proxy host resolution will be cached (PROXY_ADDR). */
	enum { PROXY_UNKNOWN=0, PROXY_NONE, PROXY_HOST, PROXY_ADDR } proxystate;
};

void httpdata_init(struct httpdata *e);
void httpdata_reset(struct httpdata *e);
void httpdata_free(struct httpdata *e);

/* There is a whole lot of MIME types for the same thing.
   the function will reduce it to a combination of these flags */
#define IS_FILE 1
#define IS_LIST 2
#define IS_M3U  4
#define IS_PLS  8

#define HTTP_MAX_RELOCATIONS 20

int debunk_mime(const char* mime);

/*Previously static functions, shared for win32_net_support */
int proxy_init(struct httpdata *hd);
int translate_url(const char *url, mpg123_string *purl);
size_t accept_length(void);
int fill_request(mpg123_string *request, mpg123_string *host, mpg123_string *port, mpg123_string *httpauth1, const char * const *client_head);
void get_header_string(mpg123_string *response, const char *fieldname, mpg123_string *store);
char *get_header_val(const char *hname, mpg123_string *response);

#define icy_yes "Icy-MetaData: 1"
#define icy_no "Icy-MetaData: 0"

// Append an accept header line to the string, without line end.
int append_accept(mpg123_string *s);

// Open HTTP URL with internal network code.
int http_open(const char* url, struct httpdata *hd, const char * const *client_head);

#endif
