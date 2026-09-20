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

#include "klattschInput.h"

#include <klattsch/banks.hpp>
#include <cstring>

namespace KlattschInput {

static bool isPrefix(const char* buf, const char* candidate) {
  while (*buf) {
    if (*buf++!=*candidate++) return false;
    if (!*candidate && *buf) return false;
  }
  return true;
}

const klattsch::PhonemeBank& resolveBank(const char* bankName) {
  const klattsch::BankRegistry& reg=klattsch::builtInBanks();
  return reg.resolve(bankName?bankName:reg.defaultName());
}

MatchResult match(const char* bankName, const char* buffer) {
  if (!buffer || !*buffer) return {MatchKind::Invalid,-1};
  const klattsch::PhonemeBank& bank=resolveBank(bankName);
  int exactIndex=-1;
  bool hasPrefix=false;
  for (std::size_t i=0; i<bank.count; i++) {
    const char* name=bank.records[i].name;
    if (std::strcmp(buffer,name)==0) {
      exactIndex=static_cast<int>(i);
    } else if (isPrefix(buffer,name)) {
      hasPrefix=true;
    }
  }
  if (exactIndex>=0 && hasPrefix) return {MatchKind::Ambiguous,exactIndex};
  if (exactIndex>=0) return {MatchKind::Complete,exactIndex};
  if (hasPrefix) return {MatchKind::Pending,-1};
  return {MatchKind::Invalid,-1};
}

const char* phonemeName(const klattsch::PhonemeBank& bank, int index) {
  if (index<0 || static_cast<std::size_t>(index)>=bank.count) return "??";
  return bank.records[index].name;
}

}
