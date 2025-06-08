/*
** Copyright (C) 2002-2014 Erik de Castro Lopo <erikd@mega-nerd.com>
** Copyright (C) 2003 Ross Bencina <rbencina@iprimus.com.au>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/*
**	The file is split into three sections as follows:
**		- The top section (USE_WINDOWS_API == 0) for Linux, Unix and MacOSX
**			systems (including Cygwin).
**		- The middle section (USE_WINDOWS_API == 1) for microsoft windows
**			(including MinGW) using the native windows API.
**		- A legacy windows section which attempted to work around grevious
**			bugs in microsoft's POSIX implementation.
*/

/*
**	The header file sfconfig.h MUST be included before the others to ensure
**	that large file support is enabled correctly on Unix systems.
*/

#include "sfconfig.h"

#if USE_WINDOWS_API

/* Don't include rarely used headers, speed up build */
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#else
#include <io.h>
#endif

#if (HAVE_DECL_S_IRGRP == 0)
#include <sf_unistd.h>
#endif

#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#include "sndfile.h"
#include "common.h"

#define	SENSIBLE_SIZE	(0x40000000)

/*
**	Neat solution to the Win32/OS2 binary file flage requirement.
**	If O_BINARY isn't already defined by the inclusion of the system
**	headers, set it to zero.
*/
#ifndef O_BINARY
#define O_BINARY 0
#endif

static void psf_log_syserr (SF_PRIVATE *psf, int error) ;

int
psf_copy_filename (SF_PRIVATE *psf, const char *path)
{	const char *ccptr ;
	char *cptr ;

	if (strlen (path) > 1 && strlen (path) - 1 >= sizeof (psf->file.path))
	{	psf->error = SFE_FILENAME_TOO_LONG ;
		return psf->error ;
		} ;

	snprintf (psf->file.path, sizeof (psf->file.path), "%s", path) ;
	if ((ccptr = strrchr (path, '/')) || (ccptr = strrchr (path, '\\')))
		ccptr ++ ;
	else
		ccptr = path ;

	snprintf (psf->file.name, sizeof (psf->file.name), "%s", ccptr) ;

	/* Now grab the directory. */
	snprintf (psf->file.dir, sizeof (psf->file.dir), "%s", path) ;
	if ((cptr = strrchr (psf->file.dir, '/')) || (cptr = strrchr (psf->file.dir, '\\')))
		cptr [1] = 0 ;
	else
		psf->file.dir [0] = 0 ;

	return 0 ;
} /* psf_copy_filename */

#if (USE_WINDOWS_API == 0)

/*------------------------------------------------------------------------------
** Win32 stuff at the bottom of the file. Unix and other sensible OSes here.
*/

static int psf_close_fd (int fd) ;
static int psf_open_fd (PSF_FILE * pfile) ;
static sf_count_t psf_get_filelen_fd (int fd) ;

int
psf_fopen (SF_PRIVATE *psf)
{
	psf->error = 0 ;
	psf->file.filedes = psf_open_fd (&psf->file) ;

	if (psf->file.filedes == - SFE_BAD_OPEN_MODE)
	{	psf->error = SFE_BAD_OPEN_MODE ;
		psf->file.filedes = -1 ;
		return psf->error ;
		} ;

	if (psf->file.filedes == -1)
		psf_log_syserr (psf, errno) ;

	return psf->error ;
} /* psf_fopen */

int
psf_fclose (SF_PRIVATE *psf)
{	int retval ;

	if (psf->virtual_io)
		return 0 ;

	if (psf->file.do_not_close_descriptor)
	{	psf->file.filedes = -1 ;
		return 0 ;
		} ;

	if ((retval = psf_close_fd (psf->file.filedes)) == -1)
		psf_log_syserr (psf, errno) ;

	psf->file.filedes = -1 ;

	return retval ;
} /* psf_fclose */

