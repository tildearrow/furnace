/*
  Native File Dialog

  http://www.frogtoss.com/labs
 */


#ifdef __MINGW32__
// Explicitly setting NTDDI version, this is necessary for the MinGW compiler
#ifdef NTDDI_VERSION
#undef NTDDI_VERSION
#endif
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define NTDDI_VERSION NTDDI_VISTA
#define _WIN32_WINNT _WIN32_WINNT_VISTA
#endif

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

/* only locally define UNICODE in this compilation unit */
#ifndef UNICODE
#define UNICODE
#endif

#include <wchar.h>
#include <stdio.h>
#include <assert.h>
#include <windows.h>
#include <shobjidl.h>
#include "nfd_common.h"

// hack I know
#include "../../../src/utfutils.h"

// hack 2...
#include "../../../src/ta-log.h"

class NFDWinEvents: public IFileDialogEvents {
  nfdselcallback_t selCallback;
  size_t refCount;

  virtual ~NFDWinEvents() {
  }
  public:
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) {
      logV("%p: QueryInterface called DAMN IT",(const void*)this);
      *ppv=NULL;
      return E_NOTIMPL;
    }

    IFACEMETHODIMP_(ULONG) AddRef() {
      logV("%p: AddRef() called",(const void*)this);
      return InterlockedIncrement(&refCount);
    }

    IFACEMETHODIMP_(ULONG) Release() {
      logV("%p: Release() called",(const void*)this);
      LONG ret=InterlockedDecrement(&refCount);
      if (ret==0) {
        logV("%p: Destroying the final object.",(const void*)this);
        delete this;
      }
      return ret;
    }

    IFACEMETHODIMP OnFileOk(IFileDialog*) { return S_OK; }
    IFACEMETHODIMP OnFolderChange(IFileDialog*) { return E_NOTIMPL; }
    IFACEMETHODIMP OnFolderChanging(IFileDialog*, IShellItem*) { return E_NOTIMPL; }
    IFACEMETHODIMP OnOverwrite(IFileDialog*, IShellItem*, FDE_OVERWRITE_RESPONSE*) { return E_NOTIMPL; }
    IFACEMETHODIMP OnShareViolation(IFileDialog*, IShellItem*, FDE_SHAREVIOLATION_RESPONSE*) { return E_NOTIMPL; }
    IFACEMETHODIMP OnTypeChange(IFileDialog*) { return E_NOTIMPL; }

    IFACEMETHODIMP OnSelectionChange(IFileDialog* dialog) {
      // Get the file name
      logV("%p: OnSelectionChange() called",(const void*)this);
      ::IShellItem *shellItem(NULL);
      logV("%p: GetCurrentSelection",(const void*)this);
      HRESULT result = dialog->GetCurrentSelection(&shellItem);
      if ( !SUCCEEDED(result) )
      {
        logV("%p: failure!",(const void*)this);
        return S_OK;
      }
      wchar_t *filePath(NULL);
      result = shellItem->GetDisplayName(::SIGDN_FILESYSPATH, &filePath);
      if ( !SUCCEEDED(result) )
      {
          logV("%p: GDN failure!",(const void*)this);
          shellItem->Release();
          return S_OK;
      }
      std::string utf8FilePath=utf16To8(filePath);
      if (selCallback!=NULL) {
        logV("%p: calling back.",(const void*)this);
        selCallback(utf8FilePath.c_str());
        logV("%p: end of callback",(const void*)this);
      } else {
        logV("%p: no callback.",(const void*)this);
      }
      logV("%p: I got you for a value of %s",(const void*)this,utf8FilePath.c_str());
      shellItem->Release();
      logV("%p: shellItem->Release()",(const void*)this);
      return S_OK;
    }
    NFDWinEvents(nfdselcallback_t callback):
      selCallback(callback),
      refCount(1) {
        logV("%p: CONSTRUCT!",(const void*)this);
    }
};

#define COM_INITFLAGS ::COINIT_APARTMENTTHREADED | ::COINIT_DISABLE_OLE1DDE

static BOOL COMIsInitialized(HRESULT coResult)
{
    if (coResult == RPC_E_CHANGED_MODE)
    {
        // If COM was previously initialized with different init flags,
        // NFD still needs to operate. Eat this warning.
        return TRUE;
    }

    return SUCCEEDED(coResult);
}

static HRESULT COMInit(void)
{
    return ::CoInitializeEx(NULL, COM_INITFLAGS);
}

