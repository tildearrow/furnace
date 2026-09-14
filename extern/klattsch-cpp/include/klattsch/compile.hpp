#ifndef KLATTSCH_COMPILE_HPP
#define KLATTSCH_COMPILE_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <klattsch/klattsch.hpp>

namespace klattsch {

class BankRegistry;

struct CompileOptions {
  float baseF0 = 120.0f;
  float rate = 110.0f;
  float scale = 1.0f;
  float vibratoDepth = 0.0f;
  float vibratoRate = 5.0f;
  float tremoloDepth = 0.0f;
  float tremoloRate = 5.0f;
  float aspiration = 0.0f;
  float tilt = 0.0f;
  float effort = 0.5f;
};

struct ParseError {
  std::string message;
  std::size_t byteOffset;
  std::size_t tokenIndex;
};

struct PhraseSpan {
  std::size_t srcStart;
  std::size_t srcEnd;
  std::uint32_t tStartMs;
  std::uint32_t tEndMs;
  std::string text;
};

struct CompileResult {
  std::vector<ScheduleEvent> events;
  std::uint32_t totalSamples = 0;
  std::uint32_t totalMs = 0;
  std::vector<PhraseSpan> phrases;
  std::vector<ParseError> errors;
};

CompileResult compileString(const std::string& src,
                            const CompileOptions& options,
                            const BankRegistry& banks,
                            std::uint32_t sampleRate = 48000) noexcept;

}  // namespace klattsch

#endif