int
psf_open_rsrc (SF_PRIVATE *psf)
{	size_t count ;

	if (psf->rsrc.filedes > 0)
		return 0 ;

	/* Test for MacOSX style resource fork on HPFS or HPFS+ filesystems. */
	count = snprintf (psf->rsrc.path, sizeof (psf->rsrc.path), "%s/..namedfork/rsrc", psf->file.path) ;
	psf->error = SFE_NO_ERROR ;
	if (count < sizeof (psf->rsrc.path))
	{	if ((psf->rsrc.filedes = psf_open_fd (&psf->rsrc)) >= 0)
		{	psf->rsrclength = psf_get_filelen_fd (psf->rsrc.filedes) ;
			if (psf->rsrclength > 0 || (psf->rsrc.mode & SFM_WRITE))
				return SFE_NO_ERROR ;
			psf_close_fd (psf->rsrc.filedes) ;
			psf->rsrc.filedes = -1 ;
			} ;

		if (psf->rsrc.filedes == - SFE_BAD_OPEN_MODE)
		{	psf->error = SFE_BAD_OPEN_MODE ;
			return psf->error ;
			} ;
		} ;

	/*
	** Now try for a resource fork stored as a separate file in the same
	** directory, but preceded with a dot underscore.
	*/
	count = snprintf (psf->rsrc.path, sizeof (psf->rsrc.path), "%s._%s", psf->file.dir, psf->file.name) ;
	psf->error = SFE_NO_ERROR ;
	if (count < sizeof (psf->rsrc.path) && (psf->rsrc.filedes = psf_open_fd (&psf->rsrc)) >= 0)
	{	psf->rsrclength = psf_get_filelen_fd (psf->rsrc.filedes) ;
		return SFE_NO_ERROR ;
		} ;

	/*
	** Now try for a resource fork stored in a separate file in the
	** .AppleDouble/ directory.
	*/
	count = snprintf (psf->rsrc.path, sizeof (psf->rsrc.path), "%s.AppleDouble/%s", psf->file.dir, psf->file.name) ;
	psf->error = SFE_NO_ERROR ;
	if (count < sizeof (psf->rsrc.path))
	{	if ((psf->rsrc.filedes = psf_open_fd (&psf->rsrc)) >= 0)
		{	psf->rsrclength = psf_get_filelen_fd (psf->rsrc.filedes) ;
			return SFE_NO_ERROR ;
			} ;

		/* No resource file found. */
		if (psf->rsrc.filedes == -1)
			psf_log_syserr (psf, errno) ;
		}
	else
	{	psf->error = SFE_OPEN_FAILED ;
		} ;

	psf->rsrc.filedes = -1 ;

	return psf->error ;
} /* psf_open_rsrc */

sf_count_t
psf_get_filelen (SF_PRIVATE *psf)
{	sf_count_t	filelen ;

	if (psf->virtual_io)
		return psf->vio.get_filelen (psf->vio_user_data) ;

	filelen = psf_get_filelen_fd (psf->file.filedes) ;

	if (filelen == -1)
	{	psf_log_syserr (psf, errno) ;
		return (sf_count_t) -1 ;
		} ;

	if (filelen == -SFE_BAD_STAT_SIZE)
	{	psf->error = SFE_BAD_STAT_SIZE ;
		return (sf_count_t) -1 ;
		} ;

	switch (psf->file.mode)
	{	case SFM_WRITE :
			filelen = filelen - psf->fileoffset ;
			break ;

		case SFM_READ :
			if (psf->fileoffset > 0 && psf->filelength > 0)
				filelen = psf->filelength ;
			break ;

		case SFM_RDWR :
			/*
			** Cannot open embedded files SFM_RDWR so we don't need to
			** subtract psf->fileoffset. We already have the answer we
			** need.
			*/
			break ;

		default :
			/* Shouldn't be here, so return error. */
			filelen = -1 ;
		} ;

	return filelen ;
} /* psf_get_filelen */

int
psf_close_rsrc (SF_PRIVATE *psf)
{	psf_close_fd (psf->rsrc.filedes) ;
	psf->rsrc.filedes = -1 ;
	return 0 ;
} /* psf_close_rsrc */

int
psf_set_stdio (SF_PRIVATE *psf)
{	int	error = 0 ;

	switch (psf->file.mode)
	{	case SFM_RDWR :
				error = SFE_OPEN_PIPE_RDWR ;
				break ;

		case SFM_READ :
				psf->file.filedes = 0 ;
				break ;

		case SFM_WRITE :
				psf->file.filedes = 1 ;
				break ;

		default :
				error = SFE_BAD_OPEN_MODE ;
				break ;
		} ;
	psf->filelength = 0 ;

	return error ;
} /* psf_set_stdio */

void
psf_set_file (SF_PRIVATE *psf, int fd)
{	psf->file.filedes = fd ;
} /* psf_set_file */

int
psf_file_valid (SF_PRIVATE *psf)
{	return (psf->file.filedes >= 0) ? SF_TRUE : SF_FALSE ;
} /* psf_set_file */

