#include <klattsch/compile.hpp>
#include <klattsch/banks.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace klattsch {

namespace {

// Unicode normalization (subset of JS normalize())
// Strips zero-width characters and maps Greek/Cyrillic homoglyphs to ASCII.
// This is a practical subset of NFKC without requiring ICU.

static std::string normalizeUnicode(const std::string& input) {
  std::string out;
  out.reserve(input.size());
  const auto* p = reinterpret_cast<const unsigned char*>(input.data());
  const auto* end = p + input.size();

  while (p < end) {
    // ASCII fast path
    if (p[0] < 0x80) {
      out += static_cast<char>(p[0]);
      ++p;
      continue;
    }

    // 3-byte sequences (U+0800..U+FFFF): zero-width characters
    if (p[0] >= 0xE0 && p + 2 < end) {
      unsigned char b0 = p[0], b1 = p[1], b2 = p[2];
      // U+200B ZERO WIDTH SPACE: E2 80 8B
      // U+200C ZERO WIDTH NON-JOINER: E2 80 8C
      // U+200D ZERO WIDTH JOINER: E2 80 8D
      // U+2060 WORD JOINER: E2 81 A0
      // U+FEFF BOM / ZERO WIDTH NO-BREAK SPACE: EF BB BF
      if (b0 == 0xE2 && b1 == 0x80 && (b2 == 0x8B || b2 == 0x8C || b2 == 0x8D)) {
        p += 3;
        continue;
      }
      if (b0 == 0xE2 && b1 == 0x81 && b2 == 0xA0) {
        p += 3;
        continue;
      }
      if (b0 == 0xEF && b1 == 0xBB && b2 == 0xBF) {
        p += 3;
        continue;
      }
      // Not a zero-width char; copy the 3-byte sequence as-is
      out += static_cast<char>(b0);
      out += static_cast<char>(b1);
      out += static_cast<char>(b2);
      p += 3;
      continue;
    }

    // 2-byte sequences (U+0080..U+07FF): Greek and Cyrillic homoglyphs
    if (p[0] >= 0xC0 && p + 1 < end) {
      unsigned char b0 = p[0], b1 = p[1];
      char mapped = 0;

      // Greek uppercase (U+0391..U+0396 range, CE 91..CE 96+)
      if (b0 == 0xCE) {
        switch (b1) {
          case 0x91: mapped = 'A'; break;  // Α
          case 0x92: mapped = 'B'; break;  // Β
          case 0x95: mapped = 'E'; break;  // Ε
          case 0x97: mapped = 'H'; break;  // Η
          case 0x99: mapped = 'I'; break;  // Ι
          case 0x9A: mapped = 'K'; break;  // Κ
          case 0x9C: mapped = 'M'; break;  // Μ
          case 0x9D: mapped = 'N'; break;  // Ν
          case 0x9F: mapped = 'O'; break;  // Ο
          case 0xA1: mapped = 'P'; break;  // Ρ
          case 0xA4: mapped = 'T'; break;  // Τ
          case 0xA5: mapped = 'Y'; break;  // Υ
          case 0x96: mapped = 'Z'; break;  // Ζ
        }
      }
      // Cyrillic uppercase (U+0410..U+0422 range, D0 90..D0 A2+)
      else if (b0 == 0xD0) {
        switch (b1) {
          case 0x90: mapped = 'A'; break;  // А
          case 0x92: mapped = 'B'; break;  // В
          case 0xA1: mapped = 'C'; break;  // С
          case 0x95: mapped = 'E'; break;  // Е
          case 0x9D: mapped = 'H'; break;  // Н
          case 0x9A: mapped = 'K'; break;  // К
          case 0x9C: mapped = 'M'; break;  // М
          case 0x9E: mapped = 'O'; break;  // О
          case 0xA0: mapped = 'P'; break;  // Р
          case 0xA2: mapped = 'T'; break;  // Т
        }
      }
      // Cyrillic lowercase (D0 B0+, D1 80+)
      else if (b0 == 0xD0) {
        // handled above
      }

      // Cyrillic lowercase: а(D0 B0), с(D1 81), е(D0 B5), о(D0 BE), р(D1 80)
      if (!mapped && b0 == 0xD0) {
        switch (b1) {
          case 0xB0: mapped = 'a'; break;  // а
          case 0xB5: mapped = 'e'; break;  // е
          case 0xBE: mapped = 'o'; break;  // о
        }
      }
      if (!mapped && b0 == 0xD1) {
        switch (b1) {
          case 0x81: mapped = 'c'; break;  // с
          case 0x80: mapped = 'p'; break;  // р
        }
      }

      if (mapped) {
        out += mapped;
        p += 2;
        continue;
      }

      // Unknown 2-byte sequence, copy as-is
      out += static_cast<char>(b0);
      out += static_cast<char>(b1);
      p += 2;
      continue;
    }

    // Fallback: copy byte as-is
    out += static_cast<char>(*p);
    ++p;
  }
  return out;
}

// Defaults (matching JS DEFAULTS)

constexpr float kStressDurationFactor = 1.5f;
constexpr float kStressF0Lift = 8.0f;
constexpr float kStopBurstMs = 25.0f;
constexpr float kDefaultTransitionMs = 35.0f;
constexpr float kSentenceFinalHoldMs = 0.0f;
constexpr float kFadeOutMs = 100.0f;
constexpr float kTrailOffMs = 150.0f;

// Token types

enum class TokenType {
  Phoneme,
  Directive,
  BankSwitch,
  SyllableOpen,
  SyllableClose,
  Pause,
  Unknown,
};

enum class DirectiveMode {
  Absolute,
  Relative,
  Reset,
};

struct Token {
  TokenType type = TokenType::Unknown;
  std::string text;

