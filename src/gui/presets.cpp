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

#include "gui.h"
#include "../baseutils.h"
#include "../fileutils.h"
#include <fmt/printf.h>
#include "IconsFontAwesome4.h"
#include <imgui.h>
#include "misc/cpp/imgui_stdlib.h"

#include "presets/presets.h"

void FurnaceGUI::initSystemPresets() {
  sysCategories.clear();

  INIT_ALL_SYSTEM_PRESETS(sysCategories);
}

void FurnaceGUISysDef::bake() {
  int index=0;
  definition="";
  for (FurnaceGUISysDefChip& i: orig) {
    definition+=fmt::sprintf(
      "id%d=%d\nvol%d=%f\npan%d=%f\nflags%d=%s\n",
      index,
      DivEngine::systemToFileFur(i.sys),
      index,
      i.vol,
      index,
      i.pan,
      index,
      taEncodeBase64(i.flags)
    );
    if (i.chans>0) {
      definition+=fmt::sprintf("chans%d=%d\n",index,i.chans);
    }
    index++;
  }
  if (!extra.empty()) {
    definition+=extra;
  }
}

FurnaceGUISysDef::FurnaceGUISysDef(const char* n, std::initializer_list<FurnaceGUISysDefChip> def, const char* e):
  name(n),
  extra((e==NULL)?"":e) {
  orig=def;
  bake();
}

FurnaceGUISysDef::FurnaceGUISysDef(const char* n, const char* def, DivEngine* e):
  name(n),
  definition(taDecodeBase64(def)) {
  // extract definition
  DivConfig conf;
  conf.loadFromMemory(definition.c_str());
  for (int i=0; i<DIV_MAX_CHIPS; i++) {
    String nextStr=fmt::sprintf("id%d",i);
    int id=conf.getInt(nextStr.c_str(),0);
    if (id==0) break;
    conf.remove(nextStr.c_str());

    nextStr=fmt::sprintf("vol%d",i);
    float vol=conf.getFloat(nextStr.c_str(),1.0f);
    conf.remove(nextStr.c_str());
    nextStr=fmt::sprintf("pan%d",i);
    float pan=conf.getFloat(nextStr.c_str(),0.0f);
    conf.remove(nextStr.c_str());
    nextStr=fmt::sprintf("fr%d",i);
    float panFR=conf.getFloat(nextStr.c_str(),0.0f);
    conf.remove(nextStr.c_str());
    nextStr=fmt::sprintf("flags%d",i);
    String flags=taDecodeBase64(conf.getString(nextStr.c_str(),"").c_str());
    conf.remove(nextStr.c_str());
    nextStr=fmt::sprintf("chans%d",i);
    int chans=conf.getInt(nextStr.c_str(),0);
    conf.remove(nextStr.c_str());

    orig.push_back(FurnaceGUISysDefChip(e->systemFromFileFur(id),vol,pan,flags.c_str(),panFR,chans));
  }
  // extract extra
  extra=conf.toString();
}