sf_count_t
psf_fseek (SF_PRIVATE *psf, sf_count_t offset, int whence)
{	sf_count_t	absolute_position ;

	if (psf->virtual_io)
		return psf->vio.seek (offset, whence, psf->vio_user_data) ;

	/* When decoding from pipes sometimes see seeks to the pipeoffset, which appears to mean do nothing. */
	if (psf->is_pipe)
	{	if (whence != SEEK_SET || offset != psf->pipeoffset)
			psf_log_printf (psf, "psf_fseek : pipe seek to value other than pipeoffset\n") ;
		return offset ;
		}

	switch (whence)
	{	case SEEK_SET :
				offset += psf->fileoffset ;
				break ;

		case SEEK_END :
				break ;

		case SEEK_CUR :
				break ;

		default :
				/* We really should not be here. */
				psf_log_printf (psf, "psf_fseek : whence is %d *****.\n", whence) ;
				return 0 ;
		} ;

	absolute_position = lseek (psf->file.filedes, offset, whence) ;

	if (absolute_position < 0)
		psf_log_syserr (psf, errno) ;

	return absolute_position - psf->fileoffset ;
} /* psf_fseek */

sf_count_t
psf_fread (void *ptr, sf_count_t bytes, sf_count_t items, SF_PRIVATE *psf)
{	sf_count_t total = 0 ;
	ssize_t	count ;

	if (psf->virtual_io)
		return psf->vio.read (ptr, bytes*items, psf->vio_user_data) / bytes ;

	items *= bytes ;

	/* Do this check after the multiplication above. */
	if (items <= 0)
		return 0 ;

	while (items > 0)
	{	/* Break the read down to a sensible size. */
		count = (items > SENSIBLE_SIZE) ? SENSIBLE_SIZE : (ssize_t) items ;

		count = read (psf->file.filedes, ((char*) ptr) + total, (size_t) count) ;

		if (count == -1)
		{	if (errno == EINTR)
				continue ;

			psf_log_syserr (psf, errno) ;
			break ;
			} ;

		if (count == 0)
			break ;

		total += count ;
		items -= count ;
		} ;

	if (psf->is_pipe)
		psf->pipeoffset += total ;

	return total / bytes ;
} /* psf_fread */

sf_count_t
psf_fwrite (const void *ptr, sf_count_t bytes, sf_count_t items, SF_PRIVATE *psf)
{	sf_count_t total = 0 ;
	ssize_t	count ;

	if (bytes == 0 || items == 0)
		return 0 ;

	if (psf->virtual_io)
		return psf->vio.write (ptr, bytes*items, psf->vio_user_data) / bytes ;

	items *= bytes ;

	/* Do this check after the multiplication above. */
	if (items <= 0)
		return 0 ;

	while (items > 0)
	{	/* Break the writes down to a sensible size. */
		count = (items > SENSIBLE_SIZE) ? SENSIBLE_SIZE : items ;

		count = write (psf->file.filedes, ((const char*) ptr) + total, count) ;

		if (count == -1)
		{	if (errno == EINTR)
				continue ;

			psf_log_syserr (psf, errno) ;
			break ;
			} ;

		if (count == 0)
			break ;

		total += count ;
		items -= count ;
		} ;

	if (psf->is_pipe)
		psf->pipeoffset += total ;

	return total / bytes ;
} /* psf_fwrite */

sf_count_t
psf_ftell (SF_PRIVATE *psf)
{	sf_count_t pos ;

	if (psf->virtual_io)
		return psf->vio.tell (psf->vio_user_data) ;

	if (psf->is_pipe)
		return psf->pipeoffset ;

	pos = lseek (psf->file.filedes, 0, SEEK_CUR) ;

	if (pos == ((sf_count_t) -1))
	{	psf_log_syserr (psf, errno) ;
		return -1 ;
		} ;

	return pos - psf->fileoffset ;
} /* psf_ftell */

static int
psf_close_fd (int fd)
{	int retval ;

	if (fd < 0)
		return 0 ;

	while ((retval = close (fd)) == -1 && errno == EINTR)
		/* Do nothing. */ ;

	return retval ;
} /* psf_close_fd */

sf_count_t
psf_fgets (char *buffer, sf_count_t bufsize, SF_PRIVATE *psf)
{	sf_count_t	k = 0 ;
	sf_count_t		count ;

	while (k < bufsize - 1)
	{	count = read (psf->file.filedes, &(buffer [k]), 1) ;

		if (count == -1)
		{	if (errno == EINTR)
				continue ;

			psf_log_syserr (psf, errno) ;
			break ;
			} ;

		if (count == 0 || buffer [k++] == '\n')
			break ;
		} ;

	buffer [k] = 0 ;

	return k ;
} /* psf_fgets */

