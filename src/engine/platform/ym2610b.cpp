/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2022 tildearrow and contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "ym2610b.h"
#include "sound/ymfm/ymfm.h"
#include "../engine.h"
#include <string.h>
#include <math.h>

#include "ym2610shared.h"

#include "fmshared_OPN.h"

static unsigned char konOffs[6]={
  0, 1, 2, 4, 5, 6
};

#define CHIP_DIVIDER 32

const char* regCheatSheetYM2610B[]={
  // SSG
  "SSG_FreqL_A",     "000",
  "SSG_FreqH_A",     "001",
  "SSG_FreqL_B",     "002",
  "SSG_FreqH_B",     "003",
  "SSG_FreqL_C",     "004",
  "SSG_FreqH_C",     "005",
  "SSG_FreqNoise",   "006",
  "SSG_Enable",      "007",
  "SSG_Volume_A",    "008",
  "SSG_Volume_B",    "009",
  "SSG_Volume_C",    "00A",
  "SSG_FreqL_Env",   "00B",
  "SSG_FreqH_Env",   "00C",
  "SSG_Control_Env", "00D",
  // ADPCM-B
  "ADPCMB_Control",  "010",
  "ADPCMB_L_R",      "011",
  "ADPCMB_StartL",   "012",
  "ADPCMB_StartH",   "013",
  "ADPCMB_EndL",     "014",
  "ADPCMB_EndH",     "015",
  "ADPCMB_FreqL",    "019",
  "ADPCMB_FreqH",    "01A",
  "ADPCMB_Volume",   "01B",
  "ADPCM_Flag",      "01C",
  // FM (Common)
  "FM_Test",         "021",
  "FM_LFOFreq",      "022",
  "ClockA1",         "024",
  "ClockA2",         "025",
  "ClockB",          "026",
  "FM_Control",      "027",
  "FM_NoteCtl",      "028",
  // FM (Channel 1-3)
  "FM1_Op1_DT_MULT", "030",
  "FM2_Op1_DT_MULT", "031",
  "FM3_Op1_DT_MULT", "032",
  "FM1_Op2_DT_MULT", "034",
  "FM2_Op2_DT_MULT", "035",
  "FM3_Op2_DT_MULT", "036",
  "FM1_Op3_DT_MULT", "038",
  "FM2_Op3_DT_MULT", "039",
  "FM3_Op3_DT_MULT", "03A",
  "FM1_Op4_DT_MULT", "03C",
  "FM2_Op4_DT_MULT", "03D",
  "FM3_Op4_DT_MULT", "03E",
  "FM1_Op1_TL",      "040",
  "FM2_Op1_TL",      "041",
  "FM3_Op1_TL",      "042",
  "FM1_Op2_TL",      "044",
  "FM2_Op2_TL",      "045",
  "FM3_Op2_TL",      "046",
  "FM1_Op3_TL",      "048",
  "FM2_Op3_TL",      "049",
  "FM3_Op3_TL",      "04A",
  "FM1_Op4_TL",      "04C",
  "FM2_Op4_TL",      "04D",
  "FM3_Op4_TL",      "04E",
  "FM1_Op1_KS_AR",   "050",
  "FM2_Op1_KS_AR",   "051",
  "FM3_Op1_KS_AR",   "052",
  "FM1_Op2_KS_AR",   "054",
  "FM2_Op2_KS_AR",   "055",
  "FM3_Op2_KS_AR",   "056",
  "FM1_Op3_KS_AR",   "058",
  "FM2_Op3_KS_AR",   "059",
  "FM3_Op3_KS_AR",   "05A",
  "FM1_Op4_KS_AR",   "05C",
  "FM2_Op4_KS_AR",   "05D",
  "FM3_Op4_KS_AR",   "05E",
  "FM1_Op1_AM_DR",   "060",
  "FM2_Op1_AM_DR",   "061",
  "FM3_Op1_AM_DR",   "062",
  "FM1_Op2_AM_DR",   "064",
  "FM2_Op2_AM_DR",   "065",
  "FM3_Op2_AM_DR",   "066",
  "FM1_Op3_AM_DR",   "068",
  "FM2_Op3_AM_DR",   "069",
  "FM3_Op3_AM_DR",   "06A",
  "FM1_Op4_AM_DR",   "06C",
  "FM2_Op4_AM_DR",   "06D",
  "FM3_Op4_AM_DR",   "06E",
  "FM1_Op1_SR",      "070",
  "FM2_Op1_SR",      "071",
  "FM3_Op1_SR",      "072",
  "FM1_Op2_SR",      "074",
  "FM2_Op2_SR",      "075",
  "FM3_Op2_SR",      "076",
  "FM1_Op3_SR",      "078",
  "FM2_Op3_SR",      "079",
  "FM3_Op3_SR",      "07A",
  "FM1_Op4_SR",      "07C",
  "FM2_Op4_SR",      "07D",
  "FM3_Op4_SR",      "07E",
  "FM1_Op1_SL_RR",   "080",
  "FM2_Op1_SL_RR",   "081",
  "FM3_Op1_SL_RR",   "082",
  "FM1_Op2_SL_RR",   "084",
  "FM2_Op2_SL_RR",   "085",
  "FM3_Op2_SL_RR",   "086",
  "FM1_Op3_SL_RR",   "088",
  "FM2_Op3_SL_RR",   "089",
  "FM3_Op3_SL_RR",   "08A",
  "FM1_Op4_SL_RR",   "08C",
  "FM2_Op4_SL_RR",   "08D",
  "FM3_Op4_SL_RR",   "08E",
  "FM1_Op1_SSG_EG",  "090",
  "FM2_Op1_SSG_EG",  "091",
  "FM3_Op1_SSG_EG",  "092",
  "FM1_Op2_SSG_EG",  "094",
  "FM2_Op2_SSG_EG",  "095",
  "FM3_Op2_SSG_EG",  "096",
  "FM1_Op3_SSG_EG",  "098",
  "FM2_Op3_SSG_EG",  "099",
  "FM3_Op3_SSG_EG",  "09A",
  "FM1_Op4_SSG_EG",  "09C",
  "FM2_Op4_SSG_EG",  "09D",
  "FM3_Op4_SSG_EG",  "09E",
  "FM1_FNum1",       "0A0",
  "FM2_FNum1",       "0A1",
  "FM3_(Op1)FNum1",  "0A2",
  "FM1_FNum2",       "0A4",
  "FM2_FNum2",       "0A5",
  "FM3_(Op1)FNum2",  "0A6",
  "FM3_Op2_FNum1",   "0A8",
  "FM3_Op3_FNum1",   "0A9",
  "FM3_Op4_FNum1",   "0AA",
  "FM3_Op2_FNum2",   "0AC",
  "FM3_Op3_FNum2",   "0AD",
  "FM3_Op4_FNum2",   "0AE",
  "FM1_FB_ALG",      "0B0",
  "FM2_FB_ALG",      "0B1",
  "FM3_FB_ALG",      "0B2",
  "FM1_Pan_LFO",     "0B4",
  "FM2_Pan_LFO",     "0B5",
  "FM3_Pan_LFO",     "0B6",
  // ADPCM-A
  "ADPCMA_Control",  "100",
  "ADPCMA_MVol",     "101",
  "ADPCMA_Test",     "102",
  "ADPCMA_Ch1_Vol",  "108",
  "ADPCMA_Ch2_Vol",  "109",
  "ADPCMA_Ch3_Vol",  "10A",
  "ADPCMA_Ch4_Vol",  "10B",
  "ADPCMA_Ch5_Vol",  "10C",
  "ADPCMA_Ch6_Vol",  "10D",
  "ADPCMA_Ch1_StL",  "110",
  "ADPCMA_Ch2_StL",  "111",
  "ADPCMA_Ch3_StL",  "112",
  "ADPCMA_Ch4_StL",  "113",
  "ADPCMA_Ch5_StL",  "114",
  "ADPCMA_Ch6_StL",  "115",
  "ADPCMA_Ch1_StH",  "118",
  "ADPCMA_Ch2_StH",  "119",
  "ADPCMA_Ch3_StH",  "11A",
  "ADPCMA_Ch4_StH",  "11B",
  "ADPCMA_Ch5_StH",  "11C",
  "ADPCMA_Ch6_StH",  "11D",
  "ADPCMA_Ch1_EdL",  "120",
  "ADPCMA_Ch2_EdL",  "121",
  "ADPCMA_Ch3_EdL",  "122",
  "ADPCMA_Ch4_EdL",  "123",
  "ADPCMA_Ch5_EdL",  "124",
  "ADPCMA_Ch6_EdL",  "125",
  "ADPCMA_Ch1_EdH",  "128",
  "ADPCMA_Ch2_EdH",  "129",
  "ADPCMA_Ch3_EdH",  "12A",
  "ADPCMA_Ch4_EdH",  "12B",
  "ADPCMA_Ch5_EdH",  "12C",
  "ADPCMA_Ch6_EdH",  "12D",
  // FM (Channel 4-6)
  "FM4_Op1_DT_MULT", "130",
  "FM5_Op1_DT_MULT", "131",
  "FM6_Op1_DT_MULT", "132",
  "FM4_Op2_DT_MULT", "134",
  "FM5_Op2_DT_MULT", "135",
  "FM6_Op2_DT_MULT", "136",
  "FM4_Op3_DT_MULT", "138",
  "FM5_Op3_DT_MULT", "139",
  "FM6_Op3_DT_MULT", "13A",
  "FM4_Op4_DT_MULT", "13C",
  "FM5_Op4_DT_MULT", "13D",
  "FM6_Op4_DT_MULT", "13E",
  "FM4_Op1_TL",      "140",
  "FM5_Op1_TL",      "141",
  "FM6_Op1_TL",      "142",
  "FM4_Op2_TL",      "144",
  "FM5_Op2_TL",      "145",
  "FM6_Op2_TL",      "146",
  "FM4_Op3_TL",      "148",
  "FM5_Op3_TL",      "149",
  "FM6_Op3_TL",      "14A",
  "FM4_Op4_TL",      "14C",
  "FM5_Op4_TL",      "14D",
  "FM6_Op4_TL",      "14E",
  "FM4_Op1_KS_AR",   "150",
  "FM5_Op1_KS_AR",   "151",
  "FM6_Op1_KS_AR",   "152",
  "FM4_Op2_KS_AR",   "154",
  "FM5_Op2_KS_AR",   "155",
  "FM6_Op2_KS_AR",   "156",
  "FM4_Op3_KS_AR",   "158",
  "FM5_Op3_KS_AR",   "159",
  "FM6_Op3_KS_AR",   "15A",
  "FM4_Op4_KS_AR",   "15C",
  "FM5_Op4_KS_AR",   "15D",
  "FM6_Op4_KS_AR",   "15E",
  "FM4_Op1_AM_DR",   "160",
  "FM5_Op1_AM_DR",   "161",
  "FM6_Op1_AM_DR",   "162",
  "FM4_Op2_AM_DR",   "164",
  "FM5_Op2_AM_DR",   "165",
  "FM6_Op2_AM_DR",   "166",
  "FM4_Op3_AM_DR",   "168",
  "FM5_Op3_AM_DR",   "169",
  "FM6_Op3_AM_DR",   "16A",
  "FM4_Op4_AM_DR",   "16C",
  "FM5_Op4_AM_DR",   "16D",
  "FM6_Op4_AM_DR",   "16E",
  "FM4_Op1_SR",      "170",
  "FM5_Op1_SR",      "171",
  "FM6_Op1_SR",      "172",
  "FM4_Op2_SR",      "174",
  "FM5_Op2_SR",      "175",
  "FM6_Op2_SR",      "176",
  "FM4_Op3_SR",      "178",
  "FM5_Op3_SR",      "179",
  "FM6_Op3_SR",      "17A",
  "FM4_Op4_SR",      "17C",
  "FM5_Op4_SR",      "17D",
  "FM6_Op4_SR",      "17E",
  "FM4_Op1_SL_RR",   "180",
  "FM5_Op1_SL_RR",   "181",
  "FM6_Op1_SL_RR",   "182",
  "FM4_Op2_SL_RR",   "184",
  "FM5_Op2_SL_RR",   "185",
  "FM6_Op2_SL_RR",   "186",
  "FM4_Op3_SL_RR",   "188",
  "FM5_Op3_SL_RR",   "189",
  "FM6_Op3_SL_RR",   "18A",
  "FM4_Op4_SL_RR",   "18C",
  "FM5_Op4_SL_RR",   "18D",
  "FM6_Op4_SL_RR",   "18E",
  "FM4_Op1_SSG_EG",  "190",
  "FM5_Op1_SSG_EG",  "191",
  "FM6_Op1_SSG_EG",  "192",
  "FM4_Op2_SSG_EG",  "194",
  "FM5_Op2_SSG_EG",  "195",
  "FM6_Op2_SSG_EG",  "196",
  "FM4_Op3_SSG_EG",  "198",
  "FM5_Op3_SSG_EG",  "199",
  "FM6_Op3_SSG_EG",  "19A",
  "FM4_Op4_SSG_EG",  "19C",
  "FM5_Op4_SSG_EG",  "19D",
  "FM6_Op4_SSG_EG",  "19E",
  "FM4_FNum1",       "1A0",
  "FM5_FNum1",       "1A1",
  "FM6_FNum1",       "1A2",
  "FM4_FNum2",       "1A4",
  "FM5_FNum2",       "1A5",
  "FM6_FNum2",       "1A6",
  "FM4_FB_ALG",      "1B0",
  "FM5_FB_ALG",      "1B1",
  "FM6_FB_ALG",      "1B2",
  "FM4_Pan_LFO",     "1B4",
  "FM5_Pan_LFO",     "1B5",
  "FM6_Pan_LFO",     "1B6",
  NULL
};

