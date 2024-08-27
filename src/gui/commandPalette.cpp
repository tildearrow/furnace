/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2024 tildearrow and contributors
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

// @TODO: when there's a tie on both within and before-needle costs, we have options.
// It's reasonable to let the original order stand, but also reasonable to favor
// minimizing the number of chars that follow afterward. Leaving this code in for now,
// but disabling until further thought/discussion.
// #define MATCH_SCORE_PREFER_LOWER_CHARS_AFTER_NEEDLE

struct MatchScore {
  bool valid=true;
  enum Cost { COST_BEFORE_NEEDLE, COST_WITHIN_NEEDLE, COST_AFTER_NEEDLE, COST_COUNT };
  size_t costs[COST_COUNT] = {0, 0, 0};

  static MatchScore INVALID() {
    MatchScore score;
    score.valid=false;
    return score;
  }

  static bool IsFirstPreferable(const MatchScore& a, const MatchScore& b) {
    auto PreferenceForAAmount=[&](Cost cost) {
      // prefer a if lower cost
      return b.costs[cost]-a.costs[cost];
    };

    if (a.valid && b.valid) {
      int prefA;
      prefA=PreferenceForAAmount(COST_WITHIN_NEEDLE);
      if (prefA!=0) return prefA>0;
      prefA=PreferenceForAAmount(COST_BEFORE_NEEDLE);
      if (prefA!=0) return prefA>0;
#ifdef MATCH_SCORE_PREFER_LOWER_CHARS_AFTER_NEEDLE
      // prefA=PreferenceForAAmount(COST_AFTER_NEEDLE);
      // if (prefA!=0) return prefA>0;
#endif
      return false;
    } else {
      return a.valid;
    }
  }
};

static inline MatchScore matchFuzzy(const char* haystack, const char* needle) {
  MatchScore score;
  size_t h_i=0; // haystack idx
  size_t n_i=0; // needle idx
  while (needle[n_i]!='\0') {
    size_t cost=0;
    for (; std::tolower(haystack[h_i])!=std::tolower(needle[n_i]); h_i++, cost++) {
      // needle not completed, return invalid
      if (haystack[h_i]=='\0')
        return MatchScore::INVALID();
    }

    // contribute this run of non-matches toward pre-needle or within-needle cost
    if (n_i==0) {
      score.costs[MatchScore::COST_BEFORE_NEEDLE]=cost;
    } else {
      score.costs[MatchScore::COST_WITHIN_NEEDLE]+=cost;
    }

    n_i+=1;
  }

#ifdef MATCH_SCORE_PREFER_LOWER_CHARS_AFTER_NEEDLE
  // count the remaining chars in haystack as a tie-breaker (we won't reach this if it's a failed
  // match anyway)
  for (; haystack[h_i]!='\0'; h_i++, score.costs[MatchScore::COST_AFTER_NEEDLE]++) {}
#endif

  score.valid=true;
  return score;
}

void FurnaceGUI::drawPalette() {
  bool accepted=false;

  if (paletteFirstFrame)
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

  if (ImGui::InputTextWithHint("##CommandPaletteSearch",hint,&paletteQuery) || paletteFirstFrame) {
    paletteSearchResults.clear();
    std::vector<MatchScore> matchScores;

    auto Evaluate=[&](int i, const char* name) {
      MatchScore score=matchFuzzy(name,paletteQuery.c_str());
      if (score.valid) {
        paletteSearchResults.push_back(i);
        matchScores.push_back(score);
      }
    };

    switch (curPaletteType) {
    case CMDPAL_TYPE_MAIN:
      for (int i=0; i<GUI_ACTION_MAX; i++) {
        if (guiActions[i].defaultBind==-1) continue; // not a bind
        Evaluate(i,guiActions[i].friendlyName);
      }
      break;

    case CMDPAL_TYPE_RECENT:
      for (int i=0; i<(int)recentFile.size(); i++) {
        Evaluate(i,recentFile[i].c_str());
      }
      break;

    case CMDPAL_TYPE_INSTRUMENTS:
    case CMDPAL_TYPE_INSTRUMENT_CHANGE:
      Evaluate(0,_("- None -"));
      for (int i=0; i<e->song.insLen; i++) {
        String s=fmt::sprintf("%02X: %s", i, e->song.ins[i]->name.c_str());
        Evaluate(i+1,s.c_str()); // because over here ins=0 is 'None'
      }
      break;

    case CMDPAL_TYPE_SAMPLES:
      for (int i=0; i<e->song.sampleLen; i++) {
        Evaluate(i,e->song.sample[i]->name.c_str());
      }
      break;

    case CMDPAL_TYPE_ADD_CHIP:
      for (int i=0; availableSystems[i]; i++) {
        int ds=availableSystems[i];
        const char* sysname=getSystemName((DivSystem)ds);
        Evaluate(ds,sysname);
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
    for (size_t i=0; i<sortingIndices.size(); ++i) sortingIndices[i]=(int)paletteSearchResults[sortingIndices[i]];
    for (size_t i=0; i<sortingIndices.size(); ++i) paletteSearchResults[i]=sortingIndices[i];
  }

  ImVec2 avail=ImGui::GetContentRegionAvail();
  avail.y-=ImGui::GetFrameHeightWithSpacing();

  if (ImGui::BeginChild("CommandPaletteList",avail,false,0)) {
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

    for (int i=0; i<(int)paletteSearchResults.size(); i++) {
      bool current=(i==curPaletteChoice);
      int id=paletteSearchResults[i];

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

      if (ImGui::Selectable(s.c_str(),current)) {
        curPaletteChoice=i;
        accepted=true;
      }
      if ((navigated || paletteFirstFrame) && current) ImGui::SetScrollHereY();
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
      int i=paletteSearchResults[curPaletteChoice];
      switch (curPaletteType) {
        case CMDPAL_TYPE_MAIN:
          doAction(i);
          break;
        case CMDPAL_TYPE_RECENT:
          openRecentFile(recentFile[i]);
          break;
        case CMDPAL_TYPE_INSTRUMENTS:
          curIns=i-1;
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