int
psf_is_pipe (SF_PRIVATE *psf)
{	struct stat statbuf ;

	if (psf->virtual_io)
		return SF_FALSE ;

	if (fstat (psf->file.filedes, &statbuf) == -1)
	{	psf_log_syserr (psf, errno) ;
		/* Default to maximum safety. */
		return SF_TRUE ;
		} ;

	if (S_ISFIFO (statbuf.st_mode) || S_ISSOCK (statbuf.st_mode))
		return SF_TRUE ;

	return SF_FALSE ;
} /* psf_is_pipe */

static sf_count_t
psf_get_filelen_fd (int fd)
{
#if (SIZEOF_OFF_T == 4 && HAVE_FSTAT64)
	struct stat64 statbuf ;

	if (fstat64 (fd, &statbuf) == -1)
		return (sf_count_t) -1 ;

	return statbuf.st_size ;
#else
	struct stat statbuf ;

	if (fstat (fd, &statbuf) == -1)
		return (sf_count_t) -1 ;

	return statbuf.st_size ;
#endif
} /* psf_get_filelen_fd */

int
psf_ftruncate (SF_PRIVATE *psf, sf_count_t len)
{	int retval ;

	/* Returns 0 on success, non-zero on failure. */
	if (len < 0)
		return -1 ;

	if ((sizeof (off_t) < sizeof (sf_count_t)) && len > 0x7FFFFFFF)
		return -1 ;

	retval = ftruncate (psf->file.filedes, len) ;

	if (retval == -1)
		psf_log_syserr (psf, errno) ;

	return retval ;
} /* psf_ftruncate */

void
psf_init_files (SF_PRIVATE *psf)
{	psf->file.filedes = -1 ;
	psf->rsrc.filedes = -1 ;
	psf->file.savedes = -1 ;
} /* psf_init_files */

void
psf_use_rsrc (SF_PRIVATE *psf, int on_off)
{
	if (on_off)
	{	if (psf->file.filedes != psf->rsrc.filedes)
		{	psf->file.savedes = psf->file.filedes ;
			psf->file.filedes = psf->rsrc.filedes ;
			} ;
		}
	else if (psf->file.filedes == psf->rsrc.filedes)
		psf->file.filedes = psf->file.savedes ;

	return ;
} /* psf_use_rsrc */

static int
psf_open_fd (PSF_FILE * pfile)
{	int fd, oflag, mode ;

	/*
	** Sanity check. If everything is OK, this test and the printfs will
	** be optimised out. This is meant to catch the problems caused by
	** "sfconfig.h" being included after <stdio.h>.
	*/
	if (sizeof (sf_count_t) != 8)
	{	puts ("\n\n*** Fatal error : sizeof (sf_count_t) != 8") ;
		puts ("*** This means that libsndfile was not configured correctly.\n") ;
		exit (1) ;
		} ;

	switch (pfile->mode)
	{	case SFM_READ :
				oflag = O_RDONLY | O_BINARY ;
				mode = 0 ;
				break ;

		case SFM_WRITE :
				oflag = O_WRONLY | O_CREAT | O_TRUNC | O_BINARY ;
				mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH ;
				break ;

		case SFM_RDWR :
				oflag = O_RDWR | O_CREAT | O_BINARY ;
				mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH ;
				break ;

		default :
				return - SFE_BAD_OPEN_MODE ;
				break ;
		} ;

	if (mode == 0)
		fd = open (pfile->path, oflag) ;
	else
		fd = open (pfile->path, oflag, mode) ;

	return fd ;
} /* psf_open_fd */

static void
psf_log_syserr (SF_PRIVATE *psf, int error)
{
	/* Only log an error if no error has been set yet. */
	if (psf->error == 0)
	{	psf->error = SFE_SYSTEM ;
		snprintf (psf->syserr, sizeof (psf->syserr), "System error : %s.", strerror (error)) ;
		} ;

	return ;
} /* psf_log_syserr */

void
psf_fsync (SF_PRIVATE *psf)
{
#if HAVE_FSYNC
	if (psf->file.mode == SFM_WRITE || psf->file.mode == SFM_RDWR)
		fsync (psf->file.filedes) ;
#else
	psf = NULL ;
#endif
} /* psf_fsync */

#else

/* Win32 file i/o functions implemented using native Win32 API */

#ifndef WINAPI_PARTITION_SYSTEM
#define WINAPI_PARTITION_SYSTEM 0
#endif

static int psf_close_handle (HANDLE handle) ;
static HANDLE psf_open_handle (PSF_FILE * pfile) ;
static sf_count_t psf_get_filelen_handle (HANDLE handle) ;

