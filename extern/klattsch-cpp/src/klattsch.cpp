#include <klattsch/klattsch.hpp>
#include <klattsch/klattsch_c.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <utility>
#include <vector>

namespace klattsch {
namespace {

// Internal synthesis precision. Defaults to double (like JS uses). Define
// KLATTSCH_SINGLE_PRECISION to use float on single-precision-FPU targets. The
// public API is floats either way.
#if defined(KLATTSCH_SINGLE_PRECISION)
using real = float;
#else
using real = double;
#endif

constexpr real kPi = static_cast<real>(3.14159265358979323846);
constexpr real kTau = static_cast<real>(2) * kPi;
constexpr float kSoftClipThreshold = 0.85f;

inline float clampf(float x, float lo, float hi) noexcept {
  return std::min(std::max(x, lo), hi);
}

inline real clampr(real x, real lo, real hi) noexcept {
  return std::min(std::max(x, lo), hi);
}

class BandpassBiquad {
 public:
  BandpassBiquad() = default;

  void setFreq(float f, float bw, float sr) noexcept {
    if (f == lastF_ && bw == lastBw_) return;
    lastF_ = f;
    lastBw_ = bw;
    const real fd = clampr(static_cast<real>(f), static_cast<real>(40),
                           static_cast<real>(sr) * static_cast<real>(0.45));
    const real bwd = std::max(static_cast<real>(bw), static_cast<real>(20));
    const real w0 = static_cast<real>(2) * kPi * fd / static_cast<real>(sr);
    const real cosw0 = std::cos(w0);
    const real sinw0 = std::sin(w0);
    const real q = fd / bwd;
    const real alpha = sinw0 / (static_cast<real>(2) * q);
    const real a0 = static_cast<real>(1) + alpha;
    b0_ = alpha / a0;
    b1_ = static_cast<real>(0);
    b2_ = -alpha / a0;
    a1_ = static_cast<real>(-2) * cosw0 / a0;
    a2_ = (static_cast<real>(1) - alpha) / a0;
  }

  real process(real x) noexcept {
    const real y = b0_ * x + b1_ * x1_ + b2_ * x2_ - a1_ * y1_ - a2_ * y2_;
    x2_ = x1_;
    x1_ = x;
    y2_ = y1_;
    y1_ = y;
    return y;
  }

  void reset() noexcept {
    x1_ = x2_ = y1_ = y2_ = static_cast<real>(0);
  }

 private:
  real x1_ = 0, x2_ = 0, y1_ = 0, y2_ = 0;
  real b0_ = 0, b1_ = 0, b2_ = 0, a1_ = 0, a2_ = 0;
  float lastF_ = -1.0f;
  float lastBw_ = -1.0f;
};

class Xorshift32 {
 public:
  explicit Xorshift32(std::uint32_t seed = 0xACE1ACE1u) noexcept : state_(seed) {}

  real nextSample() noexcept {
    std::uint32_t x = state_;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    state_ = x;
    return static_cast<real>(static_cast<std::int32_t>(x)) /
           static_cast<real>(2147483648.0);
  }

  void reset(std::uint32_t seed) noexcept { state_ = seed; }

  std::uint32_t state() const noexcept { return state_; }

 private:
  std::uint32_t state_;
};

inline real glottalPulse(real phase, real effort) noexcept {
  const real e = clampr(effort, static_cast<real>(0), static_cast<real>(1));
  const real tp = static_cast<real>(0.5) - e * static_cast<real>(0.2);
  const real tn = static_cast<real>(0.25) - e * static_cast<real>(0.17);
  constexpr real kNorm = static_cast<real>(0.1);
  if (phase < tp) {
    return kNorm * static_cast<real>(0.5) * (kPi / tp) *
           std::sin(kPi * phase / tp);
  }
  if (phase < tp + tn) {
    return -kNorm * (kPi / (static_cast<real>(2) * tn)) *
           std::sin(kPi * (phase - tp) / (static_cast<real>(2) * tn));
  }
  return static_cast<real>(0);
}

inline float softClip(float x) noexcept {
  const float a = std::fabs(x);
  if (a <= kSoftClipThreshold) return x;
  const float sign = x < 0.0f ? -1.0f : 1.0f;
  const float excess = a - kSoftClipThreshold;
  return sign * (kSoftClipThreshold + (1.0f - kSoftClipThreshold) * excess / (excess + 1.0f));
}

inline std::array<real, kNumParams> paramsToArray(const Params& p) noexcept {
  return {{
      p.f0, p.voicing,
      p.f1, p.bw1, p.a1,
      p.f2, p.bw2, p.a2,
      p.f3, p.bw3, p.a3,
      p.gain,
      p.vibratoDepth, p.vibratoRate,
      p.tremoloDepth, p.tremoloRate,
      p.aspiration, p.tilt, p.effort,
  }};
}

inline real& at(std::array<real, kNumParams>& a, ParamId id) noexcept {
  return a[static_cast<std::size_t>(id)];
}

inline real at(const std::array<real, kNumParams>& a, ParamId id) noexcept {
  return a[static_cast<std::size_t>(id)];
}

}  // namespace

struct Synth::State {
  float sr;
  std::uint32_t sampleRate;
  std::uint32_t noiseSeed;

