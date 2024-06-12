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

#include "halfsiphash.c"

#ifdef ANDROID
#include <SDL_rwops.h>
#define MO_FREE SDL_free
#else
#define MO_FREE free
#endif

static char curLocale[64];
static char curLocaleNoCountry[64];
static char tempPath[4096];

enum StackInstruction {
  MOMO_STACK_END=0,
  MOMO_STACK_PUSH,
  MOMO_STACK_PUSH_N,
  MOMO_STACK_ADD,
  MOMO_STACK_SUB,
  MOMO_STACK_MUL,
  MOMO_STACK_DIV,
  MOMO_STACK_MOD,
  MOMO_STACK_CMP_EQ,
  MOMO_STACK_CMP_NE,
  MOMO_STACK_CMP_GT,
  MOMO_STACK_CMP_LT,
  MOMO_STACK_CMP_GE,
  MOMO_STACK_CMP_LE,
  MOMO_STACK_CMP_AND,
  MOMO_STACK_CMP_OR,
  MOMO_STACK_BEQ,
  MOMO_STACK_BNE,
  MOMO_STACK_EXIT,
};

static const char* stackInsNames[]={
  "end", "push", "push n", "add", "sub", "mul", "div", "mod", "cmp eq", "cmp ne",
  "cmp gt", "cmp lt", "cmp ge", "cmp le", "cmp and", "cmp or", "beq", "bne", "exit"
};
static unsigned char stackTakesArg[]={
  0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0
};

struct StackData {
  unsigned char ins;
  unsigned int param;
};

