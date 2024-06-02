/* Momo - portable gettext() implementation
 * Copyright (C) 2024 tildearrow 
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "momo.h"

#ifdef ANDROID
#include <SDL_rwops.h>
#define MO_FREE SDL_free
#else
#define MO_FREE free
#endif

static char curLocale[64];
static char tempPath[4096];

struct LocaleDomain {
  char path[4096];
  char name[64];
  unsigned char* mo;
  size_t moLen;
  const char** stringPtr;
  const char** transPtr;
  size_t stringCount;
  size_t firstString[256];
};

struct MOHeader {
  unsigned int magic;
  unsigned int version;
  unsigned int stringCount;
  unsigned int stringPtr;
  unsigned int transPtr;
  unsigned int hashSize;
  unsigned int hashPtr;
};

static struct LocaleDomain* curDomain=NULL;
static struct LocaleDomain** domains=NULL;
static size_t domainsLen=0;

// utility

unsigned char domainsInsert(struct LocaleDomain* item) {
  struct LocaleDomain** newDomains=malloc(sizeof(struct LocaleDomain*)*(domainsLen+1));
  if (newDomains==NULL) return 0;
  if (domains!=NULL) {
    memcpy(newDomains,domains,sizeof(struct LocaleDomain*)*domainsLen);
    free(domains);
  }
  domains=newDomains;
  domains[domainsLen++]=item;
  return 1;
}

unsigned char domainsRemove(struct LocaleDomain* item) {
  if (domains==NULL) return 0;
  unsigned char found=0;
  for (size_t i=0; i<domainsLen; i++) {
    if (domains[i]==item) {
      found=1;
      break;
    }
  }
  if (!found) return 0;
  if (domainsLen==1) {
    domainsLen=0;
    free(domains);
    domains=NULL;
    return 1;
  } 
  struct LocaleDomain** newDomains=malloc(sizeof(struct LocaleDomain*)*(domainsLen-1));
  if (newDomains==NULL) return 0;
  size_t d=0;
  found=0;
  for (size_t i=0; i<domainsLen; i++) {
    if (domains[i]!=item || found) {
      newDomains[d++]=domains[i];
    } else {
      found=1;
    }
  }
  domainsLen--;
  free(domains);
  domains=newDomains;
  return 1;
}

// implementation

const char* momo_setlocale(int type, const char* locale) {
  if (locale==NULL) {
    return curLocale;
  }

  if (locale[0]==0) {
    // get the locale from environment
    locale=getenv("LC_ALL");
    if (locale==NULL) {
      locale=getenv("LC_MESSAGES");
      if (locale==NULL) {
        locale=getenv("LANG");
        if (locale==NULL) {
          locale="C";
        }
      }
    }
  }
  strncpy(curLocale,locale,64);
  // cut anything after the dot (we only support UTF-8)
  char* dotPos=strchr(curLocale,'.');
  if (dotPos) {
    *dotPos=0;
  }
  return curLocale;
}

const char* momo_bindtextdomain(const char* domainName, const char* dirName) {
  if (strcmp(curLocale,"C")==0) return dirName;
  if (strcmp(curLocale,"POSIX")==0) return dirName;
  if (strcmp(curLocale,"en")==0) return dirName;
  if (strcmp(curLocale,"en_US")==0) return dirName;

  struct LocaleDomain* newDomain=NULL;
  unsigned char found=0;
  if (domains!=NULL) {
    // search for domain
    for (size_t i=0; i<domainsLen; i++) {
      if (strcmp(domains[i]->name,domainName)==0) {
        newDomain=domains[i];
        found=1;
        break;
      }
    }
  }
  if (newDomain==NULL) {
    // create new domain
    newDomain=malloc(sizeof(struct LocaleDomain));
    if (newDomain==NULL) {
      errno=ENOMEM;
      return NULL;
    }
    memset(newDomain,0,sizeof(struct LocaleDomain));
  }
  strncpy(newDomain->name,domainName,64);
  if (dirName==NULL) {
    if (!found) {
      free(newDomain);
      return NULL;
    }
    return newDomain->path;
  } else {
    strncpy(newDomain->path,dirName,4096);
  }

  // load domain
  if (newDomain->mo==NULL) {
    snprintf(tempPath,4096,"%s/%s/LC_MESSAGES/%s.mo",newDomain->path,curLocale,newDomain->name);

#ifdef ANDROID
   newDomain->mo=SDL_LoadFile(tempPath,&newDomain->moLen);
   if (newDomain->mo==NULL) {
      // try without country
      char* cPos=strchr(curLocale,'_');
      if (cPos) {
        *cPos=0;
      } 
      snprintf(tempPath,4096,"%s/%s/LC_MESSAGES/%s.mo",newDomain->path,curLocale,newDomain->name);
      newDomain->mo=SDL_LoadFile(tempPath,&newDomain->moLen);
      if (newDomain->mo==NULL) {
        // give up
        if (found) {
          if (newDomain==curDomain) curDomain=NULL;
          domainsRemove(newDomain);
        }
        free(newDomain);
        return NULL;
      }

   }
#else
    FILE* f=fopen(tempPath,"rb");
    if (f==NULL) {
      // try without country
      char* cPos=strchr(curLocale,'_');
      if (cPos) {
        *cPos=0;
      } 
      snprintf(tempPath,4096,"%s/%s/LC_MESSAGES/%s.mo",newDomain->path,curLocale,newDomain->name);
      f=fopen(tempPath,"rb");
      if (f==NULL) {
        // give up
        if (found) {
          if (newDomain==curDomain) curDomain=NULL;
          domainsRemove(newDomain);
        }
        free(newDomain);
        return NULL;
      }
    }

    if (fseek(f,0,SEEK_END)!=0) {
      // give up
      fclose(f);
      if (found) {
        if (newDomain==curDomain) curDomain=NULL;
        domainsRemove(newDomain);
      }
      free(newDomain);
      return NULL;
    }

    long moSize=ftell(f);
    if (moSize<sizeof(struct MOHeader)) {
      // give up
      fclose(f);
      if (found) {
        if (newDomain==curDomain) curDomain=NULL;
        domainsRemove(newDomain);
      }
      free(newDomain);
      return NULL;
    }

    newDomain->moLen=moSize;

    if (fseek(f,0,SEEK_SET)!=0) {
      // give up
      fclose(f);
      if (found) {
        if (newDomain==curDomain) curDomain=NULL;
        domainsRemove(newDomain);
      }
      free(newDomain);
      return NULL;
    }

    // allocate
    newDomain->mo=malloc(newDomain->moLen);
    if (newDomain->mo==NULL) {
      // give up
      fclose(f);
      if (found) {
        if (newDomain==curDomain) curDomain=NULL;
        domainsRemove(newDomain);
      }
      free(newDomain);
      errno=ENOMEM;
      return NULL;
    }
    memset(newDomain->mo,0,newDomain->moLen);

    // read
    if (fread(newDomain->mo,1,newDomain->moLen,f)!=newDomain->moLen) {
      // give up
      free(newDomain->mo);
      fclose(f);
      if (found) {
        if (newDomain==curDomain) curDomain=NULL;
        domainsRemove(newDomain);
      }
      free(newDomain);
      return NULL;
    }
    fclose(f);
#endif

    // parse
    struct MOHeader* header=(struct MOHeader*)newDomain->mo;
    if (header->magic!=0x950412de) {
      // give up
      MO_FREE(newDomain->mo);
      if (found) {
        if (newDomain==curDomain) curDomain=NULL;
        domainsRemove(newDomain);
      }
      free(newDomain);
      return NULL;
    }

    if (header->stringPtr+(header->stringCount*8)>newDomain->moLen ||
        header->transPtr+(header->stringCount*8)>newDomain->moLen ||
        header->hashPtr+(header->hashSize*4)>newDomain->moLen) {
      // give up
      MO_FREE(newDomain->mo);
      if (found) {
        if (newDomain==curDomain) curDomain=NULL;
        domainsRemove(newDomain);
      }
      free(newDomain);
      return NULL;
    }

    newDomain->stringCount=header->stringCount;
    if (newDomain->stringCount) {
      newDomain->stringPtr=malloc(newDomain->stringCount*sizeof(const char*));
      newDomain->transPtr=malloc(newDomain->stringCount*sizeof(const char*));
    }

    unsigned int* strTable=(unsigned int*)(&newDomain->mo[header->stringPtr]);
    unsigned int* transTable=(unsigned int*)(&newDomain->mo[header->transPtr]);

    unsigned short curChar=0;
    for (size_t i=0; i<newDomain->stringCount; i++) {
      newDomain->stringPtr[i]=(const char*)(&newDomain->mo[strTable[1+(i<<1)]]);
      newDomain->transPtr[i]=(const char*)(&newDomain->mo[transTable[1+(i<<1)]]);

      while (curChar<=(unsigned char)newDomain->stringPtr[i][0]) {
        newDomain->firstString[curChar]=i;
        curChar++;
      }
    }
    while (curChar<256) {
      newDomain->firstString[curChar]=newDomain->stringCount;
      curChar++;
    }
  }

  // add to domain list
  if (!found) {
    if (!domainsInsert(newDomain)) {
      if (newDomain->mo) MO_FREE(newDomain->mo);
      free(newDomain);
      errno=ENOMEM;
      return NULL;
    }
  }
  return newDomain->path;
}

const char* momo_textdomain(const char* domainName) {
  if (strcmp(curLocale,"C")==0) return domainName;
  if (strcmp(curLocale,"POSIX")==0) return domainName;
  if (strcmp(curLocale,"en")==0) return domainName;
  if (strcmp(curLocale,"en_US")==0) return domainName;

  if (domainName==NULL) {
    if (curDomain==NULL) return NULL;
    return curDomain->name;
  }
  // set the domain
  if (domains==NULL) return NULL;
  for (size_t i=0; i<domainsLen; i++) {
    if (strcmp(domains[i]->name,domainName)==0) {
      curDomain=domains[i];
      return curDomain->name;
    }
  }
  return NULL;
}

const char* momo_gettext(const char* str) {
  if (curDomain==NULL) {
    return str;
  }
  if (str==NULL) return NULL;
  // TODO: optimize
  for (size_t i=curDomain->firstString[(unsigned char)(str[0])]; i<curDomain->stringCount; i++) {
    if (strcmp(curDomain->stringPtr[i],str)==0) {
      return curDomain->transPtr[i];
    }
  }
  return str;
}

const char* momo_ngettext(const char* str1, const char* str2, unsigned long amount) {
  if (curDomain==NULL) {
    if (amount==1) return str1;
    return str2;
  }
  // TODO: implement
  return str1;
}