  // Phoneme fields
  bool stressed = false;
  double pitchDelta = 0.0;
  bool transient = false;

  // Directive fields
  std::string directiveKey;
  double directiveValue = 0.0;
  DirectiveMode directiveMode = DirectiveMode::Absolute;

  // Pause fields
  double pauseMs = 0.0;

  // Bank switch: empty string = reset to default
  std::string bankName;

  std::size_t srcStart = 0;
  std::size_t srcEnd = 0;
};

// Map single-letter compact directive codes to their key names.
static const char* compactKeyMap(char letter) {
  switch (letter) {
    case 'b': return "base";
    case 'r': return "rate";
    case 'p': return "pause";
    case 's': return "scale";
    case 'v': return "vibrato";
    case 'w': return "vibratoRate";
    case 'm': return "tremolo";
    case 'n': return "tremoloRate";
    case 'h': return "aspiration";
    case 't': return "tilt";
    case 'g': return "effort";
    default: return nullptr;
  }
}

// Try to parse a double from str starting at pos. Returns true on success,
// advances pos past the consumed characters, and stores the result in out.
// Uses double precision to match JS Number semantics.
static bool parseDouble(const std::string& str, std::size_t& pos, double& out) {
  if (pos >= str.size()) return false;
  const char* start = str.c_str() + pos;
  char* end = nullptr;
  double val = std::strtod(start, &end);
  if (end == start) return false;
  pos += static_cast<std::size_t>(end - start);
  out = val;
  return true;
}

// Note-name to Hz conversion

// Semitone offsets from C within an octave.
static int noteSemitone(char letter) {
  switch (letter) {
    case 'C': return 0;
    case 'D': return 2;
    case 'E': return 4;
    case 'F': return 5;
    case 'G': return 7;
    case 'A': return 9;
    case 'B': return 11;
    default: return -1;
  }
}

// Convert note name like "C4", "C#5", "Db3", "A-1" to Hz.
// Returns 0.0 on parse failure.
static double noteToHz(const std::string& name) {
  if (name.empty()) return 0.0;
  int semi = noteSemitone(name[0]);
  if (semi < 0) return 0.0;
  std::size_t pos = 1;
  if (pos < name.size() && name[pos] == '#') { semi += 1; ++pos; }
  else if (pos < name.size() && name[pos] == 'b') { semi -= 1; ++pos; }
  // Parse octave (possibly negative)
  if (pos >= name.size()) return 0.0;
  char* end = nullptr;
  long octave = std::strtol(name.c_str() + pos, &end, 10);
  if (end != name.c_str() + name.size()) return 0.0;
  int midi = static_cast<int>((octave + 1) * 12 + semi);
  return 440.0 * std::pow(2.0, (midi - 69) / 12.0);
}

// Tokenizer

static std::vector<Token> tokenize(const std::string& input) {
  std::vector<Token> tokens;
  const std::size_t len = input.size();
  std::size_t i = 0;

  while (i < len) {
    // Skip whitespace
    if (std::isspace(static_cast<unsigned char>(input[i]))) {
      ++i;
      continue;
    }

    // Line comment: # at boundary
    if (input[i] == '#' && (i == 0 || std::isspace(static_cast<unsigned char>(input[i - 1])))) {
      while (i < len && input[i] != '\n') ++i;
      continue;
    }

    // Block comment
    if (input[i] == '/' && i + 1 < len && input[i + 1] == '*') {
      auto end = input.find("*/", i + 2);
      i = (end == std::string::npos) ? len : end + 2;
      continue;
    }

    // Collect non-whitespace token
    std::size_t srcStart = i;
    std::string part;
    while (i < len && !std::isspace(static_cast<unsigned char>(input[i]))) {
      // Inline block comment
      if (input[i] == '/' && i + 1 < len && input[i + 1] == '*') {
        auto end = input.find("*/", i + 2);
        i = (end == std::string::npos) ? len : end + 2;
        continue;
      }
      part += input[i];
      ++i;
    }
    std::size_t srcEnd = i;
    if (part.empty()) continue;

    Token tok;
    tok.srcStart = srcStart;
    tok.srcEnd = srcEnd;

    // Syllable grouping
    if (part == "(") {
      tok.type = TokenType::SyllableOpen;
      tokens.push_back(tok);
      continue;
    }
    if (part == ")") {
      tok.type = TokenType::SyllableClose;
      tokens.push_back(tok);
      continue;
    }

    // Pause punctuation
    if (part == ",") {
      tok.type = TokenType::Pause;
      tok.pauseMs = 100.0;
      tokens.push_back(tok);
      continue;
    }
    if (part == ";") {
      tok.type = TokenType::Pause;
      tok.pauseMs = 200.0;
      tokens.push_back(tok);
      continue;
    }
    if (part == ".") {
      tok.type = TokenType::Pause;
      tok.pauseMs = 300.0;
      tokens.push_back(tok);
      continue;
    }

    // Check for post-fix stress mark: ' or !
    if (part == "!" || part == "'") {
      for (auto it = tokens.rbegin(); it != tokens.rend(); ++it) {
        if (it->type == TokenType::Phoneme) {
          it->stressed = true;
          break;
        }
      }
      continue;
    }

    // Bracketed tokens: [bank=NAME], [bank], [key=value]
    if (part.size() >= 3 && part.front() == '[' && part.back() == ']') {
      std::string inner = part.substr(1, part.size() - 2);

      // [bank] = reset to default bank
      if (inner == "bank") {
        tok.type = TokenType::BankSwitch;
        tokens.push_back(tok);
        continue;
      }

      auto eq = inner.find('=');
      if (eq != std::string::npos) {
        std::string key = inner.substr(0, eq);
        std::string valStr = inner.substr(eq + 1);

        // [bank=NAME] = switch to named bank
        if (key == "bank") {
          tok.type = TokenType::BankSwitch;
          tok.bankName = valStr;
          tokens.push_back(tok);
          continue;
        }

        // Numeric directive: [key=number]
        char* end = nullptr;
        double val = std::strtod(valStr.c_str(), &end);
        if (end != valStr.c_str() && static_cast<std::size_t>(end - valStr.c_str()) == valStr.size()) {
          tok.type = TokenType::Directive;
          tok.directiveKey = key;
          tok.directiveValue = val;
          tok.directiveMode = DirectiveMode::Absolute;
          tokens.push_back(tok);
          continue;
        }
      }
    }

    // Compact directive: single lowercase letter + optional value
    if (!part.empty() && std::islower(static_cast<unsigned char>(part[0]))) {
      const char* key = compactKeyMap(part[0]);

      // Note-name form: bC4, bC#5, bDb3, bA-1 (b + optional = + note name)
      if (part[0] == 'b' && part.size() > 1) {
        std::size_t npos = 1;
        if (npos < part.size() && part[npos] == '=') ++npos;
        if (npos < part.size() && std::isupper(static_cast<unsigned char>(part[npos]))) {
          double hz = noteToHz(part.substr(npos));
          if (hz > 0.0) {
            tok.type = TokenType::Directive;
            tok.directiveKey = "base";
            tok.directiveValue = hz;
            tok.directiveMode = DirectiveMode::Absolute;
            tokens.push_back(tok);
            continue;
          }
        }
      }

      if (key && part.size() == 1) {
        // Bare letter = reset (but bare `p` is dropped, matching JS)
        if (std::strcmp(key, "pause") == 0) continue;
        tok.type = TokenType::Directive;
        tok.directiveKey = key;
        tok.directiveMode = DirectiveMode::Reset;
        tokens.push_back(tok);
        continue;
      }
      if (key && part.size() > 1) {
        std::size_t pos = 1;
        bool hasEquals = (part[pos] == '=');
        if (hasEquals) ++pos;
        bool hasSign = (pos < part.size() && (part[pos] == '+' || part[pos] == '-'));
        double val = 0.0;
        if (parseDouble(part, pos, val) && pos == part.size()) {
          tok.type = TokenType::Directive;
          tok.directiveKey = key;
          tok.directiveValue = val;
          // Relative if no '=' and the value had an explicit +/- sign
          tok.directiveMode = (!hasEquals && hasSign) ? DirectiveMode::Relative
                                                      : DirectiveMode::Absolute;
          tokens.push_back(tok);
          continue;
        }
      }
    }

    // Phoneme: uppercase letters, optional stress mark, optional pitch delta
    {
      std::size_t pos = 0;
      // Consume uppercase letters
      while (pos < part.size() && std::isupper(static_cast<unsigned char>(part[pos])))
        ++pos;

      if (pos > 0) {
        std::string code = part.substr(0, pos);

        // Optional inline stress mark
        bool stressed = false;
        if (pos < part.size() && (part[pos] == '\'' || part[pos] == '!')) {
          stressed = true;
          ++pos;
        }

        double pitchDelta = 0.0;
        bool isTransient = false;

        if (pos < part.size() && part[pos] == '(') {
          // Transient pitch delta: AE(+40)
          ++pos;  // skip '('
          double val = 0.0;
          if (parseDouble(part, pos, val) && pos < part.size() && part[pos] == ')') {
            ++pos;  // skip ')'
            pitchDelta = val;
            isTransient = true;
          }
        } else if (pos < part.size() && (part[pos] == '+' || part[pos] == '-')) {
          // Sticky pitch delta: AY+15
          double val = 0.0;
          if (parseDouble(part, pos, val)) {
            pitchDelta = val;
            isTransient = false;
          }
        }

        if (pos == part.size()) {
          tok.type = TokenType::Phoneme;
          tok.text = code;
          tok.stressed = stressed;
          tok.pitchDelta = pitchDelta;
          tok.transient = isTransient;
          tokens.push_back(tok);
          continue;
        }
      }
    }

    // Unknown token
    tok.type = TokenType::Unknown;
    tok.text = part;
    tokens.push_back(tok);
  }

  return tokens;
}

// Scaled params builder

// Build a ParamUpdate from a phoneme record, applying scale to formants.
static ParamUpdate buildUpdate(const PhonemeRecord& rec, double f0,
                               double scale, bool useGlide,
                               double vibratoDepth, double vibratoRate,
                               double tremoloDepth, double tremoloRate,
                               double aspiration, double tilt, double effort) {
  const Params& src = useGlide ? rec.glideTo : rec.params;

  ParamUpdate u;
  u.set(ParamId::F0, static_cast<float>(f0));
  u.set(ParamId::Voicing, src.voicing);
  u.set(ParamId::F1, static_cast<float>(src.f1 * scale));
  u.set(ParamId::BW1, static_cast<float>(src.bw1 * scale));
  u.set(ParamId::A1, src.a1);
  u.set(ParamId::F2, static_cast<float>(src.f2 * scale));
  u.set(ParamId::BW2, static_cast<float>(src.bw2 * scale));
  u.set(ParamId::A2, src.a2);
  u.set(ParamId::F3, static_cast<float>(src.f3 * scale));
  u.set(ParamId::BW3, static_cast<float>(src.bw3 * scale));
  u.set(ParamId::A3, src.a3);
  u.set(ParamId::VibratoDepth, static_cast<float>(vibratoDepth));
  u.set(ParamId::VibratoRate, static_cast<float>(vibratoRate));
  u.set(ParamId::TremoloDepth, static_cast<float>(tremoloDepth));
  u.set(ParamId::TremoloRate, static_cast<float>(tremoloRate));
  u.set(ParamId::Aspiration, static_cast<float>(aspiration));
  u.set(ParamId::Tilt, static_cast<float>(tilt));
  u.set(ParamId::Effort, static_cast<float>(effort));
  return u;
}

// Build a silence ParamUpdate (A1=0, A2=0, A3=0 + stateExtras).
static ParamUpdate buildSilence(double vibratoDepth, double vibratoRate,
                                double tremoloDepth, double tremoloRate,
                                double aspiration, double tilt, double effort) {
  ParamUpdate u;
  u.set(ParamId::A1, 0.0f);
  u.set(ParamId::A2, 0.0f);
  u.set(ParamId::A3, 0.0f);
  u.set(ParamId::VibratoDepth, static_cast<float>(vibratoDepth));
  u.set(ParamId::VibratoRate, static_cast<float>(vibratoRate));
  u.set(ParamId::TremoloDepth, static_cast<float>(tremoloDepth));
  u.set(ParamId::TremoloRate, static_cast<float>(tremoloRate));
  u.set(ParamId::Aspiration, static_cast<float>(aspiration));
  u.set(ParamId::Tilt, static_cast<float>(tilt));
  u.set(ParamId::Effort, static_cast<float>(effort));
  return u;
}

}  // namespace