const char** DivPlatformYM2610B::getRegisterSheet() {
  return regCheatSheetYM2610B;
}

const char* DivPlatformYM2610B::getEffectName(unsigned char effect) {
  switch (effect) {
    case 0x10:
      return "10xy: Setup LFO (x: enable; y: speed)";
      break;
    case 0x11:
      return "11xx: Set feedback (0 to 7)";
      break;
    case 0x12:
      return "12xx: Set level of operator 1 (0 highest, 7F lowest)";
      break;
    case 0x13:
      return "13xx: Set level of operator 2 (0 highest, 7F lowest)";
      break;
    case 0x14:
      return "14xx: Set level of operator 3 (0 highest, 7F lowest)";
      break;
    case 0x15:
      return "15xx: Set level of operator 4 (0 highest, 7F lowest)";
      break;
    case 0x16:
      return "16xy: Set operator multiplier (x: operator from 1 to 4; y: multiplier)";
      break;
    case 0x18:
      return "18xx: Toggle extended channel 3 mode";
      break;
    case 0x19:
      return "19xx: Set attack of all operators (0 to 1F)";
      break;
    case 0x1a:
      return "1Axx: Set attack of operator 1 (0 to 1F)";
      break;
    case 0x1b:
      return "1Bxx: Set attack of operator 2 (0 to 1F)";
      break;
    case 0x1c:
      return "1Cxx: Set attack of operator 3 (0 to 1F)";
      break;
    case 0x1d:
      return "1Dxx: Set attack of operator 4 (0 to 1F)";
      break;
    case 0x20:
      return "20xx: Set SSG channel mode (bit 0: square; bit 1: noise; bit 2: envelope)";
      break;
    case 0x21:
      return "21xx: Set SSG noise frequency (0 to 1F)";
      break;
    case 0x22:
      return "22xy: Set SSG envelope mode (x: shape, y: enable for this channel)";
      break;
    case 0x23:
      return "23xx: Set SSG envelope period low byte";
      break;
    case 0x24:
      return "24xx: Set SSG envelope period high byte";
      break;
    case 0x25:
      return "25xx: SSG envelope slide up";
      break;
    case 0x26:
      return "26xx: SSG envelope slide down";
      break;
    case 0x29:
      return "29xy: Set SSG auto-envelope (x: numerator; y: denominator)";
      break;
    case 0x30:
      return "30xx: Toggle hard envelope reset on new notes";
      break;
    case 0x50:
      return "50xy: Set AM (x: operator from 1 to 4 (0 for all ops); y: AM)";
      break;
    case 0x51:
      return "51xy: Set sustain level (x: operator from 1 to 4 (0 for all ops); y: sustain)";
      break;
    case 0x52:
      return "52xy: Set release (x: operator from 1 to 4 (0 for all ops); y: release)";
      break;
    case 0x53:
      return "53xy: Set detune (x: operator from 1 to 4 (0 for all ops); y: detune where 3 is center)";
      break;
    case 0x54:
      return "54xy: Set envelope scale (x: operator from 1 to 4 (0 for all ops); y: scale from 0 to 3)";
      break;
    case 0x55:
      return "55xy: Set SSG envelope (x: operator from 1 to 4 (0 for all ops); y: 0-7 on, 8 off)";
      break;
    case 0x56:
      return "56xx: Set decay of all operators (0 to 1F)";
      break;
    case 0x57:
      return "57xx: Set decay of operator 1 (0 to 1F)";
      break;
    case 0x58:
      return "58xx: Set decay of operator 2 (0 to 1F)";
      break;
    case 0x59:
      return "59xx: Set decay of operator 3 (0 to 1F)";
      break;
    case 0x5a:
      return "5Axx: Set decay of operator 4 (0 to 1F)";
      break;
    case 0x5b:
      return "5Bxx: Set decay 2 of all operators (0 to 1F)";
      break;
    case 0x5c:
      return "5Cxx: Set decay 2 of operator 1 (0 to 1F)";
      break;
    case 0x5d:
      return "5Dxx: Set decay 2 of operator 2 (0 to 1F)";
      break;
    case 0x5e:
      return "5Exx: Set decay 2 of operator 3 (0 to 1F)";
      break;
    case 0x5f:
      return "5Fxx: Set decay 2 of operator 4 (0 to 1F)";
      break;
  }
  return NULL;
}

