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

#include "gui.h"
#include "guiConst.h"
#include "commandPalette.h"
#include "misc/cpp/imgui_stdlib.h"
#include <fmt/printf.h>
#include <algorithm>
#include <ctype.h>
#include "../ta-log.h"
#include "util.h"

struct MatchScore {
  size_t charsBeforeNeedle=0;
  size_t charsWithinNeedle=0;
  static bool IsFirstPreferable(const MatchScore& a, const MatchScore& b) {
    int aBetter;
    aBetter=b.charsWithinNeedle-a.charsWithinNeedle;
    if (aBetter!=0) return aBetter>0;
    aBetter=b.charsBeforeNeedle-a.charsBeforeNeedle;
    if (aBetter!=0) return aBetter>0;
    return false;
  }
};

struct MatchResult {
  MatchScore score;
  std::vector<int> highlightChars;
};

static bool charMatch(const char* a, const char* b) {
  // stub for future utf8 support, possibly with matching for related chars?
  return std::tolower(*a)==std::tolower(*b);
}

// #define MATCH_GREEDY
// #define RUN_MATCH_TEST

static bool matchFuzzy(const char* haystack, int haystackLen, const char* needle, int needleLen, MatchResult* result) {
  if (needleLen==0) {
    result->score.charsBeforeNeedle=0;
    result->score.charsWithinNeedle=0;
    result->highlightChars.clear();
    return true;
  }

  std::vector<MatchResult> matchPool(needleLen+1);
  std::vector<MatchResult*> unusedMatches(needleLen+1);
  std::vector<MatchResult*> matchesByLen(needleLen+1);
  for (int i=0; i<needleLen+1; i++) {
    unusedMatches[i]=&matchPool[i];
    matchesByLen[i]=NULL;
  }

  for (int hIdx=0; hIdx<haystackLen; hIdx++) {
    // try to continue our in-flight valid matches
    for (int matchLen=needleLen-1; matchLen>=0; matchLen--) {
      MatchResult*& m=matchesByLen[matchLen];

      // ignore null matches except for 0
      if (matchLen>0 && !m) continue;

#ifdef MATCH_GREEDY
      // in greedy mode, don't start any new matches once we've already started matching.
      // this will still return the correct bool result, but its score could be much poorer
      // than the optimal match.  consider the case:
      //
      //    find "gl" in "google"
      //
      // greedy will see the match "g...l.", which has charsWithinNeedle of 3, while the
      // fully algorithm will find the tighter match "...gl.", which has
      // charsWithinNeedle of 0

      if (matchLen==0 && unusedMatches.size() < matchPool.size()) {
        continue;
      }
#endif

      // check match!
      if (charMatch(haystack+hIdx, needle+matchLen)) {

        // pull a fresh match from the pool if necessary
        if (matchLen==0) {
          m=unusedMatches.back();
          unusedMatches.pop_back();
          m->score.charsBeforeNeedle=hIdx;
          m->score.charsWithinNeedle=0;
          m->highlightChars.clear();
        }

        m->highlightChars.push_back(hIdx);

        // advance, replacing the previous match of an equal len, which can only have been
        // worse because it existed before us, so we can prune it out
        if (matchesByLen[matchLen+1]) {
          unusedMatches.push_back(matchesByLen[matchLen+1]);
        }

        matchesByLen[matchLen+1]=m;
        m=NULL;

      } else {
        // tally up charsWithinNeedle
        if (matchLen>0) {
          matchesByLen[matchLen]->score.charsWithinNeedle++;
        }
      }
    }
  }

  if (matchesByLen[needleLen]) {
    if (result) *result=*matchesByLen[needleLen];
    return true;
  }

  return false;
}

#ifdef RUN_MATCH_TEST
static void matchFuzzyTest() {
  String hay="a__i_a_i__o";
  String needle="aio";
  MatchResult match;
  matchFuzzy(hay.c_str(), hay.length(), needle.c_str(), needle.length(), &match);
  logI( "match.score.charsWithinNeedle: %d", match.score.charsWithinNeedle );
}
#endif