/* USE_WINDOWS_API */ int
psf_fopen (SF_PRIVATE *psf)
{
	psf->error = 0 ;
	psf->file.handle = psf_open_handle (&psf->file) ;

	if (psf->file.handle == INVALID_HANDLE_VALUE)
		psf_log_syserr (psf, GetLastError ()) ;

	return psf->error ;
} /* psf_fopen */

/* USE_WINDOWS_API */ int
psf_fclose (SF_PRIVATE *psf)
{	int retval ;

	if (psf->virtual_io)
		return 0 ;

	if (psf->file.do_not_close_descriptor)
	{	psf->file.handle = INVALID_HANDLE_VALUE ;
		return 0 ;
		} ;

	if ((retval = psf_close_handle (psf->file.handle)) == -1)
		psf_log_syserr (psf, GetLastError ()) ;

	psf->file.handle = INVALID_HANDLE_VALUE ;

	return retval ;
} /* psf_fclose */

/* USE_WINDOWS_API */ int
psf_open_rsrc (SF_PRIVATE *psf)
{
	if (psf->rsrc.handle != INVALID_HANDLE_VALUE)
		return 0 ;

	/* Test for MacOSX style resource fork on HPFS or HPFS+ filesystems. */
	snprintf (psf->rsrc.path, sizeof (psf->rsrc.path), "%s/rsrc", psf->file.path) ;
	psf->error = SFE_NO_ERROR ;
	if ((psf->rsrc.handle = psf_open_handle (&psf->rsrc)) != INVALID_HANDLE_VALUE)
	{	psf->rsrclength = psf_get_filelen_handle (psf->rsrc.handle) ;
		return SFE_NO_ERROR ;
		} ;

	/*
	** Now try for a resource fork stored as a separate file in the same
	** directory, but preceded with a dot underscore.
	*/
	snprintf (psf->rsrc.path, sizeof (psf->rsrc.path), "%s._%s", psf->file.dir, psf->file.name) ;
	psf->error = SFE_NO_ERROR ;
	if ((psf->rsrc.handle = psf_open_handle (&psf->rsrc)) != INVALID_HANDLE_VALUE)
	{	psf->rsrclength = psf_get_filelen_handle (psf->rsrc.handle) ;
		return SFE_NO_ERROR ;
		} ;

	/*
	** Now try for a resource fork stored in a separate file in the
	** .AppleDouble/ directory.
	*/
	snprintf (psf->rsrc.path, sizeof (psf->rsrc.path), "%s.AppleDouble/%s", psf->file.dir, psf->file.name) ;
	psf->error = SFE_NO_ERROR ;
	if ((psf->rsrc.handle = psf_open_handle (&psf->rsrc)) != INVALID_HANDLE_VALUE)
	{	psf->rsrclength = psf_get_filelen_handle (psf->rsrc.handle) ;
		return SFE_NO_ERROR ;
		} ;

	/* No resource file found. */
	if (psf->rsrc.handle == INVALID_HANDLE_VALUE)
		psf_log_syserr (psf, GetLastError ()) ;

	return psf->error ;
} /* psf_open_rsrc */

/* USE_WINDOWS_API */ sf_count_t
psf_get_filelen (SF_PRIVATE *psf)
{	sf_count_t	filelen ;

	if (psf->virtual_io)
		return psf->vio.get_filelen (psf->vio_user_data) ;

	filelen = psf_get_filelen_handle (psf->file.handle) ;

	if (filelen == -1)
	{	psf_log_syserr (psf, errno) ;
		return (sf_count_t) -1 ;
		} ;

	if (filelen == -SFE_BAD_STAT_SIZE)
	{	psf->error = SFE_BAD_STAT_SIZE ;
		return (sf_count_t) -1 ;
		} ;

	switch (psf->file.mode)
	{	case SFM_WRITE :
			filelen = filelen - psf->fileoffset ;
			break ;

		case SFM_READ :
			if (psf->fileoffset > 0 && psf->filelength > 0)
				filelen = psf->filelength ;
			break ;

		case SFM_RDWR :
			/*
			** Cannot open embedded files SFM_RDWR so we don't need to
			** subtract psf->fileoffset. We already have the answer we
			** need.
			*/
			break ;

		default :
			/* Shouldn't be here, so return error. */
			filelen = -1 ;
		} ;

	return filelen ;
} /* psf_get_filelen */

/* USE_WINDOWS_API */ void
psf_init_files (SF_PRIVATE *psf)
{	psf->file.handle = INVALID_HANDLE_VALUE ;
	psf->rsrc.handle = INVALID_HANDLE_VALUE ;
	psf->file.hsaved = INVALID_HANDLE_VALUE ;
} /* psf_init_files */

