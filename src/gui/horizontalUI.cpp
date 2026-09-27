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
#include "horizontalPattern.h"
#ifdef HAVE_FREETYPE
#include "misc/freetype/imgui_freetype.h"
#endif
#include "IconsFontAwesome4.h"
#include <cstdlib>
#include <cmath>

namespace {
using namespace FurnaceHorizontalPattern;

ImU32 tint(ImVec4 color, float alpha) {
  color.w=alpha;
  return ImGui::GetColorU32(color);
}

bool blackKey(int note) {
  int key=note%12;
  return key==1 || key==3 || key==6 || key==8 || key==10;
}

// Commit complete hexadecimal entries with Enter. Empty input clears a cell.
bool hexInput(const char* label, int& value, int maximum) {
  char text[16]="";
  if (value>=0) snprintf(text,sizeof(text),"%02X",value);
  if (!ImGui::InputTextWithHint(label,"..",text,sizeof(text),ImGuiInputTextFlags_CharsHexadecimal|ImGuiInputTextFlags_AutoSelectAll|ImGuiInputTextFlags_EnterReturnsTrue)) return false;
  value=text[0]?CLAMP((int)strtol(text,NULL,16),0,maximum):-1;
  return true;
}

struct PatternPayload {
  int channel, pattern;
};

// FL Playlist navigation, also used by the roll. Handle wheel input ourselves
// so modifier zoom never also scrolls a parent. Return a pending horizontal
// offset to keep the notes and event lanes synchronized on the following frame.
float wheelNavigation(float& width, float& height, ImVec2 minimum, ImVec2 maximum,
                      ImVec2 pinned, float scale) {
  ImGuiWindow* window=ImGui::GetCurrentWindow();
  window->Flags|=ImGuiWindowFlags_NoScrollWithMouse;
  if (!ImGui::IsWindowHovered() || ImGui::IsAnyItemActive()) return -1.0f;
  const ImGuiIO& io=ImGui::GetIO();
  if (io.MouseWheel==0 && io.MouseWheelH==0) return -1.0f;
  float x=ImGui::GetScrollX(), y=ImGui::GetScrollY();
  ImVec2 origin=ImGui::GetCursorScreenPos();
  if (io.KeyCtrl || io.KeySuper) {
    float old=width;
    width=CLAMP(width*powf(1.15f,io.MouseWheel),minimum.x,maximum.x);
    float anchor=MAX(0.0f,io.MousePos.x-(origin.x+x+pinned.x));
    x=MAX(0.0f,(x+anchor)*width/old-anchor);
    ImGui::SetScrollX(x);
    return x;
  }
  if (io.KeyAlt) {
    float old=height;
    height=CLAMP(height*powf(1.15f,io.MouseWheel),minimum.y,maximum.y);
    float anchor=MAX(0.0f,io.MousePos.y-(origin.y+y+pinned.y));
    ImGui::SetScrollY(MAX(0.0f,(y+anchor)*height/old-anchor));
    return -1.0f;
  }
  float horizontal=io.MouseWheelH+(io.KeyShift?io.MouseWheel:0.0f);
  if (!io.KeyShift && io.MouseWheel!=0) ImGui::SetScrollY(MAX(0.0f,y-io.MouseWheel*60.0f*scale));
  if (horizontal!=0) {
    x=CLAMP(x-horizontal*60.0f*scale,0.0f,ImGui::GetScrollMaxX());
    ImGui::SetScrollX(x);
    return x;
  }
  return -1.0f;
}

void wheelHelp() {
  ImGui::SameLine();
  ImGui::TextDisabled("(?)");
  if (ImGui::IsItemHovered()) ImGui::SetTooltip(_("Wheel: scroll vertically\nShift + wheel: scroll horizontally\nCtrl / Cmd + wheel: horizontal zoom\nAlt / Option + wheel: vertical zoom"));
}
}

void FurnaceGUIHorizontal::buildFont(FurnaceGUI& g) {
  idFont=g.mainFont;
  if (g.safeMode || g.mainFont->Sources.empty()) return;
  const ImFontConfig& source=*g.mainFont->Sources[0];
  ImFontConfig config;
  config.FontDataOwnedByAtlas=false;
  config.FontNo=source.FontNo;
  config.OversampleH=1;
  config.OversampleV=1;
#ifdef HAVE_FREETYPE
  if (g.settings.fontBackend==1) {
    config.FontLoaderFlags=ImGuiFreeTypeLoaderFlags_Bold|ImGuiFreeTypeLoaderFlags_Oblique;
  }
#endif
  snprintf(config.Name,sizeof(config.Name),"Pattern IDs Bold Italic");
  // Reuse the UI font's existing data. FreeType styles its outlines before
  // rasterization, retaining smooth edges without embedding another font.
  ImFont* font=ImGui::GetIO().Fonts->AddFontFromMemoryTTF(source.FontData,source.FontDataSize,MAX(1.0f,g.settings.mainFontSize*g.dpiScale*1.5f),&config);
  if (font) idFont=font;
}

void FurnaceGUIHorizontal::stopPreview(FurnaceGUI& g) {
  if (preview<0) return;
  int note=preview;
  g.e->synchronized([&g,note]() { g.e->autoNoteOff(-1,note); });
  preview=-1;
}

void FurnaceGUIHorizontal::select(FurnaceGUI& g, int ord, int ch) {
  g.setOrder(ord);
  g.orderCursor=ch;
  g.curNibble=0;
  g.cursor.order=ord;
  g.cursor.xCoarse=ch;
  g.cursor.xFine=0;
  g.cursor.y=CLAMP(row,0,g.e->curSubSong->patLen-1);
  g.selStart=g.selEnd=g.cursor;
}

void FurnaceGUIHorizontal::open(FurnaceGUI& g, int ord, int ch) {
  stopPreview(g);
  order=ord;
  channel=ch;
  row=0;
  dragMode=0;
  scrollX=0;
  editorOpen=focusEditor=centerPitch=true;
  select(g,ord,ch);
  DivPattern* pat=g.e->curPat[ch].getPattern(g.e->curOrders->ord[ch][ord],false);
  pitch=CLAMP(g.curOctave*12+60,0,179);
  for (int r=0; r<g.e->curSubSong->patLen; r++) {
    if (pitched(pat->newData[r][DIV_PAT_NOTE])) {
      pitch=pat->newData[r][DIV_PAT_NOTE];
      break;
    }
  }
}

