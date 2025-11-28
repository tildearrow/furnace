#include "config.h"
#include "version.h"
#include "net123.h"
#include "compat/compat.h"
#include "common/debug.h"
#include <ws2tcpip.h>
#include <winhttp.h>

// The network implementation defines the struct for private use.
// The purpose is just to keep enough context to be able to
// call net123_read() and net123_close() afterwards.
#define URL_COMPONENTS_LENGTH 255
typedef struct {
  HINTERNET session;
  HINTERNET connect;
  HINTERNET request;
  URL_COMPONENTS comps;
  wchar_t lpszHostName[URL_COMPONENTS_LENGTH];
  wchar_t lpszUserName[URL_COMPONENTS_LENGTH];
  wchar_t lpszPassword[URL_COMPONENTS_LENGTH];
  wchar_t lpszUrlPath[URL_COMPONENTS_LENGTH];
  wchar_t lpszExtraInfo[URL_COMPONENTS_LENGTH];
  DWORD supportedAuth, firstAuth, authTarget, authTried;
  char *headers;
  size_t headers_pos, headers_len;
  DWORD internetStatus, internetStatusLength;
  LPVOID additionalInfo;
} winhttp_handle;

#define MPG123CONCAT_(x,y) x ## y
#define MPG123CONCAT(x,y) MPG123CONCAT_(x,y)
#define MPG123STRINGIFY_(x) #x
#define MPG123STRINGIFY(x) MPG123STRINGIFY_(x)
#define MPG123WSTR(x) MPG123CONCAT(L,MPG123STRINGIFY(x))

static DWORD wrap_auth(winhttp_handle *nh){
  DWORD mode;
  DWORD ret;

  if(nh->comps.dwUserNameLength) {
    if(!nh->authTried) {
      ret = WinHttpQueryAuthSchemes(nh->request, &nh->supportedAuth, &nh->firstAuth, &nh->authTarget);
      if(!ret) return GetLastError();
      nh->authTried = 1;
    }

    mode = nh->supportedAuth & WINHTTP_AUTH_SCHEME_DIGEST ? WINHTTP_AUTH_SCHEME_DIGEST :
           nh->supportedAuth & WINHTTP_AUTH_SCHEME_BASIC ? WINHTTP_AUTH_SCHEME_BASIC : 0;

    /* no supported mode? */
    if(!mode)
      return ERROR_WINHTTP_INTERNAL_ERROR;

    ret = WinHttpSetCredentials(nh->request, WINHTTP_AUTH_TARGET_SERVER, mode, nh->comps.lpszUserName, nh->comps.lpszPassword, NULL);
    return GetLastError();
  }
  return TRUE;
}

