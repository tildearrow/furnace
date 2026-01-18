/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
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

// most of this code written by LTVA
// ported to Furnace by tildearrow

// portions apparently taken from FamiTracker source, which is under GPLv2+

// TODO:
// - format code?

#include "fileOpsCommon.h"

#define CHECK_BLOCK_VERSION(x) \
  if (blockVersion>x) { \
    logW("incompatible block version %d for %s!", blockVersion, blockName); \
  }

enum FTEffects {
  FT_EF_NONE = 0,
  FT_EF_SPEED,               // Speed
  FT_EF_JUMP,                // Jump
  FT_EF_SKIP,                // Skip
  FT_EF_HALT,                // Halt
  FT_EF_VOLUME,              // Volume
  FT_EF_PORTAMENTO,          // Porta on
  FT_EF_PORTAOFF,            // Porta off unused
  FT_EF_SWEEPUP,             // Sweep up
  FT_EF_SWEEPDOWN,           // Sweep down
  FT_EF_ARPEGGIO,            // Arpeggio
  FT_EF_VIBRATO,             // Vibrato
  FT_EF_TREMOLO,             // Tremolo
  FT_EF_PITCH,               // Pitch
  FT_EF_DELAY,               // Note delay
  FT_EF_DAC,                 // DAC setting
  FT_EF_PORTA_UP,            // Portamento up
  FT_EF_PORTA_DOWN,          // Portamento down
  FT_EF_DUTY_CYCLE,          // Duty cycle
  FT_EF_SAMPLE_OFFSET,       // Sample offset
  FT_EF_SLIDE_UP,            // Slide up
  FT_EF_SLIDE_DOWN,          // Slide down
  FT_EF_VOLUME_SLIDE,        // Volume slide
  FT_EF_NOTE_CUT,            // Note cut
  FT_EF_RETRIGGER,           // DPCM retrigger
  FT_EF_DELAYED_VOLUME,      // // // Delayed channel volume
  FT_EF_FDS_MOD_DEPTH,       // FDS modulation depth
  FT_EF_FDS_MOD_SPEED_HI,    // FDS modulation speed hi
  FT_EF_FDS_MOD_SPEED_LO,    // FDS modulation speed lo
  FT_EF_DPCM_PITCH,          // DPCM Pitch
  FT_EF_SUNSOFT_ENV_TYPE,    // Sunsoft envelope type
  FT_EF_SUNSOFT_ENV_HI,      // Sunsoft envelope high
  FT_EF_SUNSOFT_ENV_LO,      // Sunsoft envelope low
  FT_EF_SUNSOFT_NOISE,       // // // 050B Sunsoft noise period
  FT_EF_VRC7_PORT,           // // // 050B VRC7 custom patch port
  FT_EF_VRC7_WRITE,          // // // 050B VRC7 custom patch write
  FT_EF_NOTE_RELEASE,        // // // Delayed release
  FT_EF_GROOVE,              // // // Groove
  FT_EF_TRANSPOSE,           // // // Delayed transpose
  FT_EF_N163_WAVE_BUFFER,    // // // N163 wave buffer
  FT_EF_FDS_VOLUME,          // // // FDS volume envelope
  FT_EF_FDS_MOD_BIAS,        // // // FDS auto-FM bias
  FT_EF_PHASE_RESET,         // Reset waveform phase without retriggering note (VRC6-only so far)
  FT_EF_HARMONIC,            // Multiply the note pitch by an integer
  FT_EF_TARGET_VOLUME_SLIDE, // // !! Target volume slide

  FT_EF_COUNT
};

const int ftEffectMap[]={
  -1, // none
  0x0f,
  0x0b,
  0x0d,
  0xff,
  -1, // volume? not supported in Furnace yet
  0x03,
  0x03, // unused?
  0x13, 0x14, 0x00, 0x04, 0x07, 0xe5, 0xed, 0x11,
  0x01, // porta up
  0x02, // porta down
  0x12,
  0x90, // sample offset - not supported yet
  0xe1, // Slide up
  0xe2, // Slide down
  0x0a, 0xec, 0x0c,
  -1,   // delayed volume - not supported yet
  0x11, // FDS modulation depth
  0x12, // FDS modulation speed hi
  0x13, // FDS modulation speed lo
  0x20, // DPCM pitch
  0x22, // Sunsoft envelope type
  0x24, // Sunsoft envelope high
  0x23, // Sunsoft envelope low
  0x21, // 050B Sunsoft noise period
  -1,   // VRC7 "custom patch port" - not supported?
  -1,   // VRC7 "custom patch write"
  0xfc, // delayed release
  0x09, // select groove
  0xe6, // delayed note transpose
  0x11, // Namco 163 wave RAM offset
  -1,   // FDS vol env - not supported
  -1,   // FDS auto FM - not supported yet
  -1,   // phase reset - not supported
  -1,   // harmonic - not supported
  -1,   // target volume slide - not supported
};

enum EFTEffects {
  EFT_EF_NONE = 0,
  EFT_EF_SPEED,                // Speed
  EFT_EF_JUMP,                 // Jump
  EFT_EF_SKIP,                 // Skip
  EFT_EF_HALT,                 // Halt
  EFT_EF_VOLUME,               // Volume
  EFT_EF_PORTAMENTO,           // Porta on
  EFT_EF_PORTAOFF,             // Porta off unused
  EFT_EF_SWEEPUP,              // Sweep up
  EFT_EF_SWEEPDOWN,            // Sweep down
  EFT_EF_ARPEGGIO,             // Arpeggio
  EFT_EF_VIBRATO,              // Vibrato
  EFT_EF_TREMOLO,              // Tremolo
  EFT_EF_PITCH,                // Pitch
  EFT_EF_DELAY,                // Note delay
  EFT_EF_DAC,                  // DAC setting
  EFT_EF_PORTA_UP,             // Portamento up
  EFT_EF_PORTA_DOWN,           // Portamento down
  EFT_EF_DUTY_CYCLE,           // Duty cycle
  EFT_EF_SAMPLE_OFFSET,        // Sample offset
  EFT_EF_SLIDE_UP,             // Slide up
  EFT_EF_SLIDE_DOWN,           // Slide down
  EFT_EF_VOLUME_SLIDE,         // Volume slide
  EFT_EF_NOTE_CUT,             // Note cut
  EFT_EF_RETRIGGER,            // DPCM retrigger
  EFT_EF_DELAYED_VOLUME,       // // // Delayed channel volume
  EFT_EF_FDS_MOD_DEPTH,        // FDS modulation depth
  EFT_EF_FDS_MOD_SPEED_HI,     // FDS modulation speed hi
  EFT_EF_FDS_MOD_SPEED_LO,     // FDS modulation speed lo
  EFT_EF_DPCM_PITCH,           // DPCM Pitch
  EFT_EF_SUNSOFT_ENV_TYPE,     // Sunsoft envelope type
  EFT_EF_SUNSOFT_ENV_HI,       // Sunsoft envelope high
  EFT_EF_SUNSOFT_ENV_LO,       // Sunsoft envelope low
  EFT_EF_SUNSOFT_NOISE,        // // // 050B Sunsoft noise period
  EFT_EF_AY8930_PULSE_WIDTH,   // // // AY8930 pulse width
  EFT_EF_AY8930_AND_MASK,      // // // AY8930 noise AND mask
  EFT_EF_AY8930_OR_MASK,       // // // AY8930 noise OR mask
  EFT_EF_AY8930_VOL,           // // // AY8930 extra volume bit
  EFT_EF_VRC7_PORT,            // // // 050B VRC7 custom patch port
  EFT_EF_VRC7_WRITE,           // // // 050B VRC7 custom patch write
  EFT_EF_NOTE_RELEASE,         // // // Delayed release
  EFT_EF_GROOVE,               // // // Groove
  EFT_EF_TRANSPOSE,            // // // Delayed transpose
  EFT_EF_N163_WAVE_BUFFER,     // // // N163 wave buffer
  EFT_EF_FDS_VOLUME,           // // // FDS volume envelope
  EFT_EF_FDS_MOD_BIAS,         // // // FDS auto-FM bias
  EFT_EF_PHASE_RESET,          // Reset waveform phase without retriggering note (VRC6-only so far)
  EFT_EF_HARMONIC,             // Multiply the note pitch by an integer
  EFT_EF_PWM,                  // // // Pulse width modulation effect
  EFT_EF_VOLUME_OFFSET,        // // // Relative volume change
  EFT_EF_SAA_NOISE_MODE,       // // // SAA1099 noise mode
  EFT_EF_SID_FILTER_RESONANCE, // // // SID filter resonance
  EFT_EF_SID_FILTER_CUTOFF_HI, // // // SID filter cutoff hi
  EFT_EF_SID_FILTER_CUTOFF_LO, // // // SID filter cutoff lo
  EFT_EF_SID_FILTER_MODE,      // // // SID filter mode
  EFT_EF_SID_ENVELOPE,         // // // SID envelope parameters
  EFT_EF_SID_RING,             // // // SID ringmod

  EFT_EF_COUNT
};

const int eftEffectMap[] = {
  -1, // none
  0x0f,
  0x0b,
  0x0d,
  0xff,
  -1, // volume? not supported in Furnace yet
  0x03,
  0x03, // unused?
  0x13,
  0x14,
  0x00,
  0x04,
  0x07,
  0xe5,
  0xed,
  0x11,
  0x01, // porta up
  0x02, // porta down
  0x12,
  0x90, // sample offset - not supported yet
  0xe1, // Slide up
  0xe2, // Slide down
  0x0a,
  0xec,
  0x0c,
  -1,    // delayed volume - not supported yet
  0x11,  // FDS modulation depth
  0x12,  // FDS modulation speed hi
  0x13,  // FDS modulation speed lo
  0x20,  // DPCM pitch
  0x22,  // Sunsoft envelope type
  0x24,  // Sunsoft envelope high
  0x23,  // Sunsoft envelope low
  0x21,  // 050B Sunsoft noise period
  0x12,  // // // AY8930 pulse width
  0x27,  // // // AY8930 noise AND mask
  0x28,  // // // AY8930 noise OR mask
  0x100, // // // AY8930 extra volume bit
  -1,    // VRC7 "custom patch port" - not supported?
  -1,    // VRC7 "custom patch write"
  0xfc,  // delayed release
  0x09,  // select groove
  0xe6,  // delayed note transpose
  0x11,  // Namco 163 wave RAM offset
  -1,    // FDS vol env - not supported
  -1,    // FDS auto FM - not supported yet
  -1,    // phase reset - not supported
  -1,    // harmonic - not supported
  -1,    // // // Pulse width modulation effect
  -1,    // // // Relative volume change
  -1,    // // // SAA1099 noise mode
  0x13,  // // // SID filter resonance
  0x40,  // // // SID filter cutoff hi
  0x40,  // // // SID filter cutoff lo
  0x14,  // // // SID filter mode
  -1,    // // // SID envelope parameters
  -1,    // // // SID ringmod
};

const int eff_conversion_050[][2] = {
  {FT_EF_SUNSOFT_NOISE, FT_EF_NOTE_RELEASE},
  {FT_EF_VRC7_PORT, FT_EF_GROOVE},
  {FT_EF_VRC7_WRITE, FT_EF_TRANSPOSE},
  {FT_EF_NOTE_RELEASE, FT_EF_N163_WAVE_BUFFER},
  {FT_EF_GROOVE, FT_EF_FDS_VOLUME},
  {FT_EF_TRANSPOSE, FT_EF_FDS_MOD_BIAS},
  {FT_EF_N163_WAVE_BUFFER, FT_EF_SUNSOFT_NOISE},
  {FT_EF_FDS_VOLUME, FT_EF_VRC7_PORT},
  {FT_EF_FDS_MOD_BIAS, FT_EF_VRC7_WRITE},
  {0xFF, 0xFF}, // end mark
};

constexpr int ftEffectMapSize = sizeof(ftEffectMap) / sizeof(int);
constexpr int eftEffectMapSize = sizeof(eftEffectMap) / sizeof(int);

int convertMacros2A03[5] = {(int)DIV_MACRO_VOL, (int)DIV_MACRO_ARP, (int)DIV_MACRO_PITCH, -1, (int)DIV_MACRO_DUTY};
int convertMacrosVRC6[5] = {(int)DIV_MACRO_VOL, (int)DIV_MACRO_ARP, (int)DIV_MACRO_PITCH, -1, (int)DIV_MACRO_DUTY};
int convertMacrosN163[5] = {(int)DIV_MACRO_VOL, (int)DIV_MACRO_ARP, (int)DIV_MACRO_PITCH, -1, (int)DIV_MACRO_WAVE};
int convertMacros5B[5] = {(int)DIV_MACRO_VOL, (int)DIV_MACRO_ARP, (int)DIV_MACRO_PITCH, -1, (int)DIV_MACRO_DUTY};

