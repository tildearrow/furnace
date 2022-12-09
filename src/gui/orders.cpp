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
#include <fmt/printf.h>
#include "IconsFontAwesome4.h"
#include "imgui_internal.h"

void FurnaceGUI::drawMobileOrderSel() {
  if (!portrait) return;

  if (!orderScrollLocked) {
    if (orderScroll>(float)curOrder-0.005f && orderScroll<(float)curOrder+0.005f) {
      orderScroll=curOrder;
    } else if (orderScroll<curOrder) {
      orderScroll+=MAX(0.05f,(curOrder-orderScroll)*0.2f);
      if (orderScroll>curOrder) orderScroll=curOrder;
      WAKE_UP;
    } else {
      orderScroll-=MAX(0.05f,(orderScroll-curOrder)*0.2f);
      if (orderScroll<curOrder) orderScroll=curOrder;
      WAKE_UP;
    }
  }

  ImGui::SetNextWindowPos(ImVec2(0.0f,mobileMenuPos*-0.65*canvasH));
  ImGui::SetNextWindowSize(ImVec2(canvasW,0.12*canvasW));
  if (ImGui::Begin("OrderSel",NULL,globalWinFlags)) {
    ImDrawList* dl=ImGui::GetWindowDrawList();
    ImGuiWindow* window=ImGui::GetCurrentWindow();
    ImGuiStyle& style=ImGui::GetStyle();

    ImVec2 size=ImGui::GetContentRegionAvail();

    ImVec2 minArea=window->DC.CursorPos;
    ImVec2 maxArea=ImVec2(
      minArea.x+size.x,
      minArea.y+size.y
    );
    ImRect rect=ImRect(minArea,maxArea);
    ImGui::ItemSize(size,style.FramePadding.y);
    if (ImGui::ItemAdd(rect,ImGui::GetID("OrderSelW"))) {
      ImVec2 centerPos=ImLerp(minArea,maxArea,ImVec2(0.5,0.5));
      
      for (int i=0; i<e->curSubSong->ordersLen; i++) {
        ImVec2 pos=centerPos;
        ImVec4 color=uiColors[GUI_COLOR_TEXT];
        pos.x+=(i-orderScroll)*40.0*dpiScale;
        if (pos.x<-200.0*dpiScale) continue;
        if (pos.x>canvasW+200.0*dpiScale) break;
        String text=fmt::sprintf("%.2X",i);
        float targetSize=size.y-fabs(i-orderScroll)*2.0*dpiScale;
        if (targetSize<8.0*dpiScale) targetSize=8.0*dpiScale;
        color.w*=CLAMP(2.0f*(targetSize/size.y-0.5f),0.0f,1.0f);

        ImGui::PushFont(bigFont);
        ImVec2 textSize=ImGui::CalcTextSize(text.c_str());
        ImGui::PopFont();

        pos.x-=textSize.x*0.5*(targetSize/textSize.y);
        pos.y-=targetSize*0.5;

        dl->AddText(bigFont,targetSize,pos,ImGui::GetColorU32(color),text.c_str());
      }
    }
    if (ImGui::IsItemClicked()) {
      orderScrollSlideOrigin=ImGui::GetMousePos().x+orderScroll*40.0f*dpiScale;
      orderScrollRealOrigin=ImGui::GetMousePos();
      orderScrollLocked=true;
      orderScrollTolerance=true;
    }
  }
  ImGui::End();
}

