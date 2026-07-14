/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
 */

#ifndef _KLATTSCH_INPUT_H
#define _KLATTSCH_INPUT_H

namespace klattsch {
  struct PhonemeBank;
}

namespace KlattschInput {

enum class MatchKind {
  Invalid,
  Pending,
  Ambiguous,
  Complete
};

struct MatchResult {
  MatchKind kind;
  int phonemeIndex;  // valid for Ambiguous and Complete
};

// buffer must use uppercase ARPABET.
MatchResult match(const char* bankName, const char* buffer);

const klattsch::PhonemeBank& resolveBank(const char* bankName);

// returns "??" when index is outside the bank.
const char* phonemeName(const klattsch::PhonemeBank& bank, int index);

}

#endif