void FurnaceGUIHorizontal::orderMenu(FurnaceGUI& g) {
  if (ImGui::MenuItem(_("Add order"))) orderAction=GUI_ACTION_ORDERS_ADD;
  if (ImGui::MenuItem(_("Duplicate (shared patterns)"))) orderAction=GUI_ACTION_ORDERS_DUPLICATE;
  if (ImGui::MenuItem(_("Clone (independent patterns)"))) orderAction=GUI_ACTION_ORDERS_DEEP_CLONE;
  if (ImGui::MenuItem(_("Append shared copy"))) orderAction=GUI_ACTION_ORDERS_DUPLICATE_END;
  if (ImGui::MenuItem(_("Append independent clone"))) orderAction=GUI_ACTION_ORDERS_DEEP_CLONE_END;
  ImGui::Separator();
  if (ImGui::MenuItem(_("Move order left"),NULL,false,g.curOrder>0)) orderAction=GUI_ACTION_ORDERS_MOVE_UP;
  if (ImGui::MenuItem(_("Move order right"),NULL,false,g.curOrder+1<g.e->curSubSong->ordersLen)) orderAction=GUI_ACTION_ORDERS_MOVE_DOWN;
  if (ImGui::MenuItem(_("Remove order"),NULL,false,g.e->curSubSong->ordersLen>1)) orderAction=GUI_ACTION_ORDERS_REMOVE;
  ImGui::Separator();
  ImGui::MenuItem(_("Follow playback"),NULL,&g.followOrders);
  ImGui::MenuItem(_("Change pattern IDs across all channels"),NULL,&g.changeAllOrders);
}

