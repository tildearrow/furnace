/*
	streamdump: Dumping a copy of the input data.

	This evolved into the generic I/O interposer for direct file or http stream
	access, with explicit buffering for getline.

	copyright 2010-2023 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Michael Hipp
*/

// setenv
#define _XOPEN_SOURCE 600
#define _POSIX_C_SOURCE 200112L

#include "streamdump.h"
#include <fcntl.h>
#include <errno.h>
#include "common/debug.h"

#ifndef O_BINARY
#define O_BINARY 0
#endif

#ifndef HTTP_MAX_RELOCATIONS
#define HTTP_MAX_RELOCATIONS 20
#endif

#if defined(NETWORK) && !defined(NET123)
#error "NETWORK only with NET123 from now on!"
#endif
#if !defined(NETWORK) && defined(NET123)
#error "NET123 only with NETWORK from now on!"
#endif


#ifdef NETWORK

const char *net123_backends[] =
{
	"internal"
#ifdef NET123_EXEC
,	"wget"
,	"curl"
#endif
#ifdef NET123_WINHTTP
,	"winhttp"
#endif
#ifdef NET123_WININET
,       "wininet"
#endif
,	NULL
};

// Net123 variant for our internal code that has safer legacy support for HTTP shoutcast.

static size_t net123_read_internal( struct net123_handle_struct *nh,
void *buf, size_t bufsize )
{
	if(!nh)
		return 0;
	int *fdp = nh->parts;
#ifdef WANT_WIN32_SOCKETS
	return win32_net_read(*fdp, buf, bufsize);
#else
	return INT123_unintr_read(*fdp, buf, bufsize);
#endif

}

static void net123_close_internal(struct net123_handle_struct *nh)
{
	if(nh)
		return;
	int *fdp = nh->parts;
#ifdef WANT_WIN32_SOCKETS
	if(*fdp != SOCKET_ERROR)
		win32_net_close(*fdp);
#else
	close(*fdp);
#endif
	free(fdp);
	free(nh);
}

static net123_handle *net123_open_internal( const char *url
,	const char * const  *client_head, struct httpdata *hd )
{
	net123_handle *nh = malloc(sizeof(net123_handle));
	if(!nh)
		return NULL;
	int *fdp = malloc(sizeof(int));
	if(!fdp)
	{
		free(nh);
		return NULL;
	}
	nh->parts = fdp;
	nh->read = net123_read_internal;
	nh->close = net123_close_internal;
	// Handles win32_net internally.
	*fdp = http_open(url, hd, client_head);
	if(*fdp >= 0)
		return nh;
	free(fdp);
	free(nh);
	return NULL;
}

// Decide which backend to load.
static net123_handle *net123_open( const char *url
,	const char * const  *client_head, struct httpdata *hd )
{
	int autochoose = !strcmp("auto", param.network_backend);
	int https      = !strncasecmp("https://", url, 8);
	if(param.proxyurl)
	{
		if(strcmp(param.proxyurl, "none"))
		{
#ifdef HAVE_SETENV
			setenv("http_proxy",  param.proxyurl, 1);
			setenv("HTTP_PROXY",  param.proxyurl, 1);
			setenv("https_proxy", param.proxyurl, 1);
			setenv("HTTPS_PROXY", param.proxyurl, 1);
#endif
		} else
		{
#ifdef HAVE_UNSETENV
			unsetenv("http_proxy");
			unsetenv("HTTP_PROXY");
			unsetenv("https_proxy");
			unsetenv("HTTPS_PROXY");
#endif
		}
	}
	if(   (autochoose && !https)
	   || !strcmp("internal", param.network_backend) )
	{
		if(https && !param.quiet)
			fprintf(stderr, "Note: HTTPS will fail with internal network code.\n");
		return net123_open_internal(url, client_head, hd);
	}
#ifdef NET123_EXEC
	if( autochoose
		|| !strcmp("wget", param.network_backend)
		|| !strcmp("curl", param.network_backend) )
		return net123_open_exec(url, client_head);
#endif
#ifdef NET123_WININET
	if(autochoose || !strcmp("wininet", param.network_backend))
		return net123_open_wininet(url, client_head);
#endif
#ifdef NET123_WINHTTP
        if(autochoose || !strcmp("winhttp", param.network_backend))
                return net123_open_winhttp(url, client_head);
#endif
	merror("no network backend for %s", https ? "HTTPS" : "HTTP");
	return NULL;
}
#endif