static void COMUninit(HRESULT coResult)
{
    // do not uninitialize if RPC_E_CHANGED_MODE occurred -- this
    // case does not refcount COM.
    if (SUCCEEDED(coResult))
        ::CoUninitialize();
}

// allocs the space in outPath -- call free()
static void CopyWCharToNFDChar( const wchar_t *inStr, nfdchar_t **outStr )
{
    int inStrCharacterCount = static_cast<int>(wcslen(inStr)); 
    int bytesNeeded = WideCharToMultiByte( CP_UTF8, 0,
                                           inStr, inStrCharacterCount,
                                           NULL, 0, NULL, NULL );    
    assert( bytesNeeded );
    bytesNeeded += 1;

    *outStr = (nfdchar_t*)NFDi_Malloc( bytesNeeded );
    if ( !*outStr )
        return;

    int bytesWritten = WideCharToMultiByte( CP_UTF8, 0,
                                            inStr, -1,
                                            *outStr, bytesNeeded,
                                            NULL, NULL );
    assert( bytesWritten ); _NFD_UNUSED( bytesWritten );
}

/* includes NULL terminator byte in return */
static size_t GetUTF8ByteCountForWChar( const wchar_t *str )
{
    size_t bytesNeeded = WideCharToMultiByte( CP_UTF8, 0,
                                              str, -1,
                                              NULL, 0, NULL, NULL );
    assert( bytesNeeded );
    return bytesNeeded+1;
}

// write to outPtr -- no free() necessary.
static int CopyWCharToExistingNFDCharBuffer( const wchar_t *inStr, nfdchar_t *outPtr )
{
    int bytesNeeded = static_cast<int>(GetUTF8ByteCountForWChar( inStr ));

    /* invocation copies null term */
    int bytesWritten = WideCharToMultiByte( CP_UTF8, 0,
                                            inStr, -1,
                                            outPtr, bytesNeeded,
                                            NULL, 0 );
    assert( bytesWritten );

    return bytesWritten;

}


// allocs the space in outStr -- call free()
static void CopyNFDCharToWChar( const nfdchar_t *inStr, wchar_t **outStr )
{
    int inStrByteCount = static_cast<int>(strlen(inStr));
    int charsNeeded = MultiByteToWideChar(CP_UTF8, 0,
                                          inStr, inStrByteCount,
                                          NULL, 0 );    
    assert( charsNeeded );
    assert( !*outStr );
    charsNeeded += 1; // terminator
    
    *outStr = (wchar_t*)NFDi_Malloc( charsNeeded * sizeof(wchar_t) );    
    if ( !*outStr )
        return;        

    int ret = MultiByteToWideChar(CP_UTF8, 0,
                                  inStr, inStrByteCount,
                                  *outStr, charsNeeded);
    (*outStr)[charsNeeded-1] = '\0';

#ifdef _DEBUG
    int inStrCharacterCount = static_cast<int>(NFDi_UTF8_Strlen(inStr));
    if (ret!=inStrCharacterCount) {
      logW("length does not match! %d != %d",ret,inStrCharacterCount);
    }
#else
    _NFD_UNUSED(ret);
#endif
}

static nfdresult_t AddFiltersToDialog( ::IFileDialog *fileOpenDialog, const std::vector<std::string>& filterList )
{
    const wchar_t WILDCARD[] = L"*.*";

    if (filterList.empty())
        return NFD_OKAY;

    // list size has to be an even number (name/filter)
    if (filterList.size()&1)
        return NFD_ERROR;

    // Count rows to alloc
    UINT filterCount = filterList.size()>>1; /* guaranteed to have one filter on a correct, non-empty parse */

    if (filterCount==0) filterCount=1;

    if ( !filterCount )
    {
        NFDi_SetError("Error parsing filters.");
        return NFD_ERROR;
    }

    /* filterCount plus 1 because we hardcode the *.* wildcard after the while loop */
    COMDLG_FILTERSPEC *specList = (COMDLG_FILTERSPEC*)NFDi_Malloc( sizeof(COMDLG_FILTERSPEC) * ((size_t)filterCount) );
    if ( !specList )
    {
        return NFD_ERROR;
    }
    for (UINT i = 0; i < filterCount; ++i )
    {
        specList[i].pszName = NULL;
        specList[i].pszSpec = NULL;
    }

    size_t specIdx = 0;

    for (size_t i=0; i<filterList.size(); i+=2) {
      String name=filterList[i];
      String spec=filterList[i+1];
      for (char& i: spec) {
        if (i==' ') i=';';
      }
      if (spec==".*") spec="*.*";

      CopyNFDCharToWChar( name.c_str(), (wchar_t**)&specList[specIdx].pszName );
      CopyNFDCharToWChar( spec.c_str(), (wchar_t**)&specList[specIdx].pszSpec );
      ++specIdx;
    }

    /* Add wildcard if specIdx is 0 */
    if (specIdx==0) {
      specList[specIdx].pszSpec = WILDCARD;
      specList[specIdx].pszName = WILDCARD;
    }
    
    fileOpenDialog->SetFileTypes( filterCount, specList );

    /* free speclist */
    for ( size_t i = 0; i < filterCount; ++i )
    {
        NFDi_Free( (void*)specList[i].pszSpec );
    }
    NFDi_Free( specList );    

    return NFD_OKAY;
}

