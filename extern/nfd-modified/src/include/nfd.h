/*
  Native File Dialog

  User API

  http://www.frogtoss.com/labs
 */


#ifndef _NFD_H
#define _NFD_H

#include <stddef.h>
#include <functional>
#include <string>
#include <vector>

/* denotes UTF-8 char */
typedef char nfdchar_t;

typedef std::function<void(const char*)> nfdselcallback_t;

/* opaque data structure -- see NFD_PathSet_* */
typedef struct {
    nfdchar_t *buf;
    size_t *indices; /* byte offsets into buf */
    size_t count;    /* number of indices into buf */
}nfdpathset_t;

typedef enum {
    NFD_ERROR,       /* programmatic error */
    NFD_OKAY,        /* user pressed okay, or successful return */
    NFD_CANCEL       /* user pressed cancel */
}nfdresult_t;
    

/* nfd_<targetplatform>.c */

/* single file open dialog */    
nfdresult_t NFD_OpenDialog( const std::vector<std::string>& filterList,
                            const nfdchar_t *defaultPath,
                            nfdchar_t **outPath,
                            nfdselcallback_t selCallback = NULL,
                            const nfdchar_t *defaultFileName = NULL );

/* multiple file open dialog */    
nfdresult_t NFD_OpenDialogMultiple( const std::vector<std::string>& filterList,
                                    const nfdchar_t *defaultPath,
                                    nfdpathset_t *outPaths,
                                    nfdselcallback_t selCallback = NULL,
                            const nfdchar_t *defaultFileName = NULL );

/* save dialog */
nfdresult_t NFD_SaveDialog( const std::vector<std::string>& filterList,
                            const nfdchar_t *defaultPath,
                            nfdchar_t **outPath,
                            nfdselcallback_t selCallback = NULL,
                            const nfdchar_t *defaultFileName = NULL );


/* select folder dialog */
nfdresult_t NFD_PickFolder( const nfdchar_t *defaultPath,
                            nfdchar_t **outPath);

/* nfd_common.c */

/* get last error -- set when nfdresult_t returns NFD_ERROR */
const char *NFD_GetError( void );
/* get the number of entries stored in pathSet */
size_t      NFD_PathSet_GetCount( const nfdpathset_t *pathSet );
/* Get the UTF-8 path at offset index */
nfdchar_t  *NFD_PathSet_GetPath( const nfdpathset_t *pathSet, size_t index );
/* Free the pathSet */    
void        NFD_PathSet_Free( nfdpathset_t *pathSet );

#endif
