// Based on ymf278b.c written by R. Belmont and O. Galibert

// Improved by Valley Bell, 2018
// Thanks to niekniek and l_oliveira for providing recordings from OPL4 hardware.
// Thanks to superctr and wouterv for discussing changes.
//
// Improvements:
// - added TL interpolation, recordings show that internal TL levels are 0x00..0xff
// - fixed ADSR speeds, attack rate 15 is now instant
// - correct clamping of intermediate Rate Correction values
// - emulation of "loop glitch" (going out-of-bounds by playing a sample faster than it the loop is long)
// - made calculation of sample position cleaner and closer to how the HW works
// - increased output resolution from TL (0.375dB) to envelope (0.09375dB)
// - fixed volume table -6dB steps are done using bit shifts, steps in between are multiplicators
// - made octave -8 freeze the sample
// - verified that TL and envelope levels are applied separately, both go silent at -60dB
// - implemented pseudo-reverb and damping according to manual
// - made pseudo-reverb ignore Rate Correction (real hardware ignores it)
// - reimplemented LFO, speed exactly matches the formulas that were probably used when creating the manual
// - fixed LFO (tremolo) amplitude modulation
// - made LFO vibrato and tremolo accurate to hardware
//
// Known issues:
// - Octave -8 was only tested with fnum 0. Other fnum values might behave differently.

// This class doesn't model a full YMF278b chip. Instead it only models the
// wave part. The FM part in modeled in YMF262 (it's almost 100% compatible,
// the small differences are handled in YMF262). The status register and
// interaction with the FM registers (e.g. the NEW2 bit) is currently handled
// in the MSXMoonSound class.

// MODIFIED:
// Add YMW258 support by Grauw
// Add DO1 output support by cam900

#include "ymf278.h"
#include <algorithm>
#include <cassert>

// envelope output entries
// fixed to match recordings from actual OPL4 -Valley Bell
constexpr int MAX_ATT_INDEX = 0x280; // makes attack phase right and also goes well with "envelope stops at -60dB"
constexpr int MIN_ATT_INDEX = 0;
constexpr int TL_SHIFT      = 2; // envelope values are 4x as fine as TL levels

constexpr unsigned LFO_SHIFT = 18; // LFO period of up to 0x40000 sample
constexpr unsigned LFO_PERIOD = 1 << LFO_SHIFT;

// Envelope Generator phases
constexpr int EG_ATT = 4;
constexpr int EG_DEC = 3;
constexpr int EG_SUS = 2;
constexpr int EG_REL = 1;
constexpr int EG_OFF = 0;

// Pan values, units are -3dB, i.e. 8.
constexpr uint8_t pan_left[16]  = {
	0, 8, 16, 24, 32, 40, 48, 255, 255,   0,  0,  0,  0,  0,  0, 0
};
constexpr uint8_t pan_right[16] = {
	0, 0,  0,  0,  0,  0,  0,   0, 255, 255, 48, 40, 32, 24, 16, 8
};

// decay level table (3dB per step)
// 0 - 15: 0, 3, 6, 9,12,15,18,21,24,27,30,33,36,39,42,93 (dB)
static constexpr int16_t SC(int dB) { return int16_t(dB / 3 * 0x20); }
constexpr int16_t dl_tab[16] = {
 SC( 0), SC( 3), SC( 6), SC( 9), SC(12), SC(15), SC(18), SC(21),
 SC(24), SC(27), SC(30), SC(33), SC(36), SC(39), SC(42), SC(93)
};

constexpr byte RATE_STEPS = 8;
constexpr byte eg_inc[15 * RATE_STEPS] = {
//cycle:0  1   2  3   4  5   6  7
	0, 1,  0, 1,  0, 1,  0, 1, //  0  rates 00..12 0 (increment by 0 or 1)
	0, 1,  0, 1,  1, 1,  0, 1, //  1  rates 00..12 1
	0, 1,  1, 1,  0, 1,  1, 1, //  2  rates 00..12 2
	0, 1,  1, 1,  1, 1,  1, 1, //  3  rates 00..12 3

	1, 1,  1, 1,  1, 1,  1, 1, //  4  rate 13 0 (increment by 1)
	1, 1,  1, 2,  1, 1,  1, 2, //  5  rate 13 1
	1, 2,  1, 2,  1, 2,  1, 2, //  6  rate 13 2
	1, 2,  2, 2,  1, 2,  2, 2, //  7  rate 13 3

	2, 2,  2, 2,  2, 2,  2, 2, //  8  rate 14 0 (increment by 2)
	2, 2,  2, 4,  2, 2,  2, 4, //  9  rate 14 1
	2, 4,  2, 4,  2, 4,  2, 4, // 10  rate 14 2
	2, 4,  4, 4,  2, 4,  4, 4, // 11  rate 14 3

	4, 4,  4, 4,  4, 4,  4, 4, // 12  rates 15 0, 15 1, 15 2, 15 3 for decay
	8, 8,  8, 8,  8, 8,  8, 8, // 13  rates 15 0, 15 1, 15 2, 15 3 for attack (zero time)
	0, 0,  0, 0,  0, 0,  0, 0, // 14  infinity rates for attack and decay(s)
};

