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

#ifndef FURNACE_HORIZONTAL_UI_H
#define FURNACE_HORIZONTAL_UI_H

#include <cstddef>

class FurnaceGUI;
struct DivSubSong;
struct ImFont;

// All transient state belongs to this view; the song remains ordinary Furnace data.
class FurnaceGUIHorizontal {
  bool editorOpen=false, focusEditor=false, centerPitch=false;
  int order=0, channel=0, row=0, pitch=108, length=4, snap=1;
  int preview=-1, dragMode=0, dragRow=0, dragPitch=0, dragEnd=0;
  int dragTargetRow=0, dragTargetPitch=0, dragTargetEnd=0;
  int dragPattern=-1, dragMouseRow=0, orderAction=0, lastPlayOrder=-1;
  float orderWidth=156.0f, orderHeight=56.0f;
  float rowWidth=24.0f, keyHeight=18.0f, eventZoom=1.0f, scrollX=0.0f;
  const DivSubSong* song=NULL;
  ImFont* idFont=NULL;
  void select(FurnaceGUI& gui, int ord, int ch);
  void open(FurnaceGUI& gui, int ord, int ch);
  void orderMenu(FurnaceGUI& gui);
  void songView(FurnaceGUI& gui);
  void patternView(FurnaceGUI& gui);
  void pianoRoll(FurnaceGUI& gui, float height);
  void eventLanes(FurnaceGUI& gui, float height);
  void inspector(FurnaceGUI& gui);
  void setCell(FurnaceGUI& gui, int col, int value);
  void editNote(FurnaceGUI& gui, int mode, int from, int to, int note, int end);
  void stopPreview(FurnaceGUI& gui);
public:
  void buildFont(FurnaceGUI& gui);
  bool draw(FurnaceGUI& gui);
};

#endif
