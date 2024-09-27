/*
  Native File Dialog

  http://www.frogtoss.com/labs
 */

#include <AppKit/AppKit.h>
#include "nfd.h"
#include "nfd_common.h"

// this language is a mess!
// who thought it was a good idea to combine Objective-C and C++ together
// when you could just have used C++ and call it a day!!!
//
// might as well make Objective-Ruswift++...

static NSArray *BuildAllowedFileTypes( const std::vector<std::string>& filterList )
{
    // Commas and semicolons are the same thing on this platform

    // like what about THIS INSTEAD!
    // NSMutableArray *buildFilterList = NSMutableArray::alloc()->init();
    NSMutableArray *buildFilterList = [[NSMutableArray alloc] init];

    std::string typebuf;
    int index=-1;
    for (const std::string& i: filterList) {
      index++;
      if (!(index&1)) {
        continue;
      }
      typebuf="";
      for (const char& j: i) {
        if (j==' ' || j==',' || j ==';') {
          // or this: NSString::stringWithUTF8String(typebuf);
          // buildFilterList->addObject(thisType);
          // really? did you have to make this mess?!
          const char* typebufC=typebuf.c_str();
          NSString *thisType = [NSString stringWithUTF8String:typebufC];
          [buildFilterList addObject:thisType];
          typebuf="";
        } else if (j!='.' && j!='*') {
          typebuf+=j;
        }
      }
      if (!typebuf.empty()) {
        // I don't think this will work, but come on...
        const char* typebufC=typebuf.c_str();
        NSString *thisType = [NSString stringWithUTF8String:typebufC];
        [buildFilterList addObject:thisType];
      }
    }

    NSArray *returnArray = [NSArray arrayWithArray:buildFilterList];

    [buildFilterList release];
    return returnArray;
}

static void AddFilterListToDialog( NSSavePanel *dialog, const std::vector<std::string>& filterList )
{
    if ( filterList.size()&1 )
        return;

    NSArray *allowedFileTypes = BuildAllowedFileTypes( filterList );
    if ( [allowedFileTypes count] != 0 )
    {
        [dialog setAllowedFileTypes:allowedFileTypes];
    }
}

static void SetDefaultPath( NSSavePanel *dialog, const nfdchar_t *defaultPath )
{
    if ( !defaultPath || strlen(defaultPath) == 0 )
        return;

    NSString *defaultPathString = [NSString stringWithUTF8String: defaultPath];
    NSURL *url = [NSURL fileURLWithPath:defaultPathString isDirectory:YES];
    [dialog setDirectoryURL:url];    
}


/* fixme: pathset should be pathSet */
static nfdresult_t AllocPathSet( NSArray *urls, nfdpathset_t *pathset )
{
    assert(pathset);
    assert([urls count]);

    pathset->count = (size_t)[urls count];
    pathset->indices = (size_t*)NFDi_Malloc( sizeof(size_t)*pathset->count );
    if ( !pathset->indices ) 
    {
        return NFD_ERROR;
    }

    // count the total space needed for buf
    size_t bufsize = 0;
    for ( NSURL *url in urls )
    {
        NSString *path = [url path];
        bufsize += [path lengthOfBytesUsingEncoding:NSUTF8StringEncoding] + 1;
    }

    pathset->buf = (nfdchar_t*)NFDi_Malloc( sizeof(nfdchar_t) * bufsize );
    if ( !pathset->buf )
    {
        return NFD_ERROR;
    }

    // fill buf
    nfdchar_t *p_buf = pathset->buf;
    size_t count = 0;
    for ( NSURL *url in urls )
    {
        NSString *path = [url path];
        const nfdchar_t *utf8Path = [path UTF8String];
        size_t byteLen = [path lengthOfBytesUsingEncoding:NSUTF8StringEncoding] + 1;
        memcpy( p_buf, utf8Path, byteLen );

        ptrdiff_t index = p_buf - pathset->buf;
        assert( index >= 0 );
        pathset->indices[count] = (size_t)index;

        p_buf += byteLen;
        ++count;
    }

    return NFD_OKAY;
}

/* public */


nfdresult_t NFD_OpenDialog( const std::vector<std::string>& filterList,
                            const nfdchar_t *defaultPath,
                            nfdchar_t **outPath,
                            nfdselcallback_t selCallback,
                            const nfdchar_t *defaultFileName )
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    NSWindow *keyWindow = [[NSApplication sharedApplication] keyWindow];    
    NSOpenPanel *dialog = [NSOpenPanel openPanel];
    [dialog setAllowsMultipleSelection:NO];

    // Build the filter list
    AddFilterListToDialog(dialog, filterList);

    // Set the starting directory
    SetDefaultPath(dialog, defaultPath);

    nfdresult_t nfdResult = NFD_CANCEL;
    if ( [dialog runModal] == NSModalResponseOK )
    {
        NSURL *url = [dialog URL];
        const char *utf8Path = [[url path] UTF8String];

        // byte count, not char count
        size_t len = strlen(utf8Path);//NFDi_UTF8_Strlen(utf8Path);

        *outPath = (nfdchar_t*)NFDi_Malloc( len+1 );
        if ( !*outPath )
        {
            [pool release];
            [keyWindow makeKeyAndOrderFront:nil];            
            return NFD_ERROR;
        }
        memcpy( *outPath, utf8Path, len+1 ); /* copy null term */
        nfdResult = NFD_OKAY;
    }
    [pool release];

    [keyWindow makeKeyAndOrderFront:nil];
    return nfdResult;
}