static nfdresult_t AllocPathSet( IShellItemArray *shellItems, nfdpathset_t *pathSet )
{
    const char ERRORMSG[] = "Error allocating pathset.";

    assert(shellItems);
    assert(pathSet);
    
    // How many items in shellItems?
    DWORD numShellItems;
    HRESULT result = shellItems->GetCount(&numShellItems);
    if ( !SUCCEEDED(result) )
    {
        NFDi_SetError(ERRORMSG);
        return NFD_ERROR;
    }

    pathSet->count = static_cast<size_t>(numShellItems);
    assert( pathSet->count > 0 );

    pathSet->indices = (size_t*)NFDi_Malloc( sizeof(size_t)*pathSet->count );
    if ( !pathSet->indices )
    {
        return NFD_ERROR;
    }

    /* count the total bytes needed for buf */
    size_t bufSize = 0;
    for ( DWORD i = 0; i < numShellItems; ++i )
    {
        ::IShellItem *shellItem;
        result = shellItems->GetItemAt(i, &shellItem);
        if ( !SUCCEEDED(result) )
        {
            NFDi_SetError(ERRORMSG);
            return NFD_ERROR;
        }

        // Confirm SFGAO_FILESYSTEM is true for this shellitem, or ignore it.
        SFGAOF attribs;
        result = shellItem->GetAttributes( SFGAO_FILESYSTEM, &attribs );
        if ( !SUCCEEDED(result) )
        {
            NFDi_SetError(ERRORMSG);
            return NFD_ERROR;
        }
        if ( !(attribs & SFGAO_FILESYSTEM) )
            continue;

        LPWSTR name;
        shellItem->GetDisplayName(SIGDN_FILESYSPATH, &name);

        // Calculate length of name with UTF-8 encoding
        bufSize += GetUTF8ByteCountForWChar( name );
        
        CoTaskMemFree(name);
    }

    assert(bufSize);

    pathSet->buf = (nfdchar_t*)NFDi_Malloc( sizeof(nfdchar_t) * bufSize );
    memset( pathSet->buf, 0, sizeof(nfdchar_t) * bufSize );

    /* fill buf */
    nfdchar_t *p_buf = pathSet->buf;
    for (DWORD i = 0; i < numShellItems; ++i )
    {
        ::IShellItem *shellItem;
        result = shellItems->GetItemAt(i, &shellItem);
        if ( !SUCCEEDED(result) )
        {
            NFDi_SetError(ERRORMSG);
            return NFD_ERROR;
        }

        // Confirm SFGAO_FILESYSTEM is true for this shellitem, or ignore it.
        SFGAOF attribs;
        result = shellItem->GetAttributes( SFGAO_FILESYSTEM, &attribs );
        if ( !SUCCEEDED(result) )
        {
            NFDi_SetError(ERRORMSG);
            return NFD_ERROR;
        }
        if ( !(attribs & SFGAO_FILESYSTEM) )
            continue;

        LPWSTR name;
        shellItem->GetDisplayName(SIGDN_FILESYSPATH, &name);

        int bytesWritten = CopyWCharToExistingNFDCharBuffer(name, p_buf);
        CoTaskMemFree(name);

        ptrdiff_t index = p_buf - pathSet->buf;
        assert( index >= 0 );
        pathSet->indices[i] = static_cast<size_t>(index);
        
        p_buf += bytesWritten; 
    }
     
    return NFD_OKAY;
}


