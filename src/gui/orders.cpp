/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2022 tildearrow and contributors
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
#include "IconsFontAwesome4.h"

void FurnaceGUI::drawOrders() {
  char selID[64];
  if (nextWindow==GUI_WINDOW_ORDERS) {
    ordersOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!ordersOpen) return;
  if (ImGui::Begin("Orders",&ordersOpen)) {
    float regionX=ImGui::GetContentRegionAvail().x;
    ImVec2 prevSpacing=ImGui::GetStyle().ItemSpacing;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,ImVec2(1.0f*dpiScale,1.0f*dpiScale));
    ImGui::Columns(2,NULL,false);
    ImGui::SetColumnWidth(-1,regionX-24.0f*dpiScale);
    int displayChans=0;
    for (int i=0; i<e->getTotalChannelCount(); i++) {
      if (e->song.chanShow[i]) displayChans++;
    }
    if (ImGui::BeginTable("OrdersTable",1+displayChans,ImGuiTableFlags_SizingStretchSame|ImGuiTableFlags_ScrollX|ImGuiTableFlags_ScrollY)) {
      ImGui::PushFont(patFont);
      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,prevSpacing);
      ImGui::TableSetupScrollFreeze(1,1);
      float lineHeight=(ImGui::GetTextLineHeight()+4*dpiScale);
      int curOrder=e->getOrder();
      if (e->isPlaying()) {
        if (followOrders) {
          ImGui::SetScrollY((curOrder+1)*lineHeight-(ImGui::GetContentRegionAvail().y/2));
        }
      }
      ImGui::TableNextRow(0,lineHeight);
      ImGui::TableNextColumn();
      ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_ROW_INDEX]);
      for (int i=0; i<e->getTotalChannelCount(); i++) {
        if (!e->song.chanShow[i]) continue;
        ImGui::TableNextColumn();
        ImGui::Text("%s",e->getChannelShortName(i));
      }
      ImGui::PopStyleColor();
      for (int i=0; i<e->song.ordersLen; i++) {
        ImGui::TableNextRow(0,lineHeight);
        if (oldOrder1==i) ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,0x40ffffff);
        ImGui::TableNextColumn();
        ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PATTERN_ROW_INDEX]);
        bool highlightLoop=(i>=loopOrder && i<=loopEnd);
        if (highlightLoop) ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,ImGui::GetColorU32(uiColors[GUI_COLOR_SONG_LOOP]));
        if (settings.orderRowsBase==1) {
          snprintf(selID,64,"%.2X##O_S%.2x",i,i);
        } else {
          snprintf(selID,64,"%d##O_S%.2x",i,i);
        }
        if (ImGui::Selectable(selID)) {
          e->setOrder(i);
          curNibble=false;
          orderCursor=-1;
        }
        ImGui::PopStyleColor();
        for (int j=0; j<e->getTotalChannelCount(); j++) {
          if (!e->song.chanShow[j]) continue;
          ImGui::TableNextColumn();
          snprintf(selID,64,"%.2X##O_%.2x_%.2x",e->song.orders.ord[j][i],j,i);
          if (ImGui::Selectable(selID,(orderEditMode!=0 && curOrder==i && orderCursor==j))) {
            if (curOrder==i) {
              if (orderEditMode==0) {
                prepareUndo(GUI_UNDO_CHANGE_ORDER);
                if (changeAllOrders) {
                  for (int k=0; k<e->getTotalChannelCount(); k++) {
                    if (e->song.orders.ord[k][i]<0x7f) e->song.orders.ord[k][i]++;
                  }
                } else {
                  if (e->song.orders.ord[j][i]<0x7f) e->song.orders.ord[j][i]++;
                }
                e->walkSong(loopOrder,loopRow,loopEnd);
                makeUndo(GUI_UNDO_CHANGE_ORDER);
              } else {
                orderCursor=j;
                curNibble=false;
              }
            } else {
              e->setOrder(i);
              e->walkSong(loopOrder,loopRow,loopEnd);
              if (orderEditMode!=0) {
                orderCursor=j;
                curNibble=false;
              }
            }
          }
          if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
            if (curOrder==i) {
              if (orderEditMode==0) {
                prepareUndo(GUI_UNDO_CHANGE_ORDER);
                if (changeAllOrders) {
                  for (int k=0; k<e->getTotalChannelCount(); k++) {
                    if (e->song.orders.ord[k][i]>0) e->song.orders.ord[k][i]--;
                  }
                } else {
                  if (e->song.orders.ord[j][i]>0) e->song.orders.ord[j][i]--;
                }
                e->walkSong(loopOrder,loopRow,loopEnd);
                makeUndo(GUI_UNDO_CHANGE_ORDER);
              } else {
                orderCursor=j;
                curNibble=false;
              }
            } else {
              e->setOrder(i);
              e->walkSong(loopOrder,loopRow,loopEnd);
              if (orderEditMode!=0) {
                orderCursor=j;
                curNibble=false;
              }
            }
          }
        }
      }
      ImGui::PopStyleVar();
      ImGui::PopFont();
      ImGui::EndTable();
    }
    ImGui::NextColumn();
    if (ImGui::Button(ICON_FA_PLUS)) {
      // add order row (new)
      doAction(GUI_ACTION_ORDERS_ADD);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Add new order");
    }
    if (ImGui::Button(ICON_FA_MINUS)) {
      // remove this order row
      doAction(GUI_ACTION_ORDERS_REMOVE);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Remove order");
    }
    if (ImGui::Button(ICON_FA_FILES_O)) {
      // duplicate order row
      doAction(GUI_ACTION_ORDERS_DUPLICATE);
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
      doAction(GUI_ACTION_ORDERS_DEEP_CLONE);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Duplicate order (right-click to deep clone)");
    }
    if (ImGui::Button(ICON_FA_ANGLE_UP)) {
      // move order row up
      doAction(GUI_ACTION_ORDERS_MOVE_UP);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Move order up");
    }
    if (ImGui::Button(ICON_FA_ANGLE_DOWN)) {
      // move order row down
      doAction(GUI_ACTION_ORDERS_MOVE_DOWN);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Move order down");
    }
    if (ImGui::Button(ICON_FA_ANGLE_DOUBLE_DOWN)) {
      // duplicate order row at end
      doAction(GUI_ACTION_ORDERS_DUPLICATE_END);
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
      doAction(GUI_ACTION_ORDERS_DEEP_CLONE_END);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Duplicate order at end of song (right-click to deep clone)");
    }
    if (ImGui::Button(changeAllOrders?ICON_FA_LINK"##ChangeAll":ICON_FA_CHAIN_BROKEN"##ChangeAll")) {
      // whether to change one or all orders in a row
      changeAllOrders=!changeAllOrders;
    }
    if (ImGui::IsItemHovered()) {
      if (changeAllOrders) {
        ImGui::SetTooltip("Order change mode: entire row");
      } else {
        ImGui::SetTooltip("Order change mode: one");
      }
    }
    const char* orderEditModeLabel="?##OrderEditMode";
    if (orderEditMode==3) {
      orderEditModeLabel=ICON_FA_ARROWS_V "##OrderEditMode";
    } else if (orderEditMode==2) {
      orderEditModeLabel=ICON_FA_ARROWS_H "##OrderEditMode";
    } else if (orderEditMode==1) {
      orderEditModeLabel=ICON_FA_I_CURSOR "##OrderEditMode";
    } else {
      orderEditModeLabel=ICON_FA_MOUSE_POINTER "##OrderEditMode";
    }
    if (ImGui::Button(orderEditModeLabel)) {
      orderEditMode++;
      if (orderEditMode>3) orderEditMode=0;
      curNibble=false;
    }
    if (ImGui::IsItemHovered()) {
      if (orderEditMode==3) {
        ImGui::SetTooltip("Order edit mode: Select and type (scroll vertically)");
      } else if (orderEditMode==2) {
        ImGui::SetTooltip("Order edit mode: Select and type (scroll horizontally)");
      } else if (orderEditMode==1) {
        ImGui::SetTooltip("Order edit mode: Select and type (don't scroll)");
      } else {
        ImGui::SetTooltip("Order edit mode: Click to change");
      }
    }
    ImGui::PopStyleVar();
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_ORDERS;
  oldOrder1=e->getOrder();
  ImGui::End();
}