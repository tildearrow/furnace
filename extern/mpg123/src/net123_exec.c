/*
	net123_exec: network (HTTP(S)) streaming for mpg123 via fork+exec

	copyright 2022-2023 by the mpg123 project
	free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Thomas Orgis

	This avoids linking any network code directly into mpg123, just using external
	tools at runtime.

	This calls wget with fallback to curl by default, one of those two
	specifically if param.network_backend is set accordingly.
*/

// kill
#define _XOPEN_SOURCE 600
#define _POSIX_C_SOURCE 200112L

#include "config.h"
#include "version.h"
#include "net123.h"
// for strings
#include "mpg123.h"

// Just for parameter struct that we use for HTTP auth and proxy info.
#include "mpg123app.h"

#include "compat/compat.h"

#include "common/debug.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Those are set via environment variables:
// http_proxy
// https_proxy
//    If set, the http_proxy and https_proxy variables should contain the
//    URLs of the proxies for HTTP and HTTPS connections respectively.
// ftp_proxy
//    same
// wget --user=... --password=... 
// Alternatively: Have them in .netrc.

typedef struct
{
	int fd;
	pid_t worker;
} exec_handle;

// Combine two given strings into one newly allocated one.
// Use: (--parameter=, value) -> --parameter=value
static char *catstr(const char *par, const char *value)
{
	char *res = malloc(strlen(par)+strlen(value)+1);
	if(res)
	{
		res[0] = 0;
		strcat(res, par);
		strcat(res, value);
	}
	return res;
}

// < 0: not checked, 0: not there, 1: present
static int got_curl = -1;
static int got_wget = -1;

// Check if program executes, also test if given token occurs in its output.
// Returns 0 if not, 1 if exec works, 2 if also token found.
// Token has to be < 1024 characters in length.
static int check_program(char * const *argv, const char *token)
{
	int fd[2];
	int gottoken = 0;
	if(token)
	{
		if(pipe(fd))
			return 0;
		INT123_compat_binmode(fd[0], TRUE);
		INT123_compat_binmode(fd[1], TRUE);
	}
	pid_t pid = fork();
	if(pid == 0)
	{
		int outfd = fd[1];
		if(token)
			close(fd[0]);
		else
			outfd = open("/dev/null", O_WRONLY);
		dup2(outfd, STDOUT_FILENO);
		int infd  = open("/dev/null", O_RDONLY);
		dup2(infd,  STDIN_FILENO);
		int errfd = open("/dev/null", O_WRONLY);
		dup2(errfd, STDERR_FILENO);
		execvp(argv[0], argv);
		exit(1);
	}
	else if(pid > 0)
	{
		if(token)
		{
			char buf[1024];
			close(fd[1]);
			size_t toklen = strlen(token);
			if(toklen > 0 && toklen < sizeof(buf))
			{
				size_t bufoff = 0;
				size_t got;
				while( (got = INT123_unintr_read(fd[0], buf+bufoff, sizeof(buf)-1-bufoff)) )
				{
					bufoff += got;
					buf[bufoff] = 0; // Now it's a terminated string.
					if(!gottoken && strstr(buf, token))
						gottoken = 1;
					if(gottoken)
						bufoff = 0; // just forget everything
					else if(bufoff > toklen)
					{
						// Remember the last toklen-1 bytes to compare later.
						memmove(buf, buf+bufoff-toklen+1, toklen-1);
						bufoff = toklen-1;
					}
				}
			}
			close(fd[0]);
		}
		int stat;
		if( (waitpid(pid, &stat, 0) == pid)
			&& WIFEXITED(stat) && WEXITSTATUS(stat)==0 )
			return 1+gottoken;
	} else if(token)
	{
		close(fd[0]);
		close(fd[1]);
	}
	return 0; // false, not there
}

