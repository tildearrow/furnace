/*
	term_win32: Windows-specifc terminal functionality

	This is a very lightweight terminal library, just the minimum to

	- get at the width of the terminal (if there is one)
	- be able to read single keys being pressed for control
	- maybe also switch of echoing of input

	copyright 2008-2022 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Thomas Orgis and Jonathan Yong
*/


#include "config.h"
#include "compat/compat.h"

#include "terms.h"

#define WIN32_LEAN_AND_MEAN 1
#include <io.h>
#include <ctype.h>
#include <windows.h>
#include <wincon.h>

#include "common/debug.h"

static HANDLE consoleintput = INVALID_HANDLE_VALUE;
static HANDLE consoleoutput = INVALID_HANDLE_VALUE;
static HANDLE getconsoleintput(void){
  DWORD mode;
  if(consoleintput == INVALID_HANDLE_VALUE){
    consoleintput = CreateFileW(L"CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if(consoleintput == INVALID_HANDLE_VALUE || consoleintput == NULL)
      return consoleintput;
    GetConsoleMode(consoleintput, &mode);
    mode |= ENABLE_LINE_INPUT|ENABLE_PROCESSED_INPUT|ENABLE_WINDOW_INPUT;
    mode &= ~(ENABLE_ECHO_INPUT|ENABLE_QUICK_EDIT_MODE|ENABLE_MOUSE_INPUT);
    SetConsoleMode(consoleintput, mode);
  }
  return consoleintput;
}

static HANDLE getconsole(void){
  if(consoleoutput == INVALID_HANDLE_VALUE){
    consoleoutput = CreateFileW(L"CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
  }
  return consoleoutput;
}

// No fun for windows until we reorganize control character stuff.
int term_have_fun(int fd, int want_visuals)
{
        return 0;
}

int term_setup(void)
{
  return 0;
}

void term_restore(void){
  CloseHandle(consoleintput);
  CloseHandle(consoleoutput);
  consoleintput = INVALID_HANDLE_VALUE;
  consoleoutput = INVALID_HANDLE_VALUE;
}

int term_width(int fd)
{
  CONSOLE_SCREEN_BUFFER_INFO pinfo;
  HANDLE h;

  h = getconsole();

  if(h == INVALID_HANDLE_VALUE || h == NULL)
    return -1;
  if(GetConsoleScreenBufferInfo(h, &pinfo))
    // One less than actual width, as Terminal advances
    // to next line with the last character.
    return pinfo.dwMaximumWindowSize.X -1;
  return -1;
}

int term_present(void){
  return GetConsoleWindow() ? 1 : 0;
}

/* Get the next pressed key, if any.
   Returns 1 when there is a key, 0 if not. */
int term_get_key(int stopped, int do_delay, char *val){
  INPUT_RECORD record;
  HANDLE input;
  DWORD res;

  input = getconsoleintput();
  if(input == NULL || input == INVALID_HANDLE_VALUE)
    return 0;

  while(WaitForSingleObject(input, stopped ? INFINITE : (do_delay ? 10 : 0)) == WAIT_OBJECT_0){
    do_delay = 0;
    if(!ReadConsoleInput(input, &record, 1, &res))
      return 0;
    if(record.EventType == KEY_EVENT && record.Event.KeyEvent.bKeyDown){
      *val = record.Event.KeyEvent.uChar.AsciiChar;
      return 1;
    }
  }

  return 0;
}