void FurnaceGUI::drawOrders() {
  static char selID[4096];
  if (nextWindow==GUI_WINDOW_ORDERS) {
    ordersOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!ordersOpen) return;
  if (mobileUI) {
    patWindowPos=(portrait?ImVec2(0.0f,(mobileMenuPos*-0.65*canvasH)):ImVec2((0.16*canvasH)+0.5*canvasW*mobileMenuPos,0.0f));
    patWindowSize=(portrait?ImVec2(canvasW,canvasH-(0.16*canvasW)):ImVec2(canvasW-(0.16*canvasH),canvasH));
    ImGui::SetNextWindowPos(patWindowPos);
    ImGui::SetNextWindowSize(patWindowSize);
  } else {
    //ImGui::SetNextWindowSizeConstraints(ImVec2(440.0f*dpiScale,400.0f*dpiScale),ImVec2(canvasW,canvasH));
  }
  if (ImGui::Begin("Orders",&ordersOpen,globalWinFlags)) {
    float regionX=ImGui::GetContentRegionAvail().x;
    ImVec2 prevSpacing=ImGui::GetStyle().ItemSpacing;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,ImVec2(1.0f*dpiScale,1.0f*dpiScale));
    ImGui::Columns(2,NULL,false);
    ImGui::SetColumnWidth(-1,regionX-24.0f*dpiScale);
    int displayChans=0;
    for (int i=0; i<e->getTotalChannelCount(); i++) {
      if (e->curSubSong->chanShow[i]) displayChans++;
    }
    ImGui::PushFont(patFont);
    bool tooSmall=((displayChans+1)>((ImGui::GetContentRegionAvail().x)/(ImGui::CalcTextSize("AA").x+2.0*ImGui::GetStyle().ItemInnerSpacing.x)));
    ImGui::PopFont();
    if (ImGui::BeginTable("OrdersTable",1+displayChans,(tooSmall?ImGuiTableFlags_SizingFixedFit:ImGuiTableFlags_SizingStretchSame)|ImGuiTableFlags_ScrollX|ImGuiTableFlags_ScrollY)) {
      ImGui::PushFont(patFont);
      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,prevSpacing);
      ImGui::TableSetupScrollFreeze(1,1);
      float lineHeight=(ImGui::GetTextLineHeight()+4*dpiScale);
      if (e->isPlaying()) {
        if (followOrders) {
          ImGui::SetScrollY((e->getOrder()+1)*lineHeight-(ImGui::GetContentRegionAvail().y/2));
        }
      }
      ImGui::TableNextRow(0,lineHeight);
      ImGui::TableNextColumn();
      ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_ORDER_ROW_INDEX]);
      for (int i=0; i<e->getTotalChannelCount(); i++) {
        if (!e->curSubSong->chanShow[i]) continue;
        ImGui::TableNextColumn();
        ImGui::Text("%s",e->getChannelShortName(i));
      }
      ImGui::PopStyleColor();
      for (int i=0; i<e->curSubSong->ordersLen; i++) {
        ImGui::TableNextRow(0,lineHeight);
        if (oldOrder1==i) ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,ImGui::GetColorU32(uiColors[GUI_COLOR_ORDER_ACTIVE]));
        ImGui::TableNextColumn();
        ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_ORDER_ROW_INDEX]);
        bool highlightLoop=(i>=loopOrder && i<=loopEnd);
        if (highlightLoop) ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,ImGui::GetColorU32(uiColors[GUI_COLOR_SONG_LOOP]));
        if (settings.orderRowsBase==1) {
          snprintf(selID,4096,"%.2X##O_S%.2x",i,i);
        } else {
          snprintf(selID,4096,"%d##O_S%.2x",i,i);
        }
        if (ImGui::Selectable(selID)) {
          setOrder(i);
          curNibble=false;
          orderCursor=-1;

          if (orderEditMode==0) {
            handleUnimportant;
          }
        }
        ImGui::PopStyleColor();
        for (int j=0; j<e->getTotalChannelCount(); j++) {
          if (!e->curSubSong->chanShow[j]) continue;
          ImGui::TableNextColumn();
          DivPattern* pat=e->curPat[j].getPattern(e->curOrders->ord[j][i],false);
          /*if (!pat->name.empty()) {
            snprintf(selID,4096,"%s##O_%.2x_%.2x",pat->name.c_str(),j,i);
          } else {*/
            snprintf(selID,4096,"%.2X##O_%.2x_%.2x",e->curOrders->ord[j][i],j,i);
          //}

          ImGui::PushStyleColor(ImGuiCol_Text,(curOrder==i || e->curOrders->ord[j][i]==e->curOrders->ord[j][curOrder])?uiColors[GUI_COLOR_ORDER_SIMILAR]:uiColors[GUI_COLOR_ORDER_INACTIVE]);
          if (ImGui::Selectable(selID,settings.ordersCursor?(cursor.xCoarse==j && oldOrder1!=i):false)) {
            if (curOrder==i) {
              if (orderEditMode==0) {
                prepareUndo(GUI_UNDO_CHANGE_ORDER);
                e->lockSave([this,i,j]() {
                  if (changeAllOrders) {
                    for (int k=0; k<e->getTotalChannelCount(); k++) {
                      if (e->curOrders->ord[k][i]<(unsigned char)(DIV_MAX_PATTERNS-1)) e->curOrders->ord[k][i]++;
                    }
                  } else {
                    if (e->curOrders->ord[j][i]<(unsigned char)(DIV_MAX_PATTERNS-1)) e->curOrders->ord[j][i]++;
                  }
                });
                e->walkSong(loopOrder,loopRow,loopEnd);
                makeUndo(GUI_UNDO_CHANGE_ORDER);
              } else {
                orderCursor=j;
                curNibble=false;
              }
            } else {
              setOrder(i);
              e->walkSong(loopOrder,loopRow,loopEnd);
              if (orderEditMode!=0) {
                orderCursor=j;
                curNibble=false;
              }
            }

            if (orderEditMode==0) {
              handleUnimportant;
            }
          }
          ImGui::PopStyleColor();
          if (orderEditMode!=0 && curOrder==i && orderCursor==j) {
            // draw a border
            ImDrawList* dl=ImGui::GetWindowDrawList();
            dl->AddRect(ImGui::GetItemRectMin(),ImGui::GetItemRectMax(),ImGui::GetColorU32(uiColors[GUI_COLOR_TEXT]),2.0f*dpiScale);
          }
          if (!pat->name.empty() && ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s",pat->name.c_str());
          }
          if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
            if (curOrder==i) {
              if (orderEditMode==0) {
                prepareUndo(GUI_UNDO_CHANGE_ORDER);
                e->lockSave([this,i,j]() {
                  if (changeAllOrders) {
                    for (int k=0; k<e->getTotalChannelCount(); k++) {
                      if (e->curOrders->ord[k][i]>0) e->curOrders->ord[k][i]--;
                    }
                  } else {
                    if (e->curOrders->ord[j][i]>0) e->curOrders->ord[j][i]--;
                  }
                });
                e->walkSong(loopOrder,loopRow,loopEnd);
                makeUndo(GUI_UNDO_CHANGE_ORDER);
              } else {
                orderCursor=j;
                curNibble=false;
              }
            } else {
              setOrder(i);
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
    if (ImGui::Button(ICON_FA_PLUS)) { handleUnimportant
      // add order row (new)
      doAction(GUI_ACTION_ORDERS_ADD);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Add new order");
    }
    if (ImGui::Button(ICON_FA_MINUS)) { handleUnimportant
      // remove this order row
      doAction(GUI_ACTION_ORDERS_REMOVE);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Remove order");
    } 
    if (ImGui::Button(ICON_FA_FILES_O)) { handleUnimportant
      // duplicate order row
      doAction(GUI_ACTION_ORDERS_DUPLICATE);
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
      doAction(GUI_ACTION_ORDERS_DEEP_CLONE);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Duplicate order (right-click to deep clone)");
    }
    if (ImGui::Button(ICON_FA_ANGLE_UP)) { handleUnimportant
      // move order row up
      doAction(GUI_ACTION_ORDERS_MOVE_UP);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Move order up");
    }
    if (ImGui::Button(ICON_FA_ANGLE_DOWN)) { handleUnimportant
      // move order row down
      doAction(GUI_ACTION_ORDERS_MOVE_DOWN);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Move order down");
    }
    if (ImGui::Button(ICON_FA_ANGLE_DOUBLE_DOWN)) { handleUnimportant
      // duplicate order row at end
      doAction(GUI_ACTION_ORDERS_DUPLICATE_END);
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
      doAction(GUI_ACTION_ORDERS_DEEP_CLONE_END);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Duplicate order at end of song (right-click to deep clone)");
    }
    if (ImGui::Button(changeAllOrders?ICON_FA_LINK"##ChangeAll":ICON_FA_CHAIN_BROKEN"##ChangeAll")) { handleUnimportant
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
    if (ImGui::Button(orderEditModeLabel)) { handleUnimportant
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