/* USE_WINDOWS_API */ void
psf_use_rsrc (SF_PRIVATE *psf, int on_off)
{
	if (on_off)
	{	if (psf->file.handle != psf->rsrc.handle)
		{	psf->file.hsaved = psf->file.handle ;
			psf->file.handle = psf->rsrc.handle ;
			} ;
		}
	else if (psf->file.handle == psf->rsrc.handle)
		psf->file.handle = psf->file.hsaved ;

	return ;
} /* psf_use_rsrc */

/* USE_WINDOWS_API */ static HANDLE
psf_open_handle (PSF_FILE * pfile)
{	DWORD dwDesiredAccess ;
	DWORD dwShareMode ;
	DWORD dwCreationDistribution ;
	HANDLE handle ;
	LPWSTR pwszPath = NULL ;

	switch (pfile->mode)
	{	case SFM_READ :
				dwDesiredAccess = GENERIC_READ ;
				dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE ;
				dwCreationDistribution = OPEN_EXISTING ;
				break ;

		case SFM_WRITE :
				dwDesiredAccess = GENERIC_WRITE ;
				dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE ;
				dwCreationDistribution = CREATE_ALWAYS ;
				break ;

		case SFM_RDWR :
				dwDesiredAccess = GENERIC_READ | GENERIC_WRITE ;
				dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE ;
				dwCreationDistribution = OPEN_ALWAYS ;
				break ;

		default :
				return INVALID_HANDLE_VALUE ;
		} ;

	int nResult = MultiByteToWideChar (CP_UTF8, 0, pfile->path, -1, NULL, 0) ;
	pwszPath = malloc (nResult * sizeof (WCHAR)) ;
	if (!pwszPath)
		return INVALID_HANDLE_VALUE ;
	
	int nResult2 = MultiByteToWideChar (CP_UTF8, 0, pfile->path, -1, pwszPath, nResult) ;
	if (nResult != nResult2)
	{	free (pwszPath) ;
		return INVALID_HANDLE_VALUE ;
		} ;

#if defined (WINAPI_FAMILY_PARTITION) && !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP | WINAPI_PARTITION_SYSTEM)
	CREATEFILE2_EXTENDED_PARAMETERS cfParams = { 0 } ;
	cfParams.dwSize = sizeof (CREATEFILE2_EXTENDED_PARAMETERS) ;
	cfParams.dwFileAttributes = FILE_ATTRIBUTE_NORMAL ;

	handle = CreateFile2 (pwszPath, dwDesiredAccess, dwShareMode, dwCreationDistribution, &cfParams) ;
#else
	handle = CreateFileW (
				pwszPath,					/* pointer to name of the file */
				dwDesiredAccess,			/* access (read-write) mode */
				dwShareMode,				/* share mode */
				0,							/* pointer to security attributes */
				dwCreationDistribution,		/* how to create */
				FILE_ATTRIBUTE_NORMAL,		/* file attributes (could use FILE_FLAG_SEQUENTIAL_SCAN) */
				NULL						/* handle to file with attributes to copy */
				) ;
#endif
	free (pwszPath) ;

	return handle ;
} /* psf_open_handle */

/* USE_WINDOWS_API */ static void
psf_log_syserr (SF_PRIVATE *psf, int error)
{	LPVOID lpMsgBuf ;

	/* Only log an error if no error has been set yet. */
	if (psf->error == 0)
	{	psf->error = SFE_SYSTEM ;

		FormatMessage (
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			error,
			MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0,
			NULL
			) ;

		snprintf (psf->syserr, sizeof (psf->syserr), "System error : %s", (char*) lpMsgBuf) ;
		LocalFree (lpMsgBuf) ;
		} ;

	return ;
} /* psf_log_syserr */


/* USE_WINDOWS_API */ int
psf_close_rsrc (SF_PRIVATE *psf)
{	psf_close_handle (psf->rsrc.handle) ;
	psf->rsrc.handle = INVALID_HANDLE_VALUE ;
	return 0 ;
} /* psf_close_rsrc */


/* USE_WINDOWS_API */ int
psf_set_stdio (SF_PRIVATE *psf)
{	HANDLE	handle = INVALID_HANDLE_VALUE ;
	int	error = 0 ;

	switch (psf->file.mode)
	{	case SFM_RDWR :
				error = SFE_OPEN_PIPE_RDWR ;
				break ;

		case SFM_READ :
				handle = GetStdHandle (STD_INPUT_HANDLE) ;
				psf->file.do_not_close_descriptor = 1 ;
				break ;

		case SFM_WRITE :
				handle = GetStdHandle (STD_OUTPUT_HANDLE) ;
				psf->file.do_not_close_descriptor = 1 ;
				break ;

		default :
				error = SFE_BAD_OPEN_MODE ;
				break ;
		} ;

	psf->file.handle = handle ;
	psf->filelength = 0 ;

	return error ;
} /* psf_set_stdio */

