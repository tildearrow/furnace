// license:BSD-3-Clause
// copyright-holders:ElSemi, R. Belmont
//
// Freestanding port: MAME framework dependencies removed.
//   - emu.h replaced with <cstdint>/<cstring>/<algorithm>.
//   - MAME's BIT(v,n) and util::sext() replaced with local inlines.
//   - address_space *space replaced with uint8_t *RAM. SCSP sound RAM is
//     big-endian word-addressed; ring-buffer reads/writes target byte
//     offsets after `ADDR <<= 1`.
//
#include "scspdsp.h"

#include <algorithm>
#include <cstdint>
#include <cstring>

namespace {

// ── MAME framework helpers, reimplemented locally ─────────────────────────
constexpr int BIT(int32_t v, int n) { return (v >> n) & 1; }

// Sign-extend `value` from `bits`-bit two's complement to int32_t.
constexpr int32_t sext(int32_t value, unsigned bits)
{
	const uint32_t mask = uint32_t(1) << (bits - 1);
	return int32_t((uint32_t(value) ^ mask) - mask);
}

// std::clamp is C++17; Furnace builds as C++14. Local replacement.
template <typename T> constexpr T clamp(T v, T lo, T hi)
{
	return (v < lo) ? lo : ((v > hi) ? hi : v);
}

// SCSP sound RAM is big-endian word-addressed. ADDR here is a byte offset
// (post-shift in Step()); RAM/Mask are supplied by the host via SCSPDSP.
inline uint16_t ram_read_word_be(const uint8_t *RAM, uint32_t addr)
{
	return uint16_t((uint16_t(RAM[addr]) << 8) | RAM[addr + 1]);
}

inline void ram_write_word_be(uint8_t *RAM, uint32_t addr, uint16_t val)
{
	RAM[addr]     = uint8_t(val >> 8);
	RAM[addr + 1] = uint8_t(val & 0xFF);
}

uint16_t PACK(int32_t val)
{
	int const sign = BIT(val, 23);
	uint32_t temp = (val ^ (val << 1)) & 0xFFFFFF;
	int exponent = 0;
	for (int k = 0; k < 12; k++)
	{
		if (temp & 0x800000)
			break;
		temp <<= 1;
		exponent += 1;
	}
	if (exponent < 12)
		val = (val << exponent) & 0x3FFFFF;
	else
		val <<= 11;
	val >>= 11;
	val &= 0x7FF;
	val |= sign << 15;
	val |= exponent << 11;

	return uint16_t(val);
}

static int32_t UNPACK(uint16_t val)
{
	int const sign = BIT(val, 15);
	int exponent = (val >> 11) & 0xF;
	int const mantissa = val & 0x7FF;
	int32_t uval = mantissa << 11;
	if (exponent > 11)
	{
		exponent = 11;
		uval |= sign << 22;
	}
	else
	{
		uval |= (sign ^ 1) << 22;
	}
	uval |= sign << 23;
	uval <<= 8;
	uval >>= 8;
	uval >>= exponent;

	return uval;
}

} // anonymous namespace


void SCSPDSP::Init()
{
	std::memset(this, 0, sizeof(*this));
	RBL = (8*1024); // Initial RBL is 0
	Stopped = true;
}