void FurnaceGUI::drawPalette() {
  bool accepted=false;

  if (paletteFirstFrame && !mobileUI)
    ImGui::SetKeyboardFocusHere();

  int width=ImGui::GetContentRegionAvail().x;
  ImGui::SetNextItemWidth(width);

  const char* hint=_("Search...");
  switch (curPaletteType) {
  case CMDPAL_TYPE_RECENT:
    hint=_("Search recent files...");
    break;
  case CMDPAL_TYPE_INSTRUMENTS:
    hint=_("Search instruments...");
    break;
  case CMDPAL_TYPE_SAMPLES:
    hint=_("Search samples...");
    break;
  case CMDPAL_TYPE_INSTRUMENT_CHANGE:
    hint=_("Search instruments (to change to)...");
    break;
  case CMDPAL_TYPE_ADD_CHIP:
    hint=_("Search chip (to add)...");
    break;
  }

#ifdef RUN_MATCH_TEST
  matchFuzzyTest();
#endif

  if (ImGui::InputTextWithHint("##CommandPaletteSearch",hint,&paletteQuery) || paletteFirstFrame) {
    paletteSearchResults.clear();
    std::vector<MatchScore> matchScores;

    auto Evaluate=[&](int i, const char* name, int nameLen) {
      MatchResult result;
      if (matchFuzzy(name, nameLen, paletteQuery.c_str(), paletteQuery.length(), &result)) {
        paletteSearchResults.emplace_back();
        paletteSearchResults.back().id=i;
        paletteSearchResults.back().highlightChars=std::move(result.highlightChars);
        matchScores.push_back(result.score);
      }
    };

    switch (curPaletteType) {
    case CMDPAL_TYPE_MAIN:
      for (int i=0; i<GUI_ACTION_MAX; i++) {
        if (guiActions[i].isNotABind()) continue;
        Evaluate(i,guiActions[i].friendlyName,strlen(guiActions[i].friendlyName));
      }
      break;

    case CMDPAL_TYPE_RECENT:
      for (int i=0; i<(int)recentFile.size(); i++) {
        Evaluate(i,recentFile[i].c_str(),recentFile[i].length());
      }
      break;

    case CMDPAL_TYPE_INSTRUMENTS:
    case CMDPAL_TYPE_INSTRUMENT_CHANGE: {
      const char* noneStr=_("- None -");
      Evaluate(0,noneStr,strlen(noneStr));
      for (int i=0; i<e->song.insLen; i++) {
        String s=fmt::sprintf("%02X: %s", i, e->song.ins[i]->name.c_str());
        Evaluate(i+1,s.c_str(),s.length()); // because over here ins=0 is 'None'
      }
      break;
    }

    case CMDPAL_TYPE_SAMPLES:
      for (int i=0; i<e->song.sampleLen; i++) {
        Evaluate(i,e->song.sample[i]->name.c_str(),e->song.sample[i]->name.length());
      }
      break;

    case CMDPAL_TYPE_ADD_CHIP:
      for (int i=0; availableSystems[i]; i++) {
        int ds=availableSystems[i];
        const char* sysname=getSystemName((DivSystem)ds);
        Evaluate(ds,sysname,strlen(sysname));
      }
      break;

    default:
      logE(_("invalid command palette type"));
      ImGui::CloseCurrentPopup();
      break;
    };

    // sort indices by match quality
    std::vector<int> sortingIndices(paletteSearchResults.size());
    for (size_t i=0; i<sortingIndices.size(); ++i) sortingIndices[i]=(int)i;
    std::sort(sortingIndices.begin(), sortingIndices.end(), [&](size_t a, size_t b) {
      return MatchScore::IsFirstPreferable(matchScores[a], matchScores[b]);
    });

    // update paletteSearchResults from sorted indices (taking care not to stomp while we iterate
    std::vector<PaletteSearchResult> paletteSearchResultsCopy=paletteSearchResults;
    for (size_t i=0; i<sortingIndices.size(); ++i) paletteSearchResults[i]=paletteSearchResultsCopy[sortingIndices[i]];
  }

  ImVec2 avail=ImGui::GetContentRegionAvail();
  avail.y-=ImGui::GetFrameHeightWithSpacing();

  if (ImGui::BeginChild("CommandPaletteList",avail,0,0)) {
      bool navigated=false;
      if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) && curPaletteChoice>0) {
        curPaletteChoice-=1;
        navigated=true;
      }
      if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
        curPaletteChoice+=1;
        navigated=true;
      }

      if (paletteSearchResults.size()>0 && curPaletteChoice<0) {
        curPaletteChoice=0;
        navigated=true;
      }
      if (curPaletteChoice>=(int)paletteSearchResults.size()) {
        curPaletteChoice=paletteSearchResults.size()-1;
        navigated=true;
      }

    int columnCount=curPaletteType==CMDPAL_TYPE_MAIN ? 2 : 1;
    if (ImGui::BeginTable("##commandPaletteTable",columnCount,ImGuiTableFlags_SizingStretchProp)) {
      // ImGui::TableSetupColumn("##action",ImGuiTableColumnFlags_WidthStretch);
      // ImGui::TableSetupColumn("##shortcut");
      for (int i=0; i<(int)paletteSearchResults.size(); i++) {
        bool current=(i==curPaletteChoice);
        int id=paletteSearchResults[i].id;

        String s="???";
        switch (curPaletteType) {
        case CMDPAL_TYPE_MAIN:
          s=guiActions[id].friendlyName;
          break;
        case CMDPAL_TYPE_RECENT:
          s=recentFile[id].c_str();
          break;
        case CMDPAL_TYPE_INSTRUMENTS:
        case CMDPAL_TYPE_INSTRUMENT_CHANGE:
          if (id==0) {
            s=_("- None -");
          } else {
            s=fmt::sprintf("%02X: %s", id-1, e->song.ins[id-1]->name.c_str());
          }
          break;
        case CMDPAL_TYPE_SAMPLES:
          s=e->song.sample[id]->name.c_str();
          break;
        case CMDPAL_TYPE_ADD_CHIP:
          s=getSystemName((DivSystem)id);
          break;
        default:
          logE(_("invalid command palette type"));
          break;
        };

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        ImGui::PushID(s.c_str());
        bool selectable=ImGui::Selectable("##paletteSearchItem",current,ImGuiSelectableFlags_SpanAllColumns|ImGuiSelectableFlags_AllowOverlap);
        const char* str=s.c_str();
        size_t chCursor=0;
        const std::vector<int>& highlights=paletteSearchResults[i].highlightChars;
        for (size_t ch=0; ch<highlights.size(); ch++) {
          ImGui::SameLine(0.0f,0.0f);
          ImGui::Text("%.*s", (int)(highlights[ch]-chCursor), str+chCursor);
          ImGui::SameLine(0.0f,0.0f);
          ImGui::TextColored(uiColors[GUI_COLOR_ACCENT_PRIMARY], "%.1s", str+highlights[ch]);
          chCursor=highlights[ch]+1;
        }
        ImGui::SameLine(0.0f,0.0f);
        ImGui::Text("%.*s", (int)(s.length()-chCursor), str+chCursor);

        if (curPaletteType==CMDPAL_TYPE_MAIN) {
          ImGui::TableNextColumn();
          ImGui::TextColored(uiColors[GUI_COLOR_TEXT_DISABLED], "%s", getMultiKeysName(actionKeys[paletteSearchResults[i].id].data(),actionKeys[paletteSearchResults[i].id].size(),true).c_str());
        }

        if (selectable) {
          curPaletteChoice=i;
          accepted=true;
        }

        ImGui::PopID();
        
        if ((navigated || paletteFirstFrame) && current) ImGui::SetScrollHereY();
      }
      ImGui::EndTable();
    }
  }
  ImGui::EndChild();

  if (!accepted) {
    if (curPaletteChoice>=(int)paletteSearchResults.size()) {
      curPaletteChoice=paletteSearchResults.size()-1;
    }
    accepted=ImGui::IsKeyPressed(ImGuiKey_Enter);
  }

  if (ImGui::Button(_("Cancel")) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
    ImGui::CloseCurrentPopup();
  }

  // do not move this to after the resetPalette() calls!
  // if they are called before and we're jumping from one palette to the next, the paletteFirstFrame won't be true at the start and the setup will not happen.
  paletteFirstFrame=false;

  if (accepted) {
    if (paletteSearchResults.size()>0) {
      int i=paletteSearchResults[curPaletteChoice].id;
      switch (curPaletteType) {
        case CMDPAL_TYPE_MAIN:
          doAction(i);
          break;
        case CMDPAL_TYPE_RECENT:
          openRecentFile(recentFile[i]);
          break;
        case CMDPAL_TYPE_INSTRUMENTS:
          setCurIns(i-1);
          break;
        case CMDPAL_TYPE_SAMPLES:
          curSample=i;
          break;
        case CMDPAL_TYPE_INSTRUMENT_CHANGE:
          doChangeIns(i-1);
          break;
        case CMDPAL_TYPE_ADD_CHIP:
          if (i!=DIV_SYSTEM_NULL) {
            if (!e->addSystem((DivSystem)i)) {
              showError("cannot add chip! ("+e->getLastError()+")");
            } else {
              MARK_MODIFIED;
              recalcTimestamps=true;
            }
            ImGui::CloseCurrentPopup();
            if (e->song.autoSystem) {
              autoDetectSystem();
            }
            updateWindowTitle();
          }
          break;
        default:
          logE(_("invalid command palette type"));
          break;
      };
    }
    ImGui::CloseCurrentPopup();
  }
}