  std::array<real, kNumParams> current;
  std::array<real, kNumParams> target;
  std::array<real, kNumParams> increment;
  std::uint32_t transitionSamples;

  real glottalPhase;
  real vibratoPhase;
  real tremoloPhase;
  float tiltPrev;

  BandpassBiquad bp1;
  BandpassBiquad bp2;
  BandpassBiquad bp3;

  Xorshift32 noise;

  std::vector<ScheduleEvent> schedule;
  const ScheduleEvent* schedulePtr = nullptr;
  std::size_t scheduleCount = 0;
  std::size_t scheduleIdx;
  std::uint64_t sampleClock;

  State(std::uint32_t srIn, std::uint32_t seed)
      : sr(static_cast<float>(srIn)),
        sampleRate(srIn),
        noiseSeed(seed),
        current(paramsToArray(Params{})),
        target(paramsToArray(Params{})),
        increment{},
        transitionSamples(0),
        glottalPhase(0),
        vibratoPhase(0),
        tremoloPhase(0),
        tiltPrev(0.0f),
        noise(seed),
        scheduleIdx(0),
        sampleClock(0) {}

  void applyMaskedTarget(const ParamUpdate& update) noexcept {
    for (std::size_t i = 0; i < kNumParams; ++i) {
      if (update.mask & (std::uint32_t{1} << i)) {
        target[i] = static_cast<real>(update.values[i]);
      }
    }
  }

  void recomputeIncrements() noexcept {
    const real n = static_cast<real>(std::max<std::uint32_t>(transitionSamples, 1));
    for (std::size_t i = 0; i < kNumParams; ++i) {
      increment[i] = (target[i] - current[i]) / n;
    }
  }