void SCSPDSP::Step()
{
	if (Stopped)
		return;

	std::fill(std::begin(EFREG), std::end(EFREG), 0);

#if 0
	int dump=0;
	FILE *f=nullptr;
	if(dump)
		f=fopen("dsp.txt","wt");
#endif

	int32_t ACC = 0;    //26 bit
	int32_t MEMVAL = 0;
	int32_t FRC_REG = 0;    //13 bit
	int32_t Y_REG = 0;      //24 bit
	uint32_t ADRS_REG = 0;  //13 bit

	for (int step = 0; step < /*128*/LastStep; ++step)
	{
		uint16_t *const IPtr = MPRO + (step * 4);

		//if (!IPtr[0] && !IPtr[1] && !IPtr[2] && !IPtr[3])
			//break;

		uint32_t const TRA   = (IPtr[0] >>  8) & 0x7f;
		uint32_t const TWT   = (IPtr[0] >>  7) & 0x01;
		uint32_t const TWA   = (IPtr[0] >>  0) & 0x7f;

		uint32_t const XSEL  = (IPtr[1] >> 15) & 0x01;
		uint32_t const YSEL  = (IPtr[1] >> 13) & 0x03;
		uint32_t const IRA   = (IPtr[1] >>  6) & 0x3f;
		uint32_t const IWT   = (IPtr[1] >>  5) & 0x01;
		uint32_t const IWA   = (IPtr[1] >>  0) & 0x1f;

		uint32_t const TABLE = (IPtr[2] >> 15) & 0x01;
		uint32_t const MWT   = (IPtr[2] >> 14) & 0x01;
		uint32_t const MRD   = (IPtr[2] >> 13) & 0x01;
		uint32_t const EWT   = (IPtr[2] >> 12) & 0x01;
		uint32_t const EWA   = (IPtr[2] >>  8) & 0x0f;
		uint32_t const ADRL  = (IPtr[2] >>  7) & 0x01;
		uint32_t const FRCL  = (IPtr[2] >>  6) & 0x01;
		uint32_t const SHIFT = (IPtr[2] >>  4) & 0x03;
		uint32_t const YRL   = (IPtr[2] >>  3) & 0x01;
		uint32_t const NEGB  = (IPtr[2] >>  2) & 0x01;
		uint32_t const ZERO  = (IPtr[2] >>  1) & 0x01;
		uint32_t const BSEL  = (IPtr[2] >>  0) & 0x01;

		uint32_t const NOFL  = (IPtr[3] >> 15) & 0x01;  //????
		uint32_t const COEF  = (IPtr[3] >>  9) & 0x3f;

		uint32_t const MASA  = (IPtr[3] >>  2) & 0x1f;  //???
		uint32_t const ADREB = (IPtr[3] >>  1) & 0x01;
		uint32_t const NXADR = (IPtr[3] >>  0) & 0x01;

		//operations are done at 24 bit precision
#if 0
		if (MASA)
			int a=1;
		if (NOFL)
			int a=1;

		//int dump=0;

		if (f)
		{
#define DUMP(v) fprintf(f, " " #v ": %04X", v);
			fprintf(f, "%d: ", step);
			DUMP(ACC);
			DUMP(SHIFTED);
			DUMP(X);
			DUMP(Y);
			DUMP(B);
			DUMP(INPUTS);
			DUMP(MEMVAL);
			DUMP(FRC_REG);
			DUMP(Y_REG);
			DUMP(ADDR);
			DUMP(ADRS_REG);
			fprintf(f, "\n");
#undef DUMP
		}
#endif

		//INPUTS RW
		// colmns97 hits this
		//assert(IRA < 0x32);
		int32_t INPUTS; // 24-bit
		if (IRA <= 0x1f)
			INPUTS = MEMS[IRA];
		else if (IRA <= 0x2F)
			INPUTS = MIXS[IRA - 0x20] << 4;  //MIXS is 20 bit
		else if (IRA <= 0x31)
			INPUTS = EXTS[IRA - 0x30] << 8;  //EXTS is 16 bit
		else
			return;

		INPUTS = sext(INPUTS, 24);

		if (IWT)
		{
			MEMS[IWA] = MEMVAL;  // MEMVAL was selected in previous MRD
			if (IRA == IWA)
				INPUTS = MEMVAL;
		}

		//Operand sel
		int32_t B; // 26-bit
		if (!ZERO)
		{
			if (BSEL)
				B = ACC;
			else
				B = sext(TEMP[(TRA + DEC) & 0x7f], 24);
			if (NEGB)
				B = 0 - B;
		}
		else
			B = 0;

		int32_t X; // 24-bit
		if (XSEL)
			X = INPUTS;
		else
			X = sext(TEMP[(TRA + DEC) & 0x7f], 24);

		int32_t Y = 0;  //13 bit
		if (YSEL == 0)
			Y = FRC_REG;
		else if (YSEL == 1)
			Y = this->COEF[COEF] >> 3;   //COEF is 16 bits
		else if (YSEL == 2)
			Y = (Y_REG >> 11) & 0x1fff;
		else if (YSEL == 3)
			Y = (Y_REG >> 4) & 0x0fff;

		if (YRL)
			Y_REG = INPUTS;

		//Shifter
		int32_t SHIFTED = 0;    //24 bit
		if (SHIFT == 0)
			SHIFTED = clamp<int32_t>(ACC, -0x00800000, 0x007fffff);
		else if (SHIFT == 1)
			SHIFTED = clamp<int32_t>(ACC * 2, -0x00800000, 0x007fffff);
		else if (SHIFT == 2)
			SHIFTED = sext(ACC * 2, 24);
		else if (SHIFT == 3)
			SHIFTED = sext(ACC, 24);

		//ACCUM
		Y = sext(Y, 13);

		int64_t const v = (int64_t(X) * int64_t(Y)) >> 12;
		ACC = int(v + B);

		if (TWT)
			TEMP[(TWA + DEC) & 0x7f] = SHIFTED;

		if (FRCL)
		{
			if (SHIFT == 3)
				FRC_REG = SHIFTED & 0x0fff;
			else
				FRC_REG = (SHIFTED >> 11) & 0x1fff;
		}

		if (MRD || MWT)
		//if (0)
		{
			uint32_t ADDR = MADRS[MASA];
			if (!TABLE)
				ADDR += DEC;
			if (ADREB)
				ADDR += ADRS_REG & 0x0FFF;
			if (NXADR)
				ADDR++;
			if (!TABLE)
				ADDR &= RBL - 1;
			else
				ADDR &= 0xffff;
			ADDR += RBP << 12;
			ADDR <<= 1;
			ADDR &= RAMMask;  // wrap into host-supplied SCSP sound RAM
			if (MRD && (step & 1)) //memory only allowed on odd? DoA inserts NOPs on even
			{
				if (NOFL)
					MEMVAL = ram_read_word_be(RAM, ADDR) << 8;
				else
					MEMVAL = UNPACK(ram_read_word_be(RAM, ADDR));
			}
			if (MWT && (step & 1))
			{
				if (NOFL)
					ram_write_word_be(RAM, ADDR, uint16_t(SHIFTED >> 8));
				else
					ram_write_word_be(RAM, ADDR, PACK(SHIFTED));
			}
		}

		if (ADRL)
		{
			if (SHIFT == 3)
				ADRS_REG = (SHIFTED >> 12) & 0xfff;
			else
				ADRS_REG = INPUTS >> 16;
		}

		if (EWT)
			EFREG[EWA] += SHIFTED >> 8;
	}
	--DEC;
	std::fill(std::begin(MIXS), std::end(MIXS), 0);
	//if (f)
		//fclose(f);
}

void SCSPDSP::SetSample(int32_t sample, int SEL, int MXL)
{
	//MIXS[SEL] += sample << (MXL + 1)/*7*/;
	MIXS[SEL] += sample;
	//if (MXL)
		//int a = 1;
}

void SCSPDSP::Start()
{
	Stopped = false;
	int i;
	for (i = 127; i >= 0; --i)
	{
		uint16_t const *const IPtr = MPRO + (i * 4);
		if (IPtr[0] || IPtr[1] || IPtr[2] || IPtr[3])
			break;
	}
	LastStep = i + 1;
}
