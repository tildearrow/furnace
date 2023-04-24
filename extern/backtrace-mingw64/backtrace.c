/*
        Copyright (c) 2010 ,
                Cloud Wu . All rights reserved.
 
                http://www.codingnow.com
 
        Use, modification and distribution are subject to the "New BSD License"
        as listed at <url: http://www.opensource.org/licenses/bsd-license.php >.
 
   190623 jpmattia: Updated to work for x86_64 under msys2
   230424 tildearrow: Merged with 32-bit backtrace and tied to Furnace

   filename: backtrace64.c

   how to use: Be Luigi and don't touch the controller.


*/

#define TA_64BIT

#define PACKAGE "furnace" /* for libbfd */
#define PACKAGE_VERSION 

#include <windows.h>
#include <excpt.h>
#include <imagehlp.h>
#include <bfd.h>
#include <psapi.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef TA_64BIT
#include <assert.h>
#endif
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>

#include "backtrace.h"

#define BUFFER_MAX (16*1024)

#ifndef TA_64BIT
#define BFD_ERR_OK          (0)
#define BFD_ERR_OPEN_FAIL   (1)
#define BFD_ERR_BAD_FORMAT  (2)
#define BFD_ERR_NO_SYMBOLS  (3)
#define BFD_ERR_READ_SYMBOL (4)

static const char *const bfd_errors[] = {
  "",
  "(Failed to open bfd)",
  "(Bad format)",
  "(No symbols)",
  "(Failed to read symbols)",
};
#endif

struct bfd_ctx {
  bfd *handle;
  asymbol **symbol;
};

#ifndef TA_64BIT
struct bfd_set {
  char *name;
  struct bfd_ctx *bc;
  struct bfd_set *next;
+};
#endif

struct find_info {
  asymbol **symbol;
  bfd_vma counter;
  const char *file;
  const char *func;
  unsigned line;
};

struct output_buffer {
  char *buf;
  size_t sz;
  size_t ptr;
};

static void output_init(struct output_buffer *ob, char *buf, size_t sz) {
  ob->buf = buf;
  ob->sz = sz;
  ob->ptr = 0;
  ob->buf[0] = '\0';
}

static void output_print(struct output_buffer *ob, const char *format, ...) {
  if (ob->sz == ob->ptr)
    return;
  ob->buf[ob->ptr] = '\0';
  va_list ap;
  va_start(ap, format);
  vsnprintf(ob->buf + ob->ptr, ob->sz - ob->ptr, format, ap);
  va_end(ap);

  ob->ptr = strlen(ob->buf + ob->ptr) + ob->ptr;
}

static void lookup_section(bfd * abfd, asection * sec, void *opaque_data) {
#ifdef TA_64BIT
  assert(sec);
  assert(opaque_data);
#endif
  struct find_info *data = opaque_data;

  if (data->func)
    return;

  if (!(bfd_get_section_flags(abfd, sec) & SEC_ALLOC))
    return;

  bfd_vma vma = bfd_get_section_vma(abfd, sec);
  if (data->counter < vma
      || vma + bfd_get_section_size(sec) <= data->counter)
    return;

  bfd_find_nearest_line(abfd, sec, data->symbol, data->counter - vma,
                        &(data->file), &(data->func), &(data->line));
}

static void
find(struct bfd_ctx *b, DWORD offset, const char **file, const char **func,
     unsigned *line) {
  struct find_info data;
  data.func = NULL;
  data.symbol = b->symbol;
  data.counter = offset;
  data.file = NULL;
  data.func = NULL;
  data.line = 0;

  bfd_map_over_sections(b->handle, &lookup_section, &data);
  if (file) {
    *file = data.file;
  }
  if (func) {
    *func = data.func;
  }
  if (line) {
    *line = data.line;
  }
}

