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

#define CH FurnaceGUISysDefChip
#define CATEGORY_BEGIN(x,y) FurnaceGUISysCategory cat(x,y);
#define CATEGORY_END sysCategories.push_back(cat);
#define ENTRY(...) \
  cat.systems.push_back(FurnaceGUISysDef(__VA_ARGS__));
#define SUB_ENTRY(...) \
  cat.systems[cat.systems.size()-1].subDefs.push_back(FurnaceGUISysDef(__VA_ARGS__));
#define SUB_SUB_ENTRY(...) \
  cat.systems[cat.systems.size()-1].subDefs[cat.systems[cat.systems.size()-1].subDefs.size()-1].subDefs.push_back(FurnaceGUISysDef(__VA_ARGS__));

// 1. define system categories here.

void initSystemPresetsGameConsoles(std::vector<FurnaceGUISysCategory>& sysCategories);
void initSystemPresetsComputers(std::vector<FurnaceGUISysCategory>& sysCategories);
void initSystemPresetsArcadeSystems(std::vector<FurnaceGUISysCategory>& sysCategories);
void initSystemPresetsUser(std::vector<FurnaceGUISysCategory>& sysCategories);
void initSystemPresetsFM(std::vector<FurnaceGUISysCategory>& sysCategories);
void initSystemPresetsSquare(std::vector<FurnaceGUISysCategory>& sysCategories);
void initSystemPresetsSample(std::vector<FurnaceGUISysCategory>& sysCategories);
void initSystemPresetsWavetable(std::vector<FurnaceGUISysCategory>& sysCategories);
void initSystemPresetsSpecialized(std::vector<FurnaceGUISysCategory>& sysCategories);
void initSystemPresetsDefleCompat(std::vector<FurnaceGUISysCategory>& sysCategories);

// 2. also define it here so it is called by initSystemPresets().
#define INIT_ALL_SYSTEM_PRESETS(x) \
  initSystemPresetsGameConsoles(x); \
  initSystemPresetsComputers(x); \
  initSystemPresetsArcadeSystems(x); \
  initSystemPresetsUser(x); \
  initSystemPresetsFM(x); \
  initSystemPresetsSquare(x); \
  initSystemPresetsSample(x); \
  initSystemPresetsWavetable(x); \
  initSystemPresetsSpecialized(x); \
  initSystemPresetsDefleCompat(x);

// 3. create a new source file in this directory for that category. use the following template:
// #include "presets.h"
//
// void initSystemPresetsExample(std::vector<FurnaceGUISysCategory>& sysCategories) {
//   CATEGORY_BEGIN(_("Example"),_("an example category."));
//   CATEGORY_END;
// }

// 4. add your source file to CMakeLists.txt, next to the other preset sources.

// 5. add system configurations in their respective source files.
// every entry is written in the following format:
//   ENTRY(
//     "System Name", {
//      CH(DIV_SYSTEM_???, Volume, Panning, Flags),
//      CH(DIV_SYSTEM_???, Volume, Panning, Flags),
//      ...
//     }
//   );
// flags are a string of new line-separated values.
// use SUB_ENTRY instead of ENTRY to add sub-entries to the previous entry.
