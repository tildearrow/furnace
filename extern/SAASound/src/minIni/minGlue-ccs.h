/*  minIni glue functions for FAT library by CCS, Inc. (as provided with their
 *  PIC MCU compiler)
 *
 *  By CompuPhase, 2011-2012
 *  This "glue file" is in the public domain. It is distributed without
 *  warranties or conditions of any kind, either express or implied.
 *
 *  (The FAT library is copyright (c) 2007 Custom Computer Services, and
 *  licensed at its own terms.)
 */

#define INI_BUFFERSIZE  256       /* maximum line length, maximum path length */

#ifndef FAT_PIC_C
  #error FAT library must be included before this module
#endif
#define const                     /* keyword not supported by CCS */

#define INI_FILETYPE                  FILE
#define ini_openread(filename,file)   (fatopen((filename), "r", (file)) == GOODEC)
#define ini_openwrite(filename,file)  (fatopen((filename), "w", (file)) == GOODEC)
#define ini_close(file)               (fatclose((file)) == 0)
#define ini_read(buffer,size,file)    (fatgets((buffer), (size), (file)) != NULL)
#define ini_write(buffer,file)        (fatputs((buffer), (file)) == GOODEC)
#define ini_remove(filename)          (rm_file((filename)) == 0)

#define INI_FILEPOS                   fatpos_t
#define ini_tell(file,pos)            (fatgetpos((file), (pos)) == 0)
#define ini_seek(file,pos)            (fatsetpos((file), (pos)) == 0)

#ifndef INI_READONLY
/* CCS FAT library lacks a rename function, so instead we copy the file to the
 * new name and delete the old file
 */
static int ini_rename(char *source, char *dest)
{
  FILE fr, fw;
  int n;

  if (fatopen(source, "r", &fr) != GOODEC)
    return 0;
  if (rm_file(dest) != 0)
    return 0;
  if (fatopen(dest, "w", &fw) != GOODEC)
    return 0;

  /* With some "insider knowledge", we can save some memory: the "source"
   * parameter holds a filename that was built from the "dest" parameter. It
   * was built in a local buffer with the size INI_BUFFERSIZE. We can reuse
   * this buffer for copying the file.
   */
  while (n=fatread(source, 1, INI_BUFFERSIZE, &fr))
    fatwrite(source, 1, n, &fw);

  fatclose(&fr);
  fatclose(&fw);

  /* Now we need to delete the source file. However, we have garbled the buffer
   * that held the filename of the source. So we need to build it again.
   */
  ini_tempname(source, dest, INI_BUFFERSIZE);
  return rm_file(source) == 0;
}
#endif