#ifdef TA_64BIT
static int init_bfd_ctx(struct bfd_ctx *bc, struct output_buffer *ob) {
#else
static int init_bfd_ctx(struct bfd_ctx *bc, const char *procname, int *err) {
#endif
  bc->handle = NULL;
  bc->symbol = NULL;

#ifdef TA_64BIT
  char procname[MAX_PATH];
  GetModuleFileNameA(NULL, procname, sizeof procname);

  bfd_init();
#endif
  bfd *b = bfd_openr(procname, 0);
  if (!b) {
#ifdef TA_64BIT
    output_print(ob, "Failed to init bfd\n");
#else
    if (err) {
      *err = BFD_ERR_OPEN_FAIL;
    }
#endif
    return 1;
  }

#ifdef TA_64BIT
  int r1 = bfd_check_format(b, bfd_object);
  int r2 = bfd_check_format_matches(b, bfd_object, NULL);
  int r3 = bfd_get_file_flags(b) & HAS_SYMS;
#else
  if (!bfd_check_format(b, bfd_object)) {
    bfd_close(b);
    if (err) {
      *err = BFD_ERR_BAD_FORMAT;
    }
    return 1;
  }
#endif

#ifdef TA_64BIT
  if (!(r1 && r2 && r3)) {
#else
  if (!(bfd_get_file_flags(b) & HAS_SYMS)) {
#endif
    bfd_close(b);
#ifdef TA_64BIT
    output_print(ob, "Failed to init bfd\n");
#else
    if (err) {
      *err = BFD_ERR_NO_SYMBOLS;
    }
#endif
    return 1;
  }

  void *symbol_table;

  unsigned dummy = 0;
  if (bfd_read_minisymbols(b, FALSE, &symbol_table, &dummy) == 0) {
    if (bfd_read_minisymbols(b, TRUE, &symbol_table, &dummy) < 0) {
      free(symbol_table);
      bfd_close(b);
#ifdef TA_64BIT
      output_print(ob, "Failed to init bfd\n");
#else
      if (err) {
        *err = BFD_ERR_READ_SYMBOL;
      }
#endif
      return 1;
    }
  }

  bc->handle = b;
  bc->symbol = symbol_table;

#ifndef TA_64BIT
  if (err) {
    *err = BFD_ERR_OK;
  }
#endif
  return 0;
}

static void close_bfd_ctx(struct bfd_ctx *bc) {
#ifdef TA_64BIT
  if (bc->symbol) {
    free(bc->symbol);
  }
  if (bc->handle) {
    bfd_close(bc->handle);
  }
#else
  if (bc) {
    if (bc->symbol) {
      free(bc->symbol);
    }
    if (bc->handle) {
      bfd_close(bc->handle);
    }
  }
#endif
}

#ifndef TA_64BIT
static struct bfd_ctx *get_bc(struct bfd_set *set, const char *procname,
                              int *err) {
  while (set->name) {
    if (strcmp(set->name, procname) == 0) {
      return set->bc;
    }
    set = set->next;
  }
  struct bfd_ctx bc;
  if (init_bfd_ctx(&bc, procname, err)) {
    return NULL;
  }
  set->next = calloc(1, sizeof(*set));
  set->bc = malloc(sizeof(struct bfd_ctx));
  memcpy(set->bc, &bc, sizeof(bc));
  set->name = strdup(procname);

  return set->bc;
}

static void release_set(struct bfd_set *set) {
  while (set) {
    struct bfd_set *temp = set->next;
    free(set->name);
    close_bfd_ctx(set->bc);
    free(set);
    set = temp;
  }
}
#endif

#ifdef TA_64BIT
static void
_backtrace(struct output_buffer *ob, struct bfd_ctx *bc, int depth,
           LPCONTEXT context) {
#else
static void
_backtrace(struct output_buffer *ob, struct bfd_set *set, int depth,
           LPCONTEXT context) {
#endif
#ifdef TA_64BIT
  if (init_bfd_ctx(bc, ob))
    return;
#else
  char procname[MAX_PATH];
  GetModuleFileNameA(NULL, procname, sizeof procname);

  struct bfd_ctx *bc = NULL;
  int err = BFD_ERR_OK;
#endif

  STACKFRAME frame;
  memset(&frame, 0, sizeof(frame));

#ifdef TA_64BIT
  frame.AddrPC.Offset = context->Rip;
  frame.AddrPC.Mode = AddrModeFlat;
  frame.AddrStack.Offset = context->Rsp;
  frame.AddrStack.Mode = AddrModeFlat;
  frame.AddrFrame.Offset = context->Rbp;
  frame.AddrFrame.Mode = AddrModeFlat;
#else
  frame.AddrPC.Offset = context->Eip;
  frame.AddrPC.Mode = AddrModeFlat;
  frame.AddrStack.Offset = context->Esp;
  frame.AddrStack.Mode = AddrModeFlat;
  frame.AddrFrame.Offset = context->Ebp;
  frame.AddrFrame.Mode = AddrModeFlat;
#endif

  HANDLE process = GetCurrentProcess();
  HANDLE thread = GetCurrentThread();

  char symbol_buffer[sizeof(IMAGEHLP_SYMBOL) + 255];
  char module_name_raw[MAX_PATH];

#ifdef TA_64BIT
  while (StackWalk(IMAGE_FILE_MACHINE_AMD64, /* walk the stack for the x86_64 architecture */
#else
  while (StackWalk(IMAGE_FILE_MACHINE_I386,
#endif
                   process,
                   thread,
                   &frame,
                   context,
                   0, SymFunctionTableAccess, SymGetModuleBase, 0)) {

    --depth;
    if (depth < 0)
      break;

    IMAGEHLP_SYMBOL *symbol = (IMAGEHLP_SYMBOL *) symbol_buffer;
    symbol->SizeOfStruct = (sizeof *symbol) + 255;
    symbol->MaxNameLength = 254;


#ifdef TA_64BIT
    /* SymGetModuleBase returns a DWORD (which is 32 bits).
       We need module_base for GetModuleFileNameA, which requires a 64 bit HINSTANCE, so 
       do the conversion here to avoid a compiler warning.
     */
    HINSTANCE module_base =
        (HINSTANCE) SymGetModuleBase(process, frame.AddrPC.Offset);
#else
    DWORD module_base = SymGetModuleBase(process, frame.AddrPC.Offset);
#endif

    const char *module_name = "[unknown module]";
#ifdef TA_64BIT
    if (module_base
        && GetModuleFileNameA(module_base, module_name_raw, MAX_PATH)) {
      module_name = module_name_raw;
    }
#else
    if (module_base &&
        GetModuleFileNameA((HINSTANCE) module_base, module_name_raw,
                           MAX_PATH)) {
      module_name = module_name_raw;
      bc = get_bc(set, module_name, &err);
    }
#endif

#ifdef TA_64BIT
    const char *file;
    const char *func;
    unsigned line;
#else
    const char *file = NULL;
    const char *func = NULL;
    unsigned line = 0;
#endif

#ifdef TA_64BIT
    find(bc, frame.AddrPC.Offset, &file, &func, &line);
#else
    if (bc) {
      find(bc, frame.AddrPC.Offset, &file, &func, &line);
    }
#endif

    if (file == NULL) {
#ifdef TA_64BIT
      DWORD64 dummy = 0;
#else
      DWORD dummy = 0;
#endif
      if (SymGetSymFromAddr(process, frame.AddrPC.Offset, &dummy, symbol)) {
        file = symbol->Name;
      } else {
        file = "[unknown file]";
      }
    }
#ifdef TA_64BIT
    if (func == NULL) {
      func = "[unknown func]";
    }

    output_print(ob, "0x%x : %s : %s (%d) : in function (%s) \n",
                 frame.AddrPC.Offset, module_name, file, line, func);
#else
    if (func == NULL) {
      output_print(ob, "0x%08x : %s : %s %s \n",
                   frame.AddrPC.Offset,
                   module_name, file, bfd_errors[err]);
    } else {
      output_print(ob, "0x%08x : %s : %s (%d) : in function (%s) \n",
                   frame.AddrPC.Offset, module_name, file, line, func);
    }

#endif
  }
}

static char *g_output = NULL;
static LPTOP_LEVEL_EXCEPTION_FILTER g_prev = NULL;

static LONG WINAPI exception_filter(LPEXCEPTION_POINTERS info) {
  struct output_buffer ob;
  output_init(&ob, g_output, BUFFER_MAX);

  if (!SymInitialize(GetCurrentProcess(), 0, TRUE)) {
    output_print(&ob, "Failed to init symbol context\n");
  } else {
#ifdef TA_64BIT
    struct bfd_ctx bc;
    _backtrace(&ob, &bc, 128, info->ContextRecord);
    close_bfd_ctx(&bc);
#else
    bfd_init();
    struct bfd_set *set = calloc(1, sizeof(*set));
    _backtrace(&ob, set, 128, info->ContextRecord);
    release_set(set);
#endif

    SymCleanup(GetCurrentProcess());
  }

  fputs(g_output, stderr);

#ifdef TA_64BIT
  // huh?
  exit(1);

  return 0;
#else
  return EXCEPTION_CONTINUE_SEARCH;
#endif
}

static void backtrace_register(void) {
  if (g_output == NULL) {
    g_output = malloc(BUFFER_MAX);
    g_prev = SetUnhandledExceptionFilter(exception_filter);
  }
}

static void backtrace_unregister(void) {
  if (g_output) {
    free(g_output);
    SetUnhandledExceptionFilter(g_prev);
    g_prev = NULL;
    g_output = NULL;
  }
}