static nfdresult_t SetDefaultPath( IFileDialog *dialog, const char *defaultPath )
{
    if ( !defaultPath || strlen(defaultPath) == 0 || strcmp(defaultPath,"\\")==0 )
        return NFD_OKAY;

    wchar_t *defaultPathW = {0};
    CopyNFDCharToWChar( defaultPath, &defaultPathW );

    IShellItem *folder;
    HRESULT result = SHCreateItemFromParsingName( defaultPathW, NULL, IID_PPV_ARGS(&folder) );

    // Valid non results.
    if ( result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) || result == HRESULT_FROM_WIN32(ERROR_INVALID_DRIVE) )
    {
        NFDi_Free( defaultPathW );
        return NFD_OKAY;
    }

    if ( !SUCCEEDED(result) )
    {
        NFDi_SetError("Error creating ShellItem");
        NFDi_Free( defaultPathW );
        return NFD_ERROR;
    }
    
    // Could also call SetDefaultFolder(), but this guarantees defaultPath -- more consistency across API.
    dialog->SetFolder( folder );

    NFDi_Free( defaultPathW );
    folder->Release();
    
    return NFD_OKAY;
}

/* public */


nfdresult_t NFD_OpenDialog( const std::vector<std::string>& filterList,
                            const nfdchar_t *defaultPath,
                            nfdchar_t **outPath,
                            nfdselcallback_t selCallback )
{
    nfdresult_t nfdResult = NFD_ERROR;
    NFDWinEvents* winEvents;
    bool hasEvents=true;
    DWORD eventID=0;
    
    HRESULT coResult = COMInit();
    if (!COMIsInitialized(coResult))
    {        
        NFDi_SetError("Could not initialize COM.");
        return nfdResult;
    }

    // Create dialog
    ::IFileOpenDialog *fileOpenDialog(NULL);    
    HRESULT result = ::CoCreateInstance(::CLSID_FileOpenDialog, NULL,
                                        CLSCTX_ALL, ::IID_IFileOpenDialog,
                                        reinterpret_cast<void**>(&fileOpenDialog) );
                                
    if ( !SUCCEEDED(result) )
    {
        NFDi_SetError("Could not create dialog.");
        goto end;
    }

    // Build the filter list
    if ( !AddFiltersToDialog( fileOpenDialog, filterList ) )
    {
        goto end;
    }

    // Set the default path
    if ( !SetDefaultPath( fileOpenDialog, defaultPath ) )
    {
        goto end;
    }

    // Pass the callback
    winEvents=new NFDWinEvents(selCallback);
    if ( !SUCCEEDED(fileOpenDialog->Advise(winEvents,&eventID)) ) {
      // error... ignore
      hasEvents=false;
      winEvents->Release();
    } else {
      winEvents->Release();
    }

    // Show the dialog.
    // TODO: pass the Furnace window here
    result = fileOpenDialog->Show(NULL);
    if ( SUCCEEDED(result) )
    {
        // Get the file name
        ::IShellItem *shellItem(NULL);
        result = fileOpenDialog->GetResult(&shellItem);
        if ( !SUCCEEDED(result) )
        {
            NFDi_SetError("Could not get shell item from dialog.");
            goto end;
        }
        wchar_t *filePath(NULL);
        result = shellItem->GetDisplayName(::SIGDN_FILESYSPATH, &filePath);
        if ( !SUCCEEDED(result) )
        {
            NFDi_SetError("Could not get file path for selected.");
            shellItem->Release();
            goto end;
        }

        CopyWCharToNFDChar( filePath, outPath );
        CoTaskMemFree(filePath);
        if ( !*outPath )
        {
            /* error is malloc-based, error message would be redundant */
            shellItem->Release();
            goto end;
        }

        nfdResult = NFD_OKAY;
        shellItem->Release();
    }
    else if (result == HRESULT_FROM_WIN32(ERROR_CANCELLED) )
    {
        nfdResult = NFD_CANCEL;
    }
    else
    {
        NFDi_SetError("File dialog box show failed.");
        nfdResult = NFD_ERROR;
    }

end:
    if (fileOpenDialog) {
        if (hasEvents) {
          fileOpenDialog->Unadvise(eventID);
        }
        fileOpenDialog->Release();
    }

    COMUninit(coResult);
    
    return nfdResult;
}