static constexpr byte O(int a) { return a * RATE_STEPS; }
constexpr byte eg_rate_select[64] = {
	O(14),O(14),O(14),O(14), // inf rate
	O( 0),O( 1),O( 2),O( 3),
	O( 0),O( 1),O( 2),O( 3),
	O( 0),O( 1),O( 2),O( 3),
	O( 0),O( 1),O( 2),O( 3),
	O( 0),O( 1),O( 2),O( 3),
	O( 0),O( 1),O( 2),O( 3),
	O( 0),O( 1),O( 2),O( 3),
	O( 0),O( 1),O( 2),O( 3),
	O( 0),O( 1),O( 2),O( 3),
	O( 0),O( 1),O( 2),O( 3),
	O( 0),O( 1),O( 2),O( 3),
	O( 0),O( 1),O( 2),O( 3),
	O( 4),O( 5),O( 6),O( 7),
	O( 8),O( 9),O(10),O(11),
	O(12),O(12),O(12),O(12),
};

// rate  0,    1,    2,    3,   4,   5,   6,  7,  8,  9,  10, 11, 12, 13, 14, 15
// shift 12,   11,   10,   9,   8,   7,   6,  5,  4,  3,  2,  1,  0,  0,  0,  0
// mask  4095, 2047, 1023, 511, 255, 127, 63, 31, 15, 7,  3,  1,  0,  0,  0,  0
constexpr byte eg_rate_shift[64] = {
	 12, 12, 12, 12,
	 11, 11, 11, 11,
	 10, 10, 10, 10,
	  9,  9,  9,  9,
	  8,  8,  8,  8,
	  7,  7,  7,  7,
	  6,  6,  6,  6,
	  5,  5,  5,  5,
	  4,  4,  4,  4,
	  3,  3,  3,  3,
	  2,  2,  2,  2,
	  1,  1,  1,  1,
	  0,  0,  0,  0,
	  0,  0,  0,  0,
	  0,  0,  0,  0,
	  0,  0,  0,  0,
};


// number of steps the LFO counter advances per sample
//   LFO frequency (Hz) -> LFO counter steps per sample
static constexpr int L(double a) { return int((LFO_PERIOD * a) / 44100.0 + 0.5); }
constexpr int lfo_period[8] = {
	L(0.168), // step:  1, period: 262144 samples
	L(2.019), // step: 12, period:  21845 samples
	L(3.196), // step: 19, period:  13797 samples
	L(4.206), // step: 25, period:  10486 samples
	L(5.215), // step: 31, period:   8456 samples
	L(5.888), // step: 35, period:   7490 samples
	L(6.224), // step: 37, period:   7085 samples
	L(7.066), // step: 42, period:   6242 samples
};


// formula used by Yamaha docs:
//     vib_depth_cents(x) = (log2(0x400 + x) - 10) * 1200
constexpr int16_t vib_depth[8] = {
	0,      //  0.000 cents
	2,      //  3.378 cents
	3,      //  5.065 cents
	4,      //  6.750 cents
	6,      // 10.114 cents
	12,     // 20.170 cents
	24,     // 40.106 cents
	48,     // 79.307 cents
};


// formula used by Yamaha docs:
//     am_depth_db(x) = (x-1) / 0x40 * 6.0
//     They use (x-1), because the depth is multiplied with the AM counter, which has a range of 0..0x7F.
//     Thus the maximum attenuation with x=0x80 is (0x7F * 0x80) >> 7 = 0x7F.
// reversed formula:
//     am_depth(dB) = round(dB / 6.0 * 0x40) + 1
constexpr uint8_t am_depth[8] = {
	0x00,   //  0.000 dB
	0x14,   //  1.781 dB
	0x20,   //  2.906 dB
	0x28,   //  3.656 dB
	0x30,   //  4.406 dB
	0x40,   //  5.906 dB
	0x50,   //  7.406 dB
	0x80,   // 11.910 dB
};

// divisions of 16
constexpr int mix_level[8] = {
	16,     //   0dB
	12,     //  -3dB (approx)
	8,      //  -6dB
	6,      //  -9dB (approx)
	4,      // -12dB
	3,      // -15dB (approx)
	2,      // -18dB
	0,      // -inf dB
};

YMF278::Slot::Slot()
{
	reset();
}

// Sign extend a 4-bit value to int (32-bit)
// require: x in range [0..15]
static constexpr int sign_extend_4(int x)
{
	return (x ^ 8) - 8;
}

// Params: oct in [-8 ..   +7]
//         fn  in [ 0 .. 1023]
// We want to interpret oct as a signed 4-bit number and calculate
//    ((fn | 1024) + vib) << (5 + sign_extend_4(oct))
// Though in this formula the shift can go over a negative distance (in that
// case we should shift in the other direction).
static constexpr unsigned calcStep(int8_t oct, uint16_t fn, int16_t vib = 0)
{
	if (oct == -8) return 0;
	unsigned t = (fn + 1024 + vib) << (8 + oct); // use '+' iso '|' (generates slightly better code)
	return t >> 3; // was shifted 3 positions too far
}

