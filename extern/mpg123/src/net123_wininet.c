#include "config.h"
#include "version.h"
#include "net123.h"
#include "compat/compat.h"
#include "common/debug.h"
#include <ws2tcpip.h>
#include <wininet.h>

// The network implementation defines the struct for private use.
// The purpose is just to keep enough context to be able to
// call net123_read() and net123_close() afterwards.
#define URL_COMPONENTS_LENGTH 255
typedef struct {
  HINTERNET session;
  HINTERNET connect;
  HINTERNET request;
  URL_COMPONENTSW comps;
  wchar_t lpszHostName[URL_COMPONENTS_LENGTH];
  wchar_t lpszUserName[URL_COMPONENTS_LENGTH];
  wchar_t lpszPassword[URL_COMPONENTS_LENGTH];
  wchar_t lpszUrlPath[URL_COMPONENTS_LENGTH];
  wchar_t lpszExtraInfo[URL_COMPONENTS_LENGTH];
  wchar_t lpszScheme[URL_COMPONENTS_LENGTH];
  DWORD supportedAuth, firstAuth, authTarget, authTried;
  char *headers;
  size_t headers_pos, headers_len;
  DWORD HttpQueryInfoIndex;
  DWORD internetStatus, internetStatusLength;
  LPVOID additionalInfo;
} wininet_handle;

#define MPG123CONCAT_(x,y) x ## y
#define MPG123CONCAT(x,y) MPG123CONCAT_(x,y)
#define MPG123STRINGIFY_(x) #x
#define MPG123STRINGIFY(x) MPG123STRINGIFY_(x)
#define MPG123WSTR(x) MPG123CONCAT(L,MPG123STRINGIFY(x))

#if DEBUG
static void debug_crack(URL_COMPONENTSW *comps){
  wprintf(L"dwStructSize: %lu\n", comps->dwStructSize);
  wprintf(L"lpszScheme: %s\n", comps->lpszScheme ? comps->lpszScheme : L"null");
  wprintf(L"dwSchemeLength: %lu\n", comps->dwSchemeLength);
  wprintf(L"nScheme: %u %s\n", comps->nScheme, comps->nScheme == 1 ? L"INTERNET_SCHEME_HTTP": comps->nScheme == 2 ? L"INTERNET_SCHEME_HTTPS" : L"UNKNOWN");
  wprintf(L"lpszHostName: %s\n", comps->lpszHostName ? comps->lpszHostName : L"null");
  wprintf(L"dwHostNameLength: %u\n", comps->dwHostNameLength);
  wprintf(L"nPort: %u\n", comps->nPort);
  wprintf(L"lpszUserName: %s\n", comps->lpszUserName ? comps->lpszUserName : L"null");
  wprintf(L"dwUserNameLength: %lu\n", comps->dwUserNameLength);
  wprintf(L"lpszPassword: %s\n", comps->lpszPassword ? comps->lpszPassword : L"null");
  wprintf(L"dwPasswordLength: %lu\n", comps->dwPasswordLength);
  wprintf(L"lpszUrlPath: %s\n", comps->lpszUrlPath ? comps->lpszUrlPath : L"null");
  wprintf(L"dwUrlPathLength: %lu\n", comps->dwUrlPathLength);
  wprintf(L"lpszExtraInfo: %s\n", comps->lpszExtraInfo? comps->lpszExtraInfo : L"null");
  wprintf(L"dwExtraInfoLength: %lu\n", comps->dwExtraInfoLength);
}
#else
static void debug_crack(URL_COMPONENTSW *comps){}
#endif

static
void WINAPI net123_ssl_errors(HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength){
  debug("In net123_ssl_errors");
  wininet_handle *nh = (wininet_handle *)(dwContext);
  nh->internetStatus = dwInternetStatus;
  nh->additionalInfo = lpvStatusInformation;
  nh->internetStatusLength = dwStatusInformationLength;
}

