/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2025 tildearrow and contributors
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

// for suck's fake Clang extension!
#define _USE_MATH_DEFINES
#include "gui.h"
#include "../ta-log.h"
#include "imgui_internal.h"
#include "IconsFontAwesome4.h"
#include "furIcons.h"
#include "misc/cpp/imgui_stdlib.h"
#include "guiConst.h"
#include "../utfutils.h"
#include <fmt/printf.h>

void FurnaceGUI::drawPatternNew() {
  if (nextWindow==GUI_WINDOW_PATTERN) {
    patternOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!patternOpen) return;

  if (e->isPlaying() && followPattern) {
    if (oldRowChanged || !e->isStepping()) {
      if (e->isStepping()) pendingStepUpdate=1;
      cursor.y=oldRow;
      cursor.order=curOrder;
      if (selStart.xCoarse==selEnd.xCoarse && selStart.xFine==selEnd.xFine && selStart.y==selEnd.y && selStart.order==selEnd.order && !selecting) {
        selStart=cursor;
        selEnd=cursor;
      }
    }
  }
  sel1=selStart;
  sel2=selEnd;
  if (sel2.order<sel1.order) {
    sel2.order^=sel1.order;
    sel1.order^=sel2.order;
    sel2.order^=sel1.order;
    sel2.y^=sel1.y;
    sel1.y^=sel2.y;
    sel2.y^=sel1.y;
  } else if (sel2.order==sel1.order && sel2.y<sel1.y) {
    sel2.y^=sel1.y;
    sel1.y^=sel2.y;
    sel2.y^=sel1.y;
  }
  if (sel2.xCoarse<sel1.xCoarse) {
    sel2.xCoarse^=sel1.xCoarse;
    sel1.xCoarse^=sel2.xCoarse;
    sel2.xCoarse^=sel1.xCoarse;

    sel2.xFine^=sel1.xFine;
    sel1.xFine^=sel2.xFine;
    sel2.xFine^=sel1.xFine;
  } else if (sel2.xCoarse==sel1.xCoarse && sel2.xFine<sel1.xFine) {
    sel2.xFine^=sel1.xFine;
    sel1.xFine^=sel2.xFine;
    sel2.xFine^=sel1.xFine;
  }

  //ImVec2 origWinPadding=ImGui::GetStyle().WindowPadding;
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,ImVec2(0.0f,0.0f));
  if (mobileUI) {
    patWindowPos=(portrait?ImVec2(0.0f,(mobileMenuPos*-0.65*canvasH)+(0.12*canvasW)):ImVec2((0.16*canvasH)+0.5*canvasW*mobileMenuPos,0.0f));
    patWindowSize=(portrait?ImVec2(canvasW,canvasH-(0.16*canvasW)-(0.12*canvasW)-(pianoOpen?(0.4*canvasW):0.0f)):ImVec2(canvasW-(0.16*canvasH),canvasH-(pianoOpen?(0.3*canvasH):0.0f)));
    ImGui::SetNextWindowPos(patWindowPos);
    ImGui::SetNextWindowSize(patWindowSize);
  }
  if (ImGui::Begin("PatternNew",&patternOpen,globalWinFlags|(settings.avoidRaisingPattern?ImGuiWindowFlags_NoBringToFrontOnFocus:0)|((settings.cursorFollowsWheel && !selecting)?ImGuiWindowFlags_NoScrollWithMouse:0),_("Pattern"))) {
    if (!mobileUI) {
      patWindowPos=ImGui::GetWindowPos();
      patWindowSize=ImGui::GetWindowSize();
    }

    ImGui::Text("The pattern view goes here.");

    ImGui::PushFont(patFont);
    ImGui::PopFont();
  }
  ImGui::PopStyleVar();
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_PATTERN;
  ImGui::End();
}

