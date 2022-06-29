#include "nfd.h"

#include <stdio.h>
#include <stdlib.h>

/* this test should compile on all supported platforms */

int main( void )
{
    nfdpathset_t pathSet;
    nfdresult_t result = NFD_OpenDialogMultiple( "png,jpg;pdf", NULL, &pathSet );
    if ( result == NFD_OKAY )
    {
        size_t i;
        for ( i = 0; i < NFD_PathSet_GetCount(&pathSet); ++i )
        {
            nfdchar_t *path = NFD_PathSet_GetPath(&pathSet, i);
            printf("Path %i: %s\n", (int)i, path );
        }
        NFD_PathSet_Free(&pathSet);
    }
    else if ( result == NFD_CANCEL )
    {
        puts("User pressed cancel.");
    }
    else 
    {
        printf("Error: %s\n", NFD_GetError() );
    }

    return 0;
}
