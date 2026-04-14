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

#include "newSettings.h"
#include "gui.h"

SettingEntry::SettingEntry(const char* n, std::function<bool(void)> f) {
  name=n;
  drawFunction=f;
}

SettingEntry::SettingEntry(const SettingEntry& s) {
  name=s.name;
  drawFunction=s.drawFunction;
}

SettingsCategory::SettingsCategory():
  name(NULL),
  settings({}),
  children({}),
  scrollPos(0.0f) {}

SettingsCategory::SettingsCategory(const char* n, std::initializer_list<SettingEntry> s, std::initializer_list<SettingsCategory> c) {
  name=n;
  settings=s;
  children=c;
  scrollPos=0.0f;
}

SettingsCategory::SettingsCategory(const SettingsCategory& s) {
  name=s.name;
  settings=s.settings;
  children=s.children;
  scrollPos=s.scrollPos;
}

bool SettingsCategory::drawSettings(ImGuiTextFilter* filter, bool doFilter) {
  // get Y position for scroll
  if (!(filter->IsActive() && doFilter)) scrollPos=ImGui::GetCursorPosY();
  // check whether to draw the name
  // if no settings in the category match the filter, then no
  bool drawOwnSettings=true;
  if (filter->IsActive() && doFilter) {
    // if there are no settings in the category, then no
    if (settings.empty()) drawOwnSettings=false;
    // otherwise...
    bool isCategoryEmpty=true;
    for (SettingEntry& s:settings) {
      if (filter->PassFilter(_(s.name))) {
        isCategoryEmpty=false;
        break;
      }
    }
    if (isCategoryEmpty) drawOwnSettings=false;
  }
  bool ret=false;
  if (drawOwnSettings) {
    ImGui::SeparatorText(_(name));
    // ImGui::SameLine();
    // ImGui::TextColored(ImVec4(1.0f,.5f,.5f,1.f), "%f",scrollPos);
    for (SettingEntry& s:settings) {
      if (filter->IsActive() && doFilter) {
        if (!filter->PassFilter(_(s.name))) continue;
      }
      if (s.drawFunction()) ret=true;
    }
  }
  ImGui::Indent();
    for (SettingsCategory& c:children) {
      if (c.drawSettings(filter,doFilter)) ret=true;
    }
  ImGui::Unindent();
  return ret;
}

bool SettingsCategory::categoryPassFilterRecursive(ImGuiTextFilter* filter) {
  if (filter->PassFilter(_(name))) return true;
  for (SettingsCategory& c:children) {
    if (c.categoryPassFilterRecursive(filter)) return true;
  }
  return false;
}

bool SettingsCategory::drawSidebar(ImGuiTextFilter* filter, float* targetScrollPos) {
  bool ret=false;
  if (children.empty()) {
    if (filter->IsActive()) {
      if (!filter->PassFilter(_(name))) return false;
    }
    ImGui::Selectable(_(name)); // should i use TreeNode with ImGuiTreeNodeFlags_Leaf?
    if (ImGui::IsItemClicked()) {
      *targetScrollPos=scrollPos;
      return true;
    }
  } else {
    bool popDisabled=false;
    ImGuiTreeNodeFlags treeFlags=ImGuiTreeNodeFlags_None;
    if (filter->IsActive()) {
      if (!filter->PassFilter(_(name))) {
        if (!categoryPassFilterRecursive(filter)) return false;
        else popDisabled=true;
      }
      treeFlags|=ImGuiTreeNodeFlags_DefaultOpen;
    }
    ImGui::BeginDisabled(popDisabled);
    popDisabled=true;
    if (ImGui::TreeNodeEx(_(name), treeFlags)) {
      ImGui::EndDisabled();
      popDisabled=false;
      for (SettingsCategory& c:children)
        ret=c.drawSidebar(filter, targetScrollPos);
      ImGui::TreePop();
    }
    if (popDisabled) ImGui::EndDisabled();
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) { // hack because IsItemClicked returns true when anywhere inside the node and on the rising edge
      if (*targetScrollPos==-1.0f) *targetScrollPos=scrollPos;
      ret=true;
    }
  }
  return ret;
}