  bool popDueEvent(ScheduleEvent& out) noexcept {
    if (!schedulePtr || scheduleIdx >= scheduleCount) return false;
    const ScheduleEvent& evt = schedulePtr[scheduleIdx];
    if (evt.atSample <= sampleClock) {
      out = evt;
      ++scheduleIdx;
      return true;
    }
    return false;
  }
};

Synth::Synth(std::uint32_t sampleRate, std::uint32_t noiseSeed)
    : state_(new State(sampleRate, noiseSeed)) {}

Synth::~Synth() { delete state_; }

Synth::Synth(Synth&& other) noexcept : state_(other.state_) {
  other.state_ = nullptr;
}

Synth& Synth::operator=(Synth&& other) noexcept {
  if (this != &other) {
    delete state_;
    state_ = other.state_;
    other.state_ = nullptr;
  }
  return *this;
}

std::uint32_t Synth::sampleRate() const noexcept {
  return state_ ? state_->sampleRate : 0;
}

float Synth::target(ParamId id) const noexcept {
  if (!state_) return 0.0f;
  return static_cast<float>(at(state_->target, id));
}

void Synth::reset(const Params& initial) noexcept {
  if (!state_) return;
  state_->current = paramsToArray(initial);
  state_->target = state_->current;
  state_->increment.fill(static_cast<real>(0));
  state_->transitionSamples = 0;
  state_->glottalPhase = 0;
  state_->vibratoPhase = 0;
  state_->tremoloPhase = 0;
  state_->tiltPrev = 0.0f;
  state_->bp1.reset();
  state_->bp2.reset();
  state_->bp3.reset();
  state_->noise.reset(state_->noiseSeed);
  state_->schedule.clear();
  state_->schedulePtr = nullptr;
  state_->scheduleCount = 0;
  state_->scheduleIdx = 0;
  state_->sampleClock = 0;
}

void Synth::setTarget(const ParamUpdate& update, std::uint32_t transitionSamples) noexcept {
  if (!state_) return;
  state_->transitionSamples = std::max<std::uint32_t>(transitionSamples, 1);
  state_->applyMaskedTarget(update);
  state_->recomputeIncrements();
}

void Synth::setSchedule(const ScheduleEvent* events, std::size_t count) noexcept {
  if (!state_) return;
  state_->schedule.assign(events, events + count);
  std::sort(state_->schedule.begin(), state_->schedule.end(),
            [](const ScheduleEvent& a, const ScheduleEvent& b) { return a.atSample < b.atSample; });
  state_->schedulePtr = state_->schedule.data();
  state_->scheduleCount = state_->schedule.size();
  state_->scheduleIdx = 0;
  state_->sampleClock = 0;
}

void Synth::setScheduleView(const ScheduleEvent* sortedEvents, std::size_t count) noexcept {
  if (!state_) return;
  state_->schedule.clear();
  state_->schedule.shrink_to_fit();
  state_->schedulePtr = sortedEvents;
  state_->scheduleCount = count;
  state_->scheduleIdx = 0;
  state_->sampleClock = 0;
}

void Synth::clearSchedule() noexcept {
  if (!state_) return;
  state_->schedule.clear();
  state_->schedulePtr = nullptr;
  state_->scheduleCount = 0;
  state_->scheduleIdx = 0;
}

void Synth::finishPending() noexcept {
  if (!state_) return;
  State& s = *state_;
  while (s.schedulePtr && s.scheduleIdx < s.scheduleCount) {
    s.applyMaskedTarget(s.schedulePtr[s.scheduleIdx].target);
    ++s.scheduleIdx;
  }
  s.current = s.target;
  s.increment.fill(static_cast<real>(0));
  s.transitionSamples = 0;
  s.schedule.clear();
  s.schedulePtr = nullptr;
  s.scheduleCount = 0;
  s.scheduleIdx = 0;
  s.sampleClock = 0;
}

void Synth::process(float* monoOut, std::size_t frames) noexcept {
  if (!state_ || !monoOut) return;
  State& s = *state_;
  const real srReal = static_cast<real>(s.sr);

  for (std::size_t i = 0; i < frames; ++i) {
    ScheduleEvent evt;
    while (s.popDueEvent(evt)) {
      s.transitionSamples = std::max<std::uint32_t>(evt.transitionSamples, 1);
      s.applyMaskedTarget(evt.target);
      s.recomputeIncrements();
    }
    ++s.sampleClock;

    if (s.transitionSamples > 0) {
      for (std::size_t k = 0; k < kNumParams; ++k) {
        s.current[k] += s.increment[k];
      }
      --s.transitionSamples;
      if (s.transitionSamples == 0) {
        s.current = s.target;
      }
    }

    const real curVibratoRate = at(s.current, ParamId::VibratoRate);
    const real curVibratoDepth = at(s.current, ParamId::VibratoDepth);
    const real curTremoloRate = at(s.current, ParamId::TremoloRate);
    const real curTremoloDepth = at(s.current, ParamId::TremoloDepth);
    const real curF0 = at(s.current, ParamId::F0);
    const real curVoicing = at(s.current, ParamId::Voicing);
    const real curEffort = at(s.current, ParamId::Effort);
    const real curAspiration = at(s.current, ParamId::Aspiration);
    const real curA1 = at(s.current, ParamId::A1);
    const real curA2 = at(s.current, ParamId::A2);
    const real curA3 = at(s.current, ParamId::A3);
    const real curGain = at(s.current, ParamId::Gain);
    const real curTilt = at(s.current, ParamId::Tilt);

    s.vibratoPhase += kTau * curVibratoRate / srReal;
    s.vibratoPhase -= kTau * std::floor(s.vibratoPhase / kTau);
    const real effF0 = curF0 + curVibratoDepth * std::sin(s.vibratoPhase);

    s.tremoloPhase += kTau * curTremoloRate / srReal;
    s.tremoloPhase -= kTau * std::floor(s.tremoloPhase / kTau);
    const real tremoloMod =
        static_cast<real>(1) -
        curTremoloDepth *
            (static_cast<real>(0.5) + static_cast<real>(0.5) * std::sin(s.tremoloPhase));

    const real v = clampr(curVoicing, static_cast<real>(0), static_cast<real>(1));
    const real noiseSample = s.noise.nextSample();
    const real pulseVal = glottalPulse(s.glottalPhase, curEffort);
    const real voicedGain = static_cast<real>(1) - curAspiration * static_cast<real>(0.85);
    const real exc = v * pulseVal * voicedGain +
                     (static_cast<real>(1) - v) * noiseSample * static_cast<real>(0.35) +
                     curAspiration * noiseSample * static_cast<real>(0.5);
    s.glottalPhase += effF0 / srReal;
    s.glottalPhase -= std::floor(s.glottalPhase);

    s.bp1.setFreq(static_cast<float>(at(s.current, ParamId::F1)),
                  static_cast<float>(at(s.current, ParamId::BW1)), s.sr);
    s.bp2.setFreq(static_cast<float>(at(s.current, ParamId::F2)),
                  static_cast<float>(at(s.current, ParamId::BW2)), s.sr);
    s.bp3.setFreq(static_cast<float>(at(s.current, ParamId::F3)),
                  static_cast<float>(at(s.current, ParamId::BW3)), s.sr);

    const real y = (s.bp1.process(exc) * curA1 +
                    s.bp2.process(exc) * curA2 +
                    s.bp3.process(exc) * curA3) *
                   curGain * tremoloMod;

    const real tilted = y - curTilt * static_cast<real>(s.tiltPrev);
    s.tiltPrev = static_cast<float>(y);
    monoOut[i] = softClip(static_cast<float>(tilted));
  }
}

void Synth::addTo(float* monoOut, std::size_t frames, float gain) noexcept {
  if (!state_ || !monoOut) return;
  std::vector<float> scratch(frames, 0.0f);
  process(scratch.data(), frames);
  for (std::size_t i = 0; i < frames; ++i) {
    monoOut[i] += scratch[i] * gain;
  }
}

}  // namespace klattsch

