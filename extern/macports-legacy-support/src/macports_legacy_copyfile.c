/*
 * Copyright (c) 2022
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* MP support header */
#include "MacportsLegacySupport.h"

/* copyfile and its associated functions wrap */
#if __MP_LEGACY_SUPPORT_COPYFILE_WRAP__

#include <stdlib.h>
#include <dlfcn.h>
#include <sys/stat.h>

#include <copyfile.h>

int copyfile_state_get(copyfile_state_t s, uint32_t flag, void * dst)
{
    int (*real_copyfile_state_get)(copyfile_state_t s, uint32_t flag, void * dst);
    int ret;
    char *file_name;
    int file_descriptor;
    struct stat file_stat;

    real_copyfile_state_get = dlsym(RTLD_NEXT, "copyfile_state_get");
    if (real_copyfile_state_get == NULL) {
        exit(EXIT_FAILURE);
    }

    switch(flag)
    {
    case COPYFILE_STATE_COPIED:
        /* since copyfile did not recored how many bytes were copied, return the size of the destination file */
        /* first check if the file descriptor has been set */
        ret = real_copyfile_state_get(s, COPYFILE_STATE_DST_FD, &file_descriptor);
        if ( ret < 0 ) {
            return ret;
        }
        if ( file_descriptor != -2 ) {
            ret = fstat(file_descriptor, &file_stat);
            return ret;
        } else {
	     /* the file descriptor was not set, so check the file name */
            ret = real_copyfile_state_get(s, COPYFILE_STATE_DST_FILENAME, &file_name);
            if ( ret < 0 ) {
                return ret;
            }
            if ( file_name == NULL ) {
                /* neither the file descriptor nor the file name has been set */
                return real_copyfile_state_get(s, flag, dst);
            }
            ret = stat(file_name, &file_stat);
        }
        *(off_t*)dst = file_stat.st_size;
        return 0;
        break;
    case COPYFILE_STATE_STATUS_CB:
    case COPYFILE_STATE_STATUS_CTX:
        /* copyfile did not run the callback function, so return default (which is an error) */
    default:
        return real_copyfile_state_get(s, flag, dst);
    }
    return 0;
}

#endif /* __MP_LEGACY_SUPPORT_COPYFILE_WRAP__ */
