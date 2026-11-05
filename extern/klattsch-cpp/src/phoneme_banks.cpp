#include <klattsch/klattsch.hpp>

#include <cstring>

namespace klattsch {

namespace {

constexpr Params makeParams(float voicing,
                            float f1, float f2, float f3,
                            float bw1, float bw2, float bw3,
                            float a1, float a2, float a3,
                            float aspiration = 0.0f,
                            float effort = 0.5f) {
  return Params{
      120.0f,           // f0
      voicing,
      f1, bw1, a1,
      f2, bw2, a2,
      f3, bw3, a3,
      3.5f,             // gain
      0.0f, 5.0f,       // vibrato depth, rate
      0.0f, 5.0f,       // tremolo depth, rate
      aspiration,
      0.0f,             // tilt
      effort,
  };
}

constexpr PhonemeRecord kKlatt1980En[] = {
    {"IY", makeParams(1.0f, 310, 2020, 2960, 45, 200, 400, 1.0f, 0.9f, 0.7f),
           makeParams(1.0f, 290, 2070, 2960, 45, 200, 400, 1.0f, 0.9f, 0.7f), true,  false},
    {"IH", makeParams(1.0f, 400, 1800, 2570, 50, 100, 140, 1.0f, 0.9f, 0.7f),
           makeParams(1.0f, 470, 1600, 2600, 50, 100, 140, 1.0f, 0.9f, 0.7f), true,  false},
    {"EH", makeParams(1.0f, 530, 1680, 2500, 60,  90, 200, 1.0f, 0.9f, 0.7f),
           makeParams(1.0f, 620, 1530, 2530, 60,  90, 200, 1.0f, 0.9f, 0.7f), true,  false},
    {"AE", makeParams(1.0f, 620, 1660, 2430, 70, 150, 320, 1.0f, 0.9f, 0.7f),
           makeParams(1.0f, 650, 1490, 2470, 70, 150, 320, 1.0f, 0.9f, 0.7f), true,  false},
    {"AA", makeParams(1.0f, 700, 1220, 2600, 130, 70, 160, 1.0f, 0.9f, 0.7f),
           makeParams(1.0f, 700, 1220, 2600, 130, 70, 160, 1.0f, 0.9f, 0.7f), false, false},
    {"AO", makeParams(1.0f, 600,  990, 2570, 90, 100,  80, 1.0f, 0.9f, 0.7f),
           makeParams(1.0f, 630, 1040, 2600, 90, 100,  80, 1.0f, 0.9f, 0.7f), true,  false},
    {"AH", makeParams(1.0f, 620, 1220, 2550, 80,  50, 140, 1.0f, 0.9f, 0.7f),
           makeParams(1.0f, 620, 1220, 2550, 80,  50, 140, 1.0f, 0.9f, 0.7f), false, false},
    {"UH", makeParams(1.0f, 450, 1100, 2350, 80, 100,  80, 1.0f, 0.9f, 0.7f),
           makeParams(1.0f, 500, 1180, 2390, 80, 100,  80, 1.0f, 0.9f, 0.7f), true,  false},
    {"UW", makeParams(1.0f, 350, 1250, 2200, 65, 110, 140, 1.0f, 0.9f, 0.7f),
           makeParams(1.0f, 320,  900, 2200, 65, 110, 140, 1.0f, 0.9f, 0.7f), true,  false},
    {"ER", makeParams(1.0f, 470, 1270, 1540, 100, 60, 110, 1.0f, 0.9f, 0.7f),
           makeParams(1.0f, 420, 1310, 1540, 100, 60, 110, 1.0f, 0.9f, 0.7f), true,  false},
    {"AY", makeParams(1.0f, 660, 1200, 2550, 100, 70, 200, 1.0f, 0.9f, 0.7f),
           makeParams(1.0f, 400, 1880, 2500, 100, 70, 200, 1.0f, 0.9f, 0.7f), true,  false},
    {"AW", makeParams(1.0f, 640, 1230, 2550, 80,  70, 140, 1.0f, 0.9f, 0.7f),
           makeParams(1.0f, 420,  940, 2350, 80,  70, 140, 1.0f, 0.9f, 0.7f), true,  false},
    {"EY", makeParams(1.0f, 480, 1720, 2520, 70, 100, 200, 1.0f, 0.9f, 0.7f),
           makeParams(1.0f, 330, 2020, 2600, 70, 100, 200, 1.0f, 0.9f, 0.7f), true,  false},
    {"OW", makeParams(1.0f, 540, 1100, 2300, 80,  70,  70, 1.0f, 0.9f, 0.7f),
           makeParams(1.0f, 450,  900, 2300, 80,  70,  70, 1.0f, 0.9f, 0.7f), true,  false},
    {"OY", makeParams(1.0f, 550,  960, 2400, 80,  50, 130, 1.0f, 0.9f, 0.7f),
           makeParams(1.0f, 360, 1820, 2450, 80,  50, 130, 1.0f, 0.9f, 0.7f), true,  false},
    {"W",  makeParams(1.0f, 290,  610, 2150, 50,  80,  60, 0.8f, 0.7f, 0.5f),
           makeParams(1.0f, 290,  610, 2150, 50,  80,  60, 0.8f, 0.7f, 0.5f), false, false},
    {"Y",  makeParams(1.0f, 260, 2070, 3020, 40, 250, 500, 0.8f, 0.7f, 0.5f),
           makeParams(1.0f, 260, 2070, 3020, 40, 250, 500, 0.8f, 0.7f, 0.5f), false, false},
    {"R",  makeParams(1.0f, 310, 1060, 1380, 70, 100, 120, 0.8f, 0.7f, 0.5f),
           makeParams(1.0f, 310, 1060, 1380, 70, 100, 120, 0.8f, 0.7f, 0.5f), false, false},
    {"L",  makeParams(1.0f, 310, 1050, 2880, 50, 100, 280, 0.8f, 0.7f, 0.5f),
           makeParams(1.0f, 310, 1050, 2880, 50, 100, 280, 0.8f, 0.7f, 0.5f), false, false},
    {"M",  makeParams(1.0f, 270, 1270, 2130, 40, 200, 200, 0.7f, 0.18f, 0.1f),
           makeParams(1.0f, 270, 1270, 2130, 40, 200, 200, 0.7f, 0.18f, 0.1f), false, false},
    {"N",  makeParams(1.0f, 270, 1340, 2470, 40, 300, 300, 0.7f, 0.2f, 0.12f),
           makeParams(1.0f, 270, 1340, 2470, 40, 300, 300, 0.7f, 0.2f, 0.12f), false, false},
    {"NG", makeParams(1.0f, 270, 2000, 2700, 40, 300, 300, 0.7f, 0.2f, 0.12f),
           makeParams(1.0f, 270, 2000, 2700, 40, 300, 300, 0.7f, 0.2f, 0.12f), false, false},
    {"F",  makeParams(0.0f, 340, 1100, 2080, 200, 200, 1000, 0.0f,  0.1f,  0.15f),
           makeParams(0.0f, 340, 1100, 2080, 200, 200, 1000, 0.0f,  0.1f,  0.15f), false, false},
    {"TH", makeParams(0.0f, 320, 1290, 2540, 200, 200, 1000, 0.0f,  0.08f, 0.18f),
           makeParams(0.0f, 320, 1290, 2540, 200, 200, 1000, 0.0f,  0.08f, 0.18f), false, false},
    {"S",  makeParams(0.0f, 320, 1390, 5500, 200, 200, 1000, 0.0f,  0.0f,  0.95f),
           makeParams(0.0f, 320, 1390, 5500, 200, 200, 1000, 0.0f,  0.0f,  0.95f), false, false},
    {"SH", makeParams(0.0f, 300, 1840, 2750, 200, 200, 1000, 0.0f,  0.55f, 0.65f),
           makeParams(0.0f, 300, 1840, 2750, 200, 200, 1000, 0.0f,  0.55f, 0.65f), false, false},
    {"V",  makeParams(0.45f, 220, 1100, 2080, 80, 100, 800, 0.4f, 0.12f, 0.18f),
           makeParams(0.45f, 220, 1100, 2080, 80, 100, 800, 0.4f, 0.12f, 0.18f), false, false},
    {"DH", makeParams(0.45f, 270, 1290, 2540, 80, 100, 800, 0.4f, 0.1f,  0.2f),
           makeParams(0.45f, 270, 1290, 2540, 80, 100, 800, 0.4f, 0.1f,  0.2f), false, false},
    {"Z",  makeParams(0.45f, 240, 1390, 5500, 80, 100, 800, 0.4f, 0.0f,  0.65f),
           makeParams(0.45f, 240, 1390, 5500, 80, 100, 800, 0.4f, 0.0f,  0.65f), false, false},
    {"ZH", makeParams(0.45f, 270, 1840, 2750, 80, 100, 800, 0.4f, 0.45f, 0.55f),
           makeParams(0.45f, 270, 1840, 2750, 80, 100, 800, 0.4f, 0.45f, 0.55f), false, false},
    {"HH", makeParams(0.0f, 500, 1500, 2500, 300, 200, 300, 0.4f, 0.4f,  0.3f),
           makeParams(0.0f, 500, 1500, 2500, 300, 200, 300, 0.4f, 0.4f,  0.3f), false, false},
    {"P",  makeParams(0.0f, 400, 1100, 2150, 300, 150, 220, 0.1f, 0.2f,  0.25f),
           makeParams(0.0f, 400, 1100, 2150, 300, 150, 220, 0.1f, 0.2f,  0.25f), false, true},
    {"B",  makeParams(0.6f, 200, 1100, 2150, 60, 110, 130, 0.5f, 0.2f,  0.2f),
           makeParams(0.6f, 200, 1100, 2150, 60, 110, 130, 0.5f, 0.2f,  0.2f), false, true},
    {"T",  makeParams(0.0f, 400, 1600, 2600, 300, 120, 250, 0.0f, 0.3f,  0.55f),
           makeParams(0.0f, 400, 1600, 2600, 300, 120, 250, 0.0f, 0.3f,  0.55f), false, true},
    {"D",  makeParams(0.6f, 200, 1600, 2600, 60, 100, 170, 0.5f, 0.4f,  0.5f),
           makeParams(0.6f, 200, 1600, 2600, 60, 100, 170, 0.5f, 0.4f,  0.5f), false, true},
    {"K",  makeParams(0.0f, 300, 1990, 2850, 250, 160, 330, 0.0f, 0.5f,  0.4f),
           makeParams(0.0f, 300, 1990, 2850, 250, 160, 330, 0.0f, 0.5f,  0.4f), false, true},
    {"G",  makeParams(0.6f, 200, 1990, 2850, 60, 150, 280, 0.5f, 0.5f,  0.4f),
           makeParams(0.6f, 200, 1990, 2850, 60, 150, 280, 0.5f, 0.5f,  0.4f), false, true},
    {"CH", makeParams(0.0f, 350, 1800, 2820, 200,  90, 300, 0.0f, 0.4f,  0.55f),
           makeParams(0.0f, 350, 1800, 2820, 200,  90, 300, 0.0f, 0.4f,  0.55f), false, true},
    {"JH", makeParams(0.5f, 260, 1800, 2820, 60,  80, 270, 0.4f, 0.4f,  0.5f),
           makeParams(0.5f, 260, 1800, 2820, 60,  80, 270, 0.4f, 0.4f,  0.5f), false, true},
    {"_",  makeParams(0.0f, 500, 1500, 2500, 80, 120, 160, 0.0f, 0.0f,  0.0f),
           makeParams(0.0f, 500, 1500, 2500, 80, 120, 160, 0.0f, 0.0f,  0.0f), false, false},
};

constexpr std::size_t kKlatt1980EnCount = sizeof(kKlatt1980En) / sizeof(kKlatt1980En[0]);

constexpr PhonemeRecord kJaMokhtari2000[] = {
    {"A",  makeParams(1.0f, 744, 1240, 2426, 182, 117, 222, 1.0f, 0.9f, 0.7f),
           Params{}, false, false},
    {"I",  makeParams(1.0f, 298, 2083, 2954,  67, 130, 145, 1.0f, 0.9f, 0.7f),
           Params{}, false, false},
    {"U",  makeParams(1.0f, 355, 1282, 2233,  55, 129, 135, 1.0f, 0.9f, 0.7f),
           Params{}, false, false},
    {"E",  makeParams(1.0f, 480, 1857, 2437,  65,  97, 257, 1.0f, 0.9f, 0.7f),
           Params{}, false, false},
    {"O",  makeParams(1.0f, 460,  857, 2405,  61, 114, 134, 1.0f, 0.9f, 0.7f),
           Params{}, false, false},
    {"DX", makeParams(0.6f, 255, 1325, 2740,  55, 100, 225, 0.65f, 0.55f, 0.5f),
           Params{}, false, true},
};

constexpr std::size_t kJaMokhtari2000Count =
    sizeof(kJaMokhtari2000) / sizeof(kJaMokhtari2000[0]);

// Preserve the source bank's record order. Unlike ja-mokhtari-2000, the raw
// values for ja-hecko-2026 therefore map 28-2D to I E A O U DX.
constexpr PhonemeRecord kJaHecko2026[] = {
    {"I",  makeParams(1.0f, 350, 2000, 3000,  60, 130, 140, 1.0f, 0.9f, 0.7f),
           Params{}, false, false},
    {"E",  makeParams(1.0f, 500, 1700, 2300,  60, 100, 250, 1.0f, 0.9f, 0.7f),
           Params{}, false, false},
    {"A",  makeParams(1.0f, 750, 1250, 2250, 180, 110, 220, 1.0f, 0.9f, 0.7f),
           Params{}, false, false},
    {"O",  makeParams(1.0f, 500,  850, 2250,  60, 110, 130, 1.0f, 0.9f, 0.7f),
           Params{}, false, false},
    {"U",  makeParams(1.0f, 450, 1250, 2150,  50, 120, 130, 1.0f, 0.9f, 0.7f),
           Params{}, false, false},
    {"DX", makeParams(0.6f, 255, 1325, 2740,  55, 100, 225, 0.65f, 0.55f, 0.5f),
           Params{}, false, true},
};

constexpr std::size_t kJaHecko2026Count =
    sizeof(kJaHecko2026) / sizeof(kJaHecko2026[0]);

}  // namespace

namespace banks {
const PhonemeBank klatt1980_en{"klatt1980-en", kKlatt1980En, kKlatt1980EnCount};
const PhonemeBank ja_mokhtari_2000{"ja-mokhtari-2000",
                                   kJaMokhtari2000,
                                   kJaMokhtari2000Count,
                                   "klatt1980-en"};
const PhonemeBank ja_hecko_2026{"ja-hecko-2026",
                                kJaHecko2026,
                                kJaHecko2026Count,
                                "klatt1980-en"};
}

const PhonemeRecord* findPhoneme(const PhonemeBank& bank, const char* name) noexcept {
  if (!name) return nullptr;
  for (std::size_t i = 0; i < bank.count; ++i) {
    if (std::strcmp(bank.records[i].name, name) == 0) {
      return &bank.records[i];
    }
  }
  return nullptr;
}

ParamUpdate paramsToFullUpdate(const Params& p) noexcept {
  ParamUpdate u;
  u.set(ParamId::F0, p.f0);
  u.set(ParamId::Voicing, p.voicing);
  u.set(ParamId::F1, p.f1);
  u.set(ParamId::BW1, p.bw1);
  u.set(ParamId::A1, p.a1);
  u.set(ParamId::F2, p.f2);
  u.set(ParamId::BW2, p.bw2);
  u.set(ParamId::A2, p.a2);
  u.set(ParamId::F3, p.f3);
  u.set(ParamId::BW3, p.bw3);
  u.set(ParamId::A3, p.a3);
  u.set(ParamId::Gain, p.gain);
  u.set(ParamId::VibratoDepth, p.vibratoDepth);
  u.set(ParamId::VibratoRate, p.vibratoRate);
  u.set(ParamId::TremoloDepth, p.tremoloDepth);
  u.set(ParamId::TremoloRate, p.tremoloRate);
  u.set(ParamId::Aspiration, p.aspiration);
  u.set(ParamId::Tilt, p.tilt);
  u.set(ParamId::Effort, p.effort);
  return u;
}

}  // namespace klattsch