void YMF278::Slot::reset()
{
	wave = FN = OCT = TLdest = TL = pan = vib = AM = 0;
	DL = AR = D1R = D2R = RC = RR = 0;
	PRVB = keyon = DAMP = false;
	stepptr = 0;
	step = calcStep(OCT, FN);
	bits = startaddr = loopaddr = endaddr = 0;
	env_vol = MAX_ATT_INDEX;

	lfo_active = false;
	lfo_cnt = 0;
	lfo = 0;

	state = EG_OFF;

	// not strictly needed, but avoid UMR on savestate
	pos = 0;
}

int YMF278::Slot::compute_rate(int val) const
{
	if (val == 0) {
		return 0;
	} else if (val == 15) {
		return 63;
	}
	int res = val * 4;
	if (RC != 15) {
		// clamping verified with HW tests -Valley Bell
		res += 2 * YMF_clamp(OCT + RC, 0, 15);
		res += (FN & 0x200) ? 1 : 0;
	}
	return YMF_clamp(res, 0, 63);
}

int YMF278::Slot::compute_decay_rate(int val) const
{
	if (DAMP) {
		// damping
		// The manual lists these values for time and attenuation: (44100 samples/second)
		// -12dB at  5.8ms, sample 256
		// -48dB at  8.0ms, sample 352
		// -72dB at  9.4ms, sample 416
		// -96dB at 10.9ms, sample 480
		// This results in these durations and rate values for the respective phases:
		//   0dB .. -12dB: 256 samples (5.80ms) -> 128 samples per -6dB = rate 48
		// -12dB .. -48dB:  96 samples (2.18ms) ->  16 samples per -6dB = rate 63
		// -48dB .. -72dB:  64 samples (1.45ms) ->  16 samples per -6dB = rate 63
		// -72dB .. -96dB:  64 samples (1.45ms) ->  16 samples per -6dB = rate 63
		// Damping was verified to ignore rate correction.
		if (env_vol < dl_tab[4]) {
			return 48; //   0dB .. -12dB
		} else {
			return 63; // -12dB .. -96dB
		}
	}
	if (PRVB) {
		// pseudo reverb
		// activated when reaching -18dB, overrides D1R/D2R/RR with reverb rate 5
		//
		// The manual is actually a bit unclear and just says "RATE=5",
		// referring to the D1R/D2R/RR register value. However, later
		// pages use "RATE" to refer to the "internal" rate, which is
		// (register * 4) + rate correction. HW recordings prove that
		// Rate Correction is ignored, so pseudo reverb just sets the
		// "internal" rate to a value of 4*5 = 20.
		if (env_vol >= dl_tab[6]) {
			return 20;
		}
	}
	return compute_rate(val);
}

int16_t YMF278::Slot::compute_vib() const
{
	// verified via hardware recording:
	//  With LFO speed 0 (period 262144 samples), each vibrato step takes
	//  4096 samples.
	//  -> 64 steps total
	//  Also, with vibrato depth 7 (80 cents) and an F-Num of 0x400, the
	//  final F-Nums are: 0x400 .. 0x43C, 0x43C .. 0x400, 0x400 .. 0x3C4,
	//  0x3C4 .. 0x400
	int16_t lfo_fm = lfo_cnt / (LFO_PERIOD / 0x40);
	// results in +0x00..+0x0F, +0x0F..+0x00, -0x00..-0x0F, -0x0F..-0x00
	if (lfo_fm & 0x10) lfo_fm ^= 0x1F;
	if (lfo_fm & 0x20) lfo_fm = -(lfo_fm & 0x0F);

	return (lfo_fm * vib_depth[vib]) / 12;
}

uint16_t YMF278::Slot::compute_am() const
{
	// verified via hardware recording:
	//  With LFO speed 0 (period 262144 samples), each tremolo step takes
	//  1024 samples.
	//  -> 256 steps total
	uint16_t lfo_am = lfo_cnt / (LFO_PERIOD / 0x100);
	// results in 0x00..0x7F, 0x7F..0x00
	if (lfo_am >= 0x80) lfo_am ^= 0xFF;

	return (lfo_am * am_depth[AM]) >> 7;
}