nfdresult_t NFD_OpenDialogMultiple( const std::vector<std::string>& filterList,
                                    const nfdchar_t *defaultPath,
                                    nfdpathset_t *outPaths,
                                    nfdselcallback_t selCallback,
                            const nfdchar_t *defaultFileName )
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSWindow *keyWindow = [[NSApplication sharedApplication] keyWindow];
    
    NSOpenPanel *dialog = [NSOpenPanel openPanel];
    [dialog setAllowsMultipleSelection:YES];

    // Build the fiter list.
    AddFilterListToDialog(dialog, filterList);

    // Set the starting directory
    SetDefaultPath(dialog, defaultPath);
    
    nfdresult_t nfdResult = NFD_CANCEL;
    if ( [dialog runModal] == NSModalResponseOK )
    {
        NSArray *urls = [dialog URLs];

        if ( [urls count] == 0 )
        {
            [pool release];
            [keyWindow makeKeyAndOrderFront:nil];            
            return NFD_CANCEL;
        }

        if ( AllocPathSet( urls, outPaths ) == NFD_ERROR )
        {
            [pool release];
            [keyWindow makeKeyAndOrderFront:nil];            
            return NFD_ERROR;
        }

        nfdResult = NFD_OKAY;
    }
    [pool release];

    [keyWindow makeKeyAndOrderFront:nil];    
    return nfdResult;
}


nfdresult_t NFD_SaveDialog( const std::vector<std::string>& filterList,
                            const nfdchar_t *defaultPath,
                            nfdchar_t **outPath,
                            nfdselcallback_t selCallback,
                            const nfdchar_t *defaultFileName )
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSWindow *keyWindow = [[NSApplication sharedApplication] keyWindow];
    
    NSSavePanel *dialog = [NSSavePanel savePanel];
    [dialog setExtensionHidden:NO];
    
    // Build the filter list.
    AddFilterListToDialog(dialog, filterList);

    // Set the starting directory
    SetDefaultPath(dialog, defaultPath);

    nfdresult_t nfdResult = NFD_CANCEL;
    if ( [dialog runModal] == NSModalResponseOK )
    {
        NSURL *url = [dialog URL];
        const char *utf8Path = [[url path] UTF8String];

        size_t byteLen = [url.path lengthOfBytesUsingEncoding:NSUTF8StringEncoding] + 1;
        
        *outPath = (nfdchar_t*)NFDi_Malloc( byteLen );
        if ( !*outPath )
        {
            [pool release];
            [keyWindow makeKeyAndOrderFront:nil];            
            return NFD_ERROR;
        }
        memcpy( *outPath, utf8Path, byteLen );
        nfdResult = NFD_OKAY;
    }

    [pool release];
    [keyWindow makeKeyAndOrderFront:nil];
    return nfdResult;
}

nfdresult_t NFD_PickFolder(const nfdchar_t *defaultPath,
    nfdchar_t **outPath)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    NSWindow *keyWindow = [[NSApplication sharedApplication] keyWindow];
    NSOpenPanel *dialog = [NSOpenPanel openPanel];
    [dialog setAllowsMultipleSelection:NO];
    [dialog setCanChooseDirectories:YES];
    [dialog setCanCreateDirectories:YES];
    [dialog setCanChooseFiles:NO];

    // Set the starting directory
    SetDefaultPath(dialog, defaultPath);

    nfdresult_t nfdResult = NFD_CANCEL;
    if ( [dialog runModal] == NSModalResponseOK )
    {
        NSURL *url = [dialog URL];
        const char *utf8Path = [[url path] UTF8String];

        // byte count, not char count
        size_t len = strlen(utf8Path);//NFDi_UTF8_Strlen(utf8Path);

        *outPath = (nfdchar_t*)NFDi_Malloc( len+1 );
        if ( !*outPath )
        {
            [pool release];
            [keyWindow makeKeyAndOrderFront:nil];            
            return NFD_ERROR;
        }
        memcpy( *outPath, utf8Path, len+1 ); /* copy null term */
        nfdResult = NFD_OKAY;
    }
    [pool release];

    [keyWindow makeKeyAndOrderFront:nil];
    return nfdResult;
}