double DivPlatformYM2610B::NOTE_OPNB(int ch, int note) {
  if (ch>8) { // ADPCM-B
    return NOTE_ADPCMB(note);
  } else if (ch>5) { // PSG
    return NOTE_PERIODIC(note);
  }
  // FM
  return NOTE_FNUM_BLOCK(note,11);
}

double DivPlatformYM2610B::NOTE_ADPCMB(int note) {
  if (chan[15].sample>=0 && chan[15].sample<parent->song.sampleLen) {
    double off=65535.0*(double)(parent->getSample(chan[15].sample)->centerRate)/8363.0;
    return parent->calcBaseFreq((double)chipClock/144,off,note,false);
  }
  return 0;
}

void DivPlatformYM2610B::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  static int os[2];

  ymfm::ym2610b::fm_engine* fme=fm->debug_fm_engine();
  ymfm::ssg_engine* ssge=fm->debug_ssg_engine();
  ymfm::adpcm_a_engine* aae=fm->debug_adpcm_a_engine();
  ymfm::adpcm_b_engine* abe=fm->debug_adpcm_b_engine();

  ymfm::ssg_engine::output_data ssgOut;

  ymfm::fm_channel<ymfm::opn_registers_base<true>>* fmChan[6];
  ymfm::adpcm_a_channel* adpcmAChan[6];
  for (int i=0; i<6; i++) {
    fmChan[i]=fme->debug_channel(i);
    adpcmAChan[i]=aae->debug_channel(i);
  }

  for (size_t h=start; h<start+len; h++) {
    os[0]=0; os[1]=0;
    if (!writes.empty()) {
      if (--delay<1) {
        QueuedWrite& w=writes.front();
        fm->write(0x0+((w.addr>>8)<<1),w.addr);
        fm->write(0x1+((w.addr>>8)<<1),w.val);
        regPool[w.addr&0x1ff]=w.val;
        writes.pop();
        delay=4;
      }
    }
    
    fm->generate(&fmout);

    os[0]=fmout.data[0]+(fmout.data[2]>>1);
    if (os[0]<-32768) os[0]=-32768;
    if (os[0]>32767) os[0]=32767;

    os[1]=fmout.data[1]+(fmout.data[2]>>1);
    if (os[1]<-32768) os[1]=-32768;
    if (os[1]>32767) os[1]=32767;
  
    bufL[h]=os[0];
    bufR[h]=os[1];

    
    for (int i=0; i<6; i++) {
      oscBuf[i]->data[oscBuf[i]->needle++]=(fmChan[i]->debug_output(0)+fmChan[i]->debug_output(1));
    }

    ssge->get_last_out(ssgOut);
    for (int i=6; i<9; i++) {
      oscBuf[i]->data[oscBuf[i]->needle++]=ssgOut.data[i-6];
    }

    for (int i=9; i<15; i++) {
      oscBuf[i]->data[oscBuf[i]->needle++]=adpcmAChan[i-9]->get_last_out(0)+adpcmAChan[i-9]->get_last_out(1);
    }

    oscBuf[15]->data[oscBuf[15]->needle++]=abe->get_last_out(0)+abe->get_last_out(1);
  }
}

