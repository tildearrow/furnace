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

#ifndef _SCSP_H
#define _SCSP_H

#include "../dispatch.h"
#include <string>
#include <vector>

class DivPlatformSCSP: public DivDispatch {
  protected:
    struct Channel: public SharedChannel {
      int slot;
      int sample;
      int sampleNote;
      int sampleNoteDelta;
      int pan;
      int outVol;
      bool keyOn, keyOff, sampleSet, audPosOverride;
      unsigned int audPos;
      Channel(bool linear=true):
        SharedChannel(127,linear),
        slot(-1),
        sample(-1),
        sampleNote(0),
        sampleNoteDelta(0),
        pan(15),
        outVol(127),
        keyOn(false),
        keyOff(false),
        sampleSet(false),
        audPosOverride(false),
        audPos(0) {}
    };
    Channel chan[8];
    DivDispatchOscBuffer* oscBuf[8];
    bool isMuted[8];
    DivPitchTable pitchTable;

    bool slotInUse[32];

    // Voice allocator — maps tracker channels (1 note each) onto contiguous
    // runs of hardware slots. PCM voices use 1 slot; FM voices use opCount
    // slots (1..6). At most one active voice per channel, so 8 records suffice.
    struct Voice {
      int chan;
      int note;
      int firstSlot;
      int slotCount;
      unsigned long long age;
      bool active;
      Voice():
        chan(-1), note(0), firstSlot(-1), slotCount(0), age(0), active(false) {}
    };
    Voice voices[8];
    unsigned long long allocCounter;

    int  allocateVoice(int chanIdx, int note, int numSlots);
    void releaseVoice(int chanIdx);
    void releaseAllVoices();
    int  findFreeRun(int numSlots);
    int  findLruVoice();

    unsigned int* sampleOff;
    unsigned int* sampleStored;  // frames actually uploaded (≤ sample->samples; clipped to RAM)
    bool* sampleLoaded;
    unsigned char* sampleMem;
    size_t sampleMemLen;
    DivMemoryComposition memCompo;

    static const unsigned int RAM_SIZE = 512*1024;
    static const unsigned int USER_SAMPLE_BASE = 0x5400;

    // Byte offsets in SCSP RAM of the 10 built-in FM waveforms,
    // populated during reset() by scsp_load_builtins().
    int builtinOffsets[10];

    // regPool layout (snapshot, refreshed lazily on getRegisterPool):
    //   0x000..0x3FF (1024)   32 slots × 16 regs × 2 bytes (BE)
    //   0x400..0x42F (  48)   24 common regs × 2 bytes (BE)
    //   0x430..0x82F (1024)   DSP MPRO: 128 steps × 4 words × 2 bytes (BE)
    //   0x830..0x8AF ( 128)   DSP COEF: 64 × 2 bytes (BE)
    //   0x8B0..0x8EF (  64)   DSP MADRS: 32 × 2 bytes (BE)
    // Total: 0x8F0 = 2288 bytes. Padded to 0x900 for alignment.
    static const int REG_POOL_SIZE = 0x900;
    unsigned char regPool[REG_POOL_SIZE];
    void refreshRegPool();

    // Last DSP-assembly result. Populated by reset() so the GUI editor can
    // surface errors/warnings without re-running the assembler. `dspStepsLoaded`
    // is 0 when no DSP program is active (empty source or assemble failed).
    std::vector<std::string> dspLastErrors;
    std::vector<std::string> dspLastWarnings;
    int dspStepsLoaded;

    friend void putDispatchChip(void*,int);
    friend void putDispatchChan(void*,int,int);

    void programSlot(int slot, int chanIdx);
    void programSlotFM(int slot, int chanIdx, int opIdx, int slotBase, double midiNote);
    const Voice* findVoiceByChan(int chanIdx) const;
    void updateChanDirectOutput(int chanIdx);
    void updateChanVolume(int chanIdx);
    void writeSlotPitch(int slot, int midiNote, int baseMidiNote);
    void writeSlotEnvelope(int slot, unsigned char ar, unsigned char d1r,
                           unsigned char d2r, unsigned char rr,
                           unsigned char dl, unsigned char krs);
    void writeSlotTotalLevel(int slot, unsigned char tl);
    void writeSlotPan(int slot, unsigned char disdl, unsigned char dipan);
    int  midiNoteAtNativeRate(int sampleRate);

  public:
    void acquire(short** buf, size_t len);
    int dispatch(DivCommand c);
    void muteChannel(int ch, bool mute);
    void notifyInsDeletion(void* ins);
    void notifySampleChange(int sample);
    void notifyInsAddition(int sysID);
    void notifyPitchTable(int sample=-1);
    SharedChannel* getChanState(int chan);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    int getOutputCount();
    bool hasSoftPan(int ch);
    bool keyOffAffectsArp(int ch);
    bool keyOffAffectsPorta(int ch);
    void setFlags(const DivConfig& flags);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    // Re-assemble the song's DSP source and push to the chip without
    // touching voice/RAM state. Returns true if the program loaded; on
    // failure dspLastErrors holds the assembler diagnostics.
    bool reloadDSP();
    const std::vector<std::string>& getDSPErrors() const { return dspLastErrors; }
    const std::vector<std::string>& getDSPWarnings() const { return dspLastWarnings; }
    int getDSPStepsLoaded() const { return dspStepsLoaded; }
    const void* getSampleMem(int index=0);
    size_t getSampleMemCapacity(int index=0);
    size_t getSampleMemUsage(int index=0);
    bool isSampleLoaded(int index, int sample);
    const DivMemoryComposition* getMemCompo(int index);
    void renderSamples(int chipID);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformSCSP();
};

#endif
