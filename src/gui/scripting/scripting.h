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

#include "../gui.h"

#define DIV_PAT_RAW (DIV_PAT_FXVAL(DIV_MAX_EFFECTS)+1)

#define _CF(x) \
   static int _ ## x(lua_State* s) { \
     return externGUI->sc_ ## x(s); \
   } \
   int FurnaceGUI::sc_ ## x(lua_State* s)

#define SC_ERROR(x) \
  lua_pushliteral(s,x); \
  lua_error(s); \
  return 0;

#define CHECK_ARGS(x) \
  if (lua_gettop(s)!=x) { \
    SC_ERROR("invalid argument count!") \
  }

#define CHECK_ARGS_RANGE(x,y) \
   if (lua_gettop(s)<x || lua_gettop(s)>y) { \
     SC_ERROR("invalid argument count!") \
   }

#define CHECK_TYPE(t,x) \
   if (!lua_is ## t (s,x)) { \
     SC_ERROR("invalid argument type! (expected " #t ")") \
   }

#define CHECK_TYPE_BOOLEAN(x) CHECK_TYPE(boolean,x)
#define CHECK_TYPE_FUNCTION(x) CHECK_TYPE(function,x)
#define CHECK_TYPE_NUMBER(x) CHECK_TYPE(number,x)
#define CHECK_TYPE_INTEGER(x) CHECK_TYPE(integer,x)
#define CHECK_TYPE_STRING(x) CHECK_TYPE(string,x)
#define CHECK_TYPE_TABLE(x) CHECK_TYPE(table,x)

// use for functions in the "fur" table
#define API_GLOBAL_FUNC(x) \
   lua_getglobal(s,"fur"); \
   lua_pushcfunction(s,_ ## x); \
   lua_setfield(s,-2,#x); \
   lua_pop(s,1);

// make category under the "fur" namespace and let it available on the top of the stack
#define API_MAKE_CATG(_name) \
   lua_getglobal(s,"fur"); \
   lua_newtable(s); \
   lua_pushnil(s); \
   lua_copy(s,-2,-1); \
   lua_setfield(s,-3,_name); \
   lua_remove(s,-2);

// put already-existing category under the "fur" namespace on the top of the stack; creates it if is not present
#define API_USE_CATG(_name) \
   lua_getglobal(s,"fur"); \
   lua_getfield(s,-1,_name); \
   if (lua_isnil(s,-1)) {API_MAKE_CATG(_name)}

// assuming a table T is on the top of the stack, performs T[_name]=_value. conserves stack.
#define API_ADD_VALUE(_name,_value,_type) \
   lua_push##_type(s,_value); \
   lua_setfield(s,-2,_name);

// same as API_ADD_VALUE, but for functions specifically. _funcId is an identifier.
#define API_ADD_FUNC(_name,_funcId) \
   lua_pushcfunction(s,_##_funcId); \
   lua_setfield(s,-2,_name);

#define API_CATG_END lua_pop(s,1);

class StackDiffChecker {
  private:
    lua_State* s;
    int expected, startHeight, endHeight;

  public:
    StackDiffChecker(lua_State* s, int expected=0);
    ~StackDiffChecker();
};

/**
 * Code for the "inspection" procedure.
 *
 * Fits better in a class since it has to pass around a lot of state to its different parts.
 */
class LuaValueInspector {
  lua_State* s;
  bool indentTables;
  int visitedIdx; // index for the "visited tables" table

  private:
    // figures out whether a string is a valid identifier ([a-zA-Z_][a-zA-Z0-9_]*)
    static bool isValidIdentifier(const char* s);

    void inspectString(String& dest, int indent);
    void inspectTable(String& dest, int indent);
    void inspectAny(String& dest, int indent);

  public:
    LuaValueInspector(lua_State* s, bool indentTables);
    void inspect(String& dest);
};

typedef void (*InsFeatureWriter)(const char*, const char*, int);

void writeIns(DivInstrument* ins, DivInstrumentType type, const char* key, const char* subkey, int value);
void writeMacro(DivInstrumentMacro* macro, const char* key, const char* subkey, int value);