struct LocaleDomain {
  char path[4096];
  char name[64];
  unsigned char* mo;
  size_t moLen;
  const char** stringPtr;
  const char** transPtr;
  unsigned int* hashes;
  size_t stringCount;
  size_t firstString[256];
  size_t lastString[256];
  struct StackData pluralProgram[256];
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

unsigned int runStackMachine(struct StackData* data, size_t count, unsigned int n) {
  size_t pc=0;
  unsigned int stack[256];
  unsigned char sp=0xff;

  memset(stack,0,256*sizeof(unsigned int));

  while (pc<count) {
    unsigned int param=data[pc].param;
    unsigned int op1, op2;

    switch (data[pc].ins) {
      case MOMO_STACK_END:
      case MOMO_STACK_EXIT:
        return stack[sp];
        break;
      case MOMO_STACK_PUSH:
        stack[++sp]=param;
        break;
      case MOMO_STACK_PUSH_N:
        stack[++sp]=n;
        break;
      case MOMO_STACK_ADD:
        op2=stack[sp--];
        op1=stack[sp--];
        stack[++sp]=op1+op2;
        break;
      case MOMO_STACK_SUB:
        op2=stack[sp--];
        op1=stack[sp--];
        stack[++sp]=op1-op2;
        break;
      case MOMO_STACK_MUL:
        op2=stack[sp--];
        op1=stack[sp--];
        stack[++sp]=op1*op2;
        break;
      case MOMO_STACK_DIV:
        op2=stack[sp--];
        op1=stack[sp--];
        if (op2==0) return 0;
        stack[++sp]=op1/op2;
        break;
      case MOMO_STACK_MOD:
        op2=stack[sp--];
        op1=stack[sp--];
        if (op2==0) return 0;
        stack[++sp]=op1%op2;
        break;
      case MOMO_STACK_CMP_EQ:
        op2=stack[sp--];
        op1=stack[sp--];
        stack[++sp]=(op1==op2)?1:0;
        break;
      case MOMO_STACK_CMP_NE:
        op2=stack[sp--];
        op1=stack[sp--];
        stack[++sp]=(op1!=op2)?1:0;
        break;
      case MOMO_STACK_CMP_GT:
        op2=stack[sp--];
        op1=stack[sp--];
        stack[++sp]=(op1>op2)?1:0;
        break;
      case MOMO_STACK_CMP_LT:
        op2=stack[sp--];
        op1=stack[sp--];
        stack[++sp]=(op1<op2)?1:0;
        break;
      case MOMO_STACK_CMP_GE:
        op2=stack[sp--];
        op1=stack[sp--];
        stack[++sp]=(op1>=op2)?1:0;
        break;
      case MOMO_STACK_CMP_LE:
        op2=stack[sp--];
        op1=stack[sp--];
        stack[++sp]=(op1<=op2)?1:0;
        break;
      case MOMO_STACK_CMP_AND:
        op2=stack[sp--];
        op1=stack[sp--];
        stack[++sp]=(op1 && op2)?1:0;
        break;
      case MOMO_STACK_CMP_OR:
        op2=stack[sp--];
        op1=stack[sp--];
        stack[++sp]=(op1 || op2)?1:0;
        break;
      case MOMO_STACK_BEQ:
        op1=stack[sp--];
        if (op1) pc+=(int)param;
        break;
      case MOMO_STACK_BNE:
        op1=stack[sp--];
        if (!op1) pc+=(int)param;
        break;
      default:
        return 0;
    }
    pc++;
  }

  return stack[sp];
}

#define FINISH_OP \
  /* push identifier */ \
  if (state[curState].curIdent[0]) { \
    if (strcmp(state[curState].curIdent,"n")==0) { \
      data[*pc].ins=MOMO_STACK_PUSH_N; \
      data[*pc].param=0; \
      printf("PENDING IDENT: %s\n",state[curState].curIdent); \
    } else { \
      data[*pc].ins=MOMO_STACK_PUSH; \
      printf("PENDING IDENT: %s\n",state[curState].curIdent); \
      if (sscanf(state[curState].curIdent,"%u",&data[*pc].param)!=1) { \
        printf("ERROR: invalid identifier %s\n",state[curState].curIdent); \
        return 3; \
      } \
    } \
    (*pc)++; \
  } \
 \
  /* push operation if pending */ \
  if (state[curState].curOp[0]) { \
    unsigned char isCompare=0; \
    printf("PENDING OP: %s\n",state[curState].curOp); \
    if (strcmp(state[curState].curOp,"+")==0) { \
      data[*pc].ins=MOMO_STACK_ADD; \
      data[*pc].param=0; \
      (*pc)++; \
    } else if (strcmp(state[curState].curOp,"-")==0) { \
      data[*pc].ins=MOMO_STACK_SUB; \
      data[*pc].param=0; \
      (*pc)++; \
    } else if (strcmp(state[curState].curOp,"*")==0) { \
      data[*pc].ins=MOMO_STACK_MUL; \
      data[*pc].param=0; \
      (*pc)++; \
    } else if (strcmp(state[curState].curOp,"/")==0) { \
      data[*pc].ins=MOMO_STACK_DIV; \
      data[*pc].param=0; \
      (*pc)++; \
    } else if (strcmp(state[curState].curOp,"%")==0) { \
      data[*pc].ins=MOMO_STACK_MOD; \
      data[*pc].param=0; \
      (*pc)++; \
    } else if (strcmp(state[curState].curOp,"&&")==0) { \
      /* handled later */ \
    } else if (strcmp(state[curState].curOp,"||")==0) { \
      /* handled later */ \
    } else if (strcmp(state[curState].curOp,">")==0) { \
      data[*pc].ins=MOMO_STACK_CMP_GT; \
      data[*pc].param=0; \
      (*pc)++; \
      isCompare=1; \
    } else if (strcmp(state[curState].curOp,"<")==0) { \
      data[*pc].ins=MOMO_STACK_CMP_LT; \
      data[*pc].param=0; \
      (*pc)++; \
      isCompare=1; \
    } else if (strcmp(state[curState].curOp,">=")==0) { \
      data[*pc].ins=MOMO_STACK_CMP_GE; \
      data[*pc].param=0; \
      (*pc)++; \
      isCompare=1; \
    } else if (strcmp(state[curState].curOp,"<=")==0) { \
      data[*pc].ins=MOMO_STACK_CMP_LE; \
      data[*pc].param=0; \
      (*pc)++; \
      isCompare=1; \
    } else if (strcmp(state[curState].curOp,"==")==0) { \
      data[*pc].ins=MOMO_STACK_CMP_EQ; \
      data[*pc].param=0; \
      (*pc)++; \
      isCompare=1; \
    } else if (strcmp(state[curState].curOp,"!=")==0) { \
      data[*pc].ins=MOMO_STACK_CMP_NE; \
      data[*pc].param=0; \
      (*pc)++; \
      isCompare=1; \
    } else { \
      printf("ERROR: invalid operation\n"); \
      return 4; \
    } \
    if ((state[curState].curBigOp[0] && isCompare) || state[curState].endExpr) { \
      printf("PENDING BIG OP... %s\n",state[curState].curBigOp); \
      if (strcmp(state[curState].curBigOp,"&&")==0) { \
        data[*pc].ins=MOMO_STACK_CMP_AND; \
        data[*pc].param=0; \
        (*pc)++; \
      } else if (strcmp(state[curState].curBigOp,"||")==0) { \
        data[*pc].ins=MOMO_STACK_CMP_OR; \
        data[*pc].param=0; \
        (*pc)++; \
      } \
      memset(state[curState].curBigOp,0,8); \
    } \
    if (strcmp(state[curState].curOp,"&&")==0 || strcmp(state[curState].curOp,"||")==0) { \
      printf("PREPARING BIG OP...\n"); \
      strncpy(state[curState].curBigOp,state[curState].curOp,8); \
    } \
  } \
 \
  memset(state[curState].curOp,0,8); \
  memset(state[curState].curIdent,0,32); \
  state[curState].curOpLen=0; \
  state[curState].curIdentLen=0;

// errors:
// 0: success
// 1: identifier too long
// 2: operator too long
// 3: unknown identifier
// 4: unknown operation
// 5: program too long
unsigned char compileExprSub(const char** ptr, struct StackData* data, size_t* pc, size_t count) {
  struct ExprState {
    unsigned char isOp;
    unsigned char oldIsOp;
    char curIdent[32];
    char curOp[8];
    char curBigOp[8];
    unsigned char curIdentLen;
    unsigned char curOpLen;
    unsigned char startBranch;
    unsigned char endExpr;
    size_t pendingBranch;
  } state[8];
  unsigned char curState=0;

  memset(state,0,sizeof(struct ExprState)*8);

  for (; *(*ptr); (*ptr)++) {
    char next=**ptr;
    unsigned char doNotPush=0;
    printf("%c\n",next);
    if (state[curState].curIdentLen>30) return 1;
    if (state[curState].curOpLen>6) return 2;
    if (*pc>=count) return 5;
    switch (next) {
      case '+':
        state[curState].isOp=1;
        break;
      case '-':
        state[curState].isOp=1;
        break;
      case '*':
        state[curState].isOp=1;
        break;
      case '/':
        state[curState].isOp=1;
        break;
      case '%':
        state[curState].isOp=1;
        break;
      case '!':
        state[curState].isOp=1;
        break;
      case '=':
        state[curState].isOp=1;
        break;
      case '|':
        state[curState].isOp=1;
        break;
      case '&':
        state[curState].isOp=1;
        break;
      case '>':
        state[curState].isOp=1;
        break;
      case '<':
        state[curState].isOp=1;
        break;
      case '?':
        // special case
        state[curState].startBranch=1;
        doNotPush=1;
        state[curState].isOp=1;
        FINISH_OP;
        if (state[curState].curBigOp[0]) {
          printf("PENDING BIG OP... %s\n",state[curState].curBigOp);
          if (strcmp(state[curState].curBigOp,"&&")==0) {
            data[*pc].ins=MOMO_STACK_CMP_AND;
            data[*pc].param=0;
            (*pc)++;
          } else if (strcmp(state[curState].curBigOp,"||")==0) {
            data[*pc].ins=MOMO_STACK_CMP_OR;
            data[*pc].param=0;
            (*pc)++;
          }
          memset(state[curState].curBigOp,0,8);
        }
        break;
      case ':':
        // special case
        doNotPush=1;
        state[curState].startBranch=2;
        state[curState].isOp=1;
        break;
      case '(':
        // start a new state
        printf("PUSH STATE.\n");
        state[curState].isOp=0;
        curState++;
        memset(&state[curState],0,sizeof(struct ExprState));
        continue;
        break;
      case ')':
        // pop last state
        printf("POP STATE.\n");
        FINISH_OP;
        curState--;
        if (state[curState].curBigOp[0]) {
          printf("PENDING BIG OP... %s\n",state[curState].curBigOp);
          if (strcmp(state[curState].curBigOp,"&&")==0) {
            data[*pc].ins=MOMO_STACK_CMP_AND;
            data[*pc].param=0;
            (*pc)++;
          } else if (strcmp(state[curState].curBigOp,"||")==0) {
            data[*pc].ins=MOMO_STACK_CMP_OR;
            data[*pc].param=0;
            (*pc)++;
          }
          memset(state[curState].curBigOp,0,8);
        }
        continue;
        break;
      case ' ':
        // ignore
        doNotPush=1;
        break;
      default:
        state[curState].isOp=0;
        break;
    }
    if (state[curState].isOp!=state[curState].oldIsOp) {
      state[curState].oldIsOp=state[curState].isOp;
      if (state[curState].isOp) {
        printf("OP MODE\n");
        FINISH_OP;
      } else {
        printf("IDENT MODE\n");
        // prepare
        memset(state[curState].curIdent,0,32);
        state[curState].curIdentLen=0;
      }
    }
    if (!doNotPush) {
      if (state[curState].isOp) {
        state[curState].curOp[state[curState].curOpLen++]=next;
      } else {
        state[curState].curIdent[state[curState].curIdentLen++]=next;
      }
    }
    if (state[curState].startBranch) {
      printf("START BRANCH: %d\n",state[curState].startBranch);
      if (state[curState].startBranch==2) {
        data[*pc].ins=MOMO_STACK_EXIT;
        data[*pc].param=0;
        data[state[curState].pendingBranch].param=*pc-state[curState].pendingBranch;
        state[curState].pendingBranch=0;
      } else {
        state[curState].pendingBranch=*pc;
        data[*pc].ins=MOMO_STACK_BNE;
        data[*pc].param=0;
      }
      state[curState].startBranch=0;
      (*pc)++;
    }
  }

  if (!state[curState].isOp) {
    FINISH_OP;
  }

  return 0;
}

unsigned char compileExpr(const char* expr, struct StackData* data, size_t count) {
  //plural=(n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2);

  const char* ptr=expr;
  size_t pc=0;

  printf("compiling expression... %s\n",expr);

  return compileExprSub(&ptr,data,&pc,count);
}

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
  // make no country version of locale
  strncpy(curLocaleNoCountry,curLocale,64);
  char* cPos=strchr(curLocaleNoCountry,'_');
  if (cPos) {
    *cPos=0;
  }
  char* cPos1=strchr(curLocaleNoCountry,'-');
  if (cPos1) {
    *cPos1=0;
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
      snprintf(tempPath,4096,"%s/%s/LC_MESSAGES/%s.mo",newDomain->path,curLocaleNoCountry,newDomain->name);
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
      snprintf(tempPath,4096,"%s/%s/LC_MESSAGES/%s.mo",newDomain->path,curLocaleNoCountry,newDomain->name);
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
      newDomain->hashes=malloc(newDomain->stringCount*sizeof(unsigned int));
    }