extern "C" {

struct klattsch_synth {
  klattsch::Synth impl;
  klattsch_synth(std::uint32_t sr, std::uint32_t seed) : impl(sr, seed) {}
};

klattsch_synth* klattsch_synth_new(uint32_t sample_rate, uint32_t noise_seed) {
  const auto seed = noise_seed == 0 ? klattsch::Synth::kDefaultNoiseSeed : noise_seed;
  return new klattsch_synth(sample_rate, seed);
}

void klattsch_synth_free(klattsch_synth* s) { delete s; }

void klattsch_synth_reset(klattsch_synth* s) {
  if (s) s->impl.reset();
}

void klattsch_synth_set_target(klattsch_synth* s,
                               const klattsch_param_update* update,
                               uint32_t transition_samples) {
  if (!s || !update) return;
  klattsch::ParamUpdate u;
  u.mask = update->mask;
  std::copy(std::begin(update->values), std::end(update->values), u.values.begin());
  s->impl.setTarget(u, transition_samples);
}

void klattsch_synth_set_schedule(klattsch_synth* s,
                                 const klattsch_schedule_event* events,
                                 size_t count) {
  if (!s || (count > 0 && !events)) return;
  if (count == 0) {
    s->impl.clearSchedule();
    return;
  }
  std::vector<klattsch::ScheduleEvent> tmp;
  tmp.reserve(count);
  for (size_t i = 0; i < count; ++i) {
    klattsch::ScheduleEvent e;
    e.atSample = events[i].at_sample;
    e.transitionSamples = events[i].transition_samples;
    e.target.mask = events[i].target.mask;
    std::copy(std::begin(events[i].target.values), std::end(events[i].target.values),
              e.target.values.begin());
    tmp.push_back(e);
  }
  s->impl.setSchedule(tmp.data(), tmp.size());
}

void klattsch_synth_process(klattsch_synth* s, float* mono_out, size_t frames) {
  if (s) s->impl.process(mono_out, frames);
}

void klattsch_synth_add_to(klattsch_synth* s, float* mono_out, size_t frames, float gain) {
  if (s) s->impl.addTo(mono_out, frames, gain);
}

}  // extern "C"