CompileResult compileString(const std::string& src,
                            const CompileOptions& options,
                            const BankRegistry& banks,
                            std::uint32_t sampleRate) noexcept {
  CompileResult result;

  // Initialize running state from options
  double f0 = options.baseF0;
  double rate = options.rate;
  double scale = options.scale;
  double vibrato = options.vibratoDepth;
  double vibratoRate = options.vibratoRate;
  double tremolo = options.tremoloDepth;
  double tremoloRate = options.tremoloRate;
  double aspiration = options.aspiration;
  double tilt = options.tilt;
  double effort = options.effort;

  const PhonemeBank* activeBank = &banks.resolve(banks.defaultName());

  const double msToSamples = static_cast<double>(sampleRate) / 1000.0;

  auto msToSample = [&](double ms) -> std::uint64_t {
    return static_cast<std::uint64_t>(std::floor(ms * msToSamples));
  };
  auto msToTransition = [&](double ms) -> std::uint32_t {
    auto v = static_cast<std::uint32_t>(std::floor(ms * msToSamples));
    return v < 1 ? 1 : v;
  };

  double timeMs = 0.0;

  // Emit helper
  auto emit = [&](const ParamUpdate& target, double transitionMs) {
    ScheduleEvent ev;
    ev.atSample = msToSample(timeMs);
    ev.transitionSamples = msToTransition(transitionMs);
    ev.target = target;
    result.events.push_back(ev);
  };

  // Silence helper
  auto silence = [&](double transitionMs = 30.0) {
    emit(buildSilence(vibrato, vibratoRate, tremolo, tremoloRate,
                      aspiration, tilt, effort),
         transitionMs);
  };

  // Tokenize
  std::string normalized = normalizeUnicode(src);
  std::vector<Token> tokens = tokenize(normalized);

  // Helper: apply a directive to running state.
  auto applyDirective = [&](const Token& d) {
    const std::string& key = d.directiveKey;
    double val = d.directiveValue;
    DirectiveMode mode = d.directiveMode;

    auto apply = [&](double& state, double initial) {
      if (mode == DirectiveMode::Reset) state = initial;
      else if (mode == DirectiveMode::Relative) state += val;
      else state = val;
    };

    if (key == "base" || key == "pitch") apply(f0, options.baseF0);
    else if (key == "rate") apply(rate, options.rate);
    else if (key == "scale") apply(scale, options.scale);
    else if (key == "vibrato") apply(vibrato, options.vibratoDepth);
    else if (key == "vibratoRate") apply(vibratoRate, options.vibratoRate);
    else if (key == "tremolo") apply(tremolo, options.tremoloDepth);
    else if (key == "tremoloRate") apply(tremoloRate, options.tremoloRate);
    else if (key == "aspiration") apply(aspiration, options.aspiration);
    else if (key == "tilt") apply(tilt, options.tilt);
    else if (key == "effort") apply(effort, options.effort);
    else if (key == "pause") {
      silence();
      timeMs += std::abs(val);
    }
  };

  // Helper: render a single phoneme token into the schedule.
  auto renderPhoneme = [&](const Token& t, double slotMs) {
    const PhonemeRecord* p = findPhoneme(*activeBank, t.text.c_str());
    if (!p) {
      result.errors.push_back(
          ParseError{"unknown phoneme: " + t.text, t.srcStart, 0});
      return;
    }

    double startF0 = t.stressed ? f0 + kStressF0Lift : f0;
    double endF0 = startF0 + t.pitchDelta;

    if (p->isStop) {
      double burstMs = std::min(static_cast<double>(kStopBurstMs), slotMs * 0.3);
      double silenceMs = slotMs - burstMs;
      silence(std::min(20.0, silenceMs * 0.4));
      timeMs += silenceMs;
      emit(buildUpdate(*p, startF0, scale, false,
                       vibrato, vibratoRate, tremolo, tremoloRate,
                       aspiration, tilt, effort),
           std::min(5.0, burstMs * 0.2));
      timeMs += burstMs;
    } else if (p->hasGlide) {
      double onset = slotMs * 0.25;
      double glide = slotMs * 0.50;
      double offset = slotMs * 0.25;
      emit(buildUpdate(*p, startF0, scale, false,
                       vibrato, vibratoRate, tremolo, tremoloRate,
                       aspiration, tilt, effort),
           std::min(20.0, onset));
      timeMs += onset;
      emit(buildUpdate(*p, endF0, scale, true,
                       vibrato, vibratoRate, tremolo, tremoloRate,
                       aspiration, tilt, effort),
           glide);
      timeMs += glide + offset;
    } else if (t.pitchDelta != 0.0) {
      emit(buildUpdate(*p, startF0, scale, false,
                       vibrato, vibratoRate, tremolo, tremoloRate,
                       aspiration, tilt, effort),
           std::min(25.0, slotMs * 0.25));
      timeMs += slotMs * 0.25;
      emit(buildUpdate(*p, endF0, scale, false,
                       vibrato, vibratoRate, tremolo, tremoloRate,
                       aspiration, tilt, effort),
           slotMs * 0.6);
      timeMs += slotMs * 0.75;
    } else {
      double trans = std::min(static_cast<double>(kDefaultTransitionMs), slotMs * 0.4);
      emit(buildUpdate(*p, startF0, scale, false,
                       vibrato, vibratoRate, tremolo, tremoloRate,
                       aspiration, tilt, effort),
           trans);
      timeMs += slotMs;
    }
  };

  // Syllable grouping state
  bool inSyllable = false;
  std::vector<std::size_t> syllableQueue;  // indices into tokens

  auto flushSyllable = [&]() {
    if (syllableQueue.empty()) { inSyllable = false; return; }
    double slot = rate / static_cast<double>(syllableQueue.size());
    for (std::size_t idx : syllableQueue) {
      const Token& st = tokens[idx];
      renderPhoneme(st, slot);
      if (!st.transient) f0 += st.pitchDelta;
    }
    syllableQueue.clear();
    inSyllable = false;
  };

  // Compile loop
  for (std::size_t ti = 0; ti < tokens.size(); ++ti) {
    const Token& t = tokens[ti];

    if (t.type == TokenType::Unknown) {
      result.errors.push_back(
          ParseError{"unknown token: " + t.text, t.srcStart, ti});
      continue;
    }

    if (t.type == TokenType::SyllableOpen) {
      if (inSyllable) {
        result.errors.push_back(
            ParseError{"nested ( ignored", t.srcStart, ti});
        continue;
      }
      inSyllable = true;
      syllableQueue.clear();
      continue;
    }

    if (t.type == TokenType::SyllableClose) {
      if (!inSyllable) {
        result.errors.push_back(
            ParseError{"unmatched )", t.srcStart, ti});
        continue;
      }
      flushSyllable();
      continue;
    }

    if (t.type == TokenType::Pause) {
      silence();
      timeMs += t.pauseMs;
      continue;
    }

    if (t.type == TokenType::Directive) {
      applyDirective(t);
      continue;
    }

    if (t.type == TokenType::BankSwitch) {
      if (t.bankName.empty()) {
        activeBank = &banks.resolve(banks.defaultName());
      } else {
        const PhonemeBank* b = banks.get(t.bankName);
        if (b) {
          activeBank = b;
        } else {
          result.errors.push_back(
              ParseError{"unknown bank: " + t.bankName, t.srcStart, ti});
        }
      }
      continue;
    }

    // Phoneme: defer to syllable buffer if inside (...), otherwise render directly
    if (inSyllable) {
      syllableQueue.push_back(ti);
      continue;
    }

    double phoneRate = t.stressed ? rate * kStressDurationFactor : rate;
    renderPhoneme(t, phoneRate);

    // Sticky pitch delta
    if (!t.transient) f0 += t.pitchDelta;
  }

  // Flush any unclosed syllable group
  if (inSyllable) {
    flushSyllable();
  }

  // Final fade-out
  timeMs += kSentenceFinalHoldMs;
  silence(kFadeOutMs);
  timeMs += kTrailOffMs;

  result.totalMs = static_cast<std::uint32_t>(std::round(timeMs));
  result.totalSamples = static_cast<std::uint32_t>(msToSample(timeMs));

  return result;
}

}  // namespace klattsch