int convertMacrosSID[5] = {(int)DIV_MACRO_VOL, (int)DIV_MACRO_ARP, (int)DIV_MACRO_PITCH, -1, (int)DIV_MACRO_WAVE};

int convert_vrc6_duties[4] = {1, 3, 7, 3};

int findEmptyFx(short* data) {
  for (int i=0; i<7; i++) {
    if (data[DIV_PAT_FX(i)]==-1) return i;
  }
  return -1;
}

void copyMacro(DivInstrument* ins, DivInstrumentMacro* from, int macro_type, int setting) {
  DivInstrumentMacro* to = NULL;

  switch (ins->type) {
    case DIV_INS_NES: {
      if (convertMacros2A03[macro_type] == -1)
        return;
      to = ins->std.macroByType((DivMacroType)convertMacros2A03[macro_type]);
      break;
    }
    case DIV_INS_VRC6: {
      if (convertMacrosVRC6[macro_type] == -1)
        return;
      to = ins->std.macroByType((DivMacroType)convertMacros2A03[macro_type]);
      break;
    }
    case DIV_INS_N163: {
      if (convertMacrosVRC6[macro_type] == -1)
        return;
      to = ins->std.macroByType((DivMacroType)convertMacrosN163[macro_type]);
      break;
    }
    case DIV_INS_AY: {
      if (convertMacrosVRC6[macro_type] == -1)
        return;
      to = ins->std.macroByType((DivMacroType)convertMacros5B[macro_type]);
      break;
    }
    case DIV_INS_C64: {
      if (convertMacrosVRC6[macro_type] == -1)
        return;
      to = ins->std.macroByType((DivMacroType)convertMacrosSID[macro_type]);
      break;
    }
    default:
      break;
  }

  if (to == NULL)
    return;

  for (int i = 0; i < 256; i++) {
    to->val[i] = from->val[i];

    if ((DivMacroType)convertMacros2A03[macro_type] == DIV_MACRO_ARP) {
      if (setting == 0) // absolute
      {
        if (to->val[i] > 0x60) {
          int temp = to->val[i];
          to->val[i] = -1 * (0xff - temp + 1); // 2s complement integer my beloved
        }
      }

      if (setting == 1) // fixed
      {
        to->val[i] |= (1 << 30); // 30th bit in Furnace arp macro marks fixed mode
      }
    }

    if ((DivMacroType)convertMacros2A03[macro_type] == DIV_MACRO_PITCH) {
      if (setting == 0 || setting == 1) // relative/absolute
      {
        int temp = to->val[i];

        if (temp < 0x80) {
          to->val[i] = -1 * temp;
        } else {
          to->val[i] = (0x100 - temp);
        }
      }
    }

    if ((DivMacroType)convertMacrosN163[macro_type] == DIV_MACRO_WAVE && ins->type == DIV_INS_N163) {
      // pfffff
    }

    if ((DivMacroType)convertMacrosN163[macro_type] == DIV_MACRO_PITCH && (ins->type == DIV_INS_N163 || ins->type == DIV_INS_C64)) {
      to->val[i] *= -1; // wtf is going on!!!
    }
  }

  to->len = from->len;
  to->delay = from->delay;
  to->mode = from->mode;
  to->rel = from->rel;
  to->speed = from->speed;
  to->loop = from->loop;
  to->open = from->open;

  if ((DivMacroType)convertMacros2A03[macro_type] == DIV_MACRO_ARP) {
    if (setting == 1) // fixed
    {
      if (to->loop == 255) // no loop
      {
        to->len++;
        to->val[to->len - 1] = 0; // return to orig pitch (relative mode, 0 offset)
      }
    }
  }

  if ((DivMacroType)convertMacros2A03[macro_type] == DIV_MACRO_PITCH) {
    if (setting == 0) // relative
    {
      to->mode = 1; // setting relative mode
    }
  }

  if (ins->type == DIV_INS_AY && macro_type == 4) // S5B noise/mode macro combines noise freq and tone/env/noise settings, so we need to separate them into two macros
  {
    DivInstrumentMacro* wave = &ins->std.waveMacro;
    to = &ins->std.dutyMacro;

    wave->len = to->len;
    wave->delay = to->delay;
    wave->mode = to->mode;
    wave->rel = to->rel;
    wave->speed = to->speed;
    wave->loop = to->loop;
    wave->open = to->open;

    for (int i = 0; i < to->len; i++) {
      // ? ? ? ?

      logI("%02X", to->val[i]);
      wave->val[i] = 0;

      int temp = 0;

      if (to->val[i] & 0b10000000) // noise
      {
        temp |= 2;
      }
      if (to->val[i] & 0b01000000) // tone
      {
        temp |= 1;
      }
      if (to->val[i] & 0b00100000) // envelope
      {
        temp |= 4;
      }

      wave->val[i] = temp;

      // #define S5B_ENVL 0b10000000
      // #define S5B_TONE 0b01000000
      // #define S5B_NOIS 0b00100000

      to->val[i] = to->val[i] & 31;
    }
  }
}