/* Stream dump descriptor. */
static int dump_fd = -1;

// Read without the buffer. This is used to fill the buffer explicitly in getline.
// This is the function that finally wraps around all the different types of input.

static mpg123_ssize_t stream_read_raw(struct stream *sd, void *buf, size_t count)
{
	mpg123_ssize_t ret = -1;
#ifdef NETWORK
	if(sd->nh)
		ret = (mpg123_ssize_t) sd->nh->read(sd->nh, buf, count);
#endif
	if(sd->fd >= 0) // plain file or network socket
		ret = (mpg123_ssize_t) INT123_unintr_read(sd->fd, buf, count);
	return ret;
}

static mpg123_ssize_t stream_read(struct stream *sd, void *buf, size_t count)
{
	if(!sd)
		return -1;
	char *bbuf = buf;
	mpg123_ssize_t ret = 0;
	if(count > SSIZE_MAX)
		return -1;
	while(count)
	{
		size_t get = 0;
		if(sd->fill)
		{ // drain the buffer
			get = sd->fill > count ? count : sd->fill;
			memcpy(bbuf, sd->bufp, get);
			sd->fill -= get;
			sd->bufp += get;
		} else
		{ // get it from the source
			mpg123_ssize_t rret = stream_read_raw(sd, bbuf, count);
			if(rret < 0)
				return ret > 0 ? ret : -1;
			if(rret == 0)
				return ret;
			get = rret;
		}
		bbuf  += get;
		count -= get;
		ret   += get;
	}
	return ret;
}

static off_t stream_seek(struct stream *sd, off_t pos, int whence)
{
	if(!sd)
		return -1;
#ifdef NET123
	if(sd->nh)
		return -1;
#endif
	return lseek(sd->fd, pos, whence);
}

// Read into the stream buffer, look for line endings there, copy stuff.
// This should work with \n and \r\n sequences ... even just \r sequences?
// Yes, either \r or \n ends a line, a following \n or \r is just swallowed.
// Need to catch the case where the buffer ends with \r and the next buffer
// contents start with the matching \n, and the other way round.
mpg123_ssize_t stream_getline(struct stream *sd, mpg123_string *line)
{
	if(!sd || !line)
		return -1;
	line->fill = 0; // this is EOF
	char lend = 0;
	while(1)
	{
		mdebug("getline loop with %d", sd->fill);
		// If we got just an \r, we need to ensure that we swalloed the matching \n,
		// too. This implies that we got a line stored already.
		if(sd->fill && lend)
		{
			// finish skipping over an earlier line end
			if( (*sd->bufp == '\n' || *sd->bufp == '\r') && *sd->bufp != lend)
			{
				++sd->bufp;
				--sd->fill;
			}
			// whatever happened, no half-line-end lurking anymore
			return line->fill;
		}
		if(sd->fill)
		{
			// look for line end here, copy things
			size_t i = 0;
			while(i < sd->fill && sd->bufp[i] != '\n' && sd->bufp[i] != '\r')
				++i;
			// either found an end, or hit the end
			if(!mpg123_add_substring(line, sd->bufp, 0, i))
				return -1; // out of memory
			// skip over a line end if found
			if(i == sd->fill)
			{
				// not done yet, refill, please
				sd->fill = 0;
			} else
			{
				// got end and stored complete line, but need to go on to capture full line end
				lend = sd->bufp[i]; // either \r or \n
				sd->bufp += i+1;
				sd->fill -= i+1;
			}
		} else
		{
			debug("re-filling buffer");
			// refill buffer
			mpg123_ssize_t ret = stream_read_raw(sd, sd->buf, sizeof(sd->buf));
			mdebug("raw read return: %zd", ret);
			if(ret < 0)
				return -1;
			else if(ret == 0)
				return line->fill; // A line ends at end of file.
			else
			{
				sd->fill = ret;
				sd->bufp = sd->buf;
			}
		}
	}
}

