/*
	win32_net: Windows-specific network code

	copyright 2009-2023 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Jonathan Yong (extracting out of httpget.c)
*/

#include "win32_support.h"
#include "mpg123app.h"
#include "common/debug.h"

#ifdef DEBUG
#define msgme(x) win32_net_msg(x,__FILE__,__LINE__)
#define msgme1 win32_net_msg(1,__FILE__,__LINE__)
#define msgme_sock_err(x) if ((x)==SOCKET_ERROR) {msgme1;}
#else
#define msgme(x) x
#define msgme1 do{} while(0)
#define msgme_sock_err(x) x
#endif
struct ws_local
{
  int inited;
  SOCKET local_socket; /*stores last connet in win32_net_open_connection*/
  WSADATA wsadata;
};

static struct ws_local ws;
#ifdef DEBUG
static void win32_net_msg (const int err, const char * const filedata, const int linedata)
{
  char *errbuff;
  int lc_err;
  if (err)
  {
    lc_err = WSAGetLastError();
    FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      lc_err,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPTSTR) &errbuff,
      0,
      NULL );
    fprintf(stderr, "[%s:%d] [WSA2: %d] %s", filedata, linedata, lc_err, errbuff);
    LocalFree (errbuff);
  }
}
#endif

void win32_net_init (void)
{
  ws.inited = 1;
  switch ((WSAStartup(MAKEWORD(2,2), &ws.wsadata)))
  {
    case WSASYSNOTREADY: debug("WSAStartup failed with WSASYSNOTREADY"); break;
    case WSAVERNOTSUPPORTED: debug("WSAStartup failed with WSAVERNOTSUPPORTED"); break;
    case WSAEINPROGRESS: debug("WSAStartup failed with WSAEINPROGRESS"); break;
    case WSAEPROCLIM: debug("WSAStartup failed with WSAEPROCLIM"); break;
    case WSAEFAULT: debug("WSAStartup failed with WSAEFAULT"); break;
    default:
    break;
  }
}

void win32_net_deinit (void)
{
  debug("Begin winsock cleanup");
  if (ws.inited)
  {
    if (ws.inited >= 2 && ws.local_socket != SOCKET_ERROR)
    {
      debug1("ws.local_socket = %" PRIuMAX, (uintmax_t)ws.local_socket);
      msgme_sock_err(shutdown(ws.local_socket, SD_BOTH));
      win32_net_close(ws.local_socket);
    }
    WSACleanup();
    ws.inited = 0;
  }
}

void win32_net_close (int sock)
{
    msgme_sock_err(closesocket(ws.local_socket));
}

static void win32_net_nonblock(int sock)
{
  u_long mode = 1;
  msgme_sock_err(ioctlsocket(ws.local_socket, FIONBIO, &mode));
}

static void win32_net_block(int sock)
{
  u_long mode = 0;
  msgme_sock_err(ioctlsocket(ws.local_socket, FIONBIO, &mode));
}

mpg123_ssize_t win32_net_read (int fildes, void *buf, size_t nbyte)
{
  debug1("Attempting to read %zu bytes from network.", nbyte);
  mpg123_ssize_t ret;
  msgme_sock_err(ret = (mpg123_ssize_t) recv(ws.local_socket, buf, nbyte, 0));
  debug1("Read %" PRIiMAX " bytes from network.", (intmax_t)ret);

  return ret;
}

/*
static int get_sock_ch (int sock)
{
  char c;
  int ret;
  msgme_sock_err(ret = recv (ws.local_socket, &c, 1, 0));
  if (ret == 1)
    return (((int) c)&0xff);
  return -1;
}
*/

mpg123_ssize_t win32_net_write (int fildes, const void *buf, size_t nbyte)
{
  debug1("Attempting to write %zu bytes to network.", nbyte);
  mpg123_ssize_t ret;
  msgme_sock_err((ret = (mpg123_ssize_t) send(ws.local_socket, buf, nbyte, 0)));
  debug1("wrote %" PRIiMAX " bytes to network.", (intmax_t)ret);

  return ret;
}