bool DivEngine::loadFTM(unsigned char* file, size_t len, bool dnft, bool dnft_sig, bool eft) {
  SafeReader reader = SafeReader(file, len);
  warnings = "";
  try {
    DivSong ds;
    String blockName;
    unsigned int expansions = 0;
    unsigned int tchans = 0;
    unsigned int n163Chans = 0;
    int n163WaveOff[128];
    bool hasSequence[256][8];
    unsigned char sequenceIndex[256][8];
    unsigned char macro_types[256][8];
    std::vector<DivInstrumentMacro> macros[256];
    std::vector<String> encounteredBlocks;
    unsigned char map_channels[DIV_MAX_CHANS];
    unsigned int hilightA = 4;
    unsigned int hilightB = 16;
    double customHz = 60.0;

    unsigned char fds_chan = 0xff;
    unsigned char vrc6_saw_chan = 0xff;
    unsigned char n163_chans[8] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    unsigned char vrc7_chans[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    unsigned char s5b_chans[3] = {0xff, 0xff, 0xff};
    unsigned char ay8930_chans[3] = {0xff, 0xff, 0xff};

    // these two seem to go unused, but why?
    unsigned char vrc6_chans[2] = {0xff, 0xff};
    unsigned char mmc5_chans[2] = {0xff, 0xff};

    int total_chans = 0;

    memset(n163WaveOff,0,128*sizeof(int));
    memset(hasSequence, 0, 256 * 8 * sizeof(bool));
    memset(sequenceIndex, 0, 256 * 8);
    memset(macro_types, 0, 256 * 8);
    memset(map_channels, 0xfe, DIV_MAX_CHANS * sizeof(unsigned char));

    for (int i = 0; i < 256; i++) {
      for (int j = 0; j < 8; j++) {
        macros[i].push_back(DivInstrumentMacro(DIV_MACRO_VOL));
        macros[i][j].open|=9;
      }
    }

    if (!reader.seek((dnft && dnft_sig) ? 21 : 18, SEEK_SET)) {
      logE("premature end of file!");
      lastError = "incomplete file";
      delete[] file;
      return false;
    }
    ds.version = (unsigned short)reader.readI();
    logI("module version %d (0x%.4x)", ds.version, ds.version);

    if ((ds.version > 0x0450 && !eft) || (eft && ds.version > 0x0460)) {
      logE("incompatible version %x!", ds.version);
      lastError = "incompatible version";
      delete[] file;
      return false;
    }

    for (DivSubSong* i: ds.subsong) {
      i->clearData();
      delete i;
    }
    ds.subsong.clear();

    ds.compatFlags.linearPitch = 0;

    unsigned int pal = 0;

    while (true) {
      blockName = reader.readString(3);
      if (blockName == "END") {
        // end of module
        logD("end of data");
        break;
      }

      // not the end
      if (!reader.seek(-3, SEEK_CUR)) {
        logE("couldn't seek back by 3!");
        lastError = "couldn't seek back by 3";
        delete[] file;
        return false;
      }
      blockName = reader.readString(16);
      unsigned int blockVersion = (unsigned int)reader.readI();
      unsigned int blockSize = (unsigned int)reader.readI();
      size_t blockStart = reader.tell();

      logD("reading block %s (version %d, %d bytes, position %x)", blockName, blockVersion, blockSize, reader.tell());

      for (String& i: encounteredBlocks) {
        if (blockName==i) {
          logE("duplicate block %s!",blockName);
          lastError = "duplicate block "+blockName;
          ds.unload();
          delete[] file;
          return false;
        }
      }
      encounteredBlocks.push_back(blockName);

      if (blockName == "PARAMS") {
        // versions 7-9 don't change anything?
        CHECK_BLOCK_VERSION(9);
        unsigned int oldSpeedTempo = 0;
        if (blockVersion <= 1) {
          oldSpeedTempo = reader.readI();
        }
        if (blockVersion >= 2) {
          if (eft) {
            if (blockVersion < 7) {
              expansions = reader.readC();
            } else {
              expansions = reader.readI();
            }
          } else {
            expansions = reader.readC();
          }
        }

        tchans = reader.readI();

        if (tchans>=DIV_MAX_CHANS) {
          logE("invalid channel count! %d",tchans);
          lastError = "invalid channel count";
          delete[] file;
          return false;
        }

        if (tchans == 5) {
          expansions = 0; // This is strange. Sometimes expansion chip is set to 0xFF in files
        }

        pal = reader.readI();
        if (!eft) {
          if (blockVersion >= 7) {
            // advanced Hz control
            int controlType = reader.readI();
            int readHz=reader.readI();
            if (readHz<=0) {
              customHz=60.0;
            } else switch (controlType) {
              case 1:
                customHz = 1000000.0 / (double)readHz;
                break;
              default:
                logW("unsupported tick rate control type %d",controlType);
                break;
            }
          } else {
            customHz = reader.readI();
          }
        } else {
          customHz = reader.readI();
        }

        logV("before clamp: %f",customHz);

        if (customHz>1000.0) customHz=1000.0;

        unsigned int newVibrato = 0;
        bool sweepReset = false;
        unsigned int speedSplitPoint = 0;
        if (blockVersion >= 3) {
          newVibrato = reader.readI();
        }
        if (blockVersion >= 9) {
          sweepReset = reader.readI();
        }
        if (eft) {
          if (blockVersion >= 4 && blockVersion <= 7) {
            hilightA = reader.readI();
            hilightB = reader.readI();
          }
        } else {
          if (blockVersion >= 4 && blockVersion < 7) {
            hilightA = reader.readI();
            hilightB = reader.readI();
          }
        }

        if ((expansions & 16) && blockVersion >= 5) { // N163 channels
          n163Chans = reader.readI();
          if (n163Chans<1 || n163Chans>=9) {
            logE("invalid Namco 163 channel count! %d",n163Chans);
            lastError = "invalid Namco 163 channel count";
            delete[] file;
            return false;
          }
        }
        if (blockVersion >= 6) {
          speedSplitPoint = reader.readI();
        }

        if (blockVersion == 8) {
          int fineTuneCents = reader.readC() * 100;
          fineTuneCents += reader.readC();

          ds.tuning = 440.0 * pow(2.0, (double)fineTuneCents / 1200.0);

          logV("detune: %d", fineTuneCents);
        }

        logV("old speed/tempo: %d", oldSpeedTempo);
        logV("expansions: %x", expansions);
        logV("channels: %d", tchans);
        logV("PAL: %d", pal);
        logV("custom Hz: %f", customHz);
        logV("new vibrato: %d", newVibrato);
        logV("N163 channels: %d", n163Chans);
        logV("highlight 1: %d", hilightA);
        logV("highlight 2: %d", hilightB);
        logV("split point: %d", speedSplitPoint);
        logV("sweep reset: %d", sweepReset);

        //addWarning("FamiTracker import is experimental.");

        // initialize channels
        int systemID = 0;

        int curr_chan = 0;
        int map_ch = 0;

        ds.systemChans[systemID]=5;
        ds.system[systemID++] = DIV_SYSTEM_NES;
        ds.systemFlags[0].set("resetSweep",true); // FamiTracker behavior

        if (pal) {
          ds.systemFlags[0].set("clockSel", 1); // PAL clock
        }

        for (int ch = 0; ch < 5; ch++) {
          map_channels[curr_chan] = map_ch;
          curr_chan++;
          map_ch++;
        }

        if (expansions & 1) {
          ds.systemChans[systemID]=3;
          ds.system[systemID++] = DIV_SYSTEM_VRC6;

          for (int ch = 0; ch < 3; ch++) {
            map_channels[curr_chan] = map_ch;

            if (ch < 2) {
              vrc6_chans[ch] = map_ch;
              logV("%d",vrc6_chans[ch]);
            }

            curr_chan++;
            map_ch++;
          }

          vrc6_saw_chan = map_ch - 1;
        }
        if (expansions & 8) {
          ds.systemChans[systemID]=3;
          ds.system[systemID++] = DIV_SYSTEM_MMC5;

          for (int ch = 0; ch < (eft ? 3 : 2); ch++) {
            map_channels[curr_chan] = map_ch;

            if (ch < 2) {
              mmc5_chans[ch] = map_ch;
              logV("%d",mmc5_chans[ch]);
            }

            curr_chan++;
            map_ch++;
          }

          if (!eft) {
            map_channels[curr_chan] = map_ch; // do not populate and skip MMC5 PCM channel!
            map_ch++;
          }
        }
        if (expansions & 16) {
          ds.system[systemID] = DIV_SYSTEM_N163;
          ds.systemFlags[systemID].set("channels", (int)n163Chans - 1);
          ds.systemChans[systemID]=CLAMP(n163Chans,1,8);
          systemID++;

          for (int ch = 0; ch < (int)n163Chans; ch++) {
            map_channels[curr_chan] = map_ch;
            curr_chan++;
            n163_chans[ch] = map_ch;
            map_ch++;
          }

          /*for (int ch = 0; ch < (8 - (int)n163Chans); ch++) {
            map_channels[curr_chan] = map_ch; // do not populate and skip the missing N163 channels!
            map_ch++;
          }*/
        }
        if (expansions & 4) {
          ds.systemChans[systemID]=1;
          ds.system[systemID++] = DIV_SYSTEM_FDS;

          map_channels[curr_chan] = map_ch;
          fds_chan = map_ch;
          curr_chan++;
          map_ch++;
        }
        if (expansions & 2) {
          ds.systemChans[systemID]=6;
          ds.system[systemID++] = DIV_SYSTEM_VRC7;

          for (int ch = 0; ch < 6; ch++) {
            map_channels[curr_chan] = map_ch;
            vrc7_chans[ch] = map_ch;
            curr_chan++;
            map_ch++;
          }
        }
        if (expansions & 32) {
          ds.system[systemID] = DIV_SYSTEM_AY8910;
          ds.systemChans[systemID]=3;
          ds.systemFlags[systemID++].set("chipType", 2); // Sunsoft 5B

          for (int ch = 0; ch < 3; ch++) {
            map_channels[curr_chan] = map_ch;
            s5b_chans[ch] = map_ch;
            curr_chan++;
            map_ch++;
          }
        }
        if (expansions & 64) {
          ds.systemChans[systemID]=3;
          ds.system[systemID++] = DIV_SYSTEM_AY8930;

          for (int ch = 0; ch < 3; ch++) {
            map_channels[curr_chan] = map_ch;
            ay8930_chans[ch] = map_ch;
            curr_chan++;
            map_ch++;
          }
        }
        if (expansions & 128) {
          ds.systemChans[systemID]=6;
          ds.system[systemID++] = DIV_SYSTEM_SAA1099;

          for (int ch = 0; ch < 6; ch++) {
            map_channels[curr_chan] = map_ch;
            curr_chan++;
            map_ch++;
          }
        }
        if (expansions & 256) {
          ds.systemChans[systemID]=5;
          ds.system[systemID++] = DIV_SYSTEM_5E01;

          for (int ch = 0; ch < 5; ch++) {
            map_channels[curr_chan] = map_ch;
            curr_chan++;
            map_ch++;
          }
        }
        if (expansions & 512) {
          ds.systemChans[systemID]=3;
          ds.system[systemID++] = DIV_SYSTEM_C64_6581;

          for (int ch = 0; ch < 3; ch++) {
            map_channels[curr_chan] = map_ch;
            curr_chan++;
            map_ch++;
          }
        }
        if (expansions & 1024) {
          ds.systemChans[systemID]=3;
          ds.system[systemID++] = DIV_SYSTEM_C64_8580;

          for (int ch = 0; ch < 3; ch++) {
            map_channels[curr_chan] = map_ch;
            curr_chan++;
            map_ch++;
          }
        }
        if (expansions & 2048) {
          ds.systemChans[systemID]=4;
          ds.system[systemID++] = DIV_SYSTEM_POKEY;

          for (int ch = 0; ch < 4; ch++) {
            map_channels[curr_chan] = map_ch;
            curr_chan++;
            map_ch++;
          }
        }
        ds.systemLen = systemID;

        for (int i = 0; i < curr_chan; i++) {
          logV("map ch: fami ch %d mapped to furnace ch %d", i, map_channels[i]);
        }

        unsigned int calcChans = 0;
        for (int i = 0; i < ds.systemLen; i++) {
          if (ds.system[i] == DIV_SYSTEM_MMC5 && !eft) {
            calcChans--; // no PCM channel for MMC5 in famitracker
          }

          calcChans += ds.systemChans[i];
          total_chans += ds.systemChans[i];
        }
        if (calcChans != tchans) {
          // TODO: would ignore trigger CVE? too bad if so!
          if (!eft || (eft && (expansions & 8) == 0)) // ignore since I have no idea how to tell apart E-FT versions which do or do not have PCM chan. Yes, this may lead to all the righer channels to be shifted but at least you still get note data!
          {
            logE("channel counts do not match! %d != %d", tchans, calcChans);
            lastError = "channel counts do not match";
            delete[] file;
            return false;
          }
        }
        if (tchans > DIV_MAX_CHANS) {
          logE("too many channels!");
          lastError = "too many channels";
          delete[] file;
          return false;
        }
        if (blockVersion == 9 && blockSize - (reader.tell() - blockStart) == 2) // weird
        {
          if (!reader.seek(2, SEEK_CUR)) {
            logE("could not weird-seek by 2!");
            lastError = "could not weird-seek by 2";
            delete[] file;
            return false;
          }
        }
      } else if (blockName == "INFO") {
        CHECK_BLOCK_VERSION(1);
        ds.name = reader.readString(32);
        ds.author = reader.readString(32);
        ds.copyright = reader.readString(32);
        ds.systemName = "NES";
      } else if (blockName == "HEADER") {
        CHECK_BLOCK_VERSION(4);
        unsigned char totalSongs=0;
        if (blockVersion>=2) totalSongs=reader.readC();
        logV("%d songs:", totalSongs + 1);
        ds.subsong.reserve(totalSongs);
        for (int i = 0; i <= totalSongs; i++) {
          String subSongName;
          if (blockVersion>=3) subSongName=reader.readString();
          ds.subsong.push_back(new DivSubSong);
          ds.subsong[i]->name = subSongName;
          ds.subsong[i]->hilightA = hilightA;
          ds.subsong[i]->hilightB = hilightB;
          if (customHz != 0) {
            ds.subsong[i]->hz = customHz;
            ds.subsong[i]->virtualTempoD = (short)(2.5 * customHz);
          }
          logV("- %s", subSongName);
        }
        for (unsigned int i = 0; i < tchans; i++) {
          // TODO: obey channel ID
          unsigned char chID = reader.readC();
          logV("for channel ID %d", chID);
          for (int j = 0; j <= totalSongs; j++) {
            unsigned char effectCols = reader.readC();

            if (effectCols>7) {
              logE("too many effect columns!");
              lastError = "too many effect columns";
              delete[] file;
              return false;
            }

            if (map_channels[i] == 0xfe) {
              ds.subsong[j]->pat[i].effectCols = 1;
              logV("- song %d has %d effect columns", j, effectCols);
            } else {
              ds.subsong[j]->pat[map_channels[i]].effectCols = effectCols + 1;
              logV("- song %d has %d effect columns", j, effectCols);
            }
          }
        }

        if (blockVersion >= 4) {
          for (int i = 0; i <= totalSongs; i++) {
            ds.subsong[i]->hilightA = (unsigned char)reader.readC();
            ds.subsong[i]->hilightB = (unsigned char)reader.readC();
          }
        }
      } else if (blockName == "INSTRUMENTS") {
        CHECK_BLOCK_VERSION(9);

        ds.insLen = reader.readI();
        if (ds.insLen < 0 || ds.insLen > 256) {
          logE("too many instruments/out of range!");
          lastError = "too many instruments/out of range";
          delete[] file;
          return false;
        }

        for (int i = 0; i < 128; i++) {
          DivInstrument* ins = new DivInstrument;
          ds.ins.push_back(ins);
          ds.ins[i]->type = DIV_INS_FM;
        }

        logV("instruments:");
        for (int i = 0; i < ds.insLen; i++) {
          unsigned int insIndex = reader.readI();
          if (insIndex >= ds.ins.size()) {
            logE("instrument index %d is out of range!",insIndex);
            lastError="instrument index out of range";
            delete[] file;
            return false;
          }

          DivInstrument* ins = ds.ins[insIndex];
          unsigned char insType = reader.readC();
          switch (insType) {
            case 1:
              ins->type = DIV_INS_NES;
              break;
            case 2:
              ins->type = DIV_INS_VRC6;
              break;
            case 3: // VRC7
              ins->type = DIV_INS_OPLL;
              break;
            case 4:
              ins->type = DIV_INS_FDS;
              break;
            case 5:
              ins->type = DIV_INS_N163;
              break;
            case 6: // 5B
              ins->type = DIV_INS_AY;
              break;
            case 7: // 6581 SID
              ins->type = DIV_INS_C64;
              break;
            default: {
              logE("%d: invalid instrument type %d", insIndex, insType);
              lastError = "invalid instrument type";
              delete[] file;
              return false;
            }
          }

          // instrument data
          switch (ins->type) {
            case DIV_INS_NES: {
              unsigned int totalSeqs = reader.readI();
              if (totalSeqs > 5) {
                logE("%d: too many sequences!", insIndex);
                lastError = "too many sequences";
                delete[] file;
                return false;
              }

              for (unsigned int j = 0; j < totalSeqs; j++) {
                hasSequence[insIndex][j] = reader.readC();
                sequenceIndex[insIndex][j] = reader.readC();
              }

              int dpcmNotes = (blockVersion >= 2) ? 96 : 72;

              if (blockVersion >= 7) {
                unsigned int notes = reader.readI();
                dpcmNotes = notes;
              }

              // should dpcmNotes be 96 always?
              for (int j = 0; j < dpcmNotes; j++) {
                int note = j;
                if (blockVersion >= 7) {
                  note = reader.readC();
                }
                if (note<0 || note>=120) {
                  logE("DPCM note %d out of range!",note);
                  lastError = "DPCM note out of range";
                  delete[] file;
                  return false;
                }
                ins->amiga.noteMap[note].map = (short)((unsigned char)reader.readC()) - 1;
                unsigned char freq = reader.readC();
                ins->amiga.noteMap[note].dpcmFreq = (freq & 15);       // 0-15 = 0-15 unlooped, 128-143 = 0-15 looped
                ins->amiga.noteMap[note].freq = (freq & 0x80) ? 1 : 0; // loop
                if (blockVersion >= 6) {
                  ins->amiga.noteMap[note].dpcmDelta = (unsigned char)reader.readC(); // DMC value
                }
              }

              ins->amiga.useSample = true;
              ins->amiga.useNoteMap = true;

              bool empty_note_map = true;

              for (int j = 0; j < dpcmNotes; j++) {
                if (ins->amiga.noteMap[j].map != -1) {
                  empty_note_map = false;
                }
              }

              if (empty_note_map) {
                ins->amiga.useSample = false;
                ins->amiga.useNoteMap = false;
                ins->amiga.initSample = -1;
              }
              break;
            }
            case DIV_INS_VRC6: {
              unsigned int totalSeqs = reader.readI();
              if (totalSeqs > 5) {
                logE("%d: too many sequences!", insIndex);
                lastError = "too many sequences";
                delete[] file;
                return false;
              }

              for (unsigned int j = 0; j < totalSeqs; j++) {
                hasSequence[insIndex][j] = reader.readC();
                sequenceIndex[insIndex][j] = reader.readC();
              }
              break;
            }
            case DIV_INS_OPLL: {
              ins->fm.opllPreset = (unsigned int)reader.readI();
              ins->fm.opllPreset&=15;

              unsigned char custom_patch[8];

              for (int i = 0; i < 8; i++) {
                custom_patch[i] = reader.readC();
              }

              for (int i = 0; i < 2; i++) {
                ins->fm.op[i].am = custom_patch[i] >> 7;
                ins->fm.op[i].vib = (custom_patch[i] >> 6) & 1;
                ins->fm.op[i].ssgEnv = ((custom_patch[i] >> 5) & 1) << 3;
                ins->fm.op[i].ksr = (custom_patch[i] >> 4) & 1;

                ins->fm.op[i].mult = custom_patch[i] & 15;

                ins->fm.op[i].ksl = custom_patch[i + 2] >> 6;

                ins->fm.op[i].ar = custom_patch[i + 4] >> 4;
                ins->fm.op[i].dr = custom_patch[i + 4] & 15;

                ins->fm.op[i].sl = custom_patch[i + 6] >> 4;
                ins->fm.op[i].rr = custom_patch[i + 6] & 15;
              }

              ins->fm.fms = (custom_patch[3] >> 4) & 1;
              ins->fm.ams = (custom_patch[3] >> 3) & 1;
              ins->fm.fb = custom_patch[3] & 7;

              ins->fm.op[0].tl = custom_patch[2] & 0x3f;
              ins->fm.op[1].tl = 0;
              break;
            }
            case DIV_INS_FDS: {
              DivWavetable* wave = new DivWavetable;
              wave->len = 64;
              wave->max = 63;
              for (int j = 0; j < 64; j++) {
                wave->data[j] = reader.readC();
              }
              for (int j = 0; j < 32; j++) {
                ins->fds.modTable[j] = reader.readC() - 3;
              }
              ins->fds.modSpeed = reader.readI();
              ins->fds.modDepth = reader.readI();
              reader.readI(); // this is delay. currently ignored. TODO.
              if (ds.wave.size()>=32768) {
                logW("too many waves! ignoring...");
                delete wave;
              } else {
                ins->std.waveMacro.len = 1;
                ins->std.waveMacro.val[0] = ds.wave.size();
                ds.wave.push_back(wave);
                ds.waveLen++;
              }

              unsigned int a = reader.readI();
              unsigned int b = reader.readI();

              if (!reader.seek(-8, SEEK_CUR)) {
                logE("couldn't seek back by 8 reading FDS ins");
                lastError = "couldn't seek back by 8 reading FDS ins";
                delete[] file;
                return false;
              }

              if (a < 256 && (b & 0xFF) != 0x00) {
                // don't look at me like this. I don't know why this should be like this either!
                logW("a is less than 256 and b is not zero!");
              } else {
                ins->std.volMacro.len = reader.readC();
                ins->std.volMacro.loop = reader.readI();
                ins->std.volMacro.rel = reader.readI();
                reader.readI(); // arp mode does not apply here
                for (int j = 0; j < ins->std.volMacro.len; j++) {
                  ins->std.volMacro.val[j] = reader.readC();

                  if (blockVersion <= 3) {
                    ins->std.volMacro.val[j] *= 2;
                  }
                }

                ins->std.arpMacro.len = reader.readC();
                ins->std.arpMacro.loop = reader.readI();
                ins->std.arpMacro.rel = reader.readI();
                unsigned int mode = reader.readI();
                for (int j = 0; j < ins->std.arpMacro.len; j++) {
                  ins->std.arpMacro.val[j] = reader.readC();

                  if (mode == 1) // fixed arp
                  {
                    ins->std.arpMacro.val[j] |= (1 << 30);
                  }
                }

                if (blockVersion >= 3) {
                  ins->std.pitchMacro.len = reader.readC();
                  ins->std.pitchMacro.loop = reader.readI();
                  ins->std.pitchMacro.rel = reader.readI();
                  reader.readI(); // arp mode does not apply here
                  for (int j = 0; j < ins->std.pitchMacro.len; j++) {
                    ins->std.pitchMacro.val[j] = reader.readC();

                    int temp_val = ins->std.pitchMacro.val[j];
                    int temp = temp_val;

                    if (temp_val < 0x80) {
                      ins->std.pitchMacro.val[j] = temp;
                    } else {
                      ins->std.pitchMacro.val[j] = -1 * (0x100 - temp);
                    }
                  }

                  if (mode == 0) // relative
                  {
                    ins->std.pitchMacro.mode = 1; // setting relative mode
                  }
                }
              }

              break;
            }
            case DIV_INS_N163: {
              unsigned int totalSeqs = reader.readI();
              if (totalSeqs > 5) {
                logE("%d: too many sequences!", insIndex);
                lastError = "too many sequences";
                delete[] file;
                return false;
              }

              for (unsigned int j = 0; j < totalSeqs; j++) {
                hasSequence[insIndex][j] = reader.readC();
                sequenceIndex[insIndex][j] = reader.readC();
              }

              unsigned int wave_size = reader.readI();
              unsigned int wave_pos = reader.readI();
              ins->n163.waveLen = wave_size;
              ins->n163.wavePos = wave_pos;

              if (blockVersion >= 8) {
                unsigned int autopos = reader.readI();
                logV("autopos: %d",autopos);
              }

              unsigned int wave_count = reader.readI();
              n163WaveOff[insIndex] = ds.wave.size();
              ins->n163.wave = n163WaveOff[insIndex];

              if (wave_size>256) {
                logE("wave size %d out of range",wave_size);
                lastError = "wave size out of range";
                delete[] file;
                return false;
              }

              for (unsigned int ii = 0; ii < wave_count; ii++) {
                DivWavetable* wave = new DivWavetable();
                wave->len = wave_size;
                wave->max = 15;

                for (unsigned int jj = 0; jj < wave_size; jj++) {
                  unsigned char val = reader.readC();
                  wave->data[jj] = val;
                }

                if (ds.wave.size()<32768) {
                  ds.wave.push_back(wave);
                } else {
                  logW("too many waves...");
                  delete wave;
                }
              }

              break;
            }

            case DIV_INS_AY: {
              unsigned int totalSeqs = reader.readI();
              if (totalSeqs > 5) {
                logE("%d: too many sequences!", insIndex);
                lastError ="too many sequences";
                delete[] file;
                return false;
              }

              for (unsigned int j = 0; j < totalSeqs; j++) {
                hasSequence[insIndex][j] = reader.readC();
                sequenceIndex[insIndex][j] = reader.readC();
              }

              break;
            }

            case DIV_INS_C64: {
              unsigned int instVersion = reader.readI();
              logV("C64 inst version %d", instVersion);

              int seek_amount = 94 - 4;

              if (instVersion <= 255) {
                unsigned char ad = reader.readC();
                unsigned char sr = reader.readC();
                ins->c64.a = ad >> 4;
                ins->c64.d = ad & 15;
                ins->c64.s = sr >> 4;
                ins->c64.r = sr & 15;

                ins->c64.pulseOn = true;
                ins->c64.sawOn = false;

                seek_amount -= 2;

                if (instVersion >= 2) {
                  unsigned int pwm_start = reader.readI();
                  unsigned int pwm_end = reader.readI();

                  unsigned char pwm_speed = reader.readC(); // for LFO mode * 40 / 69 * (diff / (2048 - 311)) (and divide by two for triangle mode)
                  unsigned char pwm_mode = reader.readC();  // for ADSR mode * 11 / 69

                  if (pwm_mode != 0) {
                    ins->c64.dutyIsAbs = true;

                    logV("pwm mode %d", pwm_mode);

                    // modes: 1 = saw lfo, 2 = tri lfo, 3 = oneshot ADSR, 4 = set val?

                    ins->std.dutyMacro.len = 18;

                    if (pwm_mode != 4) {
                      ins->std.dutyMacro.val[0] = pwm_start;
                      ins->std.dutyMacro.val[1] = pwm_end;
                    } else {
                      ins->std.dutyMacro.len = 1;
                      ins->std.dutyMacro.val[0] = pwm_end; // sequence mode, set last value
                    }

                    if (pwm_mode == 1 || pwm_mode == 2) {
                      ins->std.dutyMacro.open = 4 | 1; // LFO
                      ins->std.dutyMacro.mode = 2;     // LFO

                      ins->std.dutyMacro.val[11] = (int)((uint64_t)pwm_speed * (uint64_t)40 * (uint64_t)abs((int)(pwm_start - pwm_end)) / (2048 - 311) / 69); // LFO speed
                      ins->std.dutyMacro.val[12] = 1;                                                                                                         // sawtooth LFO wave

                      if (pwm_mode == 2) {
                        ins->std.dutyMacro.val[11] = (int)((uint64_t)pwm_speed * (uint64_t)40 * (uint64_t)abs((int)(pwm_start - pwm_end)) / (2048 - 311) / 69 / 2); // LFO speed multiplied by 2
                        ins->std.dutyMacro.val[12] = 0;                                                                                                             // triangle LFO wave
                      }
                    }

                    if (pwm_mode == 3) {
                      ins->std.dutyMacro.open = 2 | 1;                                                                                                       // ADSR
                      ins->std.dutyMacro.mode = 1;                                                                                                           // LFO
                      ins->std.dutyMacro.val[2] = (int)((uint64_t)pwm_speed * (uint64_t)11 * (uint64_t)abs((int)(pwm_start - pwm_end)) / (2048 - 311) / 69); // ADSR attack rate
                    }
                  }

                  seek_amount -= 10;
                }

                if (instVersion >= 3) {
                  unsigned int filter_start = reader.readI();
                  unsigned int filter_end = reader.readI();

                  unsigned char filter_speed = reader.readC();
                  unsigned char filter_mode = reader.readC();

                  if (filter_mode == 0) {
                    seek_amount -= 10;
                  } else {
                    ins->c64.filterIsAbs = true;

                    // modes: 1 = saw lfo, 2 = tri lfo, 3 = oneshot ADSR, 4 = set val?

                    ins->std.algMacro.len = 18;

                    if (filter_mode != 4) {
                      ins->std.algMacro.val[0] = filter_start;
                      ins->std.algMacro.val[1] = filter_end;
                    } else {
                      ins->std.algMacro.len = 1;
                      ins->std.algMacro.val[0] = filter_end; // sequence mode, set last value
                    }

                    if (filter_mode == 1 || filter_mode == 2) {
                      ins->std.algMacro.open = 4 | 1; // LFO
                      ins->std.algMacro.mode = 2;     // LFO

                      ins->std.algMacro.val[11] = (int)((uint64_t)filter_speed * (uint64_t)40 * (uint64_t)abs((int)(filter_start - filter_end)) / (2048 - 311) / 69); // LFO speed
                      ins->std.algMacro.val[12] = 1;                                                                                                                  // sawtooth LFO wave

                      if (filter_mode == 2) {
                        ins->std.algMacro.val[11] = (int)((uint64_t)filter_speed * (uint64_t)40 * (uint64_t)abs((int)(filter_start - filter_end)) / (2048 - 311) / 69 / 2); // LFO speed multiplied by 2
                        ins->std.algMacro.val[12] = 0;                                                                                                                      // triangle LFO wave
                      }
                    }

                    if (filter_mode == 3) {
                      ins->std.algMacro.open = 2 | 1;                                                                                                                // ADSR
                      ins->std.algMacro.mode = 1;                                                                                                                    // LFO
                      ins->std.algMacro.val[2] = (int)((uint64_t)filter_speed * (uint64_t)11 * (uint64_t)abs((int)(filter_start - filter_end)) / (2048 - 311) / 69); // ADSR attack rate
                    }
                  }
                }

                if (instVersion >= 4) {
                  unsigned int totalSeqs = reader.readI();
                  seek_amount -= 4;
                  if (totalSeqs > 5) {
                    logE("%d: too many sequences!", insIndex);
                    lastError = "too many sequences";
                    delete[] file;
                    return false;
                  }

                  for (unsigned int j = 0; j < totalSeqs; j++) {
                    hasSequence[insIndex][j] = reader.readC();
                    sequenceIndex[insIndex][j] = reader.readC();

                    seek_amount -= 2;
                  }
                }

                if (instVersion == 2) {
                  // I know right?
                  if (!reader.seek(seek_amount, SEEK_CUR)) {
                    logE("EFT seek fail");
                    lastError = "EFT seek fail";
                    delete[] file;
                    return false;
                  }
                }

                // this commented out block left here intentionally.
                // total mess of code style... for with no space, UNDEFINED CHAR, escaping the unescapable, silly var names...
                // ...whatever.
                /*for(int tti = 0; tti < 20; tti++)
                {
                  char aaaa = reader.readC();
                  logV("\'%c\'", aaaa);
                }*/
              } else {
                if (!reader.seek(-4, SEEK_CUR)) {
                  logE("EFT -4 seek fail");
                  lastError = "EFT -4 seek fail";
                  delete[] file;
                  return false;
                }
              }

              break;
            }

            default: {
              logE("%d: what's going on here?", insIndex);
              lastError = "invalid instrument type";
              delete[] file;
              return false;
            }
          }

          // name
          ins->name = reader.readString((unsigned int)reader.readI());
          logV("- %d: %s", insIndex, ins->name);
        }

        ds.insLen = 128;
      } else if (blockName == "SEQUENCES") {
        CHECK_BLOCK_VERSION(6);

        if (blockVersion < 2) {
          lastError = "sequences block version is too old";
          delete[] file;
          return false;
        }

        unsigned int seq_count = reader.readI();

        if (blockVersion == 2) {
          for (unsigned int i = 0; i < seq_count; i++) {
            unsigned int index = reader.readI();
            unsigned int type = reader.readI();
            unsigned char size = reader.readC();

            if (index>=256 || type>=8) {
              logE("%d: index/type out of range",i);
              lastError = "sequence index/type out of range";
              delete[] file;
              return false;
            }

            macros[index][type].len = size;

            for (int j = 0; j < size; j++) {
              unsigned char seq = reader.readC();
              reader.readC(); // reserved?
              macros[index][type].val[j] = seq;
            }

            for (int k = 0; k < (int)ds.ins.size(); k++) {
              DivInstrument* ins=ds.ins[k];
              if (sequenceIndex[k][type] == index && ins->type == DIV_INS_NES && hasSequence[k][type]) {
                copyMacro(ins, &macros[index][type], type, 0);
              }
            }
          }
        } else {
          unsigned char* Indices = new unsigned char[128 * 5];
          unsigned char* Types = new unsigned char[128 * 5];

          memset(Indices,0,128*5);
          memset(Types,0,128*5);

          for (unsigned int i = 0; i < seq_count; i++) {
            unsigned int index = reader.readI();
            if (index>=128*5) {
              logE("%d: index out of range",i);
              lastError = "sequence index out of range";
              delete[] file;
              return false;
            }
            Indices[i] = index;
            unsigned int type = reader.readI();
            if (type>=128*5) {
              logE("%d: type out of range",i);
              lastError = "sequence type out of range";
              delete[] file;
              return false;
            }
            Types[i] = type;

            if (index>=256 || type>=8) {
              logE("%d: index/type out of range",i);
              lastError = "sequence index/type out of range";
              delete[] file;
              return false;
            }

            unsigned char size = reader.readC();
            unsigned int setting = 0;

            macros[index][type].len = size;

            unsigned int loop = reader.readI();

            macros[index][type].loop = loop;

            if (blockVersion == 4) {
              unsigned int release = reader.readI();
              setting = reader.readI();

              macros[index][type].rel = release;
              macro_types[index][type] = setting;
            }

            for (int j = 0; j < size; j++) {
              unsigned char seq = reader.readC();
              macros[index][type].val[j] = seq;
            }

            for (int k = 0; k < (int)ds.ins.size(); k++) {
              DivInstrument* ins = ds.ins[k];
              if (sequenceIndex[k][Types[i]] == Indices[i] && ins->type == DIV_INS_NES && hasSequence[k][Types[i]]) {
                copyMacro(ins, &macros[index][type], Types[i], setting);
              }
            }
          }

          if (blockVersion == 5) // Version 5 saved the release points incorrectly, this is fixed in ver 6
          {
            for (int i = 0; i < 128; i++) {
              for (int j = 0; j < 5; j++) {
                unsigned int release = reader.readI();
                unsigned int setting = reader.readI();

                for (int k = 0; k < (int)ds.ins.size(); k++) {
                  DivInstrument* ins = ds.ins[k];
                  if (sequenceIndex[k][j] == i && ins->type == DIV_INS_NES && hasSequence[k][j]) {
                    macros[k][j].rel = release;
                    macro_types[k][j] = setting;

                    copyMacro(ins, &macros[sequenceIndex[k][j]][j], j, setting);
                  }
                }
              }
            }
          }

          if (blockVersion >= 6) // Read release points correctly stored
          {
            for (unsigned int i = 0; i < seq_count; i++) {
              unsigned int release = reader.readI();
              unsigned int setting = reader.readI();

              for (int k = 0; k < (int)ds.ins.size(); k++) {
                DivInstrument* ins = ds.ins[k];
                if (sequenceIndex[k][Types[i]] == Indices[i] && ins->type == DIV_INS_NES && hasSequence[k][Types[i]]) {
                  macros[sequenceIndex[k][Types[i]]][Types[i]].rel = release;
                  macro_types[k][Types[i]] = setting;

                  copyMacro(ins, &macros[sequenceIndex[k][Types[i]]][Types[i]], Types[i], setting);
                }
              }
            }
          }

          delete[] Indices;
          delete[] Types;
        }
      } else if (blockName == "GROOVES") {
        CHECK_BLOCK_VERSION(6);

        unsigned char num_grooves = reader.readC();
        int max_groove = 0;

        for (int i = 0; i < 256; i++) {
          ds.grooves.push_back(DivGroovePattern());
        }

        for (int gr = 0; gr < num_grooves; gr++) {
          unsigned char index = reader.readC();
          unsigned char size = reader.readC();

          if (index > max_groove) {
            max_groove = index + 1;
          }

          DivGroovePattern gp;
          gp.len = size;

          for (int sz = 0; sz < size; sz++) {
            unsigned char value = reader.readC();
            if (sz<16) {
              gp.val[sz] = value;
            }
          }

          ds.grooves[index] = gp;
        }

        ds.grooves.resize(max_groove == 0 ? 1 : max_groove);

        unsigned char subsongs = reader.readC();

        for (int sub = 0; sub < subsongs; sub++) {
          unsigned char used = reader.readC();

          if (used) {
            ds.subsong[sub]->speeds = ds.grooves[0];
          }
        }

        if ((reader.tell() - blockStart) != blockSize) {
          logE("block %s size does not match! block size %d curr pos %d", blockName, blockSize, reader.tell() - blockStart);
        }
      } else if (blockName == "FRAMES") {
        CHECK_BLOCK_VERSION(3);

        for (size_t i = 0; i < ds.subsong.size(); i++) {
          DivSubSong* s = ds.subsong[i];

          int framesLen=reader.readI();
          if (framesLen<1 || framesLen>256) {
            logE("frames out of range (%d)",framesLen);
            lastError = "frames out of range";
            delete[] file;
            return false;
          }

          s->ordersLen = framesLen;
          if (blockVersion >= 3) {
            s->speeds.val[0] = reader.readI();
          }
          if (blockVersion >= 2) {
            int tempo = reader.readI();

            logV("tempo %d", tempo);

            if (tempo == 0) {
              s->virtualTempoN = s->virtualTempoD;
            } else {
              s->virtualTempoN = tempo;
            }

            int patLen=reader.readI();
            if (patLen<1 || patLen>256) {
              logE("pattern length out of range");
              lastError = "pattern length out of range";
              delete[] file;
              return false;
            }
            s->patLen = patLen;
          }
          int why = tchans;
          if (blockVersion == 1) {
            why = reader.readI();
            if (why<0 || why>=DIV_MAX_CHANS) {
              logE("why out of range!");
              lastError = "why out of range";
              delete[] file;
              return false;
            }
          }
          logV("reading %d and %d orders", tchans, s->ordersLen);

          for (int j = 0; j < s->ordersLen; j++) {
            for (int k = 0; k < why; k++) {
              unsigned char o = reader.readC();
              if (map_channels[k]>=DIV_MAX_CHANS) continue;
              s->orders.ord[map_channels[k]][j] = o;
            }
          }
        }
      } else if (blockName == "PATTERNS") {
        CHECK_BLOCK_VERSION(6);

        size_t blockEnd = reader.tell() + blockSize;

        if (blockVersion == 1) {
          int patLenOld = reader.readI();
          if (patLenOld<1 || patLenOld>=256) {
            logE("old pattern length out of range");
            lastError = "old pattern length out of range";
            delete[] file;
            return false;
          }
          for (DivSubSong* i : ds.subsong) {
            i->patLen = patLenOld;
          }
        }

        // so it appears .ftm doesn't keep track of how many patterns are stored in the file....
        while (reader.tell() < blockEnd) {
          logV("reading pattern at %x...",reader.tell());
          int subs = 0;
          if (blockVersion >= 2)
            subs = reader.readI();
          int ch = reader.readI();
          int patNum = reader.readI();
          int numRows = reader.readI();

          logV("ch: %d",ch);
          logV("subs: %d. map_channels[ch]: %d",subs,map_channels[ch]);
          logV("patNum: %d",patNum);
          logV("rows: %d",numRows);

          if (subs<0 || subs>=(int)ds.subsong.size()) {
            logE("subsong out of range!");
            lastError = "subsong out of range";
            delete[] file;
            return false;
          }
          if (ch<0 || ch>=DIV_MAX_CHANS) {
            logE("channel out of range!");
            lastError = "channel out of range";
            delete[] file;
            return false;
          }
          if (map_channels[ch]>=DIV_MAX_CHANS) {
            logE("mapped channel out of range!");
            lastError = "mapped channel out of range";
            delete[] file;
            return false;
          }
          if (patNum<0 || patNum>=256) {
            logE("pattern number out of range!");
            lastError = "pattern number out of range";
            delete[] file;
            return false;
          }
          if (numRows<0) {
            logE("row count is negative!");
            lastError = "row count is negative";
            delete[] file;
            return false;
          }

          DivPattern* pat = ds.subsong[subs]->pat[map_channels[ch]].getPattern(patNum, true);
          for (int i = 0; i < numRows; i++) {
            unsigned int row = 0;
            if (ds.version==0x200 || blockVersion >= 6) { // row index
              row = (unsigned char)reader.readC();
            } else {
              row = reader.readI();
            }

            if (row>=256) {
              logE("row index out of range");
              lastError = "row index out of range";
              delete[] file;
              return false;
            }

            unsigned char nextNote = reader.readC();
            unsigned char nextOctave = reader.readC();

            if (blockVersion < 5 && map_channels[ch] == fds_chan) // FDS transpose
            {
              nextOctave += 2;
              nextOctave -= 2;
            }
            if (blockVersion >= 5 && map_channels[ch] == fds_chan) // FDS transpose
            {
              nextOctave -= 2;
            }

            if (map_channels[ch] != 0xff) {
              if (nextNote == 0x0d) {
                pat->newData[row][DIV_PAT_NOTE] = DIV_NOTE_REL;
              } else if (nextNote == 0x0e) {
                pat->newData[row][DIV_PAT_NOTE] = DIV_NOTE_OFF;
              } else if (nextNote == 0) {
                pat->newData[row][DIV_PAT_NOTE] = -1;
              } else if (nextNote < 0x0d) {
                pat->newData[row][DIV_PAT_NOTE] = nextOctave*12 + (nextNote - 1) + 60;
              }
            }

            unsigned char nextIns = reader.readC();
            // TODO: you sure about 0xff?
            if (map_channels[ch] != 0xff) {
              if (nextIns < 0x40 && nextNote != 0x0d && nextNote != 0x0e) {
                pat->newData[row][DIV_PAT_INS] = nextIns;
              } else {
                pat->newData[row][DIV_PAT_INS] = -1;
              }
            }

            unsigned char nextVol = reader.readC();
            if (map_channels[ch] != 0xff) {
              if (nextVol < 0x10) {
                pat->newData[row][DIV_PAT_VOL] = nextVol;
                if (map_channels[ch] == vrc6_saw_chan) // scale volume
                {
                  // TODO: shouldn't it be 32?
                  pat->newData[row][DIV_PAT_VOL] = (pat->newData[row][DIV_PAT_VOL] * 42) / 15;
                }

                if (map_channels[ch] == fds_chan) {
                  pat->newData[row][DIV_PAT_VOL] = (pat->newData[row][DIV_PAT_VOL] * 31) / 15;
                }
              } else {
                pat->newData[row][DIV_PAT_VOL] = -1;
              }
            }

            int effectCols = ds.subsong[subs]->pat[map_channels[ch]].effectCols;
            if (blockVersion >= 6)
              effectCols = 4;

            if (ds.version == 0x200) {
              effectCols = 1;
            }

            unsigned char nextEffectVal = 0;
            unsigned char nextEffect = 0;

            for (int j = 0; j < effectCols; j++) {
              nextEffect = reader.readC();

              if (nextEffect>0 && ((nextEffect < FT_EF_COUNT && !eft) || (nextEffect < EFT_EF_COUNT && eft))) {
                nextEffectVal = reader.readC();

                if (blockVersion < 3) {
                  if (nextEffect == FT_EF_PORTAOFF) {
                    nextEffect = FT_EF_PORTAMENTO;
                    nextEffectVal = 0;
                  } else if (nextEffect == FT_EF_PORTAMENTO) {
                    if (nextEffect < 0xFF)
                      nextEffectVal++;
                  }
                }
              } else if (blockVersion < 6) {
                nextEffectVal = reader.readC();
              }

              // Specific for version 2.0
              if (ds.version == 0x0200 && j == 0) {
                if (nextEffect == FT_EF_SPEED && nextEffectVal < 20)
                  nextEffectVal++;

                if (pat->newData[row][DIV_PAT_VOL] == 0)
                  pat->newData[row][DIV_PAT_VOL] = 0xf;
                else {
                  pat->newData[row][DIV_PAT_VOL]--;
                  pat->newData[row][DIV_PAT_VOL] &= 0x0F;
                }

                if (pat->newData[row][DIV_PAT_NOTE] == -1)
                  pat->newData[row][DIV_PAT_INS] = -1;
              }

              if (blockVersion == 3) {
                // Fix for VRC7 portamento
                bool is_vrc7 = false;

                for (int vrr = 0; vrr < 6; vrr++) {
                  if (map_channels[ch] == vrc7_chans[vrr]) {
                    is_vrc7 = true;
                  }
                }

                if (is_vrc7) {
                  switch (nextEffect) {
                    case FT_EF_PORTA_DOWN:
                      nextEffect = FT_EF_PORTA_UP;
                      break;
                    case FT_EF_PORTA_UP:
                      nextEffect = FT_EF_PORTA_DOWN;
                      break;
                    default:
                      break;
                  }
                }
                // FDS pitch effect fix
                else if (map_channels[ch] == fds_chan) {
                  switch (nextEffect) {
                    case FT_EF_PITCH:
                      if (nextEffectVal != 0x80)
                        nextEffectVal = (0x100 - nextEffectVal) & 0xFF;
                      break;
                    default:
                      break;
                  }
                }
              }

              for (int v = 0; v < 8; v++) {
                if (map_channels[ch] == n163_chans[v]) {
                  if (nextEffect == FT_EF_SAMPLE_OFFSET) {
                    nextEffect = FT_EF_N163_WAVE_BUFFER;
                  }
                }
              }

              if (ds.version < 0x0450 || dnft) {
                unsigned char idx = 0;

                while (eff_conversion_050[idx][0] != 0xFF) // until the end of the array
                {
                  if (nextEffect == eff_conversion_050[idx][0]) // remap the effects (0CC vs FT effects type order) bruh idk why but it should be done to correctly read some modules
                  {
                    nextEffect = eff_conversion_050[idx][1];

                    break;
                  }

                  idx++;
                }

              }

              if (map_channels[ch] != 0xff) {
                if (nextEffect == 0 && nextEffectVal == 0) {
                  pat->newData[row][DIV_PAT_FX(j)] = -1;
                  pat->newData[row][DIV_PAT_FXVAL(j)] = -1;
                } else {
                  if ((eft && nextEffect<eftEffectMapSize) || (!eft && nextEffect<ftEffectMapSize)) {
                    if (eft) {
                      pat->newData[row][DIV_PAT_FX(j)] = eftEffectMap[nextEffect];
                      pat->newData[row][DIV_PAT_FXVAL(j)] = eftEffectMap[nextEffect] == -1 ? -1 : nextEffectVal;

                      if (pat->newData[row][DIV_PAT_FX(j)] == 0x100) {
                        pat->newData[row][DIV_PAT_VOL] += pat->newData[row][DIV_PAT_FXVAL(j)] ? 0x10 : 0; // extra volume bit for AY8930
                        pat->newData[row][DIV_PAT_FX(j)] = -1;
                        pat->newData[row][DIV_PAT_FXVAL(j)] = -1;
                      }

                      if (eftEffectMap[nextEffect] == 0x0f && nextEffectVal > 0x1f) {
                        pat->newData[row][DIV_PAT_FX(j)] = 0xfd; // BPM speed change!
                      }

                      if ((eftEffectMap[nextEffect] == 0xe1 || eftEffectMap[nextEffect] == 0xe2) && (nextEffectVal & 0xf0) == 0) {
                        pat->newData[row][DIV_PAT_FXVAL(j)] |= 0x10; // in FamiTracker if e1/e2 commands speed is 0 the portamento still has some speed!
                      }
                    } else {
                      pat->newData[row][DIV_PAT_FX(j)] = ftEffectMap[nextEffect];
                      pat->newData[row][DIV_PAT_FXVAL(j)] = ftEffectMap[nextEffect] == -1 ? -1 : nextEffectVal;

                      if (ftEffectMap[nextEffect] == 0x0f && nextEffectVal > 0x1f) {
                        pat->newData[row][DIV_PAT_FX(j)] = 0xfd; // BPM speed change!
                      }

                      if ((ftEffectMap[nextEffect] == 0xe1 || ftEffectMap[nextEffect] == 0xe2) && (nextEffectVal & 0xf0) == 0) {
                        pat->newData[row][DIV_PAT_FXVAL(j)] |= 0x10; // in FamiTracker if e1/e2 commands speed is 0 the portamento still has some speed!
                      }
                    }
                    for (int v = 0; v < 8; v++) {
                      if (map_channels[ch] == n163_chans[v]) {
                        if (pat->newData[row][DIV_PAT_FX(j)] == 0x12) {
                          pat->newData[row][DIV_PAT_FX(j)] = 0x110; // N163 wave change (we'll map this later)
                        } else if (pat->newData[row][DIV_PAT_FX(j)] == 0x11) {
                          // wave position:
                          // - in FamiTracker this is in bytes
                          // - a value of 7F has special meaning
                          if (pat->newData[row][DIV_PAT_FXVAL(j)]==0x7f) {
                            pat->newData[row][DIV_PAT_FX(j)]=-1;
                            pat->newData[row][DIV_PAT_FXVAL(j)]=-1;
                          } else {
                            pat->newData[row][DIV_PAT_FXVAL(j)]=MIN(pat->newData[row][DIV_PAT_FXVAL(j)]<<1,0xff);
                          }
                        }
                      }
                    }

                    for (int vrr = 0; vrr < 6; vrr++)
                    {
                      if (map_channels[ch] == vrc7_chans[vrr])
                      {
                        if (pat->newData[row][DIV_PAT_FX(j)] == 0x12)
                        {
                          pat->newData[row][DIV_PAT_FX(j)] = 0x10; // set VRC7 patch
                        }
                      }
                    }

                    for (int v = 0; v < 3; v++) {
                      if (map_channels[ch] == s5b_chans[v] || map_channels[ch] == ay8930_chans[v]) {
                        if (pat->newData[row][DIV_PAT_FX(j)] == 0x22 && (pat->newData[row][DIV_PAT_FXVAL(j)] & 0xf0) != 0) {
                          // TODO: in the second stage of pattern refactor this will have to change.
                          pat->newData[row][DIV_PAT_FX(7)] = -666; //marker
                        }
                      }
                    }
                  } else {
                    pat->newData[row][DIV_PAT_FX(j)] = -1;
                    pat->newData[row][DIV_PAT_FXVAL(j)] = -1;
                  }
                }
              }
            }
          }
        }
      } else if (blockName == "DPCM SAMPLES") {
        CHECK_BLOCK_VERSION(1);
        unsigned char num_samples = reader.readC();

        for (int i = 0; i < 256; i++) {
          DivSample* s = new DivSample();
          ds.sample.push_back(s);
        }

        ds.sampleLen = ds.sample.size();

        unsigned int true_size = 0;
        unsigned char index = 0;

        for (unsigned char i = 0; i < num_samples; i++) {
          index = reader.readC();

          DivSample* sample = ds.sample[index];

          sample->centerRate = 33144;
          sample->depth = DIV_SAMPLE_DEPTH_1BIT_DPCM;

          sample->name = reader.readString((unsigned int)reader.readI());

          unsigned int sample_len = reader.readI();

          if (sample_len>=2097152) {
            logE("%d: sample too large! %d",index,sample_len);
            lastError = "sample too large";
            delete[] file;
            return false;
          }

          true_size = sample_len + ((1 - (int)sample_len) & 0x0f);
          sample->init(true_size * 8);
          memset(sample->dataDPCM, 0xAA, true_size);
          reader.read(sample->dataDPCM, sample_len);
        }

        int last_non_empty_sample = 0xff;

        for (int i = 255; i > 0; i--) {
          DivSample* s = ds.sample[i];

          if (s->samples>0) {
            last_non_empty_sample = i;
            break;
          }
        }

        for (int i = 255; i > last_non_empty_sample; i--) {
          ds.sample.erase(ds.sample.begin() + i);
        }

        ds.sampleLen = ds.sample.size();
      } else if (blockName == "SEQUENCES_VRC6") {
        CHECK_BLOCK_VERSION(6);

        unsigned char* Indices = new unsigned char[128 * 5];
        unsigned char* Types = new unsigned char[128 * 5];

        memset(Indices,0,128*5);
        memset(Types,0,128*5);

        unsigned int seq_count = reader.readI();

        for (unsigned int i = 0; i < seq_count; i++) {
          unsigned int index = reader.readI();
          if (index>=128*5) {
            logE("%d: index out of range",i);
            lastError = "sequence index out of range";
            delete[] file;
            return false;
          }
          Indices[i] = index;
          unsigned int type = reader.readI();
          if (type>=128*5) {
            logE("%d: type out of range",i);
            lastError = "sequence type out of range";
            delete[] file;
            return false;
          }
          Types[i] = type;

          if (index>=256 || type>=8) {
            logE("%d: index/type out of range",i);
            lastError = "sequence index/type out of range";
            delete[] file;
            return false;
          }

          unsigned char size = reader.readC();
          unsigned int setting = 0;

          macros[index][type].len = size;

          unsigned int loop = reader.readI();

          macros[index][type].loop = loop;

          if (blockVersion == 4) {
            unsigned int release = reader.readI();
            setting = reader.readI();

            macros[index][type].rel = release;
            macro_types[index][type] = setting;
          }

          for (int j = 0; j < size; j++) {
            unsigned char seq = reader.readC();
            macros[index][type].val[j] = seq;
          }

          for (int k = 0; k < (int)ds.ins.size(); k++) {
            DivInstrument* ins = ds.ins[k];
            if (sequenceIndex[k][Types[i]] == Indices[i] && ins->type == DIV_INS_VRC6 && hasSequence[k][Types[i]]) {
              copyMacro(ins, &macros[index][type], type, setting);

              if (type == 0 && setting == 1) {
                ins->type = DIV_INS_VRC6_SAW;
              }
            }
          }
        }

        if (blockVersion == 5) // Version 5 saved the release points incorrectly, this is fixed in ver 6
        {
          for (int i = 0; i < 128; i++) {
            for (int j = 0; j < 5; j++) {
              unsigned int release = reader.readI();
              unsigned int setting = reader.readI();

              for (int k = 0; k < (int)ds.ins.size(); k++) {
                DivInstrument* ins = ds.ins[k];
                if (sequenceIndex[k][j] == i && ins->type == DIV_INS_VRC6 && hasSequence[k][j]) {
                  macros[k][j].rel = release;
                  macro_types[k][j] = setting;

                  copyMacro(ins, &macros[sequenceIndex[k][j]][j], j, setting);

                  if (j == 0 && setting == 1) {
                    ins->type = DIV_INS_VRC6_SAW;
                  }
                }
              }
            }
          }
        }

        if (blockVersion >= 6) // Read release points correctly stored
        {
          for (unsigned int i = 0; i < seq_count; i++) {
            unsigned int release = reader.readI();
            unsigned int setting = reader.readI();

            for (int k = 0; k < (int)ds.ins.size(); k++) {
              DivInstrument* ins = ds.ins[k];
              if (sequenceIndex[k][Types[i]] == Indices[i] && ins->type == DIV_INS_VRC6 && hasSequence[k][Types[i]]) {
                macros[sequenceIndex[k][Types[i]]][Types[i]].rel = release;
                macro_types[k][Types[i]] = setting;

                copyMacro(ins, &macros[sequenceIndex[k][Types[i]]][Types[i]], Types[i], setting);

                if (Types[i] == 0 && setting == 1) {
                  ins->type = DIV_INS_VRC6_SAW;
                }
              }
            }
          }
        }

        delete[] Indices;
        delete[] Types;
      } else if (blockName == "SEQUENCES_N163" || blockName == "SEQUENCES_N106") {
        CHECK_BLOCK_VERSION(1);

        unsigned char* Indices = new unsigned char[128 * 5];
        unsigned char* Types = new unsigned char[128 * 5];

        memset(Indices,0,128*5);
        memset(Types,0,128*5);

        unsigned int seq_count = reader.readI();

        for (unsigned int i = 0; i < seq_count; i++) {
          unsigned int index = reader.readI();
          if (index>=128*5) {
            logE("%d: index out of range",i);
            lastError = "sequence index out of range";
            delete[] file;
            return false;
          }
          Indices[i] = index;
          unsigned int type = reader.readI();
          if (type>=128*5) {
            logE("%d: type out of range",i);
            lastError = "sequence type out of range";
            delete[] file;
            return false;
          }
          Types[i] = type;

          if (index>=256 || type>=8) {
            logE("%d: index/type out of range",i);
            lastError = "sequence index/type out of range";
            delete[] file;
            return false;
          }

          unsigned char size = reader.readC();
          unsigned int setting = 0;

          macros[index][type].len = size;

          unsigned int loop = reader.readI();

          macros[index][type].loop = loop;

          unsigned int release = reader.readI();
          setting = reader.readI();

          macros[index][type].rel = release;
          macro_types[index][type] = setting;

          for (int j = 0; j < size; j++) {
            unsigned char seq = reader.readC();
            macros[index][type].val[j] = seq;
          }

          for (int k = 0; k < (int)ds.ins.size(); k++) {
            DivInstrument* ins = ds.ins[k];
            if (sequenceIndex[k][Types[i]] == Indices[i] && ins->type == DIV_INS_N163 && hasSequence[k][Types[i]]) {
              copyMacro(ins, &macros[index][type], type, setting);
            }
          }
        }

        delete[] Indices;
        delete[] Types;

      } else if (blockName == "SEQUENCES_S5B") {
        CHECK_BLOCK_VERSION(1);

        unsigned char* Indices = new unsigned char[128 * 5];
        unsigned char* Types = new unsigned char[128 * 5];

        memset(Indices,0,128*5);
        memset(Types,0,128*5);

        unsigned int seq_count = reader.readI();

        for (unsigned int i = 0; i < seq_count; i++) {
          unsigned int index = reader.readI();
          if (index>=128*5) {
            logE("%d: index out of range",i);
            lastError = "sequence index out of range";
            delete[] file;
            return false;
          }
          Indices[i] = index;
          unsigned int type = reader.readI();
          if (type>=128*5) {
            logE("%d: type out of range",i);
            lastError = "sequence type out of range";
            delete[] file;
            return false;
          }
          Types[i] = type;

          if (index>=256 || type>=8) {
            logE("%d: index/type out of range",i);
            lastError = "sequence index/type out of range";
            delete[] file;
            return false;
          }

          unsigned char size = reader.readC();
          unsigned int setting = 0;

          macros[index][type].len = size;

          unsigned int loop = reader.readI();

          macros[index][type].loop = loop;

          unsigned int release = reader.readI();
          setting = reader.readI();

          macros[index][type].rel = release;
          macro_types[index][type] = setting;

          for (int j = 0; j < size; j++) {
            unsigned char seq = reader.readC();
            macros[index][type].val[j] = seq;
          }

          for (int k = 0; k < (int)ds.ins.size(); k++) {
            DivInstrument* ins = ds.ins[k];
            if (sequenceIndex[k][type] == Indices[i] && ins->type == DIV_INS_AY && hasSequence[k][type]) {
              copyMacro(ins, &macros[index][type], type, setting);
            }
          }
        }

        delete[] Indices;
        delete[] Types;
      } else if (blockName == "SEQUENCES_SID") {
        CHECK_BLOCK_VERSION(4);

        unsigned char* Indices = new unsigned char[128 * 5];
        unsigned char* Types = new unsigned char[128 * 5];

        memset(Indices,0,128*5);
        memset(Types,0,128*5);

        unsigned int seq_count = reader.readI();

        for (unsigned int i = 0; i < seq_count; i++) {
          unsigned int index = reader.readI();
          if (index>=128*5) {
            logE("%d: index out of range",i);
            lastError = "sequence index out of range";
            delete[] file;
            return false;
          }
          Indices[i] = index;
          unsigned int type = reader.readI();
          if (type>=128*5) {
            logE("%d: type out of range",i);
            lastError = "sequence type out of range";
            delete[] file;
            return false;
          }
          Types[i] = type;

          if (index>=256 || type>=8) {
            logE("%d: index/type out of range",i);
            lastError = "sequence index/type out of range";
            delete[] file;
            return false;
          }

          unsigned char size = reader.readC();
          unsigned int setting = 0;

          macros[index][type].len = size;

          unsigned int loop = reader.readI();

          macros[index][type].loop = loop;

          unsigned int release = reader.readI();
          setting = reader.readI();

          macros[index][type].rel = release;
          macro_types[index][type] = setting;

          for (int j = 0; j < size; j++) {
            unsigned char seq = reader.readC();
            macros[index][type].val[j] = seq;
          }

          for (int k = 0; k < (int)ds.ins.size(); k++) {
            DivInstrument* ins = ds.ins[k];
            if (sequenceIndex[k][type] == Indices[i] && ins->type == DIV_INS_C64 && hasSequence[k][type]) {
              copyMacro(ins, &macros[index][type], type, setting);
            }
          }
        }

        delete[] Indices;
        delete[] Types;
      } else if (blockName == "JSON") {
        CHECK_BLOCK_VERSION(1);
        logW("block JSON not supported...");
        reader.seek(blockSize, SEEK_CUR);
      } else if (blockName == "PARAMS_EMU") {
        CHECK_BLOCK_VERSION(1);
        logW("block PARAMS_EMU not supported...");
        reader.seek(blockSize, SEEK_CUR);
      } else if (blockName == "DETUNETABLES") {
        CHECK_BLOCK_VERSION(1);
        logW("block DETUNETABLES not supported...");
        reader.seek(blockSize, SEEK_CUR);
      } else if (blockName == "COMMENTS") {
        CHECK_BLOCK_VERSION(1);
        unsigned int display_comment = reader.readI();

        logV("displayComment: %d",display_comment);

        char ch = 0;

        // why not readString?
        while (true) {
          ch = reader.readC();
          if (ch==0) break;
          ds.subsong[0]->notes += ch;
        }

      } else if (blockName == "PARAMS_EXTRA") {
        CHECK_BLOCK_VERSION(3);
        unsigned int linear_pitch = reader.readI();

        ds.compatFlags.linearPitch = linear_pitch == 0 ? 0 : 1;

        if (blockVersion >= 2) {
          int fineTuneCents = reader.readC() * 100;
          fineTuneCents += reader.readC();

          ds.tuning = 440.0 * pow(2.0, (double)fineTuneCents / 1200.0);
        }
        if (blockVersion >= 3) {
          unsigned char flats = reader.readC();
          logV("flats: %d",(int)flats);
        }
      } else if (blockName == "TUNING") {
        CHECK_BLOCK_VERSION(1);
        if (blockVersion == 1) {
          int fineTuneCents = reader.readC() * 100;
          fineTuneCents += reader.readC();

          ds.tuning = 440.0 * pow(2.0, (double)fineTuneCents / 1200.0);
        }
      } else if (blockName == "BOOKMARKS") {
        CHECK_BLOCK_VERSION(1);
        logW("block BOOKMARKS not supported...");
        reader.seek(blockSize, SEEK_CUR);
      } else {
        logE("block %s is unknown!", blockName);
        lastError = "unknown block " + blockName;
        delete[] file;
        return false;
      }

      if ((reader.tell() - blockStart) != blockSize) {
        logE("block %s is incomplete! reader.tell()-blockStart %d blockSize %d", blockName, (reader.tell() - blockStart), blockSize);
        lastError = "incomplete block " + blockName;
        delete[] file;
        return false;
      }
    }

    ds.insLen = ds.ins.size();

    if (ds.insLen > 0) {
      for (int tries = 0; tries < 5; tries++) // de-duplicating instruments
      {
        for (int i = 0; i < 128; i++) {
          if (ds.ins.empty()) break;
          int index = i >= (int)ds.insLen ? ((int)ds.insLen - 1) : i;
          if (index < 0)
            index = 0;

          if (index>=(int)ds.ins.size()) continue;

          DivInstrument* ins = ds.ins[index];

          if (ins->type == DIV_INS_FM) {
            delete ds.ins[index];
            ds.ins.erase(ds.ins.begin() + index);
            ds.insLen = ds.ins.size();
            for (int ii = 0; ii < total_chans; ii++) {
              for (size_t j = 0; j < ds.subsong.size(); j++) {
                for (int k = 0; k < DIV_MAX_PATTERNS; k++) {
                  if (ds.subsong[j]->pat[ii].data[k] == NULL)
                    continue;
                  for (int l = 0; l < ds.subsong[j]->patLen; l++) {
                    if (ds.subsong[j]->pat[ii].data[k]->newData[l][DIV_PAT_INS] > index) {
                      ds.subsong[j]->pat[ii].data[k]->newData[l][DIV_PAT_INS]--;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }

    for (int ii = 0; ii < total_chans; ii++) {
      for (size_t j = 0; j < ds.subsong.size(); j++) {
        for (int k = 0; k < DIV_MAX_PATTERNS; k++) {
          if (ds.subsong[j]->pat[ii].data[k] == NULL)
            continue;
          for (int l = 0; l < ds.subsong[j]->patLen; l++) {
            if (ds.subsong[j]->pat[ii].data[k]->newData[l][DIV_PAT_FX(7)] == -666) {
              bool converted = false;
              // for()? if()? THESE ARE NOT FUNCTIONS!
              for (int hh = 0; hh < 7; hh++) { // oh and now you 1TBS up. oh man...
                if (ds.subsong[j]->pat[ii].data[k]->newData[l][DIV_PAT_FX(hh)] == 0x22 && !converted) {
                  int slot = findEmptyFx(ds.subsong[j]->pat[ii].data[k]->newData[l]);
                  if (slot != -1) {
                    // space your comments damn it!
                    // Hxy - Envelope automatic pitch

                    // Sets envelope period to the note period shifted by x and envelope type y.
                    // Approximate envelope frequency is note frequency * (2^|x - 8|) / 32.

                    int ftAutoEnv = (ds.subsong[j]->pat[ii].data[k]->newData[l][DIV_PAT_FXVAL(hh)] >> 4) & 15;
                    int autoEnvDen = 16; // ???? with 32 it's an octave lower...
                    int autoEnvNum = (1 << (abs(ftAutoEnv - 8)));

                    while (autoEnvNum >= 2 && autoEnvDen >= 2) {
                      autoEnvDen /= 2;
                      autoEnvNum /= 2;
                    }

                    if (autoEnvDen < 16 && autoEnvNum < 16) {
                      ds.subsong[j]->pat[ii].data[k]->newData[l][DIV_PAT_FX(slot)] = 0x29;
                      ds.subsong[j]->pat[ii].data[k]->newData[l][DIV_PAT_FXVAL(slot)] = (autoEnvNum << 4) | autoEnvDen;
                    }

                    ds.subsong[j]->pat[ii].data[k]->newData[l][DIV_PAT_FXVAL(hh)] = (ds.subsong[j]->pat[ii].data[k]->newData[l][DIV_PAT_FXVAL(hh)] & 0xf) << 4;

                    converted = true;
                  }
                }
              }

              ds.subsong[j]->pat[ii].data[k]->newData[l][DIV_PAT_FX(7)] = -1; //delete marker
            }
          }
        }
      }
    }

    for (size_t j = 0; j < ds.subsong.size(); j++) { // open hidden effect columns
      // what are your rules for spacing, really?
      DivSubSong* s = ds.subsong[j];
      for (int c = 0; c < total_chans; c++) {
        int num_fx = 1;

        for (int p = 0; p < s->ordersLen; p++) {
          for (int r = 0; r < s->patLen; r++) {
            DivPattern* pat = s->pat[c].getPattern(s->orders.ord[c][p], true);
            short* s_row_data = pat->newData[r];

            for (int eff = 0; eff < DIV_MAX_EFFECTS - 1; eff++) {
              if (s_row_data[DIV_PAT_FX(eff)] != -1 && eff + 1 > num_fx) {
                  num_fx = eff + 1;
              }
            }
          }
        }

        s->pat[c].effectCols = num_fx;
      }
    }

    // famitracker is not fucking strict with what instrument types can be used on any channel. This leads to e.g. 2A03 instuments being used on VRC6 channels.
    // Furnace is way more strict, so NES instrument in VRC6 channel just does not play properly. To fix this, we are creating copies of instruments, changing their type to please Furnace system.
    // I kinda did the same in klystrack import, tbh. - LTVA

    // actually, this is wrong. Furnace is very lax regarding instrument usage. you can put an OPM instrument in OPL channel and yeah, it'll sound weird but it'll work. - tildearrow

    // P.S. Duties conversion is based on what I really hear in FamiTracker when using wrong instrument type (and what I see on Audacity "oscilloscope") - LTVA

    // is that the best you could do? paste my rant from that other commit and change a couple words?
    // at least be creative next time...

    int ins_vrc6_conv[256][2];
    int ins_vrc6_saw_conv[256][2];
    int ins_nes_conv[256][2]; // vrc6 (or whatever) -> nes

    for (int i = 0; i < 256; i++) {
      ins_vrc6_conv[i][0] = -1;
      ins_vrc6_conv[i][1] = -1;

      ins_vrc6_saw_conv[i][0] = -1;
      ins_vrc6_saw_conv[i][1] = -1;

      ins_nes_conv[i][0] = -1;
      ins_nes_conv[i][1] = -1;
    }

    int init_inst_num = ds.ins.size();

    for (int i = 0; i < init_inst_num; i++) {
      for (int ii = 0; ii < total_chans; ii++) {
        for (size_t j = 0; j < ds.subsong.size(); j++) {
          for (int k = 0; k < DIV_MAX_PATTERNS; k++) {
            if (ds.subsong[j]->pat[ii].data[k] == NULL)
              continue;
            for (int l = 0; l < ds.subsong[j]->patLen; l++) {
              // 1TBS > GNU
              if (ds.subsong[j]->pat[ii].data[k]->newData[l][DIV_PAT_INS] == i) { // instrument
                DivInstrument* ins = ds.ins[i];
                bool go_to_end = false;

                if (ins->type != DIV_INS_VRC6 && (ii == vrc6_chans[0] || ii == vrc6_chans[1])) { // we encountered non-VRC6 instrument on VRC6 channel
                  DivInstrument* insnew = new DivInstrument;
                  ds.ins.push_back(insnew);

                  *ds.ins[ds.ins.size() - 1] = *ins;

                  ds.ins[ds.ins.size() - 1]->name += " [VRC6 copy]";
                  ds.ins[ds.ins.size() - 1]->amiga.useSample = false;
                  ds.ins[ds.ins.size() - 1]->amiga.useNoteMap = false;

                  ds.ins[ds.ins.size() - 1]->type = DIV_INS_VRC6;

                  if (ins->std.dutyMacro.len > 0) {
                    for (int mm = 0; mm < ins->std.dutyMacro.len; mm++) {
                      if (ds.ins[ds.ins.size() - 1]->std.dutyMacro.val[mm] < 4) {
                        int vall = ins->std.dutyMacro.val[mm];
                        ds.ins[ds.ins.size() - 1]->std.dutyMacro.val[mm] = convert_vrc6_duties[vall];
                      }
                    }
                  }

                  ins_vrc6_conv[i][0] = i;
                  ins_vrc6_conv[i][1] = ds.ins.size() - 1;

                  go_to_end = true;
                }

                if (go_to_end) {
                  goto end1;
                }
              }
            }
          }
        }
      }

    end1:;
    }

    for (int i = 0; i < init_inst_num; i++) {
      for (int ii = 0; ii < total_chans; ii++) {
        for (size_t j = 0; j < ds.subsong.size(); j++) {
          for (int k = 0; k < DIV_MAX_PATTERNS; k++) {
            if (ds.subsong[j]->pat[ii].data[k] == NULL)
              continue;
            for (int l = 0; l < ds.subsong[j]->patLen; l++) {
              if (ds.subsong[j]->pat[ii].data[k]->newData[l][DIV_PAT_INS] == i) // instrument
              {
                DivInstrument* ins = ds.ins[i];
                bool go_to_end = false;

                if (ins->type != DIV_INS_VRC6_SAW && ii == vrc6_saw_chan) // we encountered non-VRC6-saw instrument on VRC6 saw channel
                {
                  DivInstrument* insnew = new DivInstrument;
                  ds.ins.push_back(insnew);

                  *ds.ins[ds.ins.size() - 1] = *ins;

                  ds.ins[ds.ins.size() - 1]->name += " [VRC6 saw copy]";
                  ds.ins[ds.ins.size() - 1]->amiga.useSample = false;
                  ds.ins[ds.ins.size() - 1]->amiga.useNoteMap = false;

                  ds.ins[ds.ins.size() - 1]->type = DIV_INS_VRC6_SAW;

                  if (ins->std.volMacro.len > 0) {
                    for (int mm = 0; mm < ins->std.volMacro.len; mm++) {
                      if (ds.ins[ds.ins.size() - 1]->std.volMacro.val[mm] < 16) {
                        int vall = ins->std.volMacro.val[mm];
                        ds.ins[ds.ins.size() - 1]->std.volMacro.val[mm] = vall * 42 / 15;
                      }
                    }
                  }

                  ins_vrc6_saw_conv[i][0] = i;
                  ins_vrc6_saw_conv[i][1] = ds.ins.size() - 1;

                  go_to_end = true;
                }

                if (go_to_end) {
                  goto end2;
                }
              }
            }
          }
        }
      }

    end2:;
    }

    for (int i = 0; i < init_inst_num; i++) {
      for (int ii = 0; ii < total_chans; ii++) {
        for (size_t j = 0; j < ds.subsong.size(); j++) {
          for (int k = 0; k < DIV_MAX_PATTERNS; k++) {
            if (ds.subsong[j]->pat[ii].data[k] == NULL)
              continue;
            for (int l = 0; l < ds.subsong[j]->patLen; l++) {
              if (ds.subsong[j]->pat[ii].data[k]->newData[l][DIV_PAT_INS] == i) // instrument
              {
                DivInstrument* ins = ds.ins[i];
                bool go_to_end = false;

                if (ins->type != DIV_INS_NES && (ii == mmc5_chans[0] || ii == mmc5_chans[1] || ii < 5)) // we encountered VRC6 (or whatever?) instrument on NES/MMC5 channel
                {
                  DivInstrument* insnew = new DivInstrument;
                  ds.ins.push_back(insnew);

                  *ds.ins[ds.ins.size() - 1] = *ins;

                  ds.ins[ds.ins.size() - 1]->name += " [NES copy]";

                  ds.ins[ds.ins.size() - 1]->type = DIV_INS_NES;

                  if (ins->type == DIV_INS_VRC6) {
                    if (insnew->std.dutyMacro.len > 0) // convert duties for NES
                    {
                      for (int mm = 0; mm < insnew->std.dutyMacro.len; mm++) {
                        switch (insnew->std.dutyMacro.val[mm]) {
                          case 0:
                          case 1: {
                            insnew->std.dutyMacro.val[mm] = 0;
                            break;
                          }
                          case 2:
                          case 3:
                          case 4:
                          case 5: {
                            insnew->std.dutyMacro.val[mm] = 1;
                            break;
                          }
                          case 6:
                          case 7: {
                            insnew->std.dutyMacro.val[mm] = 2;
                            break;
                          }
                          default:
                            break;
                        }
                      }
                    }
                  }

                  ins_nes_conv[i][0] = i;
                  ins_nes_conv[i][1] = ds.ins.size() - 1;

                  go_to_end = true;
                }

                if (go_to_end) {
                  goto end3;
                }
              }
            }
          }
        }
      }

    end3:;
    }

    for (int i = 0; i < 256; i++) {
      if (ins_vrc6_conv[i][0] != -1 || ins_vrc6_saw_conv[i][0] != -1 || ins_nes_conv[i][0] != -1) {
        for (int ii = 0; ii < total_chans; ii++) {
          for (size_t j = 0; j < ds.subsong.size(); j++) {
            for (int k = 0; k < DIV_MAX_PATTERNS; k++) {
              if (ds.subsong[j]->pat[ii].data[k] == NULL)
                continue;
              for (int l = 0; l < ds.subsong[j]->patLen; l++) {
                if (ds.subsong[j]->pat[ii].data[k]->newData[l][DIV_PAT_INS] == ins_vrc6_conv[i][0] && (ii == vrc6_chans[0] || ii == vrc6_chans[1])) // change ins index
                {
                  ds.subsong[j]->pat[ii].data[k]->newData[l][DIV_PAT_INS] = ins_vrc6_conv[i][1];
                }

                if (ds.subsong[j]->pat[ii].data[k]->newData[l][DIV_PAT_INS] == ins_vrc6_saw_conv[i][0] && ii == vrc6_saw_chan) {
                  ds.subsong[j]->pat[ii].data[k]->newData[l][DIV_PAT_INS] = ins_vrc6_saw_conv[i][1];
                }

                if (ds.subsong[j]->pat[ii].data[k]->newData[l][DIV_PAT_INS] == ins_nes_conv[i][0] && (ii == mmc5_chans[0] || ii == mmc5_chans[1] || ii < 5)) {
                  ds.subsong[j]->pat[ii].data[k]->newData[l][DIV_PAT_INS] = ins_nes_conv[i][1];
                }
              }
            }
          }
        }
      }
    }

    // why? I thought FamiTracker was lax
    //ds.delayBehavior=0;

    ds.version=DIV_VERSION_FTM;
    ds.insLen = ds.ins.size();
    ds.sampleLen = ds.sample.size();
    ds.waveLen = ds.wave.size();

    // check whether virtual tempo is inside range
    for (DivSubSong* i: ds.subsong) {
      while (i->virtualTempoD>255) {
        i->virtualTempoD>>=1;
        i->virtualTempoN>>=1;
      }
      if (i->virtualTempoN<1) i->virtualTempoN=1;
      if (i->virtualTempoD<1) i->virtualTempoD=1;
    }

    // offset N163 wave macros (local -> global wave conversion)
    for (size_t i=0; i<ds.ins.size(); i++) {
      DivInstrument* ins=ds.ins[i];
      int waveOff=n163WaveOff[i];
      if (ins->type==DIV_INS_N163) {
        for (int j=0; j<ins->std.waveMacro.len; j++) {
          ins->std.waveMacro.val[j]+=waveOff;
        }
      }
    }

    // offset N163 wave change effects whether possible
    for (DivSubSong* i: ds.subsong) {
      for (int j=0; j<total_chans; j++) {
        int curWaveOff=0;
        for (int k=0; k<i->ordersLen; k++) {
          DivPattern* p=i->pat[j].getPattern(i->orders.ord[j][k],true);
          for (int l=0; l<i->patLen; l++) {
            // check for instrument change
            if (p->newData[l][DIV_PAT_INS]!=-1) {
              curWaveOff=n163WaveOff[p->newData[l][DIV_PAT_INS]&127];
            }

            // check effect columns for 0x110 (dummy wave change)
            for (int m=0; m<i->pat[j].effectCols; m++) {
              if (p->newData[l][DIV_PAT_FX(m)]==0x110) {
                // map wave
                p->newData[l][DIV_PAT_FX(m)]=0x10;
                if (p->newData[l][DIV_PAT_FXVAL(m)]==-1) {
                  p->newData[l][DIV_PAT_FXVAL(m)]=curWaveOff&0xff;
                } else {
                  p->newData[l][DIV_PAT_FXVAL(m)]=(p->newData[l][DIV_PAT_FXVAL(m)]+curWaveOff)&0xff;
                }
              }
            }
          }
        }
      }
    }

    ds.recalcChans();

    if (active) quitDispatch();
    BUSY_BEGIN_SOFT;
    saveLock.lock();
    song.unload();
    song=ds;
    hasLoadedSomething=true;
    changeSong(0);
    saveLock.unlock();
    BUSY_END;
    if (active) {
      initDispatch();
      BUSY_BEGIN;
      renderSamples();
      reset();
      BUSY_END;
    }
  } catch (EndOfFileException& e) {
    logE("premature end of file!");
    lastError = "incomplete file";
    delete[] file;
    return false;
  }
  delete[] file;
  return true;
}