#ifdef NETWORK
// Return 0 on success, non-zero when there is an error or more work to do.
// -1: error, 1: redirection to given location
static int stream_parse_headers(struct stream *sd, mpg123_string *location)
{
	int ret = 0;
	mpg123_string line;
	mpg123_init_string(&line);
	mpg123_string icyint;
	mpg123_init_string(&icyint);
	int redirect = 0;
	location->fill = 0;
	const char   *head[] = { "content-type",        "icy-name"
	,	"icy-url",        "icy-metaint", "location" };
	mpg123_string *val[] = { &sd->htd.content_type, &sd->htd.icy_name
	,	&sd->htd.icy_url, &icyint,        location };
	int hn = sizeof(head)/sizeof(char*);
	int hi = -1;
	int got_ok = 0;
	int got_line = 0;
	debug("parsing headers");
	while(stream_getline(sd, &line) > 0)
	{
		got_line = 1;
		mdebug("HTTP in: %s", line.p);
		if(line.p[0] == 0)
		{
			break; // This is the content separator line.
		}
		{ // Convert from unknown/ASCII encoding to UTF-8. Play safe.
			char *buf = NULL;
			if(unknown2utf8(&buf, line.p, -1))
			{
				error("failed converting HTTP header line");
				continue;
			}
			// Avoiding extra allocation here would be nice. mpg123_adopt_string()?
			mpg123_set_string(&line, buf);
			free(buf);
		}
		// React to HTTP error codes, but do not enforce an OK being sent as Shoutcast
		// only produces very minimal headers, not even a HTTP response code.
		// Well, ICY 200 OK could be there, but then we got other headers to know
		// things are fine.
		if(!strncasecmp("http/", line.p, 5))
		{
			// HTTP/1.1 200 OK
			char *tok = line.p;
			while(*tok && *tok != ' ' && *tok != '\t')
				++tok;
			while(*tok && (*tok == ' ' || *tok == '\t'))
				++tok;
			if(tok && *tok != '2')
			{
				if(*tok == '3')
				{
					redirect = ret = 1;
					if(param.verbose > 2)
						fprintf(stderr, "Note: HTTP redirect\n");
				} else
				{
					merror("HTTP error response: %s", line.p);
					ret = -1;
					break;
				}
			} else if(tok && *tok == '2')
			{
				if(param.verbose > 2)
					fprintf(stderr, "Note: got a positive HTTP response\n");
				got_ok = 1;
			}
		}
		if(hi >= 0 && (line.p[0] == ' ' || line.p[0] == '\t'))
		{
			debug("header continuation");
			// nh continuation line, appending to already stored value.
			char *v = line.p+1;
			while(*v == ' ' || *v == '\t'){ ++v; }
			if(!mpg123_add_string(val[hi], v))
			{
				merror("failed to grow header value for %s", head[hi]);
				hi = -1;
				continue;
			}
		}
		char *n = line.p;
		char *v = strchr(line.p, ':');
		if(!v)
			continue; // No proper header line.
		// Got a header line.
		*v = 0; // Terminate the header name.
		if(param.verbose > 2)
			fprintf(stderr, "Note: got header: %s\n", n);
		++v; // Value starts after : and whitespace.
		while(*v == ' ' || *v == '\t'){ ++v; }
		for(hi = 0; hi<hn; ++hi)
		{
			if(!strcasecmp(n, head[hi]))
				break;
		}
		if(hi == hn)
		{
			debug("skipping uninteresting header");
			hi = -1;
			continue;
		}
		if(param.verbose > 2)
			fprintf(stderr, "Note: storing HTTP header %s: %s\n", head[hi],  v);
		got_ok = 1; // When we got some header to store, things seem fine.
		if(!mpg123_set_string(val[hi], v))
		{
			error("failed to allocate header value storage");
			hi = -1;
			continue;
		}
	}
	if(!redirect && icyint.fill)
	{
		sd->htd.icy_interval = atol(icyint.p);
		if(param.verbose > 1)
			fprintf(stderr, "Info: ICY interval %li\n", (long)sd->htd.icy_interval);
	}
	if(!got_line)
	{
		error("no data at all from network resource");
		ret = -1;
	} else if(redirect && !location->fill)
	{
		error("redirect but no location given");
	} else if(!got_ok)
	{
		error("missing positive server response");
		ret = -1;
	}
	mpg123_free_string(&icyint);
	mpg123_free_string(&line);
	return ret;
}