void DivPlatformYM2610B::tick(bool sysTick) {
  // PSG
  ay->tick(sysTick);
  ay->flushWrites();
  for (DivRegWrite& i: ay->getRegisterWrites()) {
    immWrite(i.addr&15,i.val);
  }
  ay->getRegisterWrites().clear();
  
  // FM
  for (int i=0; i<6; i++) {
    if (i==2 && extMode) continue;
    chan[i].std.next();

    if (chan[i].std.vol.had) {
      chan[i].outVol=VOL_SCALE_LOG(chan[i].vol,MIN(127,chan[i].std.vol.val),127);
      for (int j=0; j<4; j++) {
        unsigned short baseAddr=chanOffs[i]|opOffs[j];
        DivInstrumentFM::Operator& op=chan[i].state.op[j];
        if (isOutput[chan[i].state.alg][j]) {
          rWrite(baseAddr+ADDR_TL,127-VOL_SCALE_LOG(127-op.tl,chan[i].outVol&0x7f,127));
        } else {
          rWrite(baseAddr+ADDR_TL,op.tl);
        }
      }
    }

    if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        if (chan[i].std.arp.mode) {
          chan[i].baseFreq=NOTE_FNUM_BLOCK(chan[i].std.arp.val,11);
        } else {
          chan[i].baseFreq=NOTE_FNUM_BLOCK(chan[i].note+(signed char)chan[i].std.arp.val,11);
        }
      }
      chan[i].freqChanged=true;
    } else {
      if (chan[i].std.arp.mode && chan[i].std.arp.finished) {
        chan[i].baseFreq=NOTE_FNUM_BLOCK(chan[i].note,11);
        chan[i].freqChanged=true;
      }
    }

    if (chan[i].std.panL.had) {
      chan[i].pan=chan[i].std.panL.val&3;
      rWrite(chanOffs[i]+ADDR_LRAF,(isMuted[i]?0:(chan[i].pan<<6))|(chan[i].state.fms&7)|((chan[i].state.ams&3)<<4));
    }

    if (chan[i].std.pitch.had) {
      if (chan[i].std.pitch.mode) {
        chan[i].pitch2+=chan[i].std.pitch.val;
        CLAMP_VAR(chan[i].pitch2,-32768,32767);
      } else {
        chan[i].pitch2=chan[i].std.pitch.val;
      }
      chan[i].freqChanged=true;
    }

    if (chan[i].std.phaseReset.had) {
      if (chan[i].std.phaseReset.val==1 && chan[i].active) {
        chan[i].keyOn=true;
      }
    }

    if (chan[i].std.alg.had) {
      chan[i].state.alg=chan[i].std.alg.val;
      rWrite(chanOffs[i]+ADDR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3));
      if (!parent->song.algMacroBehavior) for (int j=0; j<4; j++) {
        unsigned short baseAddr=chanOffs[i]|opOffs[j];
        DivInstrumentFM::Operator& op=chan[i].state.op[j];
        if (isMuted[i]) {
          rWrite(baseAddr+ADDR_TL,127);
        } else {
          if (isOutput[chan[i].state.alg][j]) {
            rWrite(baseAddr+ADDR_TL,127-VOL_SCALE_LOG(127-op.tl,chan[i].outVol&0x7f,127));
          } else {
            rWrite(baseAddr+ADDR_TL,op.tl);
          }
        }
      }
    }
    if (chan[i].std.fb.had) {
      chan[i].state.fb=chan[i].std.fb.val;
      rWrite(chanOffs[i]+ADDR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3));
    }
    if (chan[i].std.fms.had) {
      chan[i].state.fms=chan[i].std.fms.val;
      rWrite(chanOffs[i]+ADDR_LRAF,(isMuted[i]?0:(chan[i].pan<<6))|(chan[i].state.fms&7)|((chan[i].state.ams&3)<<4));
    }
    if (chan[i].std.ams.had) {
      chan[i].state.ams=chan[i].std.ams.val;
      rWrite(chanOffs[i]+ADDR_LRAF,(isMuted[i]?0:(chan[i].pan<<6))|(chan[i].state.fms&7)|((chan[i].state.ams&3)<<4));
    }
    for (int j=0; j<4; j++) {
      unsigned short baseAddr=chanOffs[i]|opOffs[j];
      DivInstrumentFM::Operator& op=chan[i].state.op[j];
      DivMacroInt::IntOp& m=chan[i].std.op[j];
      if (m.am.had) {
        op.am=m.am.val;
        rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
      }
      if (m.ar.had) {
        op.ar=m.ar.val;
        rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
      }
      if (m.dr.had) {
        op.dr=m.dr.val;
        rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
      }
      if (m.mult.had) {
        op.mult=m.mult.val;
        rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
      }
      if (m.rr.had) {
        op.rr=m.rr.val;
        rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
      }
      if (m.sl.had) {
        op.sl=m.sl.val;
        rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
      }
      if (m.tl.had) {
        op.tl=127-m.tl.val;
        if (isOutput[chan[i].state.alg][j]) {
          rWrite(baseAddr+ADDR_TL,127-VOL_SCALE_LOG(127-op.tl,chan[i].outVol&0x7f,127));
        } else {
          rWrite(baseAddr+ADDR_TL,op.tl);
        }
      }
      if (m.rs.had) {
        op.rs=m.rs.val;
        rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
      }
      if (m.dt.had) {
        op.dt=m.dt.val;
        rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
      }
      if (m.d2r.had) {
        op.d2r=m.d2r.val;
        rWrite(baseAddr+ADDR_DT2_D2R,op.d2r&31);
      }
      if (m.ssg.had) {
        op.ssgEnv=m.ssg.val;
        rWrite(baseAddr+ADDR_SSG,op.ssgEnv&15);
      }
    }

    if (chan[i].keyOn || chan[i].keyOff) {
      if (chan[i].hardReset && chan[i].keyOn) {
        for (int j=0; j<4; j++) {
          unsigned short baseAddr=chanOffs[i]|opOffs[j];
          immWrite(baseAddr+ADDR_SL_RR,0x0f);
          immWrite(baseAddr+ADDR_TL,0x7f);
          oldWrites[baseAddr+ADDR_SL_RR]=-1;
          oldWrites[baseAddr+ADDR_TL]=-1;
          //rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
        }
      }
      immWrite(0x28,0x00|konOffs[i]);
      if (chan[i].hardReset && chan[i].keyOn) {
        for (int j=0; j<4; j++) {
          unsigned short baseAddr=chanOffs[i]|opOffs[j];
          for (int k=0; k<100; k++) {
            immWrite(baseAddr+ADDR_SL_RR,0x0f);
          }
        }
      }
      chan[i].keyOff=false;
    }
  }
  // ADPCM-B
  if (chan[15].furnacePCM) {
    chan[15].std.next();

    if (chan[15].std.vol.had) {
      chan[15].outVol=(chan[15].vol*MIN(64,chan[15].std.vol.val))/64;
      immWrite(0x1b,chan[15].outVol);
    }

    if (chan[15].std.arp.had) {
      if (!chan[15].inPorta) {
        if (chan[15].std.arp.mode) {
          chan[15].baseFreq=NOTE_ADPCMB(chan[15].std.arp.val);
        } else {
          chan[15].baseFreq=NOTE_ADPCMB(chan[15].note+(signed char)chan[15].std.arp.val);
        }
      }
      chan[15].freqChanged=true;
    } else {
      if (chan[15].std.arp.mode && chan[15].std.arp.finished) {
        chan[15].baseFreq=NOTE_ADPCMB(chan[15].note);
        chan[15].freqChanged=true;
      }
    }
  }
  if (chan[15].freqChanged) {
    if (chan[15].sample>=0 && chan[15].sample<parent->song.sampleLen) {
      double off=65535.0*(double)(parent->getSample(chan[15].sample)->centerRate)/8363.0;
      chan[15].freq=parent->calcFreq(chan[15].baseFreq,chan[15].pitch,false,4,chan[15].pitch2,(double)chipClock/144,off);
    } else {
      chan[15].freq=0;
    }
    immWrite(0x19,chan[15].freq&0xff);
    immWrite(0x1a,(chan[15].freq>>8)&0xff);
    chan[15].freqChanged=false;
  }

  for (int i=16; i<512; i++) {
    if (pendingWrites[i]!=oldWrites[i]) {
      immWrite(i,pendingWrites[i]&0xff);
      oldWrites[i]=pendingWrites[i];
    }
  }

  for (int i=0; i<6; i++) {
    if (i==2 && extMode) continue;
    if (chan[i].freqChanged) {
      if (parent->song.linearPitch==2) {
        chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,false,4,chan[i].pitch2,chipClock,CHIP_FREQBASE,11);
      } else {
        int fNum=parent->calcFreq(chan[i].baseFreq&0x7ff,chan[i].pitch,false,4,chan[i].pitch2);
        int block=(chan[i].baseFreq&0xf800)>>11;
        if (fNum<0) fNum=0;
        if (fNum>2047) {
          while (block<7) {
            fNum>>=1;
            block++;
          }
          if (fNum>2047) fNum=2047;
        }
        chan[i].freq=(block<<11)|fNum;
      }
      if (chan[i].freq>0x3fff) chan[i].freq=0x3fff;
      immWrite(chanOffs[i]+ADDR_FREQH,chan[i].freq>>8);
      immWrite(chanOffs[i]+ADDR_FREQ,chan[i].freq&0xff);
      chan[i].freqChanged=false;
    }
    if (chan[i].keyOn) {
      immWrite(0x28,0xf0|konOffs[i]);
      chan[i].keyOn=false;
    }
  }
}

