/* win32 - November 16 2021
  A function implemented in Windows 8, but was added to Windows Vista and 7 in a security update in 2013.
  Used by applications to restrict the possible range of directories from which they can load DLLs.
  Windows Vista's implementation has an indirect bug where loading of UXTHEME can be blocked entirely, 
  giving applications such as VLC a completely unthemed appearance, even affecting its common control dialogs. 

  Later implementations move the guts of this function to ntdll, where LdrLoadDll evaluates the BaseDefaultDllDirectories variable instead. */


BOOL WINAPI SetDefaultDllDirectories(DWORD DirectoryFlags)
{
 
  if ( !DirectoryFlags || (DirectoryFlags & 0xFFFFE0FF) != 0 )
  {
    BaseSetLastNTError(E_INVALID_ARGS);
    return FALSE;
  }
  else
  {
    if ( (DirectoryFlags & LOAD_LIBRARY_SEARCH_APPLICATION_DIR) != 0
      && (DirectoryFlags & LOAD_LIBRARY_SEARCH_USER_DIRS) != 0
      && (DirectoryFlags & LOAD_LIBRARY_SEARCH_SYSTEM32) != 0
      && (DirectoryFlags & 0x100) == 0 )  /* This value is undocumented. But upon evaluation of LoadLibraryExW, it seems to be
                                             a secondary method of enforcing the flag LOAD_WITH_ALTERED_SEARCH_PATH if 
                                             it is not set in LoadLibraryExW. */
    {
      DirectoryFlags = LOAD_LIBRARY_SEARCH_DEFAULT_DIRS;
    }
    BaseDefaultDllDirectories = DirectoryFlags; // BaseDefaultDllDirectories is a global variable read by LoadLibraryExW
    return TRUE;                                
  }
}
