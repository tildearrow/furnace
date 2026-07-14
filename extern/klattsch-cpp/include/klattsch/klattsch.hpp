#ifndef KLATTSCH_KLATTSCH_HPP
#define KLATTSCH_KLATTSCH_HPP

#include <array>
#include <cstddef>
#include <cstdint>

namespace klattsch {

constexpr std::size_t kNumParams = 19;

enum class ParamId : std::uint8_t {
  F0 = 0,
  Voicing,
  F1,
  BW1,
  A1,
  F2,
  BW2,
  A2,
  F3,
  BW3,
  A3,
  Gain,
  VibratoDepth,
  VibratoRate,
  TremoloDepth,
  TremoloRate,
  Aspiration,
  Tilt,
  Effort,
};

struct Params {
  float f0 = 120.0f;
  float voicing = 0.0f;

  float f1 = 500.0f;
  float bw1 = 80.0f;
  float a1 = 0.0f;

  float f2 = 1500.0f;
  float bw2 = 120.0f;
  float a2 = 0.0f;

  float f3 = 2500.0f;
  float bw3 = 160.0f;
  float a3 = 0.0f;

  float gain = 3.5f;

  float vibratoDepth = 0.0f;
  float vibratoRate = 5.0f;
  float tremoloDepth = 0.0f;
  float tremoloRate = 5.0f;

  float aspiration = 0.0f;
  float tilt = 0.0f;
  float effort = 0.5f;
};

struct ParamUpdate {
  std::uint32_t mask = 0;
  std::array<float, kNumParams> values{};

  void set(ParamId id, float value) noexcept {
    const auto i = static_cast<std::uint8_t>(id);
    mask |= (std::uint32_t{1} << i);
    values[i] = value;
  }

  bool has(ParamId id) const noexcept {
    const auto i = static_cast<std::uint8_t>(id);
    return (mask & (std::uint32_t{1} << i)) != 0;
  }

  float get(ParamId id) const noexcept {
    return values[static_cast<std::uint8_t>(id)];
  }

  void clear() noexcept {
    mask = 0;
    values = {};
  }
};

struct ScheduleEvent {
  std::uint64_t atSample;
  std::uint32_t transitionSamples;
  ParamUpdate target;
};

struct PhonemeRecord {
  const char* name;
  Params params;
  Params glideTo;
  bool hasGlide;
  bool isStop;
};

struct PhonemeBank {
  const char* name;
  const PhonemeRecord* records;
  std::size_t count;
  // name of the bank this one extends (inherits phonemes from), or nullptr for
  // a base bank. resolution overlays this bank's records onto the parent's:
  // same-name records override, new records append. see BankRegistry.
  const char* extends = nullptr;
};

namespace banks {
extern const PhonemeBank klatt1980_en;
extern const PhonemeBank ja_mokhtari_2000;
extern const PhonemeBank ja_hecko_2026;
}

const PhonemeRecord* findPhoneme(const PhonemeBank& bank, const char* name) noexcept;

ParamUpdate paramsToFullUpdate(const Params& p) noexcept;

class Synth {
 public:
  static constexpr std::uint32_t kDefaultNoiseSeed = 0xACE1ACE1u;

  explicit Synth(std::uint32_t sampleRate, std::uint32_t noiseSeed = kDefaultNoiseSeed);
  ~Synth();

  Synth(const Synth&) = delete;
  Synth& operator=(const Synth&) = delete;
  Synth(Synth&&) noexcept;
  Synth& operator=(Synth&&) noexcept;

  std::uint32_t sampleRate() const noexcept;
  float target(ParamId id) const noexcept;

  void reset(const Params& initial = Params{}) noexcept;

  void setTarget(const ParamUpdate& update, std::uint32_t transitionSamples) noexcept;

  void setSchedule(const ScheduleEvent* events, std::size_t count) noexcept;
  void setScheduleView(const ScheduleEvent* sortedEvents,
                       std::size_t count) noexcept;
  void clearSchedule() noexcept;
  // Immediately reaches the final target of the active transition and all
  // remaining scheduled events. Useful when reconstructing state without
  // rendering the intervening samples.
  void finishPending() noexcept;

  void process(float* monoOut, std::size_t frames) noexcept;
  void addTo(float* monoOut, std::size_t frames, float gain = 1.0f) noexcept;

 private:
  struct State;
  State* state_;
};

}  // namespace klattsch

#endif  // KLATTSCH_KLATTSCH_HPP