int DivPlatformYM2610B::dispatch(DivCommand c) {
  if (c.chan>5 && c.chan<9) {
    c.chan-=6;
    return ay->dispatch(c);
  }
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      if (c.chan>14) { // ADPCM-B
        DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_FM);
        if (ins->type==DIV_INS_AMIGA) {
          chan[c.chan].furnacePCM=true;
        } else {
          chan[c.chan].furnacePCM=false;
        }
        if (skipRegisterWrites) break;
        if (chan[c.chan].furnacePCM) {
          chan[c.chan].macroInit(ins);
          if (!chan[c.chan].std.vol.will) {
            chan[c.chan].outVol=chan[c.chan].vol;
            immWrite(0x1b,chan[c.chan].outVol);
          }
          chan[c.chan].sample=ins->amiga.getSample(c.value);
          if (chan[c.chan].sample>=0 && chan[c.chan].sample<parent->song.sampleLen) {
            DivSample* s=parent->getSample(chan[c.chan].sample);
            immWrite(0x12,(s->offB>>8)&0xff);
            immWrite(0x13,s->offB>>16);
            int end=s->offB+s->lengthB-1;
            immWrite(0x14,(end>>8)&0xff);
            immWrite(0x15,end>>16);
            immWrite(0x11,isMuted[c.chan]?0:(chan[c.chan].pan<<6));
            immWrite(0x10,(s->loopStart>=0)?0x90:0x80); // start/repeat
            if (c.value!=DIV_NOTE_NULL) {
              chan[c.chan].note=c.value;
              chan[c.chan].baseFreq=NOTE_ADPCMB(chan[c.chan].note);
              chan[c.chan].freqChanged=true;
            }
            chan[c.chan].active=true;
            chan[c.chan].keyOn=true;
          } else {
            immWrite(0x10,0x01); // reset
            immWrite(0x12,0);
            immWrite(0x13,0);
            immWrite(0x14,0);
            immWrite(0x15,0);
            break;
          }
        } else {
          chan[c.chan].sample=-1;
          chan[c.chan].macroInit(NULL);
          chan[c.chan].outVol=chan[c.chan].vol;
          if ((12*sampleBank+c.value%12)>=parent->song.sampleLen) {
            immWrite(0x10,0x01); // reset
            immWrite(0x12,0);
            immWrite(0x13,0);
            immWrite(0x14,0);
            immWrite(0x15,0);
            break;
          }
          DivSample* s=parent->getSample(12*sampleBank+c.value%12);
          immWrite(0x12,(s->offB>>8)&0xff);
          immWrite(0x13,s->offB>>16);
          int end=s->offB+s->lengthB-1;
          immWrite(0x14,(end>>8)&0xff);
          immWrite(0x15,end>>16);
          immWrite(0x11,isMuted[c.chan]?0:(chan[c.chan].pan<<6));
          immWrite(0x10,(s->loopStart>=0)?0x90:0x80); // start/repeat
          int freq=(65536.0*(double)s->rate)/((double)chipClock/144.0);
          immWrite(0x19,freq&0xff);
          immWrite(0x1a,(freq>>8)&0xff);
        }
        break;
      }
      if (c.chan>8) { // ADPCM-A
        if (skipRegisterWrites) break;
        if ((12*sampleBank+c.value%12)>=parent->song.sampleLen) {
          immWrite(0x100,0x80|(1<<(c.chan-9)));
          immWrite(0x110+c.chan-9,0);
          immWrite(0x118+c.chan-9,0);
          immWrite(0x120+c.chan-9,0);
          immWrite(0x128+c.chan-9,0);
          break;
        }
        DivSample* s=parent->getSample(12*sampleBank+c.value%12);
        immWrite(0x110+c.chan-9,(s->offA>>8)&0xff);
        immWrite(0x118+c.chan-9,s->offA>>16);
        int end=s->offA+s->lengthA-1;
        immWrite(0x120+c.chan-9,(end>>8)&0xff);
        immWrite(0x128+c.chan-9,end>>16);
        immWrite(0x108+(c.chan-9),isMuted[c.chan]?0:((chan[c.chan].pan<<6)|chan[c.chan].vol));
        immWrite(0x100,0x00|(1<<(c.chan-9)));
        break;
      }
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_FM);
      chan[c.chan].macroInit(ins);
      if (c.chan<6) {
        if (!chan[c.chan].std.vol.will) {
          chan[c.chan].outVol=chan[c.chan].vol;
        }
      }

      if (chan[c.chan].insChanged) {
        chan[c.chan].state=ins->fm;
      }
      
      for (int i=0; i<4; i++) {
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
        if (isOutput[chan[c.chan].state.alg][i]) {
          if (!chan[c.chan].active || chan[c.chan].insChanged) {
            rWrite(baseAddr+ADDR_TL,127-VOL_SCALE_LOG(127-op.tl,chan[c.chan].outVol&0x7f,127));
          }
        } else {
          if (chan[c.chan].insChanged) {
            rWrite(baseAddr+ADDR_TL,op.tl);
          }
        }
        if (chan[c.chan].insChanged) {
          rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
          rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
          rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
          rWrite(baseAddr+ADDR_DT2_D2R,op.d2r&31);
          rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
          rWrite(baseAddr+ADDR_SSG,op.ssgEnv&15);
        }
      }
      if (chan[c.chan].insChanged) {
        rWrite(chanOffs[c.chan]+ADDR_FB_ALG,(chan[c.chan].state.alg&7)|(chan[c.chan].state.fb<<3));
        rWrite(chanOffs[c.chan]+ADDR_LRAF,(isMuted[c.chan]?0:(chan[c.chan].pan<<6))|(chan[c.chan].state.fms&7)|((chan[c.chan].state.ams&3)<<4));
      }
      chan[c.chan].insChanged=false;

      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_FNUM_BLOCK(c.value,11);
        chan[c.chan].portaPause=false;
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].keyOn=true;
      chan[c.chan].active=true;
      break;
    }
    case DIV_CMD_NOTE_OFF:
      if (c.chan>14) {
        immWrite(0x10,0x01); // reset
        break;
      }
      if (c.chan>8) {
        immWrite(0x100,0x80|(1<<(c.chan-9)));
        break;
      }
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
      chan[c.chan].active=false;
      chan[c.chan].macroInit(NULL);
      break;
    case DIV_CMD_NOTE_OFF_ENV:
      if (c.chan>14) {
        immWrite(0x10,0x01); // reset
        break;
      }
      if (c.chan>8) {
        immWrite(0x100,0x80|(1<<(c.chan-9)));
        break;
      }
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
      chan[c.chan].active=false;
      chan[c.chan].std.release();
      break;
    case DIV_CMD_ENV_RELEASE:
      chan[c.chan].std.release();
      break;
    case DIV_CMD_VOLUME: {
      chan[c.chan].vol=c.value;
      if (!chan[c.chan].std.vol.has) {
        chan[c.chan].outVol=c.value;
      }
      if (c.chan>14) { // ADPCM-B
        immWrite(0x1b,chan[c.chan].outVol);
        break;
      }
      if (c.chan>8) { // ADPCM-A
        immWrite(0x108+(c.chan-9),isMuted[c.chan]?0:((chan[c.chan].pan<<6)|chan[c.chan].vol));
        break;
      }
      for (int i=0; i<4; i++) {
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
        if (isOutput[chan[c.chan].state.alg][i]) {
          rWrite(baseAddr+ADDR_TL,127-VOL_SCALE_LOG(127-op.tl,chan[c.chan].outVol&0x7f,127));
        } else {
          rWrite(baseAddr+ADDR_TL,op.tl);
        }
      }
      break;
    }
    case DIV_CMD_GET_VOLUME: {
      return chan[c.chan].vol;
      break;
    }
    case DIV_CMD_INSTRUMENT:
      if (chan[c.chan].ins!=c.value || c.value2==1) {
        chan[c.chan].insChanged=true;
      }
      chan[c.chan].ins=c.value;
      break;
    case DIV_CMD_PANNING: {
      if (c.value==0 && c.value2==0) {
        chan[c.chan].pan=3;
      } else {
        chan[c.chan].pan=(c.value2>0)|((c.value>0)<<1);
      }
      if (c.chan>14) {
        immWrite(0x11,isMuted[c.chan]?0:(chan[c.chan].pan<<6));
        break;
      }
      if (c.chan>8) {
        immWrite(0x108+(c.chan-9),isMuted[c.chan]?0:((chan[c.chan].pan<<6)|chan[c.chan].vol));
        break;
      }
      rWrite(chanOffs[c.chan]+ADDR_LRAF,(isMuted[c.chan]?0:(chan[c.chan].pan<<6))|(chan[c.chan].state.fms&7)|((chan[c.chan].state.ams&3)<<4));
      break;
    }
    case DIV_CMD_PITCH: {
      if (c.chan==15 && !chan[c.chan].furnacePCM) break;
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_NOTE_PORTA: {
      if (c.chan>5 || parent->song.linearPitch==2) { // PSG, ADPCM-B
        int destFreq=NOTE_OPNB(c.chan,c.value2);
        bool return2=false;
        if (destFreq>chan[c.chan].baseFreq) {
          chan[c.chan].baseFreq+=c.value;
          if (chan[c.chan].baseFreq>=destFreq) {
            chan[c.chan].baseFreq=destFreq;
            return2=true;
          }
        } else {
          chan[c.chan].baseFreq-=c.value;
          if (chan[c.chan].baseFreq<=destFreq) {
            chan[c.chan].baseFreq=destFreq;
            return2=true;
          }
        }
        chan[c.chan].freqChanged=true;
        if (return2) {
          chan[c.chan].inPorta=false;
          return 2;
        }
        break;
      }
      PLEASE_HELP_ME(chan[c.chan]);
      break;
    }
    case DIV_CMD_SAMPLE_BANK:
      sampleBank=c.value;
      if (sampleBank>(parent->song.sample.size()/12)) {
        sampleBank=parent->song.sample.size()/12;
      }
      iface.sampleBank=sampleBank;
      break;
    case DIV_CMD_LEGATO: {
      if (c.chan==15 && !chan[c.chan].furnacePCM) break;
      chan[c.chan].baseFreq=NOTE_OPNB(c.chan,c.value);
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_FM_LFO: {
      rWrite(0x22,(c.value&7)|((c.value>>4)<<3));
      break;
    }
    case DIV_CMD_FM_FB: {
      if (c.chan>5) break;
      chan[c.chan].state.fb=c.value&7;
      rWrite(chanOffs[c.chan]+ADDR_FB_ALG,(chan[c.chan].state.alg&7)|(chan[c.chan].state.fb<<3));
      break;
    }
    case DIV_CMD_FM_MULT: {
      if (c.chan>5) break;
      unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
      DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
      op.mult=c.value2&15;
      rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
      break;
    }
    case DIV_CMD_FM_TL: {
      if (c.chan>5) break;
      unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
      DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
      op.tl=c.value2;
      if (isOutput[chan[c.chan].state.alg][c.value]) {
        rWrite(baseAddr+ADDR_TL,127-VOL_SCALE_LOG(127-op.tl,chan[c.chan].outVol&0x7f,127));
      } else {
        rWrite(baseAddr+ADDR_TL,op.tl);
      }
      break;
    }
    case DIV_CMD_FM_AR: {
      if (c.chan>5) break;
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
          op.ar=c.value2&31;
          unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
          rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
        }
      } else {
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
        op.ar=c.value2&31;
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
      }
      break;
    }
    case DIV_CMD_FM_RS: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
          op.rs=c.value2&3;
          unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
          rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
        op.rs=c.value2&3;
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
      }
      break;
    }
    case DIV_CMD_FM_AM: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
          op.am=c.value2&1;
          unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
          rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
        op.am=c.value2&1;
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
      }
      break;
    }
    case DIV_CMD_FM_DR: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
          op.dr=c.value2&31;
          unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
          rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
        op.dr=c.value2&31;
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
      }
      break;
    }
    case DIV_CMD_FM_SL: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
          op.sl=c.value2&15;
          unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
          rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
        op.sl=c.value2&15;
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
      }
      break;
    }
    case DIV_CMD_FM_RR: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
          op.rr=c.value2&15;
          unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
          rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
        op.rr=c.value2&15;
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
      }
      break;
    }
    case DIV_CMD_FM_D2R: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
          op.d2r=c.value2&31;
          unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
          rWrite(baseAddr+ADDR_DT2_D2R,op.d2r&31);
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
        op.d2r=c.value2&31;
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_DT2_D2R,op.d2r&31);
      }
      break;
    }
    case DIV_CMD_FM_DT: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
          op.dt=c.value&7;
          unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
          rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
        op.dt=c.value2&7;
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
      }
      break;
    }
    case DIV_CMD_FM_SSG: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
          op.ssgEnv=8^(c.value2&15);
          unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
          rWrite(baseAddr+ADDR_SSG,op.ssgEnv&15);
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
        op.ssgEnv=8^(c.value2&15);
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_SSG,op.ssgEnv&15);
      }
      break;
    }
    case DIV_CMD_FM_HARD_RESET:
      chan[c.chan].hardReset=c.value;
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 0;
      break;
    case DIV_CMD_GET_VOLMAX:
      if (c.chan>14) return 255;
      if (c.chan>8) return 31;
      if (c.chan>5) return 15;
      return 127;
      break;
    case DIV_CMD_PRE_PORTA:
      if (c.chan>5) {
        if (chan[c.chan].active && c.value2) {
          if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_FM));
        }
      }
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_PRE_NOTE:
      break;
    default:
      //printf("WARNING: unimplemented command %d\n",c.cmd);
      break;
  }
  return 1;
}