void YMF278Base::advance()
{
	eg_cnt++;

	// modulo counters for volume interpolation
	int tl_int_cnt  =  eg_cnt % 9;      // 0 .. 8
	int tl_int_step = (eg_cnt / 9) % 3; // 0 .. 2

	for (auto& op : slots) {
		// volume interpolation
		if (tl_int_cnt == 0) {
			if (tl_int_step == 0) {
				// decrease volume by one step every 27 samples
				if (op.TL < op.TLdest) ++op.TL;
			} else {
				// increase volume by one step every 13.5 samples
				if (op.TL > op.TLdest) --op.TL;
			}
		}

		if (op.lfo_active) {
			op.lfo_cnt = (op.lfo_cnt + lfo_period[op.lfo]) & (LFO_PERIOD - 1);
		}

		// Envelope Generator
		switch (op.state) {
		case EG_ATT: { // attack phase
			uint8_t rate = op.compute_rate(op.AR);
			// Verified by HW recording (and matches Nemesis' tests of the YM2612):
			// AR = 0xF during KeyOn results in instant switch to EG_DEC. (see keyOnHelper)
			// Setting AR = 0xF while the attack phase is in progress freezes the envelope.
			if (rate >= 63) {
				break;
			}
			uint8_t shift = eg_rate_shift[rate];
			if (!(eg_cnt & ((1 << shift) - 1))) {
				uint8_t select = eg_rate_select[rate];
				// >>4 makes the attack phase's shape match the actual chip -Valley Bell
				op.env_vol += (~op.env_vol * eg_inc[select + ((eg_cnt >> shift) & 7)]) >> 4;
				if (op.env_vol <= MIN_ATT_INDEX) {
					op.env_vol = MIN_ATT_INDEX;
					// TODO does the real HW skip EG_DEC completely,
					//      or is it active for 1 sample?
					op.state = op.DL ? EG_DEC : EG_SUS;
				}
			}
			break;
		}
		case EG_DEC: { // decay phase
			uint8_t rate = op.compute_decay_rate(op.D1R);
			uint8_t shift = eg_rate_shift[rate];
			if (!(eg_cnt & ((1 << shift) - 1))) {
				uint8_t select = eg_rate_select[rate];
				op.env_vol += eg_inc[select + ((eg_cnt >> shift) & 7)];
				if (op.env_vol >= op.DL) {
					op.state = (op.env_vol < MAX_ATT_INDEX) ? EG_SUS : EG_OFF;
				}
			}
			break;
		}
		case EG_SUS: { // sustain phase
			uint8_t rate = op.compute_decay_rate(op.D2R);
			uint8_t shift = eg_rate_shift[rate];
			if (!(eg_cnt & ((1 << shift) - 1))) {
				uint8_t select = eg_rate_select[rate];
				op.env_vol += eg_inc[select + ((eg_cnt >> shift) & 7)];
				if (op.env_vol >= MAX_ATT_INDEX) {
					op.env_vol = MAX_ATT_INDEX;
					op.state = EG_OFF;
				}
			}
			break;
		}
		case EG_REL: { // release phase
			uint8_t rate = op.compute_decay_rate(op.RR);
			uint8_t shift = eg_rate_shift[rate];
			if (!(eg_cnt & ((1 << shift) - 1))) {
				uint8_t select = eg_rate_select[rate];
				op.env_vol += eg_inc[select + ((eg_cnt >> shift) & 7)];
				if (op.env_vol >= MAX_ATT_INDEX) {
					op.env_vol = MAX_ATT_INDEX;
					op.state = EG_OFF;
				}
			}
			break;
		}
		case EG_OFF:
			// nothing
			break;

		default:
			UNREACHABLE;
		}
	}
}

int16_t YMF278Base::getSample(Slot& slot, uint16_t pos) const
{
	// TODO How does this behave when R#2 bit 0 = 1?
	//      As-if read returns 0xff? (Like for CPU memory reads.) Or is
	//      sound generation blocked at some higher level?
	switch (slot.bits) {
	case 0: {
		// 8 bit
		return memory[slot.startaddr + pos] << 8;
	}
	case 1: {
		// 12 bit
		unsigned addr = slot.startaddr + ((pos / 2) * 3);
		if (pos & 1) {
			return (memory[addr + 2] << 8) |
			       (memory[addr + 1] & 0xF0);
		} else {
			return (memory[addr + 0] << 8) |
			       ((memory[addr + 1] << 4) & 0xF0);
		}
	}
	case 2: {
		// 16 bit
		unsigned addr = slot.startaddr + (pos * 2);
		return (memory[addr + 0] << 8) |
		        (memory[addr + 1]);
	}
	default:
		// TODO unspecified
		return 0;
	}
}

uint16_t YMF278Base::nextPos(Slot& slot, uint16_t pos, uint16_t increment)
{
	// If there is a 4-sample loop and you advance 12 samples per step,
	// it may exceed the end offset.
	// This is abused by the "Lizard Star" song to generate noise at 0:52. -Valley Bell
	pos += increment;
	if ((uint32_t(pos) + slot.endaddr) >= 0x10000) // check position >= (negated) end address
		pos += slot.endaddr + slot.loopaddr; // This is how the actual chip does it.
	return pos;
}

bool YMF278Base::anyActive()
{
	return std::any_of(std::begin(slots), std::end(slots), [](auto& op) { return op.state != EG_OFF; });
}

// In: 'envVol', 0=max volume, others -> -3/32 = -0.09375 dB/step
// Out: 'x' attenuated by the corresponding factor.
// Note: microbenchmarks have shown that re-doing this calculation is about the
// same speed as using a 4kB lookup table.
static constexpr int vol_factor(int x, unsigned envVol)
{
	if (envVol >= MAX_ATT_INDEX) return 0; // hardware clips to silence below -60dB
	int vol_mul = 0x80 - (envVol & 0x3F); // 0x40 values per 6dB
	int vol_shift = 7 + (envVol >> 6);
	return (x * ((0x8000 * vol_mul) >> vol_shift)) >> 15;
}