/* USE_WINDOWS_API */ void
psf_set_file (SF_PRIVATE *psf, int fd)
{	HANDLE handle ;
	intptr_t osfhandle ;

	osfhandle = _get_osfhandle (fd) ;
	handle = (HANDLE) osfhandle ;

	psf->file.handle = handle ;
} /* psf_set_file */

/* USE_WINDOWS_API */ int
psf_file_valid (SF_PRIVATE *psf)
{	if (psf->file.handle == INVALID_HANDLE_VALUE)
		return SF_FALSE ;
	return SF_TRUE ;
} /* psf_set_file */

/* USE_WINDOWS_API */ sf_count_t
psf_fseek (SF_PRIVATE *psf, sf_count_t offset, int whence)
{	sf_count_t new_position ;
	LARGE_INTEGER liDistanceToMove, liNewFilePointer ;
	DWORD dwMoveMethod ;
	BOOL fResult ;
	DWORD dwError ;

	if (psf->virtual_io)
		return psf->vio.seek (offset, whence, psf->vio_user_data) ;

	switch (whence)
	{	case SEEK_SET :
				offset += psf->fileoffset ;
				dwMoveMethod = FILE_BEGIN ;
				break ;

		case SEEK_END :
				dwMoveMethod = FILE_END ;
				break ;

		default :
				dwMoveMethod = FILE_CURRENT ;
				break ;
		} ;

	liDistanceToMove.QuadPart = offset ;

	fResult = SetFilePointerEx (psf->file.handle, liDistanceToMove, &liNewFilePointer, dwMoveMethod) ;

	if (fResult == FALSE)
		dwError = GetLastError () ;
	else
		dwError = NO_ERROR ;

	if (dwError != NO_ERROR)
	{	psf_log_syserr (psf, dwError) ;
		return -1 ;
		} ;

	new_position = liNewFilePointer.QuadPart - psf->fileoffset ;

	return new_position ;
} /* psf_fseek */

/* USE_WINDOWS_API */ sf_count_t
psf_fread (void *ptr, sf_count_t bytes, sf_count_t items, SF_PRIVATE *psf)
{	sf_count_t total = 0 ;
	ssize_t count ;
	DWORD dwNumberOfBytesRead ;

	if (psf->virtual_io)
		return psf->vio.read (ptr, bytes*items, psf->vio_user_data) / bytes ;

	items *= bytes ;

	/* Do this check after the multiplication above. */
	if (items <= 0)
		return 0 ;

	while (items > 0)
	{	/* Break the writes down to a sensible size. */
		count = (items > SENSIBLE_SIZE) ? SENSIBLE_SIZE : (ssize_t) items ;

		if (ReadFile (psf->file.handle, ((char*) ptr) + total, count, &dwNumberOfBytesRead, 0) == 0)
		{	psf_log_syserr (psf, GetLastError ()) ;
			break ;
			}
		else
			count = dwNumberOfBytesRead ;

		if (count == 0)
			break ;

		total += count ;
		items -= count ;
		} ;

	if (psf->is_pipe)
		psf->pipeoffset += total ;

	return total / bytes ;
} /* psf_fread */

/* USE_WINDOWS_API */ sf_count_t
psf_fwrite (const void *ptr, sf_count_t bytes, sf_count_t items, SF_PRIVATE *psf)
{	sf_count_t total = 0 ;
	ssize_t	count ;
	DWORD dwNumberOfBytesWritten ;

	if (psf->virtual_io)
		return psf->vio.write (ptr, bytes * items, psf->vio_user_data) / bytes ;

	items *= bytes ;

	/* Do this check after the multiplication above. */
	if (items <= 0)
		return 0 ;

	while (items > 0)
	{	/* Break the writes down to a sensible size. */
		count = (items > SENSIBLE_SIZE) ? SENSIBLE_SIZE : (ssize_t) items ;

		if (WriteFile (psf->file.handle, ((const char*) ptr) + total, count, &dwNumberOfBytesWritten, 0) == 0)
		{	psf_log_syserr (psf, GetLastError ()) ;
			break ;
			}
		else
			count = dwNumberOfBytesWritten ;

		if (count == 0)
			break ;

		total += count ;
		items -= count ;
		} ;

	if (psf->is_pipe)
		psf->pipeoffset += total ;

	return total / bytes ;
} /* psf_fwrite */