static int win32_net_timeout_connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen)
{
	debug("win32_net_timeout_connect ran");
	if(param.timeout > 0)
	{
		int err;
		win32_net_nonblock(ws.local_socket);
		err = connect(ws.local_socket, serv_addr, addrlen);
		if(err != SOCKET_ERROR)
		{
			debug("immediately successful");
			win32_net_block(ws.local_socket);
			return 0;
		}
		else if(WSAGetLastError() == WSAEWOULDBLOCK) /*WSAEINPROGRESS would not work here for some reason*/
		{
			struct timeval tv;
			fd_set fds;
			tv.tv_sec = param.timeout;
			tv.tv_usec = 0;

			debug("in progress, waiting...");

			FD_ZERO(&fds);
			FD_SET(ws.local_socket, &fds);
			err = select(ws.local_socket+1, NULL, &fds, NULL, &tv);
			if(err != SOCKET_ERROR)
			{
				socklen_t len = sizeof(err);
				if((getsockopt(ws.local_socket, SOL_SOCKET, SO_ERROR, (char *)&err, &len) != SOCKET_ERROR)
				   && (err == 0) )
				{
					debug("non-blocking connect has been successful");
					win32_net_block(ws.local_socket);
					return 0;
				}
				else
				{
					//error1("connection error: %s", msgme(err));
					return -1;
				}
			}
			else if(err == 0)
			{
				error("connection timed out");
				return -1;
			}
			else
			{
				/*error1("error from select(): %s", INT123_strerror(errno));*/
				debug("error from select():");
				msgme1;
				return -1;
			}
		}
		else
		{
			/*error1("connection failed: %s", INT123_strerror(errno));*/
			debug("connection failed: ");
			msgme1;
			return err;
		}
	}
	else
	{
		if(connect(ws.local_socket, serv_addr, addrlen) == SOCKET_ERROR)
		{
			/*error1("connection failed: %s", INT123_strerror(errno));*/
			debug("connection failed");
			msgme1;
			return -1;
		}
		else {
		debug("win32_net_timeout_connect succeed");
		return 0; /* _good_ */
		}
	}
}

int win32_net_open_connection(mpg123_string *host, mpg123_string *port)
{
	struct addrinfo hints;
	struct addrinfo *addr, *addrlist;
	SOCKET addrcount;
	ws.local_socket = SOCKET_ERROR;

	if(param.verbose>1) fprintf(stderr, "Note: Attempting new-style connection to %s\n", host->p);
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family   = AF_UNSPEC; /* We accept both IPv4 and IPv6 ... and perhaps IPv8;-) */
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	debug2("Atempt resolve/connect to %s:%s", host->p, port->p);
	msgme(addrcount = getaddrinfo(host->p, port->p, &hints, &addrlist));

	if(addrcount == INVALID_SOCKET)
	{
		error3("Resolving %s:%s: %s", host->p, port->p, gai_strerror(addrcount));
		return -1;
	}

	addr = addrlist;
	while(addr != NULL)
	{
		ws.local_socket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
		if (ws.local_socket == INVALID_SOCKET)
		{
			msgme1;
		}
		else
		{
			if(win32_net_timeout_connect(ws.local_socket, addr->ai_addr, addr->ai_addrlen) == 0)
			break;
			debug("win32_net_timeout_connect error, closing socket");
			win32_net_close(ws.local_socket);
			ws.local_socket=SOCKET_ERROR;
		}
		addr=addr->ai_next;
	}
	if(ws.local_socket == SOCKET_ERROR) {error2("Cannot resolve/connect to %s:%s!", host->p, port->p);}
	else
	{
	  ws.inited = 2;
	}

	freeaddrinfo(addrlist);
	return ws.local_socket == SOCKET_ERROR ? -1 : 1;
}

int win32_net_writestring (int fd, mpg123_string *string)
{
	size_t result, bytes;
	char *ptr = string->p;
	bytes = string->fill ? string->fill-1 : 0;

	while(bytes)
	{
		result = win32_net_write(ws.local_socket, ptr, bytes);
		if(result < 0 && WSAGetLastError() != WSAEINTR)
		{
			perror ("writing http string");
			return FALSE;
		}
		else if(result == 0)
		{
			error("write: socket closed unexpectedly");
			return FALSE;
		}
		ptr   += result;
		bytes -= result;
	}
	return TRUE;
}