void FurnaceGUIHorizontal::songView(FurnaceGUI& g) {
  if (g.nextWindow==GUI_WINDOW_PATTERN || g.nextWindow==GUI_WINDOW_ORDERS) {
    g.patternOpen=true;
    ImGui::SetNextWindowFocus();
    g.nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!g.patternOpen) return;
  if (ImGui::Begin("Pattern",&g.patternOpen,g.globalWinFlags|ImGuiWindowFlags_MenuBar|ImGuiWindowFlags_NoScrollWithMouse,_("Orders - Song"))) {
    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu(_("Orders"))) {
        orderMenu(g);
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu(_("Edit"))) {
        if (ImGui::MenuItem(_("Undo"),"Ctrl+Z")) g.doUndo();
        if (ImGui::MenuItem(_("Redo"),"Ctrl+Y")) g.doRedo();
        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();
    }
    if (ImGui::Button(g.e->isPlaying()?ICON_FA_STOP:ICON_FA_PLAY)) {
      if (g.e->isPlaying()) g.stop(); else g.play();
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_PLUS "##order")) orderAction=GUI_ACTION_ORDERS_ADD;
    if (ImGui::IsItemHovered()) ImGui::SetTooltip(_("Add order"));
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FILES_O "##order")) orderAction=GUI_ACTION_ORDERS_DUPLICATE;
    if (ImGui::IsItemHovered()) ImGui::SetTooltip(_("Duplicate order using shared patterns. Use Orders > Clone for an independent copy."));
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120.0f*g.dpiScale);
    ImGui::SliderFloat(_("Zoom"),&orderWidth,48.0f,480.0f,"%.0f");
    ImGui::SameLine();
    ImGui::TextDisabled(_("Double-click a brick to edit"));
    wheelHelp();

    const int count=g.e->curSubSong->ordersLen;
    const float labelWidth=132.0f*g.dpiScale;
    const float idSize=ImGui::GetFontSize()*1.5f;
    ImFont* labelFont=idFont?idFont:ImGui::GetFont();
    const float minimumHeight=MAX(idSize+4.0f*g.dpiScale,ImGui::GetFrameHeight())/g.dpiScale;
    orderHeight=MAX(orderHeight,minimumHeight);
    // Table defaults add margins around every cell. Bricks instead meet at the
    // one-pixel grid borders, including when the user changes track height.
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding,ImVec2(0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,ImVec2(0,0));
    if (ImGui::BeginTable("HorizontalOrders",count+1,ImGuiTableFlags_ScrollX|ImGuiTableFlags_ScrollY|ImGuiTableFlags_BordersInner|ImGuiTableFlags_SizingFixedFit)) {
      wheelNavigation(orderWidth,orderHeight,ImVec2(48,minimumHeight),ImVec2(480,160),ImVec2(labelWidth,ImGui::GetTextLineHeight()),g.dpiScale);
      const float width=orderWidth*g.dpiScale;
      const float laneHeight=MAX(orderHeight*g.dpiScale,ImGui::GetFrameHeight());
      const bool showPreview=laneHeight>=idSize+24.0f*g.dpiScale;
      ImGui::TableSetupColumn("Channel",ImGuiTableColumnFlags_WidthFixed,labelWidth);
      for (int o=0; o<count; o++) ImGui::TableSetupColumn("",ImGuiTableColumnFlags_WidthFixed,width);
      ImGui::TableSetupScrollFreeze(1,1);
      if (g.e->isPlaying() && g.followOrders && lastPlayOrder!=g.playOrder) {
        ImGui::SetScrollX(MAX(0.0f,g.playOrder*(width+ImGui::GetStyle().CellPadding.x*2)-width));
      }
      lastPlayOrder=g.e->isPlaying()?g.playOrder:-1;
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(_("Channels / Orders"));
      for (int o=0; o<count; o++) {
        if (!ImGui::TableNextColumn()) continue;
        ImGui::PushID(o);
        char label[32];
        snprintf(label,sizeof(label),g.settings.orderRowsBase?"%02X":"%d",o);
        if (ImGui::Selectable(label,g.curOrder==o)) select(g,o,CLAMP(g.orderCursor,0,g.e->getTotalChannelCount()-1));
        if (ImGui::BeginPopupContextItem("OrderActions")) {
          select(g,o,CLAMP(g.orderCursor,0,g.e->getTotalChannelCount()-1));
          orderMenu(g);
          ImGui::EndPopup();
        }
        ImGui::PopID();
      }
      for (int ch=0; ch<g.e->getTotalChannelCount(); ch++) {
        if (!g.e->curSubSong->chanShow[ch]) continue;
        ImGui::PushID(ch);
        ImGui::TableNextRow(0,laneHeight);
        ImGui::TableNextColumn();
        ImVec4 color=g.channelColor(ch);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY()+(laneHeight-ImGui::GetFrameHeight())*0.5f);
        if (ImGui::SmallButton(g.e->isChannelMuted(ch)?ICON_FA_VOLUME_OFF:ICON_FA_VOLUME_UP)) g.e->toggleMute(ch);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip(_("Mute / unmute channel"));
        ImGui::SameLine(0,4.0f*g.dpiScale);
        ImGui::PushStyleColor(ImGuiCol_Text,color);
        ImGui::TextUnformatted(g.e->getChannelName(ch));
        ImGui::PopStyleColor();
        for (int o=0; o<count; o++) {
          if (!ImGui::TableNextColumn()) continue;
          ImGui::PushID(o);
          int id=g.e->curOrders->ord[ch][o];
          DivPattern* pat=g.e->curPat[ch].getPattern(id,false);
          ImVec2 a=ImGui::GetCursorScreenPos();
          ImVec2 b(a.x+width,a.y+laneHeight);
          bool selected=g.curOrder==o && g.orderCursor==ch;
          ImGui::InvisibleButton("Brick",ImVec2(b.x-a.x,b.y-a.y));
          bool hovered=ImGui::IsItemHovered();
          if (ImGui::IsItemClicked()) {
            select(g,o,ch);
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) open(g,o,ch);
          }
          if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) select(g,o,ch);
          ImDrawList* dl=ImGui::GetWindowDrawList();
          dl->AddRectFilled(a,b,tint(color,g.e->isChannelMuted(ch)?0.10f:(hovered?0.36f:0.22f)));
          dl->AddRect(a,b,selected?ImGui::GetColorU32(ImGuiCol_Text):tint(color,0.65f),0,0,selected?2.0f*g.dpiScale:1.0f);
          dl->PushClipRect(a,b,true);
          int low=179, high=0;
          bool hasNotes=false, hasEvents=false;
          for (int r=0; showPreview && r<g.e->curSubSong->patLen; r++) {
            int n=pat->newData[r][DIV_PAT_NOTE];
            if (pitched(n)) { low=MIN(low,n); high=MAX(high,n); hasNotes=true; }
            for (int c=0; c<DIV_PAT_FX(g.e->curPat[ch].effectCols); c++) {
              if (pat->newData[r][c]!=-1) hasEvents=true;
            }
          }
          if (high-low<12) { int mid=(high+low)/2; low=mid-6; high=mid+6; }
          float px=(width-12.0f*g.dpiScale)/g.e->curSubSong->patLen;
          for (int r=0; showPreview && r<g.e->curSubSong->patLen; r++) {
            int n=pat->newData[r][DIV_PAT_NOTE];
            if (pitched(n)) {
              float x=a.x+6.0f*g.dpiScale+r*px;
              float y=b.y-5.0f*g.dpiScale-(n-low)*MAX(4.0f*g.dpiScale,laneHeight-idSize-8.0f*g.dpiScale)/MAX(12,high-low);
              dl->AddRectFilled(ImVec2(x,y),ImVec2(x+MAX(2.0f,(endRow(pat->newData,g.e->curSubSong->patLen,r)-r)*px-1.0f),y+2.0f*g.dpiScale),tint(color,0.9f));
            }
          }
          char idText[8];
          snprintf(idText,sizeof(idText),"%02X",id);
          ImVec2 idTextSize=labelFont->CalcTextSizeA(idSize,FLT_MAX,0,idText);
          dl->AddText(labelFont,idSize,ImVec2(b.x-idTextSize.x-4.0f*g.dpiScale,a.y+2.0f*g.dpiScale),ImGui::GetColorU32(ImGuiCol_Text),idText);
          if (showPreview && !pat->name.empty()) {
            dl->PushClipRect(a,ImVec2(b.x-idTextSize.x-6.0f*g.dpiScale,a.y+idSize+2.0f*g.dpiScale),true);
            dl->AddText(ImVec2(a.x+5.0f*g.dpiScale,a.y+3.0f*g.dpiScale),tint(color,1.0f),pat->name.c_str());
            dl->PopClipRect();
          }
          if (showPreview && !hasNotes && hasEvents) dl->AddText(ImVec2(a.x+6.0f*g.dpiScale,b.y-ImGui::GetTextLineHeight()-6.0f*g.dpiScale),tint(color,1.0f),_("Events / FX"));
          if (g.e->isPlaying() && g.playOrder==o) {
            float x=a.x+width*g.oldRow/g.e->curSubSong->patLen;
            dl->AddLine(ImVec2(x,a.y),ImVec2(x,b.y),ImGui::GetColorU32(ImGuiCol_Text),2.0f*g.dpiScale);
          }
          dl->PopClipRect();
          if (hovered) ImGui::SetTooltip(_("Order %02X / Pattern %02X\n%s\nDrag to another order on this channel to reuse the pattern.\nRight-click for pattern assignment and order actions."),o,id,pat->name.c_str());
          if (ImGui::BeginDragDropSource()) {
            PatternPayload payload={ch,id};
            ImGui::SetDragDropPayload("FURNACE_HORIZONTAL_PATTERN",&payload,sizeof(payload));
            ImGui::Text(_("Pattern %02X"),id);
            ImGui::EndDragDropSource();
          }
          if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload=ImGui::AcceptDragDropPayload("FURNACE_HORIZONTAL_PATTERN")) {
              const PatternPayload* pattern=(const PatternPayload*)payload->Data;
              if (pattern->channel==ch && pattern->pattern!=id) {
                g.prepareUndo(GUI_UNDO_CHANGE_ORDER);
                g.e->lockSave([&g,ch,o,pattern]() { g.e->curOrders->ord[ch][o]=pattern->pattern; });
                g.makeUndo(GUI_UNDO_CHANGE_ORDER);
              }
            }
            ImGui::EndDragDropTarget();
          }
          if (ImGui::BeginPopupContextItem("BrickActions")) {
            if (ImGui::MenuItem(_("Edit pattern"))) open(g,o,ch);
            ImGui::SetNextItemWidth(100.0f*g.dpiScale);
            if (hexInput(_("Pattern ID"),id,DIV_MAX_PATTERNS-1) && id>=0) {
              g.prepareUndo(GUI_UNDO_CHANGE_ORDER);
              g.e->lockSave([&g,ch,o,id]() {
                if (g.changeAllOrders) {
                  for (int c=0; c<g.e->getTotalChannelCount(); c++) g.e->curOrders->ord[c][o]=id;
                } else g.e->curOrders->ord[ch][o]=id;
              });
              g.makeUndo(GUI_UNDO_CHANGE_ORDER);
            }
            if (ImGui::MenuItem(_("Assign unused empty pattern"))) {
              bool used[DIV_MAX_PATTERNS]={};
              for (int i=0; i<count; i++) used[g.e->curOrders->ord[ch][i]]=true;
              int free=-1;
              for (int i=0; i<DIV_MAX_PATTERNS; i++) {
                if (!used[i] && g.e->curPat[ch].getPattern(i,false)->isEmpty()) { free=i; break; }
              }
              if (free<0) g.showError(_("No free patterns available on this channel."));
              else {
                g.prepareUndo(GUI_UNDO_CHANGE_ORDER);
                g.e->lockSave([&g,ch,o,free]() { g.e->curOrders->ord[ch][o]=free; });
                g.makeUndo(GUI_UNDO_CHANGE_ORDER);
              }
            }
            ImGui::Separator();
            orderMenu(g);
            ImGui::EndPopup();
          }
          ImGui::PopID();
        }
        ImGui::PopID();
      }
      ImGui::EndTable();
    }
    ImGui::PopStyleVar(2);
    if (orderAction) {
      g.doAction(orderAction);
      orderAction=0;
      dragMode=0;
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) g.curWindow=GUI_WINDOW_ORDERS;
  ImGui::End();
}