static char **wget_argv(const char *url, const char * const * client_head)
{
	const char* base_args[] =
	{
		"wget" // begins with program name
	,	"--output-document=-"
#ifndef DEBUG
	,	"--quiet"
#endif
	,	"--save-headers"
	};
	size_t cheads = 0;
	while(client_head && client_head[cheads]){ ++cheads; }
	// Get the count of argument strings right!
	// Fixed args + agent + client headers [+ auth] + URL + NULL
	int argc = sizeof(base_args)/sizeof(char*)+1+cheads+1+1;
	char *httpauth = NULL;
	char *user = NULL;
	char *password = NULL;
	if(param.httpauth && (httpauth = INT123_compat_strdup(param.httpauth)))
	{
		char *sep = strchr(httpauth, ':');
		if(sep)
		{
			argc += 2;
			*sep = 0;
			user = httpauth;
			password = sep+1;
		}
	}
	char ** argv = malloc(sizeof(char*)*(argc+1));
	if(!argv)
	{
		error("failed to allocate argv");
		return NULL;
	}
	int an = 0;
	for(;an<sizeof(base_args)/sizeof(char*); ++an)
		argv[an] = INT123_compat_strdup(base_args[an]);
	argv[an++] = INT123_compat_strdup("--user-agent=" PACKAGE_NAME "/" MPG123_VERSION);
	for(size_t ch=0; ch < cheads; ++ch)
		argv[an++] = catstr("--header=", client_head[ch]);
	if(user)
		argv[an++] = catstr("--user=", user);
	if(password)
		argv[an++] = catstr("--password=", password);
	argv[an++] = INT123_compat_strdup(url);
	argv[an++] = NULL;
	return argv;
}

static char **curl_argv(const char *url, const char * const * client_head)
{
	const char* base_args[] =
	{
		"curl" // begins with program name
#ifdef DEBUG
	,	"--verbose"
#else
	,	"--silent"
	,	"--show-error"
#endif
	,	"--dump-header"
	,	"-"
	};
	size_t cheads = 0;
	while(client_head && client_head[cheads]){ ++cheads; }
	// Get the count of argument strings right!
	// Fixed args + agent + client headers [+ auth] + URL + NULL
	int argc = sizeof(base_args)/sizeof(char*)+2+2*cheads+1+1;
	if(got_curl > 1)
		argc++; // add --http0.9
	char *httpauth = NULL;
	if(param.httpauth && (httpauth = INT123_compat_strdup(param.httpauth)))
		argc += 2;
	char ** argv = malloc(sizeof(char*)*(argc+1));
	if(!argv)
	{
		error("failed to allocate argv");
		return NULL;
	}
	int an = 0;
	for(;an<sizeof(base_args)/sizeof(char*); ++an)
		argv[an] = INT123_compat_strdup(base_args[an]);
	if(got_curl > 1)
		argv[an++] = INT123_compat_strdup("--http0.9");
	argv[an++] = INT123_compat_strdup("--user-agent");
	argv[an++] = INT123_compat_strdup(PACKAGE_NAME "/" MPG123_VERSION);
	for(size_t ch=0; ch < cheads; ++ch)
	{
		argv[an++] = INT123_compat_strdup("--header");
		argv[an++] = INT123_compat_strdup(client_head[ch]);
	}
	if(httpauth)
	{
		argv[an++] = INT123_compat_strdup("--user");
		argv[an++] = httpauth;
	}
	argv[an++] = INT123_compat_strdup(url);
	argv[an++] = NULL;
	return argv;
}


static size_t net123_read(net123_handle *nh, void *buf, size_t bufsize)
{
	if(!nh || (bufsize && !buf))
		return 0;
	return INT123_unintr_read(((exec_handle*)nh->parts)->fd, buf, bufsize);
}