static size_t net123_read(net123_handle *nh, void *buf, size_t bufsize);
static void net123_close(net123_handle *nh);

net123_handle *net123_open_wininet(const char *url, const char * const *client_head){
  LPWSTR urlW = NULL, headers = NULL;
  size_t ii;
  WINBOOL res;
  DWORD headerlen;
  net123_handle *ret = NULL;
  const LPCWSTR useragent = MPG123WSTR(PACKAGE_NAME) L"/" MPG123WSTR(MPG123_VERSION);
  INTERNET_STATUS_CALLBACK cb;

  INT123_win32_utf8_wide(url, &urlW, NULL);
  if(urlW == NULL) goto cleanup;

  ret = calloc(1, sizeof(net123_handle));
  wininet_handle *wh = calloc(1, sizeof(wininet_handle));
  if(!ret || !wh)
  {
     if(ret)
       free(ret);
     if(wh)
       free(wh);
     return NULL;
  }

  ret->parts = wh;
  ret->read  = net123_read;
  ret->close = net123_close;

  wh->comps.dwStructSize = sizeof(wh->comps);
  wh->comps.dwSchemeLength    = URL_COMPONENTS_LENGTH - 1;
  wh->comps.dwUserNameLength  = URL_COMPONENTS_LENGTH - 1;
  wh->comps.dwPasswordLength  = URL_COMPONENTS_LENGTH - 1;
  wh->comps.dwHostNameLength  = URL_COMPONENTS_LENGTH - 1;
  wh->comps.dwUrlPathLength   = URL_COMPONENTS_LENGTH - 1;
  wh->comps.dwExtraInfoLength = URL_COMPONENTS_LENGTH - 1;
  wh->comps.lpszHostName = wh->lpszHostName;
  wh->comps.lpszUserName = wh->lpszUserName;
  wh->comps.lpszPassword = wh->lpszPassword;
  wh->comps.lpszUrlPath = wh->lpszUrlPath;
  wh->comps.lpszExtraInfo = wh->lpszExtraInfo;
  wh->comps.lpszScheme = wh->lpszScheme;

  debug1("net123_open start crack %S", urlW);

  if(!(res = InternetCrackUrlW(urlW, 0, 0, &wh->comps))) {
    debug1("net123_open crack fail %lu", GetLastError());
    goto cleanup;
  }

  debug("net123_open crack OK");
  debug_crack(&wh->comps);

  wh->session = InternetOpenW(useragent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
  free(urlW);
  urlW = NULL;
  debug("net123_open InternetOpenW OK");
  if(!wh->session) goto cleanup;

  debug2("net123_open InternetConnectW %S %u", wh->comps.lpszHostName, wh->comps.nPort);
  wh->connect = InternetConnectW(wh->session, wh->comps.lpszHostName, wh->comps.nPort,
    wh->comps.dwUserNameLength ? wh->comps.lpszUserName : NULL, wh->comps.dwPasswordLength ? wh->comps.lpszPassword : NULL,
    INTERNET_SERVICE_HTTP, 0, 0);
  if(!wh->connect) goto cleanup;
  debug("net123_open InternetConnectW OK");

  debug1("HttpOpenRequestW GET %S", wh->comps.lpszUrlPath);
  wh->request = HttpOpenRequestW(wh->connect, L"GET", wh->comps.lpszUrlPath, NULL, NULL, NULL, wh->comps.nScheme == INTERNET_SCHEME_HTTPS ? INTERNET_FLAG_SECURE : 0, (DWORD_PTR)wh);
  if(!wh->request) goto cleanup;
  debug("HttpOpenRequestW GET OK");

  cb = InternetSetStatusCallback(wh->request, (INTERNET_STATUS_CALLBACK)net123_ssl_errors);
  if(cb != NULL){
    error1("InternetSetStatusCallback failed to install callback, errors might not be reported properly! (%lu)", GetLastError());
  }

  for(ii = 0; client_head[ii]; ii++){
    INT123_win32_utf8_wide(client_head[ii], &headers, NULL);
    if(!headers)
      goto cleanup;
    debug1("HttpAddRequestHeadersW add %S", headers);
    res = HttpAddRequestHeadersW(wh->request, headers, (DWORD) -1, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
    debug2("HttpAddRequestHeadersW returns %u %lu", res, res ? 0 : GetLastError());
    free(headers);
    headers = NULL;
  }

  debug("net123_open ADD HEADERS OK");

  res = HttpSendRequestW(wh->request, NULL, 0, NULL, 0);

  if (!res) {
    res = GetLastError();
    error1("HttpSendRequestW failed with %d", res);
    goto cleanup;
  }
  debug("HttpSendRequestW OK");

  // dummy, cannot be null
  headers = calloc(1,1);
  headerlen = 1;
  if(headers == NULL) {
    error("Cannot allocate dummy buffer for HttpQueryInfoW");
    goto cleanup;
  }
  debug("Try HttpQueryInfoW pass 1");
  res = HttpQueryInfoW(wh->request, HTTP_QUERY_RAW_HEADERS_CRLF, headers, &headerlen, &wh->HttpQueryInfoIndex);
  free(headers);
  debug("HttpQueryInfoW pass 1 OK");

  if(!res && GetLastError() == ERROR_INSUFFICIENT_BUFFER && headerlen > 0) {
    /* buffer size is in bytes, not including terminator */
    headers = calloc(1, headerlen + sizeof(*headers));
    if (!headers) goto cleanup;
    res = HttpQueryInfoW(wh->request, HTTP_QUERY_RAW_HEADERS_CRLF, headers, &headerlen, &wh->HttpQueryInfoIndex);
    debug3("HttpQueryInfoW returned %u, err %u : %S", res, GetLastError(), headers ? headers : L"null");
    INT123_win32_wide_utf7(headers, &wh->headers, &wh->headers_len);
    /* bytes written, skip the terminating null, we want to stop at the \r\n\r\n */
    wh->headers_len --;
    free(headers);
    headers = NULL;
  } else {
    error("HttpQueryInfoW did not execute as expected");
    goto cleanup;
  }
  debug("net123_open OK");

  return ret;
cleanup:
  debug("net123_open error");
  if (urlW) free(urlW);
  if(ret) {
    net123_close(ret);
    ret = NULL;
  }
  return ret;
}

static size_t net123_read(net123_handle *nh, void *buf, size_t bufsize){
  if(!nh || !nh->parts)
    return 0;
  wininet_handle *wh = nh->parts;
  size_t ret;
  size_t to_copy = wh->headers_len - wh->headers_pos;
  DWORD bytesread = 0;

  if(to_copy){
     ret = to_copy <= bufsize ? to_copy : bufsize;
     memcpy(buf, wh->headers + wh->headers_pos, ret);
     wh->headers_pos += ret;
     return ret;
  }

  /* is this needed? */
  to_copy = bufsize > ULONG_MAX ? ULONG_MAX : bufsize;
  if(!InternetReadFile(wh->request, buf, to_copy, &bytesread)){
    error1("InternetReadFile exited with %ld", GetLastError());
    return EOF;
  }
  return bytesread;
}

// Call that to free up resources, end processes.
static void net123_close(net123_handle *nh){
  if(!nh)
    return;
  wininet_handle *wh = nh->parts;
  if(!wh) /*???*/
    return;
  if(wh->headers) {
    free(wh->headers);
    wh->headers = NULL;
  }
  if(wh->request) {
    InternetCloseHandle(wh->request);
    wh->request = NULL;
  }
  if(wh->connect) {
    InternetCloseHandle(wh->connect);
    wh->connect = NULL;
  }
  if(wh->session) {
    InternetCloseHandle(wh->session);
    wh->session = NULL;
  }
  free(nh);
  free(wh);
}