SettingsCategory::~SettingsCategory() {
  settings.clear();
  children.clear();
}

#define _S SettingEntry
#define _C(...) allSettings.push_back(SettingsCategory(__VA_ARGS__))
#define _CC SettingsCategory

void FurnaceGUI::initSettings() {
  _C(_N("General"),{},{
    _CC(_N("Program"),{
#ifdef HAVE_LOCALE
      _S(_N("Language"),[this]{
        bool changed=false;
        String curLocale=settings.locale;
        const char* localeRestart=locales[0][2];
        if (curLocale=="") {
          curLocale="<System>";
        } else {
          for (int i=1; locales[i][0]; i++) {
            if (strcmp(curLocale.c_str(),locales[i][1])==0) {
              curLocale=locales[i][0];
              break;
            }
          }
        }
        if (ImGui::BeginCombo(_("Language"),curLocale.c_str())) {
          for (int i=0; locales[i][0]; i++) {
            if (ImGui::Selectable(locales[i][0],strcmp(settings.locale.c_str(),locales[i][1])==0)) {
              settings.locale=locales[i][1];
              changed=true;
            }
            if (ImGui::IsItemHovered()) {
              ImGui::SetTooltip("%s",locales[i][2]);
            }
          }
          ImGui::EndCombo();
        } else {
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s",localeRestart);
          }
        }
        return changed;
      }),
#endif
      _S(_N("Render Backend"),[this]{
        String curRenderBackend=settings.renderBackend.empty()?GUI_BACKEND_DEFAULT_NAME:settings.renderBackend;
        bool changed=false;
        if (ImGui::BeginCombo(_("Render backend"),curRenderBackend.c_str())) {
#ifdef HAVE_RENDER_SDL
          if (ImGui::Selectable("SDL Renderer",curRenderBackend=="SDL")) {
            settings.renderBackend="SDL";
            changed=true;
          }
#endif
#ifdef HAVE_RENDER_DX11
          if (ImGui::Selectable("DirectX 11",curRenderBackend=="DirectX 11")) {
            settings.renderBackend="DirectX 11";
            changed=true;
          }
#endif
#ifdef HAVE_RENDER_DX9
          if (ImGui::Selectable("DirectX 9",curRenderBackend=="DirectX 9")) {
            settings.renderBackend="DirectX 9";
            changed=true;
          }
#endif
#ifdef HAVE_RENDER_METAL
          if (ImGui::Selectable("Metal",curRenderBackend=="Metal")) {
            settings.renderBackend="Metal";
            changed=true;
          }
#endif
#ifdef HAVE_RENDER_GL
#ifdef USE_GLES
          if (ImGui::Selectable("OpenGL ES 2.0",curRenderBackend=="OpenGL ES 2.0")) {
            settings.renderBackend="OpenGL ES 2.0";
            changed=true;
          }
#else
          if (ImGui::Selectable("OpenGL 3.0",curRenderBackend=="OpenGL 3.0")) {
            settings.renderBackend="OpenGL 3.0";
            changed=true;
          }
          if (ImGui::Selectable("OpenGL 2.0",curRenderBackend=="OpenGL 2.0")) {
            settings.renderBackend="OpenGL 2.0";
            changed=true;
          }
#endif
#endif
#ifdef HAVE_RENDER_GL1
          if (ImGui::Selectable("OpenGL 1.1",curRenderBackend=="OpenGL 1.1")) {
            settings.renderBackend="OpenGL 1.1";
            changed=true;
          }
#endif
          if (ImGui::Selectable("Software",curRenderBackend=="Software")) {
            settings.renderBackend="Software";
            changed=true;
          }
          ImGui::EndCombo();
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("you may need to restart Furnace for this setting to take effect."));
        }
        return changed;
      }),
      SETTING_CHECKBOX("VSync",vsync),
      SETTING_CHECKBOX("Display render time",displayRenderTime),
    },{}),
    _CC(_N("File"),{
      SETTING_CHECKBOX("Save unused patterns",saveUnusedPatterns),
      SETTING_RADIO("Play after opening song:",playOnLoad)
      SETTING_RADIO_BUTTON("No##pol0",playOnLoad,0)
      SETTING_RADIO_BUTTON("Only if already playing##pol1",playOnLoad,1)
      SETTING_RADIO_BUTTON("Yes##pol0",playOnLoad,2)
      SETTING_RADIO_END,
      SETTING_RADIO("Audio export loop/fade out time:",persistFadeOut)
      SETTING_RADIO_BUTTON("Set to these values on start-up:##fot0",persistFadeOut,0)
      {
        ImGui::BeginDisabled(settings.persistFadeOut);
        ImGui::Indent();
        if (ImGui::InputInt(_("Loops"),&settings.exportLoops,1,2)) {
          if (settings.exportLoops<0) settings.exportLoops=0;
          audioExportOptions.loops=settings.exportLoops;
          settingsChanged=true;
        }
        if (ImGui::InputDouble(_("Fade out (seconds)"),&settings.exportFadeOut,1.0,2.0,"%.1f")) {
          if (settings.exportFadeOut<0.0) settings.exportFadeOut=0.0;
          audioExportOptions.fadeOut=settings.exportFadeOut;
          settingsChanged=true;
        }
        ImGui::Unindent();
        ImGui::EndDisabled();
      }
      SETTING_RADIO_BUTTON("Remember last values##fot1",persistFadeOut,1)
      SETTING_RADIO_END,
      SETTING_CHECKBOX("Store instrument name in .fui",writeInsNames),
      SETTING_CHECKBOX("Load instrument name from .fui",readInsNames),
      SETTING_CHECKBOX("Auto-fill file name when saving",autoFillSave),
    },{}),
    _CC(_N("Start-up"),{
#ifndef NO_INTRO
      SETTING_RADIO("Play intro on start-up:",alwaysPlayIntro)
      SETTING_RADIO_BUTTON("No##pis0",alwaysPlayIntro,0)
      SETTING_RADIO_BUTTON("Short##pis1",alwaysPlayIntro,1)
      SETTING_RADIO_BUTTON("Full (short when loading song)##pis2",alwaysPlayIntro,2)
      SETTING_RADIO_BUTTON("Full (always)##pis3",alwaysPlayIntro,3)
      SETTING_RADIO_END,
#endif
      SETTING_CHECKBOX("Disable fade-in during start-up",disableFadeIn),
      SETTING_CHECKBOX("Do not maximize on start-up when the Furnace window is too big",noMaximizeWorkaround)
    },{}),
  });
  _C(_N("Audio"),{},{
    _CC(_N("Mixing"),{
      _S(_N("Quality"),[this]{
        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("Quality"));
        ImGui::SameLine();
        return ImGui::Combo("##Quality",&settings.audioQuality,LocalizedComboGetter,audioQualities,2);
      }),
      SETTING_CHECKBOX("Software clipping",clampSamples),
      SETTING_CHECKBOX("DC offset correction",audioHiPass),
    },{}),
    _CC(_N("Volumes"),{
      _S("Metronome volume",[this]{
        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("Metronome volume"));
        ImGui::SameLine();
        if (ImGui::SliderInt("##MetroVol",&settings.metroVol,0,200,"%d%%")) {
          if (settings.metroVol<0) settings.metroVol=0;
          if (settings.metroVol>200) settings.metroVol=200;
          e->setMetronomeVol(((float)settings.metroVol)/100.0f);
          return true;
        }
        return false;
      }),
      _S("Sample preview volume",[this]{
        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("Sample preview volume"));
        ImGui::SameLine();
        if (ImGui::SliderInt("##SampleVol",&settings.sampleVol,0,100,"%d%%")) {
          if (settings.sampleVol<0) settings.sampleVol=0;
          if (settings.sampleVol>100) settings.sampleVol=100;
          e->setSamplePreviewVol(((float)settings.sampleVol)/100.0f);
          return true;
        }
        return false;
      })
    },{}),
  });
}