void YMF278Base::generate(short& fleft, short& fright, short& rleft, short& rright, short* channelBufs)
{
	int sampleFLeft = 0;
	int sampleFRight = 0;
	int sampleRLeft = 0;
	int sampleRRight = 0;
	for (size_t i = 0, count = slots.size(); i < count; i++) {
		Slot& sl = slots[i];
		if (sl.state == EG_OFF) {
			//sampleLeft += 0;
			//sampleRight += 0;
			if (channelBufs != nullptr) {
				channelBufs[i] = 0;
			}
			continue;
		}

		int16_t sample = (getSample(sl, sl.pos) * (0x10000 - sl.stepptr) +
											getSample(sl, nextPos(sl, sl.pos, 1)) * sl.stepptr) >> 16;
		// TL levels are 00..FF internally (TL register value 7F is mapped to TL level FF)
		// Envelope levels have 4x the resolution (000..3FF)
		// Volume levels are approximate logarithmic. -6dB result in half volume. Steps in between use linear interpolation.
		// A volume of -60dB or lower results in silence. (value 0x280..0x3FF).
		// Recordings from actual hardware indicate that TL level and envelope level are applied separarely.
		// Each of them is clipped to silence below -60dB, but TL+envelope might result in a lower volume. -Valley Bell
		uint16_t envVol = std::min(sl.env_vol + ((sl.lfo_active && sl.AM) ? sl.compute_am() : 0),
																MAX_ATT_INDEX);
		int smplOut = vol_factor(vol_factor(sample, envVol), sl.TL << TL_SHIFT);

		// Panning is also done separately. (low-volume TL + low-volume panning goes below -60dB)
		// I'll be taking wild guess and assume that -3dB is approximated with 75%. (same as with TL and envelope levels)
		// The same applies to the PCM mix level.
		int32_t volLeft  = pan_left [sl.pan]; // note: register 0xF9 is handled externally
		int32_t volRight = pan_right[sl.pan];
		// 0 -> 0x20, 8 -> 0x18, 16 -> 0x10, 24 -> 0x0C, etc. (not using vol_factor here saves array boundary checks)
		volLeft  = (0x20 - (volLeft  & 0x0f)) >> (volLeft  >> 4);
		volRight = (0x20 - (volRight & 0x0f)) >> (volRight >> 4);

		if (sl.ch)
		{
			sampleRLeft += (smplOut * volLeft ) >> 5;
			sampleRRight += (smplOut * volRight) >> 5;
		}
		else
		{
			sampleFLeft += (smplOut * volLeft ) >> 5;
			sampleFRight += (smplOut * volRight) >> 5;
		}

		unsigned step = (sl.lfo_active && sl.vib)
									? calcStep(sl.OCT, sl.FN, sl.compute_vib())
									: sl.step;
		sl.stepptr += step;

		if (sl.stepptr >= 0x10000) {
			sl.pos = nextPos(sl, sl.pos, sl.stepptr >> 16);
			sl.stepptr &= 0xffff;
		}

		if (channelBufs != nullptr) {
			channelBufs[i] = sl.pan != 8 ? smplOut : 0;
		}
	}
	advance();

	fleft = sampleFLeft >> 4;
	fright = sampleFRight >> 4;
	rleft = sampleRLeft >> 4;
	rright = sampleRRight >> 4;
}

void YMF278Base::keyOnHelper(Slot& slot)
{
	// Unlike FM, the envelope level is reset. (And it makes sense, because you restart the sample.)
	slot.env_vol = MAX_ATT_INDEX;
	if (slot.compute_rate(slot.AR) < 63) {
		slot.state = EG_ATT;
	} else {
		// Nuke.YKT verified that the FM part does it exactly this way,
		// and the OPL4 manual says it's instant as well.
		slot.env_vol = MIN_ATT_INDEX;
		// see comment in 'case EG_ATT' in YMF278::advance()
		slot.state = slot.DL ? EG_DEC : EG_SUS;
	}
	slot.stepptr = 0;
	slot.pos = 0;
}

YMF278Base::YMF278Base(MemoryInterface& memory, int channelCount, int clockDivider, double clockFrequency)
	: memory(memory)
	, slots(channelCount)
	, channelCount(channelCount)
	, clockDivider(clockDivider)
	, clockFrequency(clockFrequency)
{
	reset();
}

YMF278Base::~YMF278Base()
{
}

int YMF278Base::getChannelCount()
{
	return channelCount;
}

int YMF278Base::getClockDivider()
{
	return clockDivider;
}

double YMF278Base::getClockFrequency()
{
	return clockFrequency;
}

void YMF278Base::setClockFrequency(double clockFrequency_)
{
	clockFrequency = clockFrequency_;
}

double YMF278Base::getSampleRate()
{
	return clockFrequency / (channelCount * clockDivider);
}

void YMF278Base::reset()
{
	eg_cnt = 0;
	for (auto& op : slots) {
		op.reset();
	}
	memory.setMemoryType(false);
}

YMF278::YMF278(MemoryInterface& memory)
	: YMF278Base(memory, 24, 32, 33868800)
	, fmMixL(0), fmMixR(0), pcmMixL(0), pcmMixR(0)
{
	memAdr = 0; // avoid UMR
	std::fill(std::begin(regs), std::end(regs), 0);
}

void YMF278::reset()
{
	YMF278Base::reset();

	regs[2] = 0; // avoid UMR
	for (int i = 0xf7; i >= 0; --i) { // reverse order to avoid UMR
		writeReg(i, 0);
	}
	writeReg(0xf8, 0x1b);
	writeReg(0xf9, 0x00);
	memAdr = 0;
}