void DivPlatformYM2610B::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  if (ch>14) { // ADPCM-B
    immWrite(0x11,isMuted[ch]?0:(chan[ch].pan<<6));
  }
  if (ch>8) { // ADPCM-A
    immWrite(0x108+(ch-9),isMuted[ch]?0:((chan[ch].pan<<6)|chan[ch].vol));
    return;
  }
  if (ch>5) { // PSG
    ay->muteChannel(ch-6,mute);
    return;
  }
  // FM
  rWrite(chanOffs[ch]+ADDR_LRAF,(isMuted[ch]?0:(chan[ch].pan<<6))|(chan[ch].state.fms&7)|((chan[ch].state.ams&3)<<4));
}

void DivPlatformYM2610B::forceIns() {
  for (int i=0; i<6; i++) {
    for (int j=0; j<4; j++) {
      unsigned short baseAddr=chanOffs[i]|opOffs[j];
      DivInstrumentFM::Operator& op=chan[i].state.op[j];
      if (isOutput[chan[i].state.alg][j]) {
        rWrite(baseAddr+ADDR_TL,127-VOL_SCALE_LOG(127-op.tl,chan[i].outVol&0x7f,127));
      } else {
        rWrite(baseAddr+ADDR_TL,op.tl);
      }
      rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
      rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
      rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
      rWrite(baseAddr+ADDR_DT2_D2R,op.d2r&31);
      rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
      rWrite(baseAddr+ADDR_SSG,op.ssgEnv&15);
    }
    rWrite(chanOffs[i]+ADDR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3));
    rWrite(chanOffs[i]+ADDR_LRAF,(isMuted[i]?0:(chan[i].pan<<6))|(chan[i].state.fms&7)|((chan[i].state.ams&3)<<4));
    if (chan[i].active) {
      chan[i].keyOn=true;
      chan[i].freqChanged=true;
    }
  }
  for (int i=9; i<16; i++) {
    chan[i].insChanged=true;
  }

  ay->forceIns();
  ay->flushWrites();
  for (DivRegWrite& i: ay->getRegisterWrites()) {
    immWrite(i.addr&15,i.val);
  }
  ay->getRegisterWrites().clear();
}