bool FurnaceGUIHorizontal::draw(FurnaceGUI& g) {
  if (!g.settings.horizontalUI || g.mobileUI) {
    stopPreview(g);
    editorOpen=false;
    dragMode=0;
    return false;
  }
  if (song!=g.e->curSubSong) {
    stopPreview(g);
    song=g.e->curSubSong;
    editorOpen=false;
    dragMode=0;
    lastPlayOrder=-1;
  }
  if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) stopPreview(g);
  if (g.e->getTotalChannelCount()<1 || g.e->curSubSong->ordersLen<1) return true;
  songView(g);
  if (editorOpen) patternView(g);
  else stopPreview(g);
  return true;
}

void FurnaceGUIHorizontal::setCell(FurnaceGUI& g, int col, int value) {
  UndoRegion region(order,channel,row,order,channel,row);
  g.prepareUndo(GUI_UNDO_PATTERN_EDIT,region);
  g.e->lockSave([&g,this,col,value]() {
    DivPattern* pat=g.e->curPat[channel].getPattern(g.e->curOrders->ord[channel][order],true);
    pat->newData[row][col]=value;
  });
  g.makeUndo(GUI_UNDO_PATTERN_EDIT,region);
}

void FurnaceGUIHorizontal::editNote(FurnaceGUI& g, int mode, int from, int to, int note, int end) {
  UndoRegion region(order,channel,0,order,channel,g.e->curSubSong->patLen-1);
  g.prepareUndo(GUI_UNDO_PATTERN_EDIT,region);
  bool accepted=false;
  g.e->lockSave([&]() {
    DivPattern* pat=g.e->curPat[channel].getPattern(g.e->curOrders->ord[channel][order],true);
    int instrument=g.curIns>=0 && g.curIns<(int)g.e->song.ins.size()?g.curIns:-1;
    accepted=FurnaceHorizontalPattern::edit(pat->newData,g.e->curSubSong->patLen,mode,from,to,note,end,instrument);
  });
  g.makeUndo(GUI_UNDO_PATTERN_EDIT,region);
  if (!accepted) g.showError(_("This edit overlaps a note or an instrument/volume event. Move it to an empty span or shorten it first."));
  else if (mode!=3) { row=to; pitch=note; }
}

void FurnaceGUIHorizontal::inspector(FurnaceGUI& g) {
  DivPattern* pat=g.e->curPat[channel].getPattern(g.e->curOrders->ord[channel][order],false);
  ImGui::SetNextItemWidth(95.0f*g.dpiScale);
  if (ImGui::InputInt(_("Row"),&row)) row=CLAMP(row,0,g.e->curSubSong->patLen-1);
  ImGui::SameLine();
  ImGui::SetNextItemWidth(115.0f*g.dpiScale);
  int note=pat->newData[row][DIV_PAT_NOTE];
  if (ImGui::BeginCombo(_("Event"),note>=180?eventLabel(note):g.noteNameNormal(note))) {
    const int values[]={-1,DIV_NOTE_OFF,DIV_NOTE_REL,DIV_MACRO_REL};
    const char* labels[]={_("Empty"),_("Note cut (OFF)"),_("Note release (REL)"),_("Macro release (MREL)")};
    for (int i=0; i<4; i++) if (ImGui::Selectable(labels[i],note==values[i])) setCell(g,DIV_PAT_NOTE,values[i]);
    ImGui::EndCombo();
  }
  ImGui::SameLine();
  ImGui::SetNextItemWidth(55.0f*g.dpiScale);
  int instrument=pat->newData[row][DIV_PAT_INS];
  if (hexInput(_("Ins"),instrument,255)) setCell(g,DIV_PAT_INS,instrument);
  ImGui::SameLine();
  ImGui::SetNextItemWidth(55.0f*g.dpiScale);
  int volume=pat->newData[row][DIV_PAT_VOL];
  if (hexInput(_("Vol"),volume,g.e->getMaxVolumeChan(channel))) setCell(g,DIV_PAT_VOL,volume);
  ImGui::SameLine();
  if (ImGui::SmallButton(_("Effects reference"))) g.nextWindow=GUI_WINDOW_EFFECT_LIST;
  if (ImGui::BeginTable("RowEffects",4,ImGuiTableFlags_SizingStretchSame)) {
    for (int fx=0; fx<g.e->curPat[channel].effectCols; fx++) {
      ImGui::TableNextColumn();
      ImGui::PushID(fx);
      ImGui::Text("FX %d",fx+1);
      ImGui::SameLine();
      int command=pat->newData[row][DIV_PAT_FX(fx)];
      int value=pat->newData[row][DIV_PAT_FXVAL(fx)];
      ImGui::SetNextItemWidth(45.0f*g.dpiScale);
      if (hexInput("##command",command,255)) setCell(g,DIV_PAT_FX(fx),command);
      if (ImGui::IsItemHovered()) ImGui::SetTooltip(_("Effect command (hex). Enter to apply; erase and Enter to clear."));
      ImGui::SameLine();
      ImGui::SetNextItemWidth(45.0f*g.dpiScale);
      if (hexInput("##value",value,255)) setCell(g,DIV_PAT_FXVAL(fx),value);
      if (ImGui::IsItemHovered()) ImGui::SetTooltip(_("Effect value (hex). Enter to apply; erase and Enter to clear."));
      ImGui::PopID();
    }
    ImGui::EndTable();
  }
}