void YMF278::writeReg(byte reg, byte data)
{
	// Handle slot registers specifically
	if (reg >= 0x08 && reg <= 0xF7) {
		int sNum = (reg - 8) % 24;
		auto& slot = slots[sNum];
		switch ((reg - 8) / 24) {
		case 0: {
			slot.wave = (slot.wave & 0x100) | data;
			int waveTblHdr = (regs[2] >> 2) & 0x7;
			int base = (slot.wave < 384 || !waveTblHdr) ?
			           (slot.wave * 12) :
			           (waveTblHdr * 0x80000 + ((slot.wave - 384) * 12));
			byte buf[12];
			for (unsigned i = 0; i < 12; ++i) {
				// TODO What if R#2 bit 0 = 1?
				//      See also getSample()
				buf[i] = memory[base + i];
			}
			slot.bits = (buf[0] & 0xC0) >> 6;
			slot.startaddr = buf[2] | (buf[1] << 8) | ((buf[0] & 0x3F) << 16);
			slot.loopaddr = buf[4] | (buf[3] << 8);
			slot.endaddr  = buf[6] | (buf[5] << 8);
			for (unsigned i = 7; i < 12; ++i) {
				// Verified on real YMF278:
				// After tone loading, if you read these
				// registers, their value actually has changed.
				writeReg(8 + sNum + (i - 2) * 24, buf[i]);
			}
			if (slot.keyon) {
				keyOnHelper(slot);
			} else {
				slot.stepptr = 0;
				slot.pos = 0;
			}
			break;
		}
		case 1: {
			slot.wave = (slot.wave & 0xFF) | ((data & 0x1) << 8);
			slot.FN = (slot.FN & 0x380) | (data >> 1);
			slot.step = calcStep(slot.OCT, slot.FN);
			break;
		}
		case 2: {
			slot.FN = (slot.FN & 0x07F) | ((data & 0x07) << 7);
			slot.PRVB = (data & 0x08) != 0;
			slot.OCT = sign_extend_4((data & 0xF0) >> 4);
			slot.step = calcStep(slot.OCT, slot.FN);
			break;
		}
		case 3: {
			uint8_t t = data >> 1;
			slot.TLdest = (t != 0x7f) ? t : 0xff; // verified on HW via volume interpolation
			if (data & 1) {
				// directly change volume
				slot.TL = slot.TLdest;
			} else {
				// interpolate volume
			}
			break;
		}
		case 4:
			slot.ch = data & 0x10;
			slot.pan = data & 0x0F;

			if (data & 0x20) {
				// LFO reset
				slot.lfo_active = false;
				slot.lfo_cnt = 0;
			} else {
				// LFO activate
				slot.lfo_active = true;
			}

			slot.DAMP = (data & 0x40) != 0;

			if (data & 0x80) {
				if (!slot.keyon) {
					slot.keyon = true;
					keyOnHelper(slot);
				}
			} else {
				if (slot.keyon) {
					slot.keyon = false;
					slot.state = EG_REL;
				}
			}
			break;
		case 5:
			slot.lfo = (data >> 3) & 0x7;
			slot.vib = data & 0x7;
			break;
		case 6:
			slot.AR  = data >> 4;
			slot.D1R = data & 0xF;
			break;
		case 7:
			slot.DL  = dl_tab[data >> 4];
			slot.D2R = data & 0xF;
			break;
		case 8:
			slot.RC = data >> 4;
			slot.RR = data & 0xF;
			break;
		case 9:
			slot.AM = data & 0x7;
			break;
		}
	} else {
		// All non-slot registers
		switch (reg) {
		case 0x00: // TEST
		case 0x01:
			break;

		case 0x02:
			// wave-table-header / memory-type / memory-access-mode
			// Simply store in regs[2]
			memory.setMemoryType(regs[2] & 2);
			break;

		case 0x03:
			// Verified on real YMF278:
			// * Don't update the 'memAdr' variable on writes to
			//   reg 3 and 4. Only store the value in the 'regs'
			//   array for later use.
			// * The upper 2 bits are not used to address the
			//   external memories (so from a HW pov they don't
			//   matter). But if you read back this register, the
			//   upper 2 bits always read as '0' (even if you wrote
			//   '1'). So we mask the bits here already.
			data &= 0x3F;
			break;

		case 0x04:
			// See reg 3.
			break;

		case 0x05:
			// Verified on real YMF278: (see above)
			// Only writes to reg 5 change the (full) 'memAdr'.
			memAdr = (regs[3] << 16) | (regs[4] << 8) | data;
			break;

		case 0x06:  // memory data
			if (regs[2] & 1) {
				memory.write(memAdr, data);
				++memAdr; // no need to mask (again) here
			} else {
				// Verified on real YMF278:
				//  - writes are ignored
				//  - memAdr is NOT increased
			}
			break;

		case 0xf8:
			fmMixL = mix_level[data & 0x7];
			fmMixR = mix_level[data >> 3 & 0x7];
			break;
		case 0xf9:
			pcmMixL = mix_level[data & 0x7];
			pcmMixR = mix_level[data >> 3 & 0x7];
			break;
		}
	}

	regs[reg] = data;
}

