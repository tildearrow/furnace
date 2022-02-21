/*  Glue functions for the minIni library, based on the "FAT Filing System"
 *  library by embedded-code.com, now IBEX UK.
 *  https://github.com/ibexuk/C_Memory_SD_Card_FAT_Driver
 *
 *  By CompuPhase, 2008-2012
 *  This "glue file" is in the public domain. It is distributed without
 *  warranties or conditions of any kind, either express or implied.
 *
 *  (The "FAT Filing System" library itself is copyright IBEX UK, and licensed
 *  at its own terms.)
 */

#define INI_BUFFERSIZE  256       /* maximum line length, maximum path length */
#include <mem-ffs.h>

#define INI_FILETYPE                  FFS_FILE*
#define ini_openread(filename,file)   ((*(file) = ffs_fopen((filename),"r")) != NULL)
#define ini_openwrite(filename,file)  ((*(file) = ffs_fopen((filename),"w")) != NULL)
#define ini_close(file)               (ffs_fclose(*(file)) == 0)
#define ini_read(buffer,size,file)    (ffs_fgets((buffer),(size),*(file)) != NULL)
#define ini_write(buffer,file)        (ffs_fputs((buffer),*(file)) >= 0)
#define ini_rename(source,dest)       (ffs_rename((source), (dest)) == 0)
#define ini_remove(filename)          (ffs_remove(filename) == 0)

#define INI_FILEPOS                   long
#define ini_tell(file,pos)            (ffs_fgetpos(*(file), (pos)) == 0)
#define ini_seek(file,pos)            (ffs_fsetpos(*(file), (pos)) == 0)
