/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "scripting.h"

StackDiffChecker::StackDiffChecker(lua_State* s, int expected):
  s(s),
  expected(expected) {
  startHeight=lua_gettop(s);
}

StackDiffChecker::~StackDiffChecker() {
  endHeight=lua_gettop(s);
  int got=endHeight-startHeight;
  if (got!=expected) {
    logE("wrong stack difference detected! expected %d, got %d",expected,got);
    assert(false);
  }
}

LuaValueInspector::LuaValueInspector(lua_State* s, bool indentTables):
  s(s),
  indentTables(indentTables),
  visitedIdx(-1) {}

void LuaValueInspector::inspect(String& dest) {
  // set up the "visited tables" table
  lua_newtable(s);
  visitedIdx=lua_absindex(s,-1);

  // push a copy of the value to inspect so we can interact with it
  lua_pushnil(s);
  lua_copy(s,-3,-1);

  // do the inspecting on the top value
  inspectAny(dest,0);

  // pop the two things we pushed
  lua_pop(s,2);
}

bool LuaValueInspector::isValidIdentifier(const char* s) {
  if (*s=='\0') return false;
  if (!isalpha(*s) && *s!='_') return false;
  while (*s) {
    if (!isalnum(*s) && *s!='_') return false;
    s++;
  }
  return true;
}

void LuaValueInspector::inspectString(String& dest, int indent) {
  const char* str=lua_tostring(s,-1);
  if (str==NULL) {
    dest+="<what?>";
    return;
  }

  dest+="\"";
  for (const char* sp=str; *sp; sp++) {
    char c=*sp;
    switch (c) {
      case '\a': dest+="\\a"; break;
      case '\b': dest+="\\b"; break;
      case '\f': dest+="\\f"; break;
      case '\n': dest+="\\n"; break;
      case '\r': dest+="\\r"; break;
      case '\t': dest+="\\t"; break;
      case '\v': dest+="\\v"; break;
      case '\\': dest+="\\\\"; break;
      case '\"': dest+="\\\""; break;
      case '\'': dest+="'"; break;
      default:
        if (isprint(c)) dest+=c;
        else dest+=fmt::sprintf("\\%03d",(unsigned char)c);
        break;
    }
  }
  dest+="\"";
}

void LuaValueInspector::inspectTable(String& dest, int indent) {
  // check if the table has already been inspected (to avoid infinite loops)
  lua_pushnil(s);
  lua_copy(s,-2,-1);
  lua_gettable(s,visitedIdx);
  if (!lua_isnil(s,-1)) {
    lua_pop(s,1);
    dest+=_("<cycle reached>");
    return;
  }
  lua_pop(s,1);

  // if we're here, it hasn't been inspected yet, so we gotta mark it as such.
  lua_pushnil(s);
  lua_copy(s,-2,-1);
  lua_pushboolean(s,true);
  lua_settable(s,visitedIdx);

  int tableIdx=lua_absindex(s,-1);

  // TODO: maybe there's a simpler way to inspect a table without this many passes?

  // figure out whether the table has array keys
  int lastArrayKey=0; // if 0, there are no array keys
  for (int i=1;; i++) {
    lua_pushinteger(s,i);
    lua_gettable(s,tableIdx);
    if (lua_isnil(s,lua_gettop(s))) {
      lua_pop(s,1);
      break;
    }
    lua_pop(s,1);
    lastArrayKey=i;
  }

  // whether there is a key-value pair on the top of the stack, where the key is one of the array keys
  const auto isTopArrayKvPair=[this,lastArrayKey](){
    if (lua_isinteger(s,-2)) {
      int n=lua_tointeger(s,-2);
      if (n>0 && n<=lastArrayKey) {
        return true;
      }
    }
    return false;
  };

  // figure out whether the table has non-array keys
  bool hasNonArrayKeys=false;
  lua_pushnil(s);
  while (lua_next(s,tableIdx)) {
    // skip array keys
    if (isTopArrayKvPair()) {
      lua_pop(s,1);
      continue;
    }

    hasNonArrayKeys=true;
    lua_pop(s,2); // remove key-value pair from stack
    break;
  }

  // at this point we know whether to indent or not
  const bool doIndent=indentTables && hasNonArrayKeys;
  const int nextIndent=doIndent?(indent + 1):indent;

  dest+="{";

  bool first=true;
  const auto maybeAddSep=[doIndent,nextIndent,&first,&dest](){
    if (first) {
      first=false;
    } else {
      dest+=",";
    }
    if (doIndent) {
      dest+="\n";
      for (int i=0; i<nextIndent; i++) dest+="  ";
    }
  };

  // first show the array entries
  for (int i=1; i<=lastArrayKey; i++) {
    lua_pushinteger(s,i);
    lua_gettable(s,tableIdx);

    if (lua_isnil(s,lua_gettop(s))) {
      lua_pop(s,1);
      logE("expected array key. why is there not one??? (k=%d)",i);
      continue;
    }

    maybeAddSep();
    inspectAny(dest,nextIndent);
    lua_pop(s,1);
  }

  // gets the top value as a string pointer if it is a valid identifier; returns NULL otherwise.
  // does not pop.
  const auto getTopIdentStr=[this]() -> const char* {
    if (lua_type(s,lua_gettop(s))!=LUA_TSTRING) return NULL;
    const char* str=lua_tostring(s,lua_gettop(s));
    if (str==NULL) return NULL;
    if (LuaValueInspector::isValidIdentifier(str)) return str;
    return NULL;
  };

  // then show the show the rest as k-v pairs
  lua_pushnil(s);
  while (lua_next(s,tableIdx)) {
    // skip "array" keys (already handled)
    if (isTopArrayKvPair()) {
      lua_pop(s,1);
      continue;
    }

    String valueRepr;
    inspectAny(valueRepr,nextIndent);
    lua_pop(s,1);

    maybeAddSep();
    const char* identStr=getTopIdentStr();
    if (identStr!=NULL) {
      dest+=identStr;
    } else {
      dest+="[";
      inspectAny(dest,nextIndent);
      dest+="]";
    }
    dest+="=";
    dest+=valueRepr;
  }

  if (doIndent) {
    dest+="\n";
    for (int i=0; i<indent; i++) dest+="  ";
  }
  dest+="}";
}

void LuaValueInspector::inspectAny(String& dest, int indent) {
  switch (lua_type(s,-1)) {
    case LUA_TNIL:
      dest+="nil";
      break;
    case LUA_TNUMBER: {
      // duplicate because lua_tostring modifies the value on the stack
      lua_pushvalue(s,-1);
      const char* str=lua_tostring(s,lua_gettop(s));
      if (str==NULL) {
        dest+="what?";
      } else {
        dest+=str;
        lua_pop(s,1);
      }
      break;
    }
    case LUA_TBOOLEAN: {
      int v=lua_toboolean(s,-1);
      dest+=(v?"true":"false");
      break;
    }
    case LUA_TSTRING:
      inspectString(dest,indent);
      break;
    case LUA_TTABLE:
      inspectTable(dest,indent);
      break;
    case LUA_TFUNCTION:
      dest+="<function>";
      break;
    case LUA_TUSERDATA:
      dest+="<userdata>";
      break;
    case LUA_TTHREAD:
      dest+="<thread>";
      break;
    case LUA_TLIGHTUSERDATA:
      dest+="<lightuserdata>";
      break;
    default:
      dest+=_("<unknown value>");
      break;
  }
}
