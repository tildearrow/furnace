/*  Glue functions for the minIni library, based on the EFS Library, see
 *  https://sourceforge.net/projects/efsl/
 *
 *  By CompuPhase, 2008-2012
 *  This "glue file" is in the public domain. It is distributed without
 *  warranties or conditions of any kind, either express or implied.
 *
 *  (EFSL is copyright 2005-2006 Lennart Ysboodt and Michael De Nil, and
 *  licensed under the GPL with an exception clause for static linking.)
 */

#define INI_BUFFERSIZE  256       /* maximum line length, maximum path length */
#define INI_LINETERM    "\r\n"    /* set line termination explicitly */

#include "efs.h"
extern EmbeddedFileSystem g_efs;

#define INI_FILETYPE                  EmbeddedFile
#define ini_openread(filename,file)   (file_fopen((file), &g_efs.myFs, (char*)(filename), 'r') == 0)
#define ini_openwrite(filename,file)  (file_fopen((file), &g_efs.myFs, (char*)(filename), 'w') == 0)
#define ini_close(file)               file_fclose(file)
#define ini_read(buffer,size,file)    (file_read((file), (size), (buffer)) > 0)
#define ini_write(buffer,file)        (file_write((file), strlen(buffer), (char*)(buffer)) > 0)
#define ini_remove(filename)          rmfile(&g_efs.myFs, (char*)(filename))

#define INI_FILEPOS                   euint32
#define ini_tell(file,pos)            (*(pos) = (file)->FilePtr))
#define ini_seek(file,pos)            file_setpos((file), (*pos))

#if ! defined INI_READONLY
/* EFSL lacks a rename function, so instead we copy the file to the new name
 * and delete the old file
 */
static int ini_rename(char *source, const char *dest)
{
  EmbeddedFile fr, fw;
  int n;

  if (file_fopen(&fr, &g_efs.myFs, source, 'r') != 0)
    return 0;
  if (rmfile(&g_efs.myFs, (char*)dest) != 0)
    return 0;
  if (file_fopen(&fw, &g_efs.myFs, (char*)dest, 'w') != 0)
    return 0;

  /* With some "insider knowledge", we can save some memory: the "source"
   * parameter holds a filename that was built from the "dest" parameter. It
   * was built in buffer and this buffer has the size INI_BUFFERSIZE. We can
   * reuse this buffer for copying the file.
   */
  while (n=file_read(&fr, INI_BUFFERSIZE, source))
    file_write(&fw, n, source);

  file_fclose(&fr);
  file_fclose(&fw);

  /* Now we need to delete the source file. However, we have garbled the buffer
   * that held the filename of the source. So we need to build it again.
   */
  ini_tempname(source, dest, INI_BUFFERSIZE);
  return rmfile(&g_efs.myFs, source) == 0;
}
#endif