static void net123_close(net123_handle *nh)
{
	if(!nh)
		return;
	exec_handle *eh = nh->parts;
	if(eh->worker)
	{
		kill(eh->worker, SIGKILL);
		errno = 0;
		if(waitpid(eh->worker, NULL, 0) < 0)
			merror("failed to wait for worker process: %s", INT123_strerror(errno));
		else if(param.verbose > 1)
			fprintf(stderr, "Note: network helper %"PRIiMAX" finished\n", (intmax_t)eh->worker);
	}
	if(eh->fd > -1)
		close(eh->fd);
	free(nh->parts);
	free(nh);
}


net123_handle *net123_open_exec(const char *url, const char * const * client_head)
{
	int use_curl = 0;
	char * const curl_check_argv[] = { "curl", "--help", "all", NULL };
	char * const wget_check_argv[] = { "wget", "--version", NULL };
	// Semi-threadsafe: The check might take place multiple times, but writing the integer
	// should be safe enough.
	if(!strcmp("auto",param.network_backend))
	{
		if(got_wget < 0)
			got_wget = check_program(wget_check_argv, NULL);
		if(!got_wget && got_curl < 0)
			got_curl = check_program(curl_check_argv, "--http0.9");
		if(got_wget < 1 && got_curl)
			use_curl = 1;
	} else if(!strcmp("curl", param.network_backend))
	{
		if(got_curl < 0) // Still need to know if HTTP/0.9 option is there.
			got_curl = check_program(curl_check_argv, "--http0.9");
		use_curl = 1;
	} else if(!strcmp("wget", param.network_backend))
	{
		if(got_wget < 0)
			got_wget = check_program(wget_check_argv, NULL);
		use_curl = 0;
	} else
	{
		merror("invalid network backend specified: %s", param.network_backend);
		return NULL;
	}
	if((!use_curl && !got_wget) || (use_curl && !got_curl))
	{
		error("missing working network helper program (wget or curl)");
		return NULL;
	}

	int fd[2];
	net123_handle *nh = malloc(sizeof(net123_handle));
	exec_handle *eh   = malloc(sizeof(exec_handle));
	if(!nh || !eh)
	{
		if(nh)
			free(nh);
		if(eh)
			free(eh);
		return NULL;
	}
	nh->parts = eh;
	nh->read  = net123_read;
	nh->close = net123_close;
	eh->fd = -1;
	eh->worker = 0;
	errno = 0;
	if(pipe(fd))
	{	
		merror("failed creating a pipe: %s", INT123_strerror(errno));
		free(nh);
		return NULL;
	}

	INT123_compat_binmode(fd[0], TRUE);
	INT123_compat_binmode(fd[1], TRUE);

	eh->worker = fork();
	if(eh->worker == -1)
	{
		merror("fork failed: %s", INT123_strerror(errno));
		free(nh);
		return NULL;
	}

	if(eh->worker == 0)
	{
		close(fd[0]);
		dup2(fd[1], STDOUT_FILENO);
		int infd  = open("/dev/null", O_RDONLY);
		dup2(infd,  STDIN_FILENO);
		// child
		// Proxy environment variables can just be set in the user and inherited here, right?
		
		char **argv = use_curl ? curl_argv(url, client_head) : wget_argv(url, client_head);
		if(!argv)
			exit(1);
		errno = 0;
		if(!param.quiet)
		{
			if(param.verbose > 2)
			{
				char **a = argv;
				fprintf(stderr, "HTTP helper command:\n");
				while(*a)
				{
					fprintf(stderr, " %s\n", *a);
					++a;
				}
			}
		} else
		{
			int errfd = open("/dev/null", O_WRONLY);
			dup2(errfd, STDERR_FILENO);
		}
		execvp(argv[0], argv);
		merror("cannot execute %s: %s", argv[0], INT123_strerror(errno));
		exit(1);
	}
	// parent
	if(param.verbose > 1)
		fprintf(stderr, "Note: started network helper with PID %"PRIiMAX"\n", (intmax_t)eh->worker);
	errno = 0;
	close(fd[1]);
	eh->fd = fd[0];
	return nh;
}
