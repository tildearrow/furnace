#ifndef DSID_H
#define DSID_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

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
    struct SIDVOICE v[3];
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

    double ctf_table[2048];

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
    int16_t lastOut[3];
    int mute_mask;
};

double dSID_render(struct SID_chip* sid);
void dSID_init(struct SID_chip* sid, double clockRate, double samplingRate, int model, unsigned char init_wf);
float dSID_getVolume(struct SID_chip* sid, int channel);
void dSID_setMuteMask(struct SID_chip* sid, int mute_mask);

void dSID_write(struct SID_chip* sid, unsigned char addr, unsigned char val);

#ifdef __cplusplus
}
#endif

#endif