void FurnaceGUIHorizontal::pianoRoll(FurnaceGUI& g, float height) {
  const int rows=g.e->curSubSong->patLen;
  const float keyboard=76.0f*g.dpiScale;
  const float ruler=ImGui::GetTextLineHeight()+6.0f*g.dpiScale;
  const float eventHeight=3*(ImGui::GetTextLineHeight()+7.0f*g.dpiScale);
  ImGui::SetNextWindowScroll(ImVec2(scrollX,-1));
  if (ImGui::BeginChild("HorizontalPiano",ImVec2(0,height),true,ImGuiWindowFlags_HorizontalScrollbar|ImGuiWindowFlags_NoNavInputs|ImGuiWindowFlags_NoScrollWithMouse)) {
    float pendingX=wheelNavigation(rowWidth,keyHeight,ImVec2(8,6),ImVec2(96,40),ImVec2(keyboard,ruler),g.dpiScale);
    const float step=rowWidth*g.dpiScale;
    const float key=keyHeight*g.dpiScale;
    if (centerPitch) {
      ImGui::SetScrollY(MAX(0.0f,(179-pitch)*key-(height-eventHeight)*0.5f));
      centerPitch=false;
    }
    ImVec2 origin=ImGui::GetCursorScreenPos();
    ImVec2 view(origin.x+ImGui::GetScrollX(),origin.y+ImGui::GetScrollY());
    ImVec2 limit(view.x+ImGui::GetWindowContentRegionMax().x-ImGui::GetWindowContentRegionMin().x,
                 view.y+ImGui::GetWindowContentRegionMax().y-ImGui::GetWindowContentRegionMin().y);
    ImVec2 eventLimit=limit;
    limit.y-=eventHeight;
    ImGui::InvisibleButton("RollCanvas",ImVec2(keyboard+rows*step,ruler+180*key+eventHeight),ImGuiButtonFlags_MouseButtonLeft|ImGuiButtonFlags_MouseButtonRight);
    bool hovered=ImGui::IsItemHovered();
    ImVec2 mouse=ImGui::GetMousePos();
    int mouseRow=CLAMP((int)floor((mouse.x-origin.x-keyboard)/step),0,rows-1);
    int mousePitch=CLAMP(179-(int)floor((mouse.y-origin.y-ruler)/key),0,179);
    int snapped=mouseRow/snap*snap;
    bool inGrid=hovered && mouse.x>=view.x+keyboard && mouse.y>=view.y+ruler && mouse.y<limit.y;
    DivPattern* pat=g.e->curPat[channel].getPattern(g.e->curOrders->ord[channel][order],false);
    ImDrawList* dl=ImGui::GetWindowDrawList();
    ImVec4 color=g.channelColor(channel);
    int hit=-1;
    int firstPitch=CLAMP(179-(int)((limit.y-origin.y-ruler)/key),0,179);
    int lastPitch=CLAMP(179-(int)((view.y-origin.y-ruler)/key),0,179);
    dl->PushClipRect(ImVec2(view.x+keyboard,view.y+ruler),limit,true);
    for (int n=firstPitch; n<=lastPitch; n++) {
      float y=origin.y+ruler+(179-n)*key;
      dl->AddRectFilled(ImVec2(view.x+keyboard,y),ImVec2(limit.x,y+key),ImGui::GetColorU32(blackKey(n)?ImGuiCol_FrameBg:ImGuiCol_WindowBg));
      dl->AddLine(ImVec2(view.x+keyboard,y+key),ImVec2(limit.x,y+key),ImGui::GetColorU32(ImGuiCol_Border));
    }
    int firstRow=CLAMP((int)(ImGui::GetScrollX()/step),0,rows-1);
    int lastRow=CLAMP((int)((limit.x-origin.x-keyboard)/step)+1,0,rows);
    int beat=MAX(1,g.e->curSubSong->hilightA);
    for (int r=firstRow; r<=lastRow; r++) {
      float x=origin.x+keyboard+r*step;
      dl->AddLine(ImVec2(x,view.y+ruler),ImVec2(x,limit.y),ImGui::GetColorU32(r%beat?ImGuiCol_Border:ImGuiCol_Separator),r%beat?1.0f:2.0f);
    }
    for (int r=0; r<rows; r++) {
      int note=pat->newData[r][DIV_PAT_NOTE];
      if (note<0 || eventLane(note)>=0) continue;
      bool special=!pitched(note);
      int displayNote=eventPitch(pat->newData,rows,r,pitch);
      int end=endRow(pat->newData,rows,r);
      float x=origin.x+keyboard+r*step;
      float y=origin.y+ruler+(179-displayNote)*key;
      if (inGrid && mousePitch==displayNote && mouseRow>=r && mouseRow<end) hit=r;
      if (end<firstRow || r>lastRow || displayNote<firstPitch || displayNote>lastPitch) continue;
      ImVec2 a(x+1.0f,y+1.0f), b(origin.x+keyboard+end*step-1.0f,y+key-1.0f);
      if (special) {
        dl->AddRect(a,b,r==row?ImGui::GetColorU32(ImGuiCol_Text):tint(color,1.0f),2.0f*g.dpiScale,0,2.0f*g.dpiScale);
        const char* label=eventLabel(note);
        float size=MIN(ImGui::GetFontSize(),MIN(key-4.0f*g.dpiScale,(b.x-a.x-4.0f*g.dpiScale)/ImGui::CalcTextSize(label).x*ImGui::GetFontSize()));
        if (size>0) {
          dl->PushClipRect(a,b,true);
          dl->AddText(ImGui::GetFont(),size,ImVec2(a.x+2.0f*g.dpiScale,a.y+(b.y-a.y-size)*0.5f),ImGui::GetColorU32(ImGuiCol_Text),label);
          dl->PopClipRect();
        }
      } else {
        dl->AddRectFilled(a,b,tint(color,r==row?0.95f:0.65f),2.0f*g.dpiScale);
        dl->AddRect(a,b,ImGui::GetColorU32(r==row?ImGuiCol_Text:ImGuiCol_Border),2.0f*g.dpiScale);
      }
      if (!special && b.x-a.x>40.0f*g.dpiScale && key>=ImGui::GetTextLineHeight()) {
        dl->PushClipRect(a,b,true);
        dl->AddText(ImVec2(a.x+3*g.dpiScale,a.y),ImGui::GetColorU32(ImGuiCol_Text),g.noteNameNormal(note));
        dl->PopClipRect();
      }
    }
    if (dragMode) {
      float x=origin.x+keyboard+dragTargetRow*step;
      float y=origin.y+ruler+(179-dragTargetPitch)*key;
      dl->AddRect(ImVec2(x,y),ImVec2(origin.x+keyboard+dragTargetEnd*step,y+key),ImGui::GetColorU32(ImGuiCol_Text),2.0f*g.dpiScale,0,2.0f*g.dpiScale);
    }
    if (g.e->isPlaying() && g.playOrder==order) {
      float x=origin.x+keyboard+g.oldRow*step;
      dl->AddLine(ImVec2(x,view.y+ruler),ImVec2(x,limit.y),ImGui::GetColorU32(ImGuiCol_Text),2.0f*g.dpiScale);
    }
    dl->PopClipRect();

    // Keyboard and ruler stay pinned while the notes scroll underneath them.
    dl->PushClipRect(ImVec2(view.x,view.y+ruler),ImVec2(view.x+keyboard,limit.y),true);
    for (int n=firstPitch; n<=lastPitch; n++) {
      float y=origin.y+ruler+(179-n)*key;
      ImVec2 a(view.x,y), b(view.x+keyboard-2*g.dpiScale,y+key);
      bool black=blackKey(n);
      dl->AddRectFilled(a,b,IM_COL32(216,220,224,255));
      ImVec2 keyEnd(black?a.x+keyboard*0.72f:b.x,b.y);
      dl->AddRectFilled(a,keyEnd,n==preview?tint(color,1.0f):(black?IM_COL32(38,41,47,255):IM_COL32(216,220,224,255)));
      dl->AddRect(a,b,IM_COL32(70,74,80,255));
      if (key>=ImGui::GetTextLineHeight() || n%12==0) {
        dl->PushClipRect(a,b,true);
        dl->AddText(ImGui::GetFont(),MIN(ImGui::GetFontSize(),key),ImVec2(a.x+5*g.dpiScale,y),black?IM_COL32(210,215,220,255):IM_COL32(32,35,40,255),g.noteNameNormal(n));
        dl->PopClipRect();
      }
    }
    dl->PopClipRect();
    dl->PushClipRect(view,ImVec2(limit.x,view.y+ruler),true);
    dl->AddRectFilled(view,ImVec2(limit.x,view.y+ruler),ImGui::GetColorU32(ImGuiCol_WindowBg));
    for (int r=firstRow; r<lastRow; r++) {
      if (r%beat && step<30*g.dpiScale) continue;
      float x=origin.x+keyboard+r*step;
      if (x<view.x+keyboard) continue;
      char label[12]; snprintf(label,sizeof(label),g.settings.orderRowsBase?"%02X":"%d",r);
      dl->AddText(ImVec2(x+2*g.dpiScale,view.y+2*g.dpiScale),ImGui::GetColorU32(ImGuiCol_Text),label);
    }
    dl->PopClipRect();

    if (hovered && mouse.x<view.x+keyboard && mouse.y>=view.y+ruler && mouse.y<limit.y && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
      stopPreview(g);
      preview=pitch=mousePitch;
      g.previewNote(channel,preview);
    }
    if (hovered && mouse.x>=view.x+keyboard && mouse.y<view.y+ruler && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) row=mouseRow;
    if (inGrid && !dragMode) {
      if (hit>=0 && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        row=hit;
        if (pitched(pat->newData[hit][DIV_PAT_NOTE])) editNote(g,3,hit,0,0,0);
        else setCell(g,DIV_PAT_NOTE,-1);
      } else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        if (hit>=0 && !pitched(pat->newData[hit][DIV_PAT_NOTE])) {
          // Unpitched events can be selected/erased, but dragging them through
          // the grid must not accidentally turn them into ordinary notes.
          row=hit;
        } else if (hit>=0) {
          row=dragRow=hit;
          pitch=dragPitch=pat->newData[hit][DIV_PAT_NOTE];
          dragEnd=endRow(pat->newData,rows,hit);
          float edge=origin.x+keyboard+dragEnd*step;
          dragMode=mouse.x>edge-MIN(step*0.35f,7.0f*g.dpiScale)?3:2;
        } else {
          row=dragRow=snapped;
          pitch=dragPitch=mousePitch;
          dragEnd=MIN(rows,dragRow+length);
          dragMode=1;
        }
        if (dragMode) {
          dragMouseRow=mouseRow;
          dragTargetRow=dragRow;
          dragTargetPitch=dragPitch;
          dragTargetEnd=dragEnd;
          dragPattern=g.e->curOrders->ord[channel][order];
        }
      }
    }
    if (dragMode) {
      if (dragPattern!=g.e->curOrders->ord[channel][order] || ImGui::IsKeyPressed(ImGuiKey_Escape)) dragMode=0;
      else if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        if (dragMode==2) {
          int delta=(mouseRow-dragMouseRow)/snap*snap;
          dragTargetRow=CLAMP(dragRow+delta,0,rows-(dragEnd-dragRow));
          dragTargetEnd=dragTargetRow+dragEnd-dragRow;
          dragTargetPitch=mousePitch;
        } else {
          dragTargetEnd=CLAMP((mouseRow/snap+1)*snap,dragRow+1,rows);
        }
      }
      if (dragMode && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        if (dragMode==1 || dragTargetRow!=dragRow || dragTargetPitch!=dragPitch || dragTargetEnd!=dragEnd) {
          editNote(g,dragMode==1?1:2,dragRow,dragTargetRow,dragTargetPitch,dragTargetEnd);
        }
        dragMode=0;
      }
    }
    if (inGrid && hit>=0 && !dragMode) {
      if (pitched(pat->newData[hit][DIV_PAT_NOTE])) {
        float edge=origin.x+keyboard+endRow(pat->newData,rows,hit)*step;
        ImGui::SetMouseCursor(mouse.x>edge-MIN(step*0.35f,7.0f*g.dpiScale)?ImGuiMouseCursor_ResizeEW:ImGuiMouseCursor_ResizeAll);
      } else {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        ImGui::SetTooltip(_("%s at row %02X\nClick to select; right-click to clear.\nVertical position uses the saved pitch or nearby note."),eventLabel(pat->newData[hit][DIV_PAT_NOTE]),hit);
      }
    }
    noteEventRows(g,origin,ImVec2(view.x,limit.y),eventLimit,hovered && !dragMode);
    scrollX=pendingX>=0?pendingX:ImGui::GetScrollX();
  }
  ImGui::EndChild();
}