// resolve relative locations given the initial full URL
// full URL ensured to either start with http:// or https://
// (case-insensitive), location non-empty
static void relocate_url(mpg123_string *location, const char *url)
{
	if(!strncasecmp(location->p, "http://", 7) || !strncasecmp(location->p, "https://", 8))
		return;
	if(!url || (strncasecmp(url, "http://", 7) && strncasecmp(url, "https://", 8)))
	{
		mpg123_resize_string(location, 0);
		return;
	}

	if(!param.quiet)
		fprintf(stderr, "NOTE: no complete URL in redirect, constructing one\n");

	mpg123_string purl;
	mpg123_string workbuf;
	mpg123_init_string(&purl);
	mpg123_init_string(&workbuf);

	if(mpg123_set_string(&purl, url) && mpg123_move_string(location, &workbuf))
	{
		debug1("relocate request_url: %s", purl.p);
		// location somewhat relative, either /some/path or even just some/path
		char* ptmp = NULL;
		// Though it's not RFC (?), accept relative URIs as wget does.
		if(workbuf.p[0] == '/')
		{
			// server-absolute only prepend http://server/
			// I null the first / after http:// or https://
			size_t off = (purl.p[4] == 's') ? 8 : 7;
			ptmp = strchr(purl.p+off,'/');
			if(ptmp != NULL)
			{
				purl.fill = ptmp-purl.p+1;
				purl.p[purl.fill-1] = 0;
			}
		}
		else
		{
			// relative to current directory
			// prepend http://server/path/
			// first cutting off parameter stuff from URL
			for(size_t i=0; i<purl.fill; ++i)
			{
				if(purl.p[i] == '?' || purl.p[i] == '#')
				{
					purl.p[i] = 0;
					purl.fill = i+1;
					break;
				}
			}
			// now the last slash, keeping it
			ptmp = strrchr(purl.p, '/');
			if(ptmp != NULL)
			{
				purl.fill = ptmp-purl.p+2;
				purl.p[purl.fill-1] = 0;
			}
		}
		// only the prefix left
		debug1("prefix=%s", purl.p);
		mpg123_add_string(location, purl.p);
	}

	mpg123_add_string(location, workbuf.p);

	mpg123_free_string(&workbuf);
	mpg123_free_string(&purl);
}

#endif

static void stream_init(struct stream *sd)
{
	sd->bufp = sd->buf;
	sd->fill = 0;
	sd->fd = -1;
#ifdef NET123
	sd->nh = NULL;
#endif
	httpdata_init(&sd->htd);
}

// Clean up things for another connection.
// Does not reset the network flag.
static void stream_reset(struct stream *sd)
{
#ifdef NET123
	if(sd->nh)
		sd->nh->close(sd->nh);
	sd->nh = NULL;
#endif
	if(sd->fd >= 0) // plain file or network socket
		close(sd->fd);
	sd->fd = -1;
	httpdata_free(&sd->htd);
	httpdata_init(&sd->htd);
	sd->bufp = sd->buf;
	sd->fill = 0;
}