    unsigned int* strTable=(unsigned int*)(&newDomain->mo[header->stringPtr]);
    unsigned int* transTable=(unsigned int*)(&newDomain->mo[header->transPtr]);

    unsigned short curChar=0;
    for (size_t i=0; i<newDomain->stringCount; i++) {
      newDomain->stringPtr[i]=(const char*)(&newDomain->mo[strTable[1+(i<<1)]]);
      newDomain->transPtr[i]=(const char*)(&newDomain->mo[transTable[1+(i<<1)]]);

      newDomain->hashes[i]=halfsiphash(newDomain->stringPtr[i],strlen(newDomain->stringPtr[i]),0);

      while (curChar<=(unsigned char)newDomain->stringPtr[i][0]) {
        newDomain->firstString[curChar]=i;
        if (curChar>0) {
          newDomain->lastString[curChar-1]=i;
        }
        curChar++;
      }
    }
    while (curChar<256) {
      newDomain->firstString[curChar]=newDomain->stringCount;
      if (curChar>0) {
        newDomain->lastString[curChar-1]=newDomain->stringCount;
      }
      curChar++;
    }
    newDomain->lastString[255]=newDomain->stringCount;

    // compile plural program
    char pluralProgram[4096];
    const char* pluralProgramLoc=strstr(newDomain->transPtr[0],"plural=");
    if (pluralProgramLoc!=NULL) {
      pluralProgramLoc+=7;
      const char* pluralProgramLocEnd=strstr(pluralProgramLoc,";");

      if (pluralProgramLocEnd!=NULL) {
        memset(pluralProgram,0,4096);
        for (size_t i=0; i<4096 && pluralProgramLoc<pluralProgramLocEnd; pluralProgramLoc++) {
          pluralProgram[i++]=*pluralProgramLoc;
        }

        unsigned char exprRet=compileExpr(pluralProgram,newDomain->pluralProgram,256);
        if (exprRet==0) {
          // dump program
          printf("compiled program:\n");
          for (int i=0; i<256; i++) {
            if (stackTakesArg[newDomain->pluralProgram[i].ins]) {
              printf("%s %u\n",stackInsNames[newDomain->pluralProgram[i].ins],newDomain->pluralProgram[i].param);
            } else {
              printf("%s\n",stackInsNames[newDomain->pluralProgram[i].ins]);
            }
            if (newDomain->pluralProgram[i].ins==0) break;
          }
        } else {
          printf("error %d\n",exprRet);
        }
      }
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
  unsigned int hash=halfsiphash(str,strlen(str),0);
  for (size_t i=curDomain->firstString[(unsigned char)(str[0])]; i<curDomain->lastString[(unsigned char)(str[0])]; i++) {
    if (hash==curDomain->hashes[i]) {
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
  // gettext("") and take plural form metadata...
  // then I don't know how are plural strings stored
  unsigned int plural=runStackMachine(curDomain->pluralProgram,256,amount);
  // TODO: optimize
  unsigned int hash=halfsiphash(str1,strlen(str1),0);
  for (size_t i=curDomain->firstString[(unsigned char)(str1[0])]; i<curDomain->lastString[(unsigned char)(str1[0])]; i++) {
    if (hash==curDomain->hashes[i]) {
      const char* ret=curDomain->transPtr[i];
      for (unsigned int j=0; j<plural; j++) {
        ret+=strlen(ret)+1;
      }
      return ret;
    }
  }

  if (amount==1) return str1;
  return str2;
}