// A fixed footer shares the piano roll's time axis, while pitch scrolling and
// zooming leave the spreadsheet-style row headings and event heights in place.
void FurnaceGUIHorizontal::noteEventRows(FurnaceGUI& g, ImVec2 origin, ImVec2 view, ImVec2 limit, bool hovered) {
  const int types[]={DIV_NOTE_OFF,DIV_NOTE_REL,DIV_MACRO_REL};
  const int rows=g.e->curSubSong->patLen;
  const float keyboard=76.0f*g.dpiScale;
  const float step=rowWidth*g.dpiScale;
  const float lane=(limit.y-view.y)/3;
  const int beat=MAX(1,g.e->curSubSong->hilightA);
  const int first=CLAMP((int)(ImGui::GetScrollX()/step),0,rows-1);
  const int last=CLAMP((int)((limit.x-origin.x-keyboard)/step)+1,0,rows);
  ImDrawList* dl=ImGui::GetWindowDrawList();
  DivPattern* pat=g.e->curPat[channel].getPattern(g.e->curOrders->ord[channel][order],false);
  ImVec4 color=g.channelColor(channel);
  dl->PushClipRect(view,limit,true);
  dl->AddRectFilled(view,limit,ImGui::GetColorU32(ImGuiCol_WindowBg));
  for (int l=0; l<3; l++) {
    float y=view.y+l*lane;
    dl->AddRectFilled(ImVec2(view.x,y),ImVec2(view.x+keyboard,y+lane),ImGui::GetColorU32(ImGuiCol_Header));
    dl->AddText(ImVec2(view.x+5*g.dpiScale,y+3*g.dpiScale),ImGui::GetColorU32(ImGuiCol_Text),eventLabel(types[l]));
    dl->AddLine(ImVec2(view.x,y),ImVec2(limit.x,y),ImGui::GetColorU32(ImGuiCol_Separator));
  }
  dl->AddLine(ImVec2(view.x+keyboard,view.y),ImVec2(view.x+keyboard,limit.y),ImGui::GetColorU32(ImGuiCol_Separator),2*g.dpiScale);
  dl->PushClipRect(ImVec2(view.x+keyboard,view.y),limit,true);
  for (int r=first; r<=last; r++) {
    float x=origin.x+keyboard+r*step;
    dl->AddLine(ImVec2(x,view.y),ImVec2(x,limit.y),ImGui::GetColorU32(r%beat?ImGuiCol_Border:ImGuiCol_Separator),r%beat?1.0f:2.0f);
  }
  ImVec2 mouse=ImGui::GetMousePos();
  int mouseRow=(int)floor((mouse.x-origin.x-keyboard)/step);
  int mouseLane=(int)floor((mouse.y-view.y)/lane);
  bool inGrid=hovered && mouse.x>=view.x+keyboard && mouse.x<limit.x && mouse.y>=view.y && mouse.y<limit.y && mouseRow>=0 && mouseRow<rows;
  int hit=-1;
  for (int r=0; r<rows; r++) {
    int l=eventLane(pat->newData[r][DIV_PAT_NOTE]);
    if (l<0) continue;
    int end=endRow(pat->newData,rows,r);
    if (inGrid && mouseLane==l && mouseRow>=r && mouseRow<end) hit=r;
    if (end<first || r>last) continue;
    ImVec2 a(origin.x+keyboard+r*step+2*g.dpiScale,view.y+l*lane+2*g.dpiScale);
    ImVec2 b(origin.x+keyboard+end*step-2*g.dpiScale,view.y+(l+1)*lane-2*g.dpiScale);
    dl->AddRect(a,b,r==row?ImGui::GetColorU32(ImGuiCol_Text):tint(color,1.0f),2*g.dpiScale,0,2*g.dpiScale);
    const char* label=eventLabel(types[l]);
    if (b.x-a.x>ImGui::CalcTextSize(label).x+4*g.dpiScale) {
      dl->AddText(ImVec2(a.x+2*g.dpiScale,a.y+1*g.dpiScale),ImGui::GetColorU32(ImGuiCol_Text),label);
    }
  }
  if (g.e->isPlaying() && g.playOrder==order) {
    float x=origin.x+keyboard+g.oldRow*step;
    dl->AddLine(ImVec2(x,view.y),ImVec2(x,limit.y),ImGui::GetColorU32(ImGuiCol_Text),2*g.dpiScale);
  }
  dl->PopClipRect();
  dl->PopClipRect();
  if (inGrid) {
    int target=hit>=0?hit:mouseRow/snap*snap;
    int note=pat->newData[target][DIV_PAT_NOTE];
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
      row=target;
      // These lanes share Furnace's note column. Do not silently replace an
      // attack or raw frequency; replacing another cut/release is intentional.
      if (note==-1 || eventLane(note)>=0) {
        if (note!=types[mouseLane]) setCell(g,DIV_PAT_NOTE,types[mouseLane]);
      } else g.showError(_("A note already starts on this row. Place the event on another row or clear the note first."));
    }
    if (hit>=0 && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
      row=hit;
      setCell(g,DIV_PAT_NOTE,-1);
    }
    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    ImGui::SetTooltip(_("%s at row %02X\nClick to draw/select; right-click a block to clear.\nSnap applies when drawing."),eventLabel(types[mouseLane]),target);
  }
}