nfdresult_t NFD_OpenDialogMultiple( const std::vector<std::string>& filterList,
                                    const nfdchar_t *defaultPath,
                                    nfdpathset_t *outPaths,
                                    nfdselcallback_t selCallback )
{
    nfdresult_t nfdResult = NFD_ERROR;
    NFDWinEvents* winEvents;
    bool hasEvents=true;
    DWORD eventID=0;


    HRESULT coResult = COMInit();
    if (!COMIsInitialized(coResult))
    {
        NFDi_SetError("Could not initialize COM.");        
        return nfdResult;
    }

    // Create dialog
    ::IFileOpenDialog *fileOpenDialog(NULL);    
    HRESULT result = ::CoCreateInstance(::CLSID_FileOpenDialog, NULL,
                                        CLSCTX_ALL, ::IID_IFileOpenDialog,
                                        reinterpret_cast<void**>(&fileOpenDialog) );
                                
    if ( !SUCCEEDED(result) )
    {
        fileOpenDialog = NULL;
        NFDi_SetError("Could not create dialog.");
        goto end;
    }

    // Build the filter list
    if ( !AddFiltersToDialog( fileOpenDialog, filterList ) )
    {
        goto end;
    }

    // Set the default path
    if ( !SetDefaultPath( fileOpenDialog, defaultPath ) )
    {
        goto end;
    }

    // Pass the callback
    winEvents=new NFDWinEvents(selCallback);
    if ( !SUCCEEDED(fileOpenDialog->Advise(winEvents,&eventID)) ) {
      // error... ignore
      hasEvents=false;
      winEvents->Release();
    } else {
      winEvents->Release();
    }

    // Set a flag for multiple options
    DWORD dwFlags;
    result = fileOpenDialog->GetOptions(&dwFlags);
    if ( !SUCCEEDED(result) )
    {
        NFDi_SetError("Could not get options.");
        goto end;
    }
    result = fileOpenDialog->SetOptions(dwFlags | FOS_ALLOWMULTISELECT);
    if ( !SUCCEEDED(result) )
    {
        NFDi_SetError("Could not set options.");
        goto end;
    }
 
    // Show the dialog.
    result = fileOpenDialog->Show(NULL);
    if ( SUCCEEDED(result) )
    {
        IShellItemArray *shellItems;
        result = fileOpenDialog->GetResults( &shellItems );
        if ( !SUCCEEDED(result) )
        {
            NFDi_SetError("Could not get shell items.");
            goto end;
        }
        
        if ( AllocPathSet( shellItems, outPaths ) == NFD_ERROR )
        {
            shellItems->Release();
            goto end;
        }

        shellItems->Release();
        nfdResult = NFD_OKAY;
    }
    else if (result == HRESULT_FROM_WIN32(ERROR_CANCELLED) )
    {
        nfdResult = NFD_CANCEL;
    }
    else
    {
        NFDi_SetError("File dialog box show failed.");
        nfdResult = NFD_ERROR;
    }

end:
    if (fileOpenDialog) {
        if (hasEvents) {
          fileOpenDialog->Unadvise(eventID);
        }
        fileOpenDialog->Release();
    }

    COMUninit(coResult);
    
    return nfdResult;
}

