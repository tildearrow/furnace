//	Altirra - Atari 800/800XL/5200 emulator
//	Copyright (C) 2008-2011 Avery Lee
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "at/ataudio/pokeytables.h"

ATPokeyTables::ATPokeyTables() {
	// The 4-bit and 5-bit polynomial counters are of the XNOR variety, which means
	// that the all-1s case causes a lockup. The INIT mode shifts zeroes into the
	// register.

	uint32 poly4 = 0;
	for(int i=0; i<131071; ++i) {
		poly4 = (poly4+poly4) + (~((poly4 >> 2) ^ (poly4 >> 3)) & 1);

		mPolyBuffer[i] = (poly4 & 1) << 3;
	}

	uint32 poly5 = 0;
	for(int i=0; i<131071; ++i) {
		poly5 = (poly5+poly5) + (~((poly5 >> 2) ^ (poly5 >> 4)) & 1);

		mPolyBuffer[i] |= (poly5 & 1) << 2;
	}

	// The 17-bit polynomial counter is also of the XNOR variety, but one big
	// difference is that you're allowed to read out 8 bits of it. The RANDOM
	// register actually reports the INVERTED state of these bits (the Q' output
	// of the flip flops is connected to the data bus). This means that even
	// though we clear the register to 0, it reads as FF.
	//
	// From the perspective of the CPU through RANDOM, the LFSR shifts to the
	// right, and new bits appear on the left. The equivalent operation for the
	// 9-bit LFSR would be to set carry equal to (D0 ^ D5) and then execute
	// a ROR.
	uint32 poly9 = 0;
	for(int i=0; i<131071; ++i) {
		// Note: This one is actually verified against a real Atari.
		// At WSYNC time, the pattern goes: 00 DF EE 16 B9....
		poly9 = (poly9 >> 1) + (~((poly9 << 8) ^ (poly9 << 3)) & 0x100);

		mPolyBuffer[i] |= (poly9 & 1) << 1;
	}

	// The 17-bit mode inserts an additional 8 register bits immediately after
	// the XNOR. The generator polynomial is unchanged.
	uint32 poly17 = 0;
	for(int i=0; i<131071; ++i) {
		poly17 = (poly17 >> 1) + (~((poly17 << 16) ^ (poly17 << 11)) & 0x10000);

		mPolyBuffer[i] |= (poly17 >> 8) & 1;
	}

	memcpy(mPolyBuffer + 131071, mPolyBuffer, 131071);

	memset(mInitModeBuffer, 0xFF, sizeof mInitModeBuffer);
}

constexpr ATPokeyHiFilterTable ATMakePokeyHiFilterTable() {
	constexpr double kRawFilter[][8] = {
		// Coefficients for a windowed-sinc LPF at 24KHz cutoff with Kaiser window. Scilab derivation:
		//
		// pi=atan(1)*4; fc = 24000/(7159090/2); M=sinc(2*pi*fc*[-223:1:223]).*(2*fc).*window('kr',447,8.6); plot(M); M2=matrix(cat(2,[0],M),[56 8]);
		// mprintf("{ %10.7ff, %10.7ff, %10.7ff, %10.7ff, %10.7ff, %10.7ff, %10.7ff, %10.7ff, },\n", M2)
		// [hm, fr] = frmag(M, 1024); plot(fr, hm)

		{  0.0000000f,  0.0000885f, -0.0009574f,  0.0030938f,  0.0134095f,  0.0030938f, -0.0009574f,  0.0000885f, },
		{  0.0000001f,  0.0000887f, -0.0009856f,  0.0033114f,  0.0134045f,  0.0028807f, -0.0009282f,  0.0000879f, },
		{  0.0000002f,  0.0000886f, -0.0010127f,  0.0035331f,  0.0133893f,  0.0026721f, -0.0008981f,  0.0000870f, },
		{  0.0000003f,  0.0000880f, -0.0010385f,  0.0037588f,  0.0133641f,  0.0024683f, -0.0008673f,  0.0000858f, },
		{  0.0000005f,  0.0000870f, -0.0010629f,  0.0039882f,  0.0133288f,  0.0022694f, -0.0008359f,  0.0000844f, },
		{  0.0000007f,  0.0000856f, -0.0010857f,  0.0042211f,  0.0132836f,  0.0020756f, -0.0008041f,  0.0000827f, },
		{  0.0000009f,  0.0000836f, -0.0011068f,  0.0044572f,  0.0132284f,  0.0018870f, -0.0007718f,  0.0000808f, },
		{  0.0000012f,  0.0000811f, -0.0011260f,  0.0046964f,  0.0131635f,  0.0017037f, -0.0007393f,  0.0000787f, },
		{  0.0000016f,  0.0000780f, -0.0011431f,  0.0049382f,  0.0130888f,  0.0015259f, -0.0007067f,  0.0000764f, },
		{  0.0000020f,  0.0000743f, -0.0011581f,  0.0051825f,  0.0130046f,  0.0013537f, -0.0006740f,  0.0000740f, },
		{  0.0000024f,  0.0000700f, -0.0011707f,  0.0054289f,  0.0129110f,  0.0011872f, -0.0006414f,  0.0000715f, },
		{  0.0000030f,  0.0000651f, -0.0011808f,  0.0056771f,  0.0128081f,  0.0010264f, -0.0006090f,  0.0000689f, },
		{  0.0000036f,  0.0000594f, -0.0011882f,  0.0059269f,  0.0126961f,  0.0008714f, -0.0005767f,  0.0000662f, },
		{  0.0000042f,  0.0000531f, -0.0011927f,  0.0061779f,  0.0125751f,  0.0007223f, -0.0005448f,  0.0000635f, },
		{  0.0000049f,  0.0000460f, -0.0011942f,  0.0064298f,  0.0124455f,  0.0005791f, -0.0005133f,  0.0000607f, },
		{  0.0000058f,  0.0000381f, -0.0011926f,  0.0066822f,  0.0123074f,  0.0004419f, -0.0004823f,  0.0000579f, },
		{  0.0000066f,  0.0000294f, -0.0011876f,  0.0069349f,  0.0121610f,  0.0003106f, -0.0004518f,  0.0000551f, },
		{  0.0000076f,  0.0000199f, -0.0011791f,  0.0071875f,  0.0120066f,  0.0001852f, -0.0004218f,  0.0000523f, },
		{  0.0000087f,  0.0000095f, -0.0011669f,  0.0074396f,  0.0118444f,  0.0000658f, -0.0003926f,  0.0000495f, },
		{  0.0000098f, -0.0000017f, -0.0011508f,  0.0076910f,  0.0116746f, -0.0000476f, -0.0003640f,  0.0000467f, },
		{  0.0000111f, -0.0000138f, -0.0011308f,  0.0079411f,  0.0114977f, -0.0001552f, -0.0003362f,  0.0000440f, },
		{  0.0000124f, -0.0000269f, -0.0011067f,  0.0081898f,  0.0113138f, -0.0002568f, -0.0003092f,  0.0000413f, },
		{  0.0000139f, -0.0000408f, -0.0010782f,  0.0084366f,  0.0111233f, -0.0003527f, -0.0002830f,  0.0000387f, },
		{  0.0000154f, -0.0000558f, -0.0010453f,  0.0086812f,  0.0109264f, -0.0004427f, -0.0002577f,  0.0000361f, },
		{  0.0000170f, -0.0000716f, -0.0010078f,  0.0089232f,  0.0107235f, -0.0005271f, -0.0002332f,  0.0000336f, },
		{  0.0000188f, -0.0000884f, -0.0009656f,  0.0091622f,  0.0105149f, -0.0006058f, -0.0002097f,  0.0000312f, },
		{  0.0000206f, -0.0001062f, -0.0009185f,  0.0093980f,  0.0103009f, -0.0006791f, -0.0001871f,  0.0000289f, },
		{  0.0000225f, -0.0001250f, -0.0008664f,  0.0096301f,  0.0100819f, -0.0007468f, -0.0001654f,  0.0000267f, },
		{  0.0000246f, -0.0001447f, -0.0008092f,  0.0098581f,  0.0098581f, -0.0008092f, -0.0001447f,  0.0000246f, },
		{  0.0000267f, -0.0001654f, -0.0007468f,  0.0100819f,  0.0096301f, -0.0008664f, -0.0001250f,  0.0000225f, },
		{  0.0000289f, -0.0001871f, -0.0006791f,  0.0103009f,  0.0093980f, -0.0009185f, -0.0001062f,  0.0000206f, },
		{  0.0000312f, -0.0002097f, -0.0006058f,  0.0105149f,  0.0091622f, -0.0009656f, -0.0000884f,  0.0000188f, },
		{  0.0000336f, -0.0002332f, -0.0005271f,  0.0107235f,  0.0089232f, -0.0010078f, -0.0000716f,  0.0000170f, },
		{  0.0000361f, -0.0002577f, -0.0004427f,  0.0109264f,  0.0086812f, -0.0010453f, -0.0000558f,  0.0000154f, },
		{  0.0000387f, -0.0002830f, -0.0003527f,  0.0111233f,  0.0084366f, -0.0010782f, -0.0000408f,  0.0000139f, },
		{  0.0000413f, -0.0003092f, -0.0002568f,  0.0113138f,  0.0081898f, -0.0011067f, -0.0000269f,  0.0000124f, },
		{  0.0000440f, -0.0003362f, -0.0001552f,  0.0114977f,  0.0079411f, -0.0011308f, -0.0000138f,  0.0000111f, },
		{  0.0000467f, -0.0003640f, -0.0000476f,  0.0116746f,  0.0076910f, -0.0011508f, -0.0000017f,  0.0000098f, },
		{  0.0000495f, -0.0003926f,  0.0000658f,  0.0118444f,  0.0074396f, -0.0011669f,  0.0000095f,  0.0000087f, },
		{  0.0000523f, -0.0004218f,  0.0001852f,  0.0120066f,  0.0071875f, -0.0011791f,  0.0000199f,  0.0000076f, },
		{  0.0000551f, -0.0004518f,  0.0003106f,  0.0121610f,  0.0069349f, -0.0011876f,  0.0000294f,  0.0000066f, },
		{  0.0000579f, -0.0004823f,  0.0004419f,  0.0123074f,  0.0066822f, -0.0011926f,  0.0000381f,  0.0000058f, },
		{  0.0000607f, -0.0005133f,  0.0005791f,  0.0124455f,  0.0064298f, -0.0011942f,  0.0000460f,  0.0000049f, },
		{  0.0000635f, -0.0005448f,  0.0007223f,  0.0125751f,  0.0061779f, -0.0011927f,  0.0000531f,  0.0000042f, },
		{  0.0000662f, -0.0005767f,  0.0008714f,  0.0126961f,  0.0059269f, -0.0011882f,  0.0000594f,  0.0000036f, },
		{  0.0000689f, -0.0006090f,  0.0010264f,  0.0128081f,  0.0056771f, -0.0011808f,  0.0000651f,  0.0000030f, },
		{  0.0000715f, -0.0006414f,  0.0011872f,  0.0129110f,  0.0054289f, -0.0011707f,  0.0000700f,  0.0000024f, },
		{  0.0000740f, -0.0006740f,  0.0013537f,  0.0130046f,  0.0051825f, -0.0011581f,  0.0000743f,  0.0000020f, },
		{  0.0000764f, -0.0007067f,  0.0015259f,  0.0130888f,  0.0049382f, -0.0011431f,  0.0000780f,  0.0000016f, },
		{  0.0000787f, -0.0007393f,  0.0017037f,  0.0131635f,  0.0046964f, -0.0011260f,  0.0000811f,  0.0000012f, },
		{  0.0000808f, -0.0007718f,  0.0018870f,  0.0132284f,  0.0044572f, -0.0011068f,  0.0000836f,  0.0000009f, },
		{  0.0000827f, -0.0008041f,  0.0020756f,  0.0132836f,  0.0042211f, -0.0010857f,  0.0000856f,  0.0000007f, },
		{  0.0000844f, -0.0008359f,  0.0022694f,  0.0133288f,  0.0039882f, -0.0010629f,  0.0000870f,  0.0000005f, },
		{  0.0000858f, -0.0008673f,  0.0024683f,  0.0133641f,  0.0037588f, -0.0010385f,  0.0000880f,  0.0000003f, },
		{  0.0000870f, -0.0008981f,  0.0026721f,  0.0133893f,  0.0035331f, -0.0010127f,  0.0000886f,  0.0000002f, },
		{  0.0000879f, -0.0009282f,  0.0028807f,  0.0134045f,  0.0033114f, -0.0009856f,  0.0000887f,  0.0000001f, },
	};

	static_assert(vdcountof(kRawFilter) == 56);

	ATPokeyHiFilterTable table {};

	for(int i=0; i<=28; ++i) {
		float sum = 0;

		for(int j=0; j<8; ++j) {
			table.mFilter[i][j] = (float)kRawFilter[i][7-j];

			sum += table.mFilter[i][j];
		}

		float scale = 1.0f / sum;

		for(float& v : table.mFilter[i])
			v *= scale;
	}

	return table;
}

constexpr ATPokeyHiFilterTable ATMakePokeyHiFilterTable2() {
	constexpr ATPokeyHiFilterTable table = ATMakePokeyHiFilterTable();
	return table;
}

extern const ATPokeyHiFilterTable g_ATPokeyHiFilterTable = ATMakePokeyHiFilterTable2();