void FurnaceGUIHorizontal::eventLanes(FurnaceGUI& g, float height) {
  const int rows=g.e->curSubSong->patLen;
  const float keyboard=76.0f*g.dpiScale;
  const int effects=g.e->curPat[channel].effectCols;
  ImGui::SetNextWindowScroll(ImVec2(scrollX,-1));
  if (ImGui::BeginChild("HorizontalEvents",ImVec2(0,height),true,ImGuiWindowFlags_HorizontalScrollbar|ImGuiWindowFlags_NoNavInputs|ImGuiWindowFlags_NoScrollWithMouse)) {
    float pendingX=wheelNavigation(rowWidth,eventZoom,ImVec2(8,1),ImVec2(96,3),ImVec2(keyboard,0),g.dpiScale);
    const float step=rowWidth*g.dpiScale;
    const float lane=(ImGui::GetTextLineHeight()+7.0f*g.dpiScale)*eventZoom;
    const float fxLane=lane+ImGui::GetTextLineHeight()*eventZoom;
    ImVec2 origin=ImGui::GetCursorScreenPos();
    ImVec2 view(origin.x+ImGui::GetScrollX(),origin.y+ImGui::GetScrollY());
    ImVec2 limit(view.x+ImGui::GetWindowContentRegionMax().x-ImGui::GetWindowContentRegionMin().x,
                 view.y+ImGui::GetWindowContentRegionMax().y-ImGui::GetWindowContentRegionMin().y);
    ImGui::InvisibleButton("EventCanvas",ImVec2(keyboard+rows*step,2*lane+effects*fxLane));
    DivPattern* pat=g.e->curPat[channel].getPattern(g.e->curOrders->ord[channel][order],false);
    ImDrawList* dl=ImGui::GetWindowDrawList();
    int first=CLAMP((int)(ImGui::GetScrollX()/step),0,rows-1);
    int last=CLAMP((int)((limit.x-origin.x-keyboard)/step)+1,0,rows);
    dl->PushClipRect(ImVec2(view.x+keyboard,view.y),limit,true);
    for (int r=first; r<last; r++) {
      float x=origin.x+keyboard+r*step;
      if (r==row) dl->AddRectFilled(ImVec2(x,view.y),ImVec2(x+step,limit.y),tint(g.channelColor(channel),0.25f));
      dl->AddLine(ImVec2(x,view.y),ImVec2(x,limit.y),ImGui::GetColorU32(ImGuiCol_Border));
      for (int l=0; l<effects+2; l++) {
        float y=origin.y+(l<2?l*lane:2*lane+(l-2)*fxLane);
        char text[24]="";
        int value=pat->newData[r][l<2?l+1:DIV_PAT_FX(l-2)];
        if (l<2) {
          if (value>=0) snprintf(text,sizeof(text),"%02X",value);
        } else {
          int param=pat->newData[r][DIV_PAT_FXVAL(l-2)];
          if (value>=0 || param>=0) {
            char cmd[8]="..", val[8]="..";
            if (value>=0) snprintf(cmd,sizeof(cmd),"%02X",value);
            if (param>=0) snprintf(val,sizeof(val),"%02X",param);
            snprintf(text,sizeof(text),"%s\n%s",cmd,val);
          }
        }
        if (*text) {
          float size=MIN(ImGui::GetFontSize(),(step-3*g.dpiScale)/MAX(1.0f,ImGui::CalcTextSize(text).x)*ImGui::GetFontSize());
          dl->AddText(ImGui::GetFont(),size,ImVec2(x+2*g.dpiScale,y+3*g.dpiScale),ImGui::GetColorU32(ImGuiCol_Text),text);
        }
      }
    }
    dl->PopClipRect();
    dl->PushClipRect(view,ImVec2(view.x+keyboard,limit.y),true);
    dl->AddRectFilled(view,ImVec2(view.x+keyboard,limit.y),ImGui::GetColorU32(ImGuiCol_WindowBg));
    for (int l=0; l<effects+2; l++) {
      float y=origin.y+(l<2?l*lane:2*lane+(l-2)*fxLane);
      char label[32];
      if (l==0) snprintf(label,sizeof(label),"%s",_("Ins"));
      else if (l==1) snprintf(label,sizeof(label),"%s",_("Vol"));
      else snprintf(label,sizeof(label),"FX %d",l-1);
      dl->AddText(ImVec2(view.x+3*g.dpiScale,y+3*g.dpiScale),ImGui::GetColorU32(ImGuiCol_Text),label);
    }
    dl->PopClipRect();
    if (ImGui::IsItemHovered() && ImGui::GetMousePos().x>=view.x+keyboard) {
      int r=CLAMP((int)((ImGui::GetMousePos().x-origin.x-keyboard)/step),0,rows-1);
      float localY=ImGui::GetMousePos().y-origin.y;
      int l=CLAMP(localY<2*lane?(int)(localY/lane):2+(int)((localY-2*lane)/fxLane),0,effects+1);
      if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) row=r;
      if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        row=r;
        if (l<2) setCell(g,l+1,-1);
        else {
          // Clear command and parameter in one undo step.
          UndoRegion region(order,channel,row,order,channel,row);
          g.prepareUndo(GUI_UNDO_PATTERN_EDIT,region);
          g.e->lockSave([&]() {
            DivPattern* target=g.e->curPat[channel].getPattern(g.e->curOrders->ord[channel][order],true);
            target->newData[row][DIV_PAT_FX(l-2)]=-1;
            target->newData[row][DIV_PAT_FXVAL(l-2)]=-1;
          });
          g.makeUndo(GUI_UNDO_PATTERN_EDIT,region);
        }
      }
      ImGui::SetTooltip(_("Row %02X: select to edit below; right-click to clear this event."),r);
    }
    scrollX=pendingX>=0?pendingX:ImGui::GetScrollX();
  }
  ImGui::EndChild();
}

