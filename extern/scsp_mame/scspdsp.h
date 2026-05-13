// license:BSD-3-Clause
// copyright-holders:ElSemi, R. Belmont
//
// Freestanding port: MAME framework dependencies removed.
//   - MAME integer aliases (s16/u16/...) replaced with <cstdint> types.
//   - address_space *space replaced with uint8_t *RAM (host-supplied
//     pointer to SCSP sound RAM, big-endian word-addressed).
//
#ifndef SCSP_MAME_SCSPDSP_H
#define SCSP_MAME_SCSPDSP_H

#pragma once

#include <cstdint>

//the DSP Context
struct SCSPDSP
{
//Config
	uint8_t *RAM;     // host-supplied pointer to SCSP sound RAM (BE word-addressed)
	uint32_t RAMMask; // (RAMSize - 1); host sets to 0x7FFFF for 512KB or 0x3FFFF for 256KB
	uint32_t RBP;     //Ring buf pointer
	uint32_t RBL;     //Delay ram (Ring buffer) size in words

//context

	int16_t  COEF[64];      //16 bit signed
	uint16_t MADRS[32];     //offsets (in words), 16 bit
	uint16_t MPRO[128*4];   //128 steps 64 bit
	int32_t  TEMP[128];     //TEMP regs,24 bit signed
	int32_t  MEMS[32];      //MEMS regs,24 bit signed
	uint32_t DEC;

//input
	int32_t  MIXS[16];      //MIXS, 24 bit signed
	int16_t  EXTS[2];       //External inputs (CDDA)    16 bit signed

//output
	int16_t  EFREG[16];     //EFREG, 16 bit signed

	bool Stopped;
	int LastStep;

	void Init();
	void SetSample(int32_t sample, int SEL, int MXL);
	void Step();
	void Start();
};

#endif // SCSP_MAME_SCSPDSP_H
