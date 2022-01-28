#ifndef _SONG_H
#define _SONG_H
#include <stdio.h>
#include <vector>

#define DIV_MAX_CHANS 128

#include "../ta-utils.h"
#include "orders.h"
#include "instrument.h"
#include "pattern.h"
#include "wavetable.h"
#include "sample.h"

enum DivSystem {
  DIV_SYSTEM_NULL=0,
  DIV_SYSTEM_YMU759,
  DIV_SYSTEM_GENESIS,
  DIV_SYSTEM_GENESIS_EXT,
  DIV_SYSTEM_SMS,
  DIV_SYSTEM_GB,
  DIV_SYSTEM_PCE,
  DIV_SYSTEM_NES,
  DIV_SYSTEM_C64_6581,
  DIV_SYSTEM_C64_8580,
  DIV_SYSTEM_ARCADE,
  DIV_SYSTEM_YM2610,
  DIV_SYSTEM_YM2610_EXT,
  
  DIV_SYSTEM_AY8910,
  DIV_SYSTEM_AMIGA,
  DIV_SYSTEM_YM2151,
  DIV_SYSTEM_YM2612,
  DIV_SYSTEM_TIA,
  DIV_SYSTEM_SAA1099,
  DIV_SYSTEM_AY8930
};

struct DivSong {
  // version number used for saving the song.
  // Furnace will save using the latest possible version,
  // but eventually I will and 0x80 to this value to indicate a Furnace module
  // known version numbers:
  // - 24: v0.12/0.13/1.0
  //   - current format version
  //   - changes pattern length from char to int, probably to allow for size 256
  // - 23: ???
  //   - what happened here?
  // - 20: v11.1 (?)
  //   - E5xx effect range is now ±1 semitone
  // - 19: v11
  //   - introduces Arcade system
  //   - changes to the FM instrument format due to YMU759 being dropped
  // - 18: v10
  //   - radically changes STD instrument for Game Boy
  // - 17: v9
  //   - changes C64 volIsCutoff flag from int to char for unknown reasons
  // - 16: v8 (?)
  //   - introduces C64 system
  // - 15: v7 (?)
  // - 14: v6 (?)
  //   - introduces NES system
  //   - changes macro and wave values from char to int
  // - 13: v5.1
  //   - introduces PC Engine system in later version (how?)
  //   - stores highlight in file
  // - 12: v5 (?)
  //   - introduces Game Boy system
  //   - introduces wavetables
  // - 11: ???
  //   - introduces Sega Master System
  //   - custom Hz support
  //   - instrument type (FM/STD) present
  //   - prior to this version the instrument type depended on the system
  // - 10: ???
  //   - introduces multiple effect columns
  // - 9: v3.9
  //   - introduces Genesis system
  //   - introduces system number
  // - 7: ???
  // - 5: BETA 3 (?)
  //   - adds arpeggio tick
  // - 3: BETA 2
  //   - possibly the first version that could save
  //   - basic format, no system number, 16 instruments, one speed, YMU759-only
  //   - if somebody manages to find a version 2 or even 1 module, please tell me as it will be worth more than a luxury vehicle
  unsigned short version;

  // system
  DivSystem system[32];
  unsigned char systemLen;
  signed char systemVol[32];
  signed char systemPan[32];
  // interpretation of these flags varies depending on system.
  // - most systems:
  //   - bit 0: PAL
  // - NES:
  //   - bit 0-1: system type
  //     - 0: NTSC
  //     - 1: PAL
  //     - 2: Dendy
  // - SMS/SN76489:
  //   - bit 0: PAL
  //   - bit 1-2: noise type
  //     - 0: Sega VDP (16-bit noise)
  //     - 1: real SN76489 (15-bit noise)
  //     - 2: real SN76489 with Atari-like short noise buzz (15-bit noise)
  //     - 3: Game Gear (16-bit noise, stereo)
  // - YM2612:
  //   - bit 0-1: clock rate
  //     - 0: Genesis NTSC (7.67MHz)
  //     - 1: Genesis PAL (7.61MHz)
  //     - 2: 8MHz
  // - YM2151:
  //   - bit 0-1: clock rate
  //     - 0: 3.58MHz (NTSC)
  //     - 1: 3.55MHz (PAL)
  //     - 2: 4MHz
  // - AY-3-8910/AY8930:
  //   - bit 0-3: clock rate
  //     - 0: 1.79MHz (MSX NTSC)
  //     - 1: 1.77MHz (ZX Spectrum, MSX PAL, etc.)
  //     - 2: 1.75MHz (ZX Spectrum)
  //     - 3: 2MHz (Atari ST)
  //     - 4: 1.5MHz (Vectrex)
  //     - 5: 1MHz (Amstrad CPC
  //     - 6: 0.89MHz (Sunsoft 5B)
  //     - 7: 1.67MHz
  //   - bit 4-5: chip type (ignored on AY8930)
  //     - 0: AY-3-8910 or similar
  //     - 1: YM2149
  //     - 2: Sunsoft 5B
  //   - bit 6: stereo
  //     - 0: mono
  //     - 1: stereo ABC
  // - SAA1099:
  //   - bit 0-1: clock rate
  //     - 0: 8MHz (SAM Coupé, Game Blaster)
  //     - 1: 7.15MHz
  //     - 2: 7.09MHz
  // - Amiga:
  //   - bit 0-1: clock rate
  //     - 0: 7.15MHz (NTSC)
  //     - 1: 7.09MHz (PAL)
  //   - bit 2: model
  //     - 0: Amiga 500
  //     - 1: Amiga 1200
  unsigned int systemFlags[32];

  // song information
  String name, author;

  // legacy song information
  String carrier, composer, vendor, category, writer, arranger, copyright, manGroup, manInfo, createdDate, revisionDate;

  // highlight
  unsigned char hilightA, hilightB;

  // module details
  unsigned char timeBase, speed1, speed2, arpLen;
  bool pal;
  bool customTempo;
  int hz, patLen, ordersLen, insLen, waveLen, sampleLen;
  float tuning;

  // compatibility flags
  bool limitSlides; // limit slide range

  DivOrders orders;
  std::vector<DivInstrument*> ins;
  DivChannelData pat[DIV_MAX_CHANS];
  std::vector<DivWavetable*> wave;
  std::vector<DivSample*> sample;

  DivInstrument nullIns;
  DivWavetable nullWave;

  void unload();

  DivSong():
    version(24),
    systemLen(1),
    name(""),
    author(""),
    carrier(""),
    composer(""),
    vendor(""),
    category(""),
    writer(""),
    arranger(""),
    copyright(""),
    manGroup(""),
    manInfo(""),
    createdDate(""),
    revisionDate(""),
    hilightA(4),
    hilightB(16),
    timeBase(0),
    speed1(6),
    speed2(6),
    arpLen(1),
    pal(true),
    customTempo(false),
    hz(60),
    patLen(64),
    ordersLen(1),
    insLen(0),
    waveLen(0),
    sampleLen(0),
    tuning(440.0f),
    limitSlides(false) {
    for (int i=0; i<32; i++) {
      system[i]=DIV_SYSTEM_NULL;
      systemVol[i]=64;
      systemPan[i]=0;
      systemFlags[i]=0;
    }
    system[0]=DIV_SYSTEM_GENESIS;
  }
};

#endif