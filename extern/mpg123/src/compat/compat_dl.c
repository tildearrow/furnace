/*
	compat_dl: dynamic loading of shared libs

	This is a separate libcompat to avoid needlessly linking all mpg123 code
	with libdl on Unix.

	copyright 2007-2019 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Thomas Orgis, Windows Unicode stuff by JonY.
*/

#include "config.h"
/* This source file does need _POSIX_SOURCE to get some sigaction. */
#define _POSIX_SOURCE
/* Fix pedantic error about w2upath being unused */
#define HIDE_w2upath
#include "compat.h"

#ifdef _MSC_VER
#include <io.h>

#if(defined(WINAPI_FAMILY) && (WINAPI_FAMILY==WINAPI_FAMILY_APP))
#define WINDOWS_UWP
#endif

#endif
#ifdef HAVE_SYS_STAT_H
#  include <sys/stat.h>
#endif
#ifdef HAVE_DIRENT_H
#  include <dirent.h>
#endif

/* Win32 is only supported with unicode now. These headers also cover
   module stuff. The WANT_WIN32_UNICODE macro is synonymous with
   "want windows-specific API, and only the unicode variants of which". */
#ifdef WANT_WIN32_UNICODE
#include <wchar.h>
#include <windows.h>
#include <winnls.h>
#include <shlwapi.h>
#endif

#ifdef USE_MODULES
#  ifdef HAVE_DLFCN_H
#    include <dlfcn.h>
#  endif
#endif

#include "../common/debug.h"


#ifdef USE_MODULES
#include "wpathconv.h"
/*
	This is what I expected the platform-specific dance for dynamic module
	support to be. Little did I know about the peculiarities of (long)
	paths and directory/file search on Windows.

	LoadLibrary throws GUI error boxes, use SetThreadErrorMode to suppress.
	It needs to be done on per-thread basis to avoid race conditions
	clobbering each other when setting/restoring across different threads.
*/

void *INT123_compat_dlopen(const char *path)
{
	void *handle = NULL;
#ifdef WANT_WIN32_UNICODE
	wchar_t *wpath;
	wpath = u2wlongpath(path);
	if(wpath) {
		DWORD emode = GetThreadErrorMode();
		int mode_ok = SetThreadErrorMode(emode | SEM_FAILCRITICALERRORS, NULL);
		handle = LoadLibraryW(wpath);
		if(mode_ok) {
			SetThreadErrorMode(emode, NULL);
		}
	}
	free(wpath);
#else
	handle = dlopen(path, RTLD_NOW);
#endif
	return handle;
}

void *INT123_compat_dlsym(void *handle, const char *name)
{
	void *sym = NULL;
	if(!handle)
		return NULL;
#ifdef WANT_WIN32_UNICODE
	sym = (void *)(uintptr_t)GetProcAddress(handle, name);
#else
	sym = dlsym(handle, name);
#endif
	return sym;
}

void INT123_compat_dlclose(void *handle)
{
	if(!handle)
		return;
#ifdef WANT_WIN32_UNICODE
	FreeLibrary(handle);
#else
	dlclose(handle);
#endif
}

#endif /* USE_MODULES */