void FurnaceGUIHorizontal::patternView(FurnaceGUI& g) {
  channel=CLAMP(channel,0,g.e->getTotalChannelCount()-1);
  order=CLAMP(order,0,g.e->curSubSong->ordersLen-1);
  row=CLAMP(row,0,g.e->curSubSong->patLen-1);
  ImGui::SetNextWindowSize(ImVec2(1000*g.dpiScale,760*g.dpiScale),ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSizeConstraints(ImVec2(650*g.dpiScale,520*g.dpiScale),ImVec2(FLT_MAX,FLT_MAX));
  if (focusEditor) {
    ImGui::SetNextWindowFocus();
    ImGui::SetNextWindowCollapsed(false);
    focusEditor=false;
  }
  ImVec4 background=ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
  background.w=1.0f;
  ImGui::PushStyleColor(ImGuiCol_WindowBg,background);
  if (ImGui::Begin("HorizontalPattern",&editorOpen,g.globalWinFlags|ImGuiWindowFlags_NoScrollWithMouse|(g.settings.allowEditDocking?0:ImGuiWindowFlags_NoDocking),_("Pattern - Piano Roll"))) {
    ImGui::SetNextItemWidth(200*g.dpiScale);
    if (ImGui::BeginCombo(_("Channel"),g.e->getChannelName(channel))) {
      for (int ch=0; ch<g.e->getTotalChannelCount(); ch++) {
        ImGui::PushID(ch);
        if (ImGui::Selectable(g.e->getChannelName(ch),channel==ch)) open(g,order,ch);
        ImGui::PopID();
      }
      ImGui::EndCombo();
    }
    ImGui::SameLine();
    ImGui::Text(_("Order %02X / Pattern %02X"),order,g.e->curOrders->ord[channel][order]);
    ImGui::SameLine();
    if (ImGui::SmallButton(_("Undo"))) { dragMode=0; g.doUndo(); }
    ImGui::SameLine();
    if (ImGui::SmallButton(_("Redo"))) { dragMode=0; g.doRedo(); }
    ImGui::SameLine();
    if (ImGui::SmallButton(_("Close"))) { editorOpen=false; dragMode=0; }
    int uses=0;
    for (int o=0; o<g.e->curSubSong->ordersLen; o++) if (g.e->curOrders->ord[channel][o]==g.e->curOrders->ord[channel][order]) uses++;
    ImGui::TextDisabled(_("Used by %d order(s). Drag to draw/move; drag right edge to resize; right-click to erase."),uses);
    wheelHelp();
    ImGui::SetNextItemWidth(100*g.dpiScale);
    if (ImGui::InputInt(_("Length"),&length)) length=CLAMP(length,1,g.e->curSubSong->patLen);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(75*g.dpiScale);
    if (ImGui::InputInt(_("Snap"),&snap)) snap=CLAMP(snap,1,16);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100*g.dpiScale);
    ImGui::SliderFloat(_("Time zoom"),&rowWidth,8,96,"%.0f");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(80*g.dpiScale);
    int effects=g.e->curPat[channel].effectCols;
    if (ImGui::InputInt(_("FX lanes"),&effects)) {
      g.e->lockSave([&]() { g.e->curPat[channel].effectCols=CLAMP(effects,1,DIV_MAX_EFFECTS); });
      g.modified=true;
    }
    float inspectorHeight=(g.e->curPat[channel].effectCols>4?4:3)*ImGui::GetFrameHeightWithSpacing()+12*g.dpiScale;
    float available=ImGui::GetContentRegionAvail().y-inspectorHeight;
    float eventsHeight=MIN(125*g.dpiScale,available*0.27f);
    pianoRoll(g,MAX(100*g.dpiScale+3*(ImGui::GetTextLineHeight()+7*g.dpiScale),available-eventsHeight-ImGui::GetStyle().ItemSpacing.y));
    eventLanes(g,MAX(50*g.dpiScale,eventsHeight));
    inspector(g);
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows) && !ImGui::GetIO().WantTextInput && !dragMode) {
      // The roll has its own navigation. Do not route tracker typing here.
      if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) row=MAX(0,row-snap);
      if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) row=MIN(g.e->curSubSong->patLen-1,row+snap);
      if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
        DivPattern* pat=g.e->curPat[channel].getPattern(g.e->curOrders->ord[channel][order],false);
        if (pitched(pat->newData[row][DIV_PAT_NOTE])) editNote(g,3,row,0,0,0);
        else setCell(g,DIV_PAT_NOTE,-1);
      }
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) g.curWindow=GUI_WINDOW_NOTHING;
  ImGui::End();
  ImGui::PopStyleColor();
}