nfdresult_t NFD_SaveDialog( const std::vector<std::string>& filterList,
                            const nfdchar_t *defaultPath,
                            nfdchar_t **outPath,
                            nfdselcallback_t selCallback )
{
    nfdresult_t nfdResult = NFD_ERROR;

    HRESULT coResult = COMInit();
    if (!COMIsInitialized(coResult))
    {
        NFDi_SetError("Could not initialize COM.");
        return nfdResult;        
    }
    
    // Create dialog
    ::IFileSaveDialog *fileSaveDialog(NULL);    
    HRESULT result = ::CoCreateInstance(::CLSID_FileSaveDialog, NULL,
                                        CLSCTX_ALL, ::IID_IFileSaveDialog,
                                        reinterpret_cast<void**>(&fileSaveDialog) );

    if ( !SUCCEEDED(result) )
    {
        fileSaveDialog = NULL;
        NFDi_SetError("Could not create dialog.");
        goto end;
    }

    // Build the filter list
    if ( !AddFiltersToDialog( fileSaveDialog, filterList ) )
    {
        goto end;
    }

    // Set the default path
    if ( !SetDefaultPath( fileSaveDialog, defaultPath ) )
    {
        goto end;
    }

    // Set a flag for no history
    DWORD dwFlags;
    result = fileSaveDialog->GetOptions(&dwFlags);
    if ( !SUCCEEDED(result) )
    {
        NFDi_SetError("Could not get options.");
        goto end;
    }
    result = fileSaveDialog->SetOptions(dwFlags | FOS_DONTADDTORECENT);
    if ( !SUCCEEDED(result) )
    {
        NFDi_SetError("Could not set options.");
        goto end;
    }

    // Show the dialog.
    result = fileSaveDialog->Show(NULL);
    if ( SUCCEEDED(result) )
    {
        // Get the file name
        ::IShellItem *shellItem;
        result = fileSaveDialog->GetResult(&shellItem);
        if ( !SUCCEEDED(result) )
        {
            NFDi_SetError("Could not get shell item from dialog.");
            goto end;
        }
        wchar_t *filePath(NULL);
        result = shellItem->GetDisplayName(::SIGDN_FILESYSPATH, &filePath);
        if ( !SUCCEEDED(result) )
        {
            shellItem->Release();
            NFDi_SetError("Could not get file path for selected.");
            goto end;
        }

        CopyWCharToNFDChar( filePath, outPath );
        CoTaskMemFree(filePath);
        if ( !*outPath )
        {
            /* error is malloc-based, error message would be redundant */
            shellItem->Release();
            goto end;
        }

        nfdResult = NFD_OKAY;
        shellItem->Release();
    }
    else if (result == HRESULT_FROM_WIN32(ERROR_CANCELLED) )
    {
        nfdResult = NFD_CANCEL;
    }
    else
    {
        NFDi_SetError("File dialog box show failed.");
        nfdResult = NFD_ERROR;
    }
    
end:
    if ( fileSaveDialog )
        fileSaveDialog->Release();

    COMUninit(coResult);
    
    return nfdResult;
}



nfdresult_t NFD_PickFolder(const nfdchar_t *defaultPath,
    nfdchar_t **outPath)
{
    nfdresult_t nfdResult = NFD_ERROR;
    DWORD dwOptions = 0;

    HRESULT coResult = COMInit();
    if (!COMIsInitialized(coResult))
    {
        NFDi_SetError("CoInitializeEx failed.");
        return nfdResult;
    }

    // Create dialog
    ::IFileOpenDialog *fileDialog(NULL);
    HRESULT result = CoCreateInstance(CLSID_FileOpenDialog,
                                      NULL,
                                      CLSCTX_ALL,
                                      IID_PPV_ARGS(&fileDialog));
    if ( !SUCCEEDED(result) )
    {        
        NFDi_SetError("CoCreateInstance for CLSID_FileOpenDialog failed.");
        goto end;
    }

    // Set the default path
    if (SetDefaultPath(fileDialog, defaultPath) != NFD_OKAY)
    {
        NFDi_SetError("SetDefaultPath failed.");
        goto end;
    }

    // Get the dialogs options
    if (!SUCCEEDED(fileDialog->GetOptions(&dwOptions)))
    {
        NFDi_SetError("GetOptions for IFileDialog failed.");
        goto end;
    }

    // Add in FOS_PICKFOLDERS which hides files and only allows selection of folders
    if (!SUCCEEDED(fileDialog->SetOptions(dwOptions | FOS_PICKFOLDERS)))
    {
        NFDi_SetError("SetOptions for IFileDialog failed.");
        goto end;
    }

    // Show the dialog to the user
    result = fileDialog->Show(NULL);
    if ( SUCCEEDED(result) )
    {
        // Get the folder name
        ::IShellItem *shellItem(NULL);

        result = fileDialog->GetResult(&shellItem);
        if ( !SUCCEEDED(result) )
        {
            NFDi_SetError("Could not get file path for selected.");
            shellItem->Release();
            goto end;
        }

        wchar_t *path = NULL;
        result = shellItem->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &path);
        if ( !SUCCEEDED(result) )
        {
            NFDi_SetError("GetDisplayName for IShellItem failed.");            
            shellItem->Release();
            goto end;
        }

        CopyWCharToNFDChar(path, outPath);
        CoTaskMemFree(path);
        if ( !*outPath )
        {
            shellItem->Release();
            goto end;
        }

        nfdResult = NFD_OKAY;
        shellItem->Release();
    }
    else if (result == HRESULT_FROM_WIN32(ERROR_CANCELLED) )
    {
        nfdResult = NFD_CANCEL;
    }
    else
    {
        NFDi_SetError("Show for IFileDialog failed.");
        nfdResult = NFD_ERROR;
    }

 end:

    if (fileDialog)
        fileDialog->Release();

    COMUninit(coResult);

    return nfdResult;
}
