#include "nfd.h"

#include <stdio.h>
#include <stdlib.h>

/* this test should compile on all supported platforms */

int main( void )
{
    nfdchar_t *savePath = NULL;
    nfdresult_t result = NFD_SaveDialog( "png,jpg;pdf", NULL, &savePath );
    if ( result == NFD_OKAY )
    {
        puts("Success!");
        puts(savePath);
        free(savePath);
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