byte YMF278::readReg(byte reg)
{
	// no need to call updateStream(time)
	byte result = peekReg(reg);
	if (reg == 6) {
		// Memory Data Register
		if (regs[2] & 1) {
			// Verified on real YMF278:
			// memAdr is only increased when 'regs[2] & 1'
			++memAdr; // no need to mask (again) here
		}
	}
	return result;
}

byte YMF278::peekReg(byte reg) const
{
	switch (reg) {
		case 2: // 3 upper bits are device ID
			return (regs[2] & 0x1F) | 0x20;

		case 6: // Memory Data Register
			if (regs[2] & 1) {
				return memory[memAdr];
			} else {
				// Verified on real YMF278
				return 0xff;
			}

		default:
			return regs[reg];
	}
}

YMW258::YMW258(MemoryInterface& memory)
	: YMF278Base(memory, 28, 8, 9878400)
{
}

void YMW258::writeReg(byte channel, byte reg, byte data)
{
	if ((channel & 0x7) == 0x7 || channel >= 0x20 || reg >= 0x8)
		return;
	int sNum = (channel >> 3) * 7 + (channel & 0x7);
	auto& slot = slots[sNum];

	switch (reg) {
		case 0: {
			slot.pan = data >> 4;
			break;
		}
		case 1: {
			slot.wave = (slot.wave & 0x100) | data;
			int base = slot.wave * 12;
			byte buf[12];
			for (unsigned i = 0; i < 12; ++i) {
				buf[i] = memory[base + i];
			}
			slot.bits = (buf[0] >> 6) == 0x3 ? 1 : 0;  // 00 / 10: 8 bit, 11: 12 bit, 01: unknown
			slot.startaddr = buf[2] | (buf[1] << 8) | ((buf[0] & 0x1F) << 16);
			slot.loopaddr = buf[4] | (buf[3] << 8);
			slot.endaddr  = buf[6] | (buf[5] << 8);
			slot.lfo = (buf[7] >> 3) & 0x7;
			slot.vib = buf[7] & 0x7;
			slot.AR  = buf[8] >> 4;
			slot.D1R = buf[8] & 0xF;
			slot.DL  = dl_tab[buf[9] >> 4];
			slot.D2R = buf[9] & 0xF;
			slot.RC = buf[10] >> 4;
			slot.RR = buf[10] & 0xF;
			slot.AM = buf[11] & 0x7;
			if (slot.keyon) {
				keyOnHelper(slot);
			} else {
				slot.stepptr = 0;
				slot.pos = 0;
			}
			break;
		}
		case 2: {
			slot.wave = (slot.wave & 0xFF) | ((data & 0x1) << 8);
			slot.FN = (slot.FN & 0x3C0) | (data >> 2);
			slot.step = calcStep(slot.OCT, slot.FN);
			break;
		}
		case 3: {
			slot.FN = (slot.FN & 0x03F) | ((data & 0x0F) << 6);
			slot.OCT = sign_extend_4((data & 0xF0) >> 4);
			slot.step = calcStep(slot.OCT, slot.FN);
			break;
		}
		case 4: {
			slot.lfo_active = true;
			if (data & 0x80) {
				if (!slot.keyon) {
					slot.keyon = true;
					keyOnHelper(slot);
				}
			} else {
				if (slot.keyon) {
					slot.keyon = false;
					slot.state = EG_REL;
				}
			}
			break;
		}
		case 5: {
			uint8_t t = data >> 1;
			slot.TLdest = (t != 0x7f) ? t : 0xff; // verified on YMF278 via volume interpolation
			if (data & 1) {
				// directly change volume
				slot.TL = slot.TLdest;
			} else {
				// interpolate volume
			}
			break;
		}
		case 6: {
			slot.lfo = (data >> 3) & 0x7;
			slot.vib = data & 0x7;
			break;
		}
		case 7: {
			slot.AM = data & 0x7;
			break;
		}
	}
}

MemoryMoonSound::MemoryMoonSound(MemoryInterface& rom, MemoryInterface& ram)
	: rom(rom)
	, ram(ram)
	, memoryType(false)
{
	if (rom.getSize() != 0x200000) { // 2MB
		assert(false);
	}
	assert((ram.getSize() & (1024 - 1)) == 0);
	int ramSize_ = ram.getSize() / 1024;
	if ((ramSize_ !=    0) &&  //   -     -
	    (ramSize_ !=  128) &&  // 128kB   -
	    (ramSize_ !=  256) &&  // 128kB  128kB
	    (ramSize_ !=  512) &&  // 512kB   -
	    (ramSize_ !=  640) &&  // 512kB  128kB
	    (ramSize_ != 1024) &&  // 512kB  512kB
	    (ramSize_ != 2048)) {  // 512kB  512kB  512kB  512kB
		assert(false);
	}
}

