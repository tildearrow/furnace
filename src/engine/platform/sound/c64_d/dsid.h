#pragma once

#include <stdint.h>

#define SID_CLK 985248
#define SID_CHA 3
#define SID_OUT_SCALE (0x10000 * SID_CHA * 16)

// TODO: prefix SID_

// CONTROL
#define GAT 0x01
#define SYN 0x02
#define RNG 0x04
#define TST 0x08
#define TRI 0x10
#define SAW 0x20
#define PUL 0x40
#define NOI 0x80

#define _HZ 0x10
#define DECSUS 0x40
#define ATK 0x80

// TODO: change
// filter mode (high)
#define LP 0x10
#define BP 0x20
#define HP 0x40
#define OFF3 0x80

extern const int Aexp[256];

struct SID_ctx_chan {
    double rcnt;
    double envcnt;
    double expcnt;
    double pacc;
    double pracc;
    int FSW;
    int nLFSR;
    double prevwfout;
    uint8_t pSR;
    int Ast;
};

struct SID_ctx {
    int sMSBrise;
    int sMSB;
    double plp;
    double pbp;
    struct SID_ctx_chan ch[3];
};

struct SIDVOICE {
    uint8_t freq_low;
    uint8_t freq_high;
    uint8_t pw_low;
    uint8_t pw_high : 4;
    uint8_t UNUSED : 4;
    uint8_t control;
    uint8_t decay : 4;
    uint8_t attack : 4;
    uint8_t susres;
    // uint8_t release : 4;
    // uint8_t sustain : 4;
};

struct SIDMEM {
    struct SIDVOICE v[SID_CHA];
    uint8_t UNUSED : 4;
    uint8_t cutoff_low : 4;
    uint8_t cutoff_high;
    uint8_t reso_rt : 4;
    uint8_t reso : 4;
    uint8_t volume : 4;
    uint8_t filter_mode : 4;
    uint8_t paddlex;
    uint8_t paddley;
    uint8_t osc3;
    uint8_t env3;
};

struct SID_globals {
    double ckr;
    double ctfr;
    double ctf_ratio_6581;

    int trsaw[4096];
    int pusaw[4096];
    int Pulsetrsaw[4096];

    double Aprd[16];
    int Astp[16];
    int model;
};

#define MemLen 65536

struct SID_chip {
    struct SID_globals g;
    struct SID_ctx SIDct[3];
    uint8_t M[MemLen];
    int mute_mask;
};

extern struct SID_chip sid;

double dSID_render();
void dSID_init(double samplingRate, int model);
float dSID_getVolume(int channel);
void dSID_setMuteMask(int mute_mask);