#if DEBUG
static void debug_crack(URL_COMPONENTS *comps){
  wprintf(L"dwStructSize: %lu\n", comps->dwStructSize);
  //wprintf(L"lpszScheme: %s\n", comps->lpszScheme ? comps->lpszScheme : L"null");
  //wprintf(L"dwSchemeLength: %lu\n", comps->dwSchemeLength);
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
static void debug_crack(URL_COMPONENTS *comps){}
#endif

static
void WINAPI net123_ssl_errors(HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength){
  winhttp_handle *nh = (winhttp_handle *)dwContext;
  nh->internetStatus = dwInternetStatus;
  nh->additionalInfo = lpvStatusInformation;
  nh->internetStatusLength = dwStatusInformationLength;
}

static size_t net123_read(net123_handle *nh, void *buf, size_t bufsize);
static void net123_close(net123_handle *nh);

net123_handle *net123_open_winhttp(const char *url, const char * const *client_head){
  LPWSTR urlW = NULL, headers = NULL;
  size_t ii;
  WINBOOL res;
  DWORD headerlen;
  const LPCWSTR useragent = MPG123WSTR(PACKAGE_NAME) L"/" MPG123WSTR(MPG123_VERSION);
  WINHTTP_STATUS_CALLBACK cb;
  net123_handle *handle = NULL;

  if(!WinHttpCheckPlatform())
    return NULL;

  INT123_win32_utf8_wide(url, &urlW, NULL);
  if(urlW == NULL) goto cleanup;

  winhttp_handle *ret = calloc(1, sizeof(winhttp_handle));
  if (!ret) goto cleanup;

  handle = calloc(1, sizeof(net123_handle));
  if (!handle) {
    free(ret);
    goto cleanup;
  }

  handle->parts = ret;
  handle->read = net123_read;
  handle->close = net123_close;

  ret->comps.dwStructSize = sizeof(ret->comps);
  ret->comps.dwSchemeLength    = 0;
  ret->comps.dwUserNameLength  = URL_COMPONENTS_LENGTH - 1;
  ret->comps.dwPasswordLength  = URL_COMPONENTS_LENGTH - 1;
  ret->comps.dwHostNameLength  = URL_COMPONENTS_LENGTH - 1;
  ret->comps.dwUrlPathLength   = URL_COMPONENTS_LENGTH - 1;
  ret->comps.dwExtraInfoLength = URL_COMPONENTS_LENGTH - 1;
  ret->comps.lpszHostName = ret->lpszHostName;
  ret->comps.lpszUserName = ret->lpszUserName;
  ret->comps.lpszPassword = ret->lpszPassword;
  ret->comps.lpszUrlPath = ret->lpszUrlPath;
  ret->comps.lpszExtraInfo = ret->lpszExtraInfo;

  debug1("net123_open start crack %S", urlW);

  if(!(res = WinHttpCrackUrl(urlW, 0, 0, &ret->comps))) {
    debug1("net123_open crack fail %lu", GetLastError());
    goto cleanup;
  }

  debug("net123_open crack OK");
  debug_crack(&ret->comps);

  ret->session = WinHttpOpen(useragent, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
  free(urlW);
  urlW = NULL;
  debug("net123_open WinHttpOpen OK");
  if(!ret->session) goto cleanup;

  debug2("net123_open WinHttpConnect %S %u", ret->comps.lpszHostName, ret->comps.nPort);
  ret->connect = WinHttpConnect(ret->session, ret->comps.lpszHostName, ret->comps.nPort, 0);
  if(!ret->connect) goto cleanup;
  debug("net123_open WinHttpConnect OK");

  debug1("WinHttpOpenRequest GET %S", ret->comps.lpszUrlPath);
  ret->request = WinHttpOpenRequest(ret->connect, L"GET", ret->comps.lpszUrlPath, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, ret->comps.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0);
  if(!ret->request) goto cleanup;
  debug("WinHttpOpenRequest GET OK");

  cb = WinHttpSetStatusCallback(ret->request, (WINHTTP_STATUS_CALLBACK)net123_ssl_errors, WINHTTP_CALLBACK_FLAG_SECURE_FAILURE, 0);
  if(cb == WINHTTP_INVALID_STATUS_CALLBACK){
    error1("WinHttpSetStatusCallback failed to install callback, errors might not be reported properly! (%lu)", GetLastError());
  }

  wrap_auth(ret);

  for(ii = 0; client_head[ii]; ii++){
    INT123_win32_utf8_wide(client_head[ii], &headers, NULL);
    if(!headers)
      goto cleanup;
    debug1("WinHttpAddRequestHeaders add %S", headers);
    res = WinHttpAddRequestHeaders(ret->request, headers, (DWORD) -1, WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE);
    debug2("WinHttpAddRequestHeaders returns %u %lu", res, res ? 0 : GetLastError());
    free(headers);
    headers = NULL;
  }

  debug("net123_open ADD HEADERS OK");

  res = WinHttpSendRequest(ret->request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, (DWORD_PTR)ret);

  if (!res) {
    res = GetLastError();
    error1("WinHttpSendRequest failed with %d", res);
    if(res == ERROR_WINHTTP_SECURE_FAILURE){
      res = *(DWORD *)ret->additionalInfo;
      error("Additionally, the ERROR_WINHTTP_SECURE_FAILURE failed with:");
      if (res & WINHTTP_CALLBACK_STATUS_FLAG_CERT_REV_FAILED) error("  WINHTTP_CALLBACK_STATUS_FLAG_CERT_REV_FAILED");
      if (res & WINHTTP_CALLBACK_STATUS_FLAG_INVALID_CERT) error("  WINHTTP_CALLBACK_STATUS_FLAG_INVALID_CERT");
      if (res & WINHTTP_CALLBACK_STATUS_FLAG_CERT_REVOKED) error("  WINHTTP_CALLBACK_STATUS_FLAG_CERT_REVOKED");
      if (res & WINHTTP_CALLBACK_STATUS_FLAG_INVALID_CA) error("  WINHTTP_CALLBACK_STATUS_FLAG_INVALID_CA");
      if (res & WINHTTP_CALLBACK_STATUS_FLAG_CERT_CN_INVALID) error("  WINHTTP_CALLBACK_STATUS_FLAG_CERT_CN_INVALID");
      if (res & WINHTTP_CALLBACK_STATUS_FLAG_CERT_DATE_INVALID) error("  WINHTTP_CALLBACK_STATUS_FLAG_CERT_DATE_INVALID");
      if (res & WINHTTP_CALLBACK_STATUS_FLAG_SECURITY_CHANNEL_ERROR) error("  WINHTTP_CALLBACK_STATUS_FLAG_SECURITY_CHANNEL_ERROR");
    }
    goto cleanup;
  }

  res = WinHttpReceiveResponse(ret->request, NULL);

  if (!res) {
    error1("WinHttpReceiveResponse failed with %lu", GetLastError());
    goto cleanup;
  }

  res = WinHttpQueryHeaders(ret->request, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX, WINHTTP_NO_OUTPUT_BUFFER, &headerlen, WINHTTP_NO_HEADER_INDEX);

  if(GetLastError() == ERROR_INSUFFICIENT_BUFFER && headerlen > 0) {
    headers = calloc(1, headerlen);
    if (!headers) goto cleanup;
    WinHttpQueryHeaders(ret->request, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX, headers, &headerlen, WINHTTP_NO_HEADER_INDEX);
    INT123_win32_wide_utf7(headers, &ret->headers, &ret->headers_len);
    /* bytes written, skip the terminating null, we want to stop at the \r\n\r\n */
    ret->headers_len --;
    free(headers);
    headers = NULL;
  } else {
    error("WinHttpQueryHeaders did not execute as expected");
    goto cleanup;
  }
  debug("net123_open OK");

  return handle;
cleanup:
  debug("net123_open error");
  if (urlW) free(urlW);
  if (handle) {
    net123_close(handle);
    handle = NULL;
  }
  return handle;
}

// Read data into buffer, return bytes read.
// This handles interrupts (EAGAIN, EINTR, ..) internally and only returns
// a short byte count on EOF or error. End of file or error is not distinguished:
// For the user, it only matters if there will be more bytes or not.
// Feel free to communicate errors via error() / merror() functions inside.
size_t net123_read(net123_handle *nh, void *buf, size_t bufsize){
  winhttp_handle *h = nh->parts;
  size_t ret;
  size_t to_copy = h->headers_len - h->headers_pos;
  DWORD bytesread = 0;

  if(to_copy){
     ret = to_copy <= bufsize ? to_copy : bufsize;
     memcpy(buf, h->headers + h->headers_pos, ret);
     h->headers_pos += ret;
     return ret;
  }

  /* is this needed? */
  to_copy = bufsize > ULONG_MAX ? ULONG_MAX : bufsize;
  if(!WinHttpReadData(h->request, buf, to_copy, &bytesread)){
    return EOF;
  }
  return bytesread;
}

// Call that to free up resources, end processes.
void net123_close(net123_handle *nh){
  if (!nh) return;
  winhttp_handle *h = nh->parts;
  if(h) {
    if(h->headers) {
      free(h->headers);
      h->headers = NULL;
    }
    if(h->request) {
      WinHttpCloseHandle(h->request);
      h->request = NULL;
    }
    if(h->connect) {
      WinHttpCloseHandle(h->connect);
      h->connect = NULL;
    }
    if(h->session) {
      WinHttpCloseHandle(h->session);
      h->session = NULL;
    }
    free(h);
  }
  free(nh);
}