void* DivPlatformYM2610B::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformYM2610B::getChanMacroInt(int ch) {
  if (ch>=6 && ch<9) return ay->getChanMacroInt(ch-6);
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformYM2610B::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformYM2610B::getRegisterPool() {
  return regPool;
}

int DivPlatformYM2610B::getRegisterPoolSize() {
  return 512;
}

void DivPlatformYM2610B::poke(unsigned int addr, unsigned short val) {
  immWrite(addr,val);
}

void DivPlatformYM2610B::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) immWrite(i.addr,i.val);
}

void DivPlatformYM2610B::reset() {
  while (!writes.empty()) writes.pop();
  memset(regPool,0,512);
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  fm->reset();
  for (int i=0; i<16; i++) {
    chan[i]=DivPlatformYM2610B::Channel();
    chan[i].std.setEngine(parent);
  }
  for (int i=0; i<6; i++) {
    chan[i].vol=0x7f;
    chan[i].outVol=0x7f;
  }
  for (int i=6; i<9; i++) {
    chan[i].vol=0x0f;
  }
  for (int i=9; i<15; i++) {
    chan[i].vol=0x1f;
  }
  chan[15].vol=0xff;

  for (int i=0; i<512; i++) {
    oldWrites[i]=-1;
    pendingWrites[i]=-1;
  }

  lastBusy=60;
  sampleBank=0;

  delay=0;

  extMode=false;

  // LFO
  immWrite(0x22,0x08);

  // PCM volume
  immWrite(0x101,0x3f); // A
  immWrite(0x1b,0xff); // B

  ay->reset();
  ay->getRegisterWrites().clear();
  ay->flushWrites();
}