/* USE_WINDOWS_API */ sf_count_t
psf_ftell (SF_PRIVATE *psf)
{	sf_count_t pos ;
	LARGE_INTEGER liDistanceToMove, liNewFilePointer ;
	BOOL fResult ;
	DWORD dwError ;

	if (psf->virtual_io)
		return psf->vio.tell (psf->vio_user_data) ;

	if (psf->is_pipe)
		return psf->pipeoffset ;

	liDistanceToMove.QuadPart = 0 ;

	fResult = SetFilePointerEx (psf->file.handle, liDistanceToMove, &liNewFilePointer, FILE_CURRENT) ;

	if (fResult == FALSE)
		dwError = GetLastError () ;
	else
		dwError = NO_ERROR ;

	if (dwError != NO_ERROR)
	{	psf_log_syserr (psf, dwError) ;
		return -1 ;
		} ;

	pos = liNewFilePointer.QuadPart ;

	return pos - psf->fileoffset ;
} /* psf_ftell */

/* USE_WINDOWS_API */ static int
psf_close_handle (HANDLE handle)
{	if (handle == INVALID_HANDLE_VALUE)
		return 0 ;

	if (CloseHandle (handle) == 0)
		return -1 ;

	return 0 ;
} /* psf_close_handle */

/* USE_WINDOWS_API */ sf_count_t
psf_fgets (char *buffer, sf_count_t bufsize, SF_PRIVATE *psf)
{	sf_count_t k = 0 ;
	sf_count_t count ;
	DWORD dwNumberOfBytesRead ;

	while (k < bufsize - 1)
	{	if (ReadFile (psf->file.handle, &(buffer [k]), 1, &dwNumberOfBytesRead, 0) == 0)
		{	psf_log_syserr (psf, GetLastError ()) ;
			break ;
			}
		else
		{	count = dwNumberOfBytesRead ;
			/* note that we only check for '\n' not other line endings such as CRLF */
			if (count == 0 || buffer [k++] == '\n')
				break ;
			} ;
		} ;

	buffer [k] = 0 ;

	return k ;
} /* psf_fgets */

/* USE_WINDOWS_API */ int
psf_is_pipe (SF_PRIVATE *psf)
{
	if (psf->virtual_io)
		return SF_FALSE ;

	if (GetFileType (psf->file.handle) == FILE_TYPE_DISK)
		return SF_FALSE ;

	/* Default to maximum safety. */
	return SF_TRUE ;
} /* psf_is_pipe */

/* USE_WINDOWS_API */ sf_count_t
psf_get_filelen_handle (HANDLE handle)
{	sf_count_t filelen ;
	LARGE_INTEGER liFileSize ;
	BOOL fResult ;
	DWORD dwError = NO_ERROR ;

	fResult = GetFileSizeEx (handle, &liFileSize) ;

	if (fResult == FALSE)
		dwError = GetLastError () ;

	if (dwError != NO_ERROR)
		return (sf_count_t) -1 ;

	filelen = liFileSize.QuadPart ;

	return filelen ;
} /* psf_get_filelen_handle */

/* USE_WINDOWS_API */ void
psf_fsync (SF_PRIVATE *psf)
{	FlushFileBuffers (psf->file.handle) ;
} /* psf_fsync */


/* USE_WINDOWS_API */ int
psf_ftruncate (SF_PRIVATE *psf, sf_count_t len)
{	int retval = 0 ;
	LARGE_INTEGER liDistanceToMove ;
	BOOL fResult ;
	DWORD dwError = NO_ERROR ;

	/* This implementation trashes the current file position.
	** should it save and restore it? what if the current position is past
	** the new end of file?
	*/

	/* Returns 0 on success, non-zero on failure. */
	if (len < 0)
		return 1 ;

	liDistanceToMove.QuadPart = (sf_count_t) len ;

	fResult = SetFilePointerEx (psf->file.handle, liDistanceToMove, NULL, FILE_BEGIN) ;

	if (fResult == FALSE)
		dwError = GetLastError () ;

	if (dwError != NO_ERROR)
	{	retval = -1 ;
		psf_log_syserr (psf, dwError) ;
		}
	else
	{	/* Note: when SetEndOfFile is used to extend a file, the contents of the
		** new portion of the file is undefined. This is unlike chsize(),
		** which guarantees that the new portion of the file will be zeroed.
		** Not sure if this is important or not.
		*/
		if (SetEndOfFile (psf->file.handle) == 0)
		{	retval = -1 ;
			psf_log_syserr (psf, GetLastError ()) ;
			} ;
		} ;

	return retval ;
} /* psf_ftruncate */

#endif