struct stream *stream_open(const char *url)
{
	struct stream *sd = malloc(sizeof(struct stream));
	if(!sd)
		return NULL;
	stream_init(sd);
	mdebug("opening resource %s", url);
	if(!strcmp(url, "-"))
	{
		sd->fd = STDIN_FILENO;
		INT123_compat_binmode(STDIN_FILENO, TRUE);
	}
#ifdef NETWORK
	else if(!strncasecmp("http://", url, 7) || !strncasecmp("https://", url, 8))
	{
		// Network stream with header parsing.
		const char *client_head[] = { NULL, NULL, NULL };
		client_head[0] = param.talk_icy ? icy_yes : icy_no;
		mpg123_string accept;
		mpg123_init_string(&accept);
		append_accept(&accept);
		client_head[1] = accept.p;
		mpg123_string location;
		mpg123_string urlcopy;
		mpg123_init_string(&location);
		mpg123_init_string(&urlcopy);
		int numrelocs = 0;
		while(sd)
		{
			sd->nh = net123_open(url, client_head, &sd->htd);
			if(!sd->nh)
			{
				stream_close(sd);
				sd = NULL;
				break;
			}
			location.fill = 0;
			if(stream_parse_headers(sd, &location))
			{
				stream_reset(sd);
				if(location.fill)
				{
					if(++numrelocs > HTTP_MAX_RELOCATIONS)
					{
						merror("too many HTTP redirections: %i", numrelocs);
						url = NULL;
					} else
					{
						relocate_url(&location, url); // resolve relative locations
						mpg123_copy_string(&location, &urlcopy);
						url = urlcopy.p;
					}
				} else
				{
					url = NULL;
				}
				if(!url)
				{
					stream_close(sd);
					sd = NULL;
				}
			} else break; // Successful end.
		}
		mpg123_free_string(&urlcopy);
		mpg123_free_string(&location);
		mpg123_free_string(&accept);
		// Either sd is NULL or we got a stream ready.
	}
#endif
	else
	{
		// plain file access
		if(!strncasecmp("file://", url, 7))
			url+= 7; // Might be useful to prepend file scheme prefix for local stuff.
		errno = 0;
		sd->fd = INT123_compat_open(url, O_RDONLY|O_BINARY);
		if(sd->fd < 0)
		{
			merror("failed to open file: %s: %s", url, INT123_strerror(errno));
			stream_close(sd);
			return NULL;
		}
	}
	return sd;
}

void stream_close(struct stream *sd)
{
	if(!sd)
		return;
	stream_reset(sd);
	free(sd);
}

/* Read data from input, write copy to dump file. */
static mpg123_ssize_t dump_read(void *handle, void *buf, size_t count)
{
	struct stream *sd = handle;
	mpg123_ssize_t ret = stream_read(sd, buf, count);
	if(ret > 0 && dump_fd > -1)
	{
		ret = INT123_unintr_write(dump_fd, buf, ret);
	}
	return ret;
}

/* Also mirror seeks, to prevent messed up dumps of seekable streams. */
static off_t dump_seek(void *handle, off_t pos, int whence)
{
	struct stream *sd = handle;
	off_t ret = stream_seek(sd, pos, whence);
	if(ret >= 0 && dump_fd > -1)
	{
		ret = lseek(dump_fd, pos, whence);
	}
	return ret;
}

/* External API... open and close. */
int dump_setup(struct stream *sd, mpg123_handle *mh)
{
	int ret = MPG123_OK;
	int do_replace = 0; // full replacement with handle

	// paranoia: if buffer active, ensure handle I/O
	if(sd->fill)
		do_replace = 1;
#ifdef NET123
	if(sd->nh)
		do_replace = 1;
#endif
	if(param.streamdump)
	{
		do_replace = 1;
		// open freshly or keep open
		if(dump_fd < 0)
		{
			if(!param.quiet)
				fprintf(stderr, "Note: Dumping stream to %s\n", param.streamdump);
			dump_fd = INT123_compat_open(param.streamdump, O_CREAT|O_TRUNC|O_RDWR);
		}
		if(dump_fd < 0)
		{
			error1("Failed to open dump file: %s\n", INT123_strerror(errno));
			return -1;
		}
#ifdef WIN32
		_setmode(dump_fd, _O_BINARY);
#endif
	}

	if( MPG123_OK != mpg123_param(mh, MPG123_ICY_INTERVAL
	,	param.icy_interval ? param.icy_interval : sd->htd.icy_interval, 0) )
		error1("Cannot set ICY interval: %s", mpg123_strerror(mh));
	if(param.icy_interval > 0 && param.verbose > 1)
		fprintf(stderr, "Info: Forced ICY interval %li\n", param.icy_interval);

	if(do_replace)
	{
		mpg123_replace_reader_handle(mh, dump_read, dump_seek, NULL);
		ret = mpg123_open_handle(mh, sd);
	} else
	{
		mpg123_replace_reader(mh, NULL, NULL);
		ret = mpg123_open_fd(mh, sd->fd);
	}
	if(ret != MPG123_OK)
	{
		error1("Unable to replace reader/open track for stream dump: %s\n", mpg123_strerror(mh));
		dump_close();
		return -1;
	}
	else return 0;
}

void dump_close(void)
{
	if(dump_fd > -1) INT123_compat_close(dump_fd);

	dump_fd = -1;
}