// This routine translates an address from the (upper) MoonSound address space
// to an address inside the (linearized) SRAM address space.
//
// The following info is based on measurements on a real MoonSound (v2.0)
// PCB. This PCB can have several possible SRAM configurations:
//   128kB:
//    1 SRAM chip of 128kB, chip enable (/CE) of this SRAM chip is connected to
//    the 1Y0 output of a 74LS139 (2-to-4 decoder). The enable input of the
//    74LS139 is connected to YMF278 pin /MCS6 and the 74LS139 1B:1A inputs are
//    connected to YMF278 pins MA18:MA17. So the SRAM is selected when /MC6 is
//    active and MA18:MA17 == 0:0.
//   256kB:
//    2 SRAM chips of 128kB. First one connected as above. Second one has /CE
//    connected to 74LS139 pin 1Y1. So SRAM2 is selected when /MSC6 is active
//    and MA18:MA17 == 0:1.
//   512kB:
//    1 SRAM chip of 512kB, /CE connected to /MCS6
//   640kB:
//    1 SRAM chip of 512kB, /CE connected to /MCS6
//    1 SRAM chip of 128kB, /CE connected to /MCS7.
//      (This means SRAM2 is potentially mirrored over a 512kB region)
//  1024kB:
//    1 SRAM chip of 512kB, /CE connected to /MCS6
//    1 SRAM chip of 512kB, /CE connected to /MCS7
//  2048kB:
//    1 SRAM chip of 512kB, /CE connected to /MCS6
//    1 SRAM chip of 512kB, /CE connected to /MCS7
//    1 SRAM chip of 512kB, /CE connected to /MCS8
//    1 SRAM chip of 512kB, /CE connected to /MCS9
//      This configuration is not so easy to create on the v2.0 PCB. So it's
//      very rare.
//
// So the /MCS6 and /MCS7 (and /MCS8 and /MCS9 in case of 2048kB) signals are
// used to select the different SRAM chips. The meaning of these signals
// depends on the 'memory access mode'. This mode can be changed at run-time
// via bit 1 in register 2. The following table indicates for which regions
// these signals are active (normally MoonSound should be used with mode=0):
//              mode=0              mode=1
//  /MCS6   0x200000-0x27FFFF   0x380000-0x39FFFF
//  /MCS7   0x280000-0x2FFFFF   0x3A0000-0x3BFFFF
//  /MCS8   0x300000-0x37FFFF   0x3C0000-0x3DFFFF
//  /MCS9   0x380000-0x3FFFFF   0x3E0000-0x3FFFFF
//
// (For completeness) MoonSound also has 2MB ROM (YRW801), /CE of this ROM is
// connected to YMF278 /MCS0. In both mode=0 and mode=1 this signal is active
// for the region 0x000000-0x1FFFFF. (But this routine does not handle ROM).
unsigned MemoryMoonSound::getRamAddress(unsigned addr) const
{
	addr -= 0x200000; // RAM starts at 0x200000
	if (memoryType) {
		// Normally MoonSound is used in 'memory access mode = 0'. But
		// in the rare case that mode=1 we adjust the address.
		if ((0x180000 <= addr) && (addr <= 0x1FFFFF)) {
			addr -= 0x180000;
			switch (addr & 0x060000) {
			case 0x000000: // [0x380000-0x39FFFF]
				// 1st 128kB of SRAM1
				break;
			case 0x020000: // [0x3A0000-0x3BFFFF]
				if (ram.getSize() == 256 * 1024) {
					// 2nd 128kB SRAM chip
				} else {
					// 2nd block of 128kB in SRAM2
					// In case of 512+128, we use mirroring
					addr += 0x080000;
				}
				break;
			case 0x040000: // [0x3C0000-0x3DFFFF]
				// 3rd 128kB block in SRAM3
				addr += 0x100000;
				break;
			case 0x060000: // [0x3EFFFF-0x3FFFFF]
				// 4th 128kB block in SRAM4
				addr += 0x180000;
				break;
			}
		} else {
			addr = unsigned(-1); // unmapped
		}
	}
	if (ram.getSize() == 640 * 1024) {
		// Verified on real MoonSound cartridge (v2.0): In case of
		// 640kB (1x512kB + 1x128kB), the 128kB SRAM chip is 4 times
		// visible. None of the other SRAM configurations show similar
		// mirroring (because the others are powers of two).
		if (addr > 0x080000) {
			addr &= ~0x060000;
		}
	}
	return addr;
}

byte MemoryMoonSound::operator[](unsigned address) const
{
	// Verified on real YMF278: address space wraps at 4MB.
	address &= 0x3FFFFF;
	if (address < 0x200000) {
		// ROM connected to /MCS0
		return rom[address];
	} else {
		unsigned ramAddr = getRamAddress(address);
		if (ramAddr < ram.getSize()) {
			return ram[ramAddr];
		} else {
			// unmapped region
			return 255; // TODO check
		}
	}
}

unsigned MemoryMoonSound::getSize() const {
	return 0x400000;
}

void MemoryMoonSound::write(unsigned address, byte value)
{
	address &= 0x3FFFFF;
	if (address < 0x200000) {
		// can't write to ROM
	} else {
		unsigned ramAddr = getRamAddress(address);
		if (ramAddr < ram.getSize()) {
			ram.write(ramAddr, value);
		} else {
			// can't write to unmapped memory
		}
	}
}

void MemoryMoonSound::clear(byte value) {
	ram.clear(value);
}

void MemoryMoonSound::setMemoryType(bool memoryType_) {
	memoryType = memoryType_;
}
