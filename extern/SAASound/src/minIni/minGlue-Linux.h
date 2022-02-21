/*  Glue functions for the minIni library, based on the C/C++ stdio library and
 *  using BSD-style file locking to "serialize" concurrent accesses to an INI
 *  file. It presumes GCC compiler extensions.
 *
 *  By CompuPhase, 2020
 *  This "glue file" is in the public domain. It is distributed without
 *  warranties or conditions of any kind, either express or implied.
 */

/* map required file I/O types and functions to the standard C library */
#include <stdio.h>
#include <unistd.h>
#include <sys/file.h>

#define INI_FILETYPE                    FILE*

static inline int ini_openread(const char *filename, INI_FILETYPE *file) {
  if ((*file = fopen((filename),"r")) == NULL)
    return 0;
  return flock(fileno(*file), LOCK_SH) == 0;
}

static inline int ini_openwrite(const char *filename, INI_FILETYPE *file) {
  if ((*file = fopen((filename),"r+")) == NULL
      && (*file = fopen((filename),"w")) == NULL)
    return 0;
  if (flock(fileno(*file), LOCK_EX) < 0)
    return 0;
  return ftruncate(fileno(*file), 0);
}

#define INI_OPENREWRITE
static inline int ini_openrewrite(const char *filename, INI_FILETYPE *file) {
  if ((*file = fopen((filename),"r+")) == NULL)
    return 0;
  return flock(fileno(*file), LOCK_EX) == 0;
}

#define ini_close(file)                 (fclose(*(file)) == 0)
#define ini_read(buffer,size,file)      (fgets((buffer),(size),*(file)) != NULL)
#define ini_write(buffer,file)          (fputs((buffer),*(file)) >= 0)
#define ini_rename(source,dest)         (rename((source), (dest)) == 0)

#define INI_FILEPOS                     long int
#define ini_tell(file,pos)              (*(pos) = ftell(*(file)))
#define ini_seek(file,pos)              (fseek(*(file), *(pos), SEEK_SET) == 0)

/* for floating-point support, define additional types and functions */
#define INI_REAL                        float
#define ini_ftoa(string,value)          sprintf((string),"%f",(value))
#define ini_atof(string)                (INI_REAL)strtod((string),NULL)