bool DivPlatformYM2610B::isStereo() {
  return true;
}

bool DivPlatformYM2610B::keyOffAffectsArp(int ch) {
  return (ch>5);
}

void DivPlatformYM2610B::notifyInsChange(int ins) {
  for (int i=0; i<16; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
  ay->notifyInsChange(ins);
}

void DivPlatformYM2610B::notifyInsDeletion(void* ins) {
  ay->notifyInsDeletion(ins);
}

void DivPlatformYM2610B::setSkipRegisterWrites(bool value) {
  DivDispatch::setSkipRegisterWrites(value);
  ay->setSkipRegisterWrites(value);
}

int DivPlatformYM2610B::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  DivPlatformYM2610Base::init(p, channels, sugRate, flags);
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<16; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  chipClock=8000000;
  rate=chipClock/16;
  for (int i=0; i<16; i++) {
    oscBuf[i]->rate=rate;
  }
  fm=new ymfm::ym2610b(iface);
  // YM2149, 2MHz
  ay=new DivPlatformAY8910;
  ay->init(p,3,sugRate,19);
  ay->toggleRegisterDump(true);
  reset();
  return 16;
}

void DivPlatformYM2610B::quit() {
  for (int i=0; i<16; i++) {
    delete oscBuf[i];
  }
  ay->quit();
  delete ay;
  delete fm;
  DivPlatformYM2610Base::quit();
}

DivPlatformYM2610B::~DivPlatformYM2610B() {
}
