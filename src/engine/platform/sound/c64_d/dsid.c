#include "dsid.h"
#include <math.h> // INFINITY
#include <stdlib.h>
#include <string.h> // memset, memcpy

// defle internals
void waveforms_add_sample(uint8_t id, int16_t left, int16_t right);

struct SID_chip sid;
static char init_wf = 1;

const int Aexp[256] = {
    1, 30, 30, 30, 30, 30, 30, 16, 16, 16, 16, 16, 16, 16, 16, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4,  4,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 1, 1, 1, 1, 1, 1};

double cmbWF(int chn, int *wfa, int index, int differ6581, struct SID_globals *g) {
    if (differ6581 && sid.g.model == 6581)
        index &= 0x7FF;

    return wfa[index];
}

void cCmbWF(int *wfa, double bitmul, double bstr, double trh) {
    for (int i = 0; i < 4096; i++) {
        wfa[i] = 0;
        for (int j = 0; j < 12; j++) {
            double blvl = 0;
            for (int k = 0; k < 12; k++) {
                blvl += (bitmul / pow(bstr, abs(k - j))) * (((i >> k) & 1) - 0.5);
            }
            wfa[i] += (blvl >= trh) ? pow(2, j) : 0;
        }
        wfa[i] *= 12;
    }
}

void dSID_init(double samplingRate, int model) {
    if (model == 6581) {
        sid.g.model = 6581;
    } else {
        sid.g.model = 8580;
    }

    // SID area
    for (int i = 0xD400; i <= 0xD7FF; i++)
        sid.M[i] = 0;
    // IO area
    for (int i = 0xDE00; i <= 0xDFFF; i++)
        sid.M[i] = 0;

    memset(sid.SIDct, 0, sizeof(sid.SIDct));
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            sid.SIDct[i].ch[j].Ast = _HZ;
            sid.SIDct[i].ch[j].nLFSR = 0x7FFFF8;
            sid.SIDct[i].ch[j].prevwfout = 0;
        }
        sid.SIDct[i].ch[0].FSW = 1;
        sid.SIDct[i].ch[1].FSW = 2;
        sid.SIDct[i].ch[2].FSW = 4;
    }

    sid.g.ctfr = -2.0 * 3.14 * (12500.0 / 256.0) / samplingRate,
    sid.g.ctf_ratio_6581 = -2.0 * 3.14 * (20000.0 / 256.0) / samplingRate;
    sid.g.ckr = (double) SID_CLK / samplingRate;

    const double bAprd[16] = {9,        32 * 1,    63 * 1,    95 * 1,   149 * 1,  220 * 1,
                              267 * 1,  313 * 1,   392 * 1,   977 * 1,  1954 * 1, 3126 * 1,
                              3907 * 1, 11720 * 1, 19532 * 1, 31251 * 1};
    const int bAstp[16] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    memcpy(&sid.g.Aprd, &bAprd, sizeof(bAprd));
    memcpy(&sid.g.Astp, &bAstp, sizeof(bAstp));
    if (init_wf) {
        cCmbWF(sid.g.trsaw, 0.8, 2.4, 0.64);
        cCmbWF(sid.g.pusaw, 1.4, 1.9, 0.68);
        cCmbWF(sid.g.Pulsetrsaw, 0.8, 2.5, 0.64);
        init_wf = 0;
    }

    double prd0 = sid.g.ckr > 9 ? sid.g.ckr : 9;
    sid.g.Aprd[0] = prd0;
    sid.g.Astp[0] = ceil(prd0 / 9);
}

double dSID_render() {
    double flin = 0, output = 0;
    double wfout = 0;
    for (int chn = 0; chn < SID_CHA; chn++) {
        struct SIDVOICE *voic = &((struct SIDMEM *) (sid.M))->v[chn];
        double pgt = (sid.SIDct->ch[chn].Ast & GAT);
        uint8_t ctrl = voic->control;
        uint8_t wf = ctrl & 0xF0;
        uint8_t test = ctrl & TST;
        uint8_t SR = voic->susres;
        double tmp = 0;
        if (pgt != (ctrl & GAT)) {
            if (pgt) {
                sid.SIDct->ch[chn].Ast &= 0xFF - (GAT | ATK | DECSUS);
            } else {
                sid.SIDct->ch[chn].Ast = (GAT | ATK | DECSUS);
                if ((SR & 0xF) > (sid.SIDct->ch[chn].pSR & 0xF))
                    tmp = 1;
            }
        }
        sid.SIDct->ch[chn].pSR = SR;
        sid.SIDct->ch[chn].rcnt += sid.g.ckr;
        if (sid.SIDct->ch[chn].rcnt >= 0x8000)
            sid.SIDct->ch[chn].rcnt -= 0x8000;

        static double step;
        double prd;

        if (sid.SIDct->ch[chn].Ast & ATK) {
            step = voic->attack;
            prd = sid.g.Aprd[(int) step];
        } else if (sid.SIDct->ch[chn].Ast & DECSUS) {
            step = voic->decay;
            prd = sid.g.Aprd[(int) step];
        } else {
            step = SR & 0xF;
            prd = sid.g.Aprd[(int) step];
        }
        step = sid.g.Astp[(int) step];
        if (sid.SIDct->ch[chn].rcnt >= prd && sid.SIDct->ch[chn].rcnt < prd + sid.g.ckr &&
            tmp == 0) {
            sid.SIDct->ch[chn].rcnt -= prd;
            if ((sid.SIDct->ch[chn].Ast & ATK) ||
                ++sid.SIDct->ch[chn].expcnt == Aexp[(int) sid.SIDct->ch[chn].envcnt]) {
                if (!(sid.SIDct->ch[chn].Ast & _HZ)) {
                    if (sid.SIDct->ch[chn].Ast & ATK) {
                        sid.SIDct->ch[chn].envcnt += step;
                        if (sid.SIDct->ch[chn].envcnt >= 0xFF) {
                            sid.SIDct->ch[chn].envcnt = 0xFF;
                            sid.SIDct->ch[chn].Ast &= 0xFF - ATK;
                        }
                    } else if (!(sid.SIDct->ch[chn].Ast & DECSUS) ||
                               sid.SIDct->ch[chn].envcnt > (SR >> 4) + (SR & 0xF0)) {
                        sid.SIDct->ch[chn].envcnt -= step;
                        if (sid.SIDct->ch[chn].envcnt <= 0 &&
                            sid.SIDct->ch[chn].envcnt + step != 0) {
                            sid.SIDct->ch[chn].envcnt = 0;
                            sid.SIDct->ch[chn].Ast |= _HZ;
                        }
                    }
                }
                sid.SIDct->ch[chn].expcnt = 0;
            } else {
            }
        }
        sid.SIDct->ch[chn].envcnt = (int) sid.SIDct->ch[chn].envcnt & 0xFF;
        double aAdd = (voic->freq_low + voic->freq_high * 256) * sid.g.ckr;
        if (test || ((ctrl & SYN) && sid.SIDct->sMSBrise)) {
            sid.SIDct->ch[chn].pacc = 0;
        } else {
            sid.SIDct->ch[chn].pacc += aAdd;
            if (sid.SIDct->ch[chn].pacc > 0xFFFFFF)
                sid.SIDct->ch[chn].pacc -= 0x1000000;
        }
        double MSB = (int) sid.SIDct->ch[chn].pacc & 0x800000;
        sid.SIDct->sMSBrise = (MSB > ((int) sid.SIDct->ch[chn].pracc & 0x800000)) ? 1 : 0;

        if (wf & NOI) {
            tmp = sid.SIDct->ch[chn].nLFSR;
            if ((((int) sid.SIDct->ch[chn].pacc & 0x100000) !=
                 ((int) sid.SIDct->ch[chn].pracc & 0x100000)) ||
                aAdd >= 0x100000) {
                step = ((int) tmp & 0x400000) ^ (((int) tmp & 0x20000) << 5);
                tmp = (((int) tmp << 1) + (step > 0 || test)) & 0x7FFFFF;
                sid.SIDct->ch[chn].nLFSR = tmp;
            }
            wfout = (wf & 0x70) ? 0
                                : (((int) tmp & 0x100000) >> 5) + (((int) tmp & 0x40000) >> 4) +
                                      (((int) tmp & 0x4000) >> 1) + (((int) tmp & 0x800) << 1) +
                                      (((int) tmp & 0x200) << 2) + (((int) tmp & 0x20) << 5) +
                                      (((int) tmp & 0x04) << 7) + (((int) tmp & 0x01) << 8);
        } else if (wf & PUL) {
            double pw = (voic->pw_low + (voic->pw_high) * 256) * 16;
            tmp = (int) aAdd >> 9;
            if (0 < pw && pw < tmp)
                pw = tmp;
            tmp = (int) tmp ^ 0xFFFF;
            if (pw > tmp)
                pw = tmp;
            tmp = (int) sid.SIDct->ch[chn].pacc >> 8;
            if (wf == PUL) {
                int lel = ((int) aAdd >> 16);
                if (lel > 0) {
                    step = 256.0 / (double) lel;
                } else {
                    step = INFINITY;
                }
                if (test)
                    wfout = 0xFFFF;
                else if (tmp < pw) {
                    double lim = (0xFFFF - pw) * step;
                    if (lim > 0xFFFF)
                        lim = 0xFFFF;
                    wfout = lim - (pw - tmp) * step;
                    if (wfout < 0)
                        wfout = 0;
                } else {
                    double lim = pw * step;
                    if (lim > 0xFFFF)
                        lim = 0xFFFF;
                    wfout = (0xFFFF - tmp) * step - lim;
                    if (wfout >= 0)
                        wfout = 0xFFFF;
                    wfout = (int) wfout & 0xFFFF;
                }
            } else {
                wfout = (tmp >= pw || test) ? 0xFFFF : 0;
                if (wf & TRI) {
                    if (wf & SAW) {
                        wfout =
                            (wfout) ? cmbWF(chn, sid.g.Pulsetrsaw, (int) tmp >> 4, 1, &sid.g) : 0;
                    } else {
                        tmp = (int) sid.SIDct->ch[chn].pacc ^ (ctrl & RNG ? sid.SIDct->sMSB : 0);
                        wfout =
                            (wfout)
                                ? cmbWF(chn, sid.g.pusaw,
                                        ((int) tmp ^ ((int) tmp & 0x800000 ? 0xFFFFFF : 0)) >> 11,
                                        0, &sid.g)
                                : 0;
                    }
                } else if (wf & SAW)
                    wfout = (wfout) ? cmbWF(chn, sid.g.pusaw, (int) tmp >> 4, 1, &sid.g) : 0;
            }
        } else if (wf & SAW) {
            wfout = (int) sid.SIDct->ch[chn].pacc >> 8;
            if (wf & TRI)
                wfout = cmbWF(chn, sid.g.trsaw, (int) wfout >> 4, 1, &sid.g);
            else {
                step = aAdd / 0x1200000;
                wfout += wfout * step;
                if (wfout > 0xFFFF)
                    wfout = 0xFFFF - (wfout - 0x10000) / step;
            }
        } else if (wf & TRI) {
            tmp = (int) sid.SIDct->ch[chn].pacc ^ (ctrl & RNG ? sid.SIDct->sMSB : 0);
            wfout = ((int) tmp ^ ((int) tmp & 0x800000 ? 0xFFFFFF : 0)) >> 7;
        }
        if (wf)
            sid.SIDct->ch[chn].prevwfout = wfout;
        else {
            wfout = sid.SIDct->ch[chn].prevwfout;
        }
        sid.SIDct->ch[chn].pracc = sid.SIDct->ch[chn].pacc;
        sid.SIDct->sMSB = MSB;
        // double preflin = flin;
        if ((sid.mute_mask & (1 << chn))) {
            if (sid.M[0x17] & sid.SIDct->ch[chn].FSW) {
                double chnout = (wfout - 0x8000) * (sid.SIDct->ch[chn].envcnt / 256);
                flin += chnout;

                // defle fake filter for solo waveform ahead
                // mostly copypasted from below
                double fakeflin = chnout;
                double fakeflout = 0;
                static double fakeplp[SID_CHA] = {0};
                static double fakepbp[SID_CHA] = {0};
                double ctf = ((double) (sid.M[0x15] & 7)) / 8 + sid.M[0x16] + 0.2;
                double reso;
                if (sid.g.model == 8580) {
                    ctf = 1 - exp(ctf * sid.g.ctfr);
                    reso = pow(2, ((double) (4 - (double) (sid.M[0x17] >> 4)) / 8));
                } else {
                    if (ctf < 24)
                        ctf = 0.035;
                    else
                        ctf = 1 - 1.263 * exp(ctf * sid.g.ctf_ratio_6581);
                    reso = (sid.M[0x17] > 0x5F) ? 8.0 / (double) (sid.M[0x17] >> 4) : 1.41;
                }
                double tmp = fakeflin + fakepbp[chn] * reso + fakeplp[chn];
                if (sid.M[0x18] & HP)
                    fakeflout -= tmp;
                tmp = fakepbp[chn] - tmp * ctf;
                fakepbp[chn] = tmp;
                if (sid.M[0x18] & BP)
                    fakeflout -= tmp;
                tmp = fakeplp[chn] + tmp * ctf;
                fakeplp[chn] = tmp;
                if (sid.M[0x18] & LP)
                    fakeflout += tmp;

                double defle_wf_out = (fakeflout / SID_OUT_SCALE) * (sid.M[0x18] & 0xF) * 65535;
                waveforms_add_sample(1 + chn, defle_wf_out, defle_wf_out);
            } else if ((chn % SID_CHA) != 2 || !(sid.M[0x18] & OFF3)) {
                double chnout = (wfout - 0x8000) * (sid.SIDct->ch[chn].envcnt / 256);
                output += chnout;

                double defle_wf_out = (chnout / SID_OUT_SCALE) * (sid.M[0x18] & 0xF) * 65535;
                waveforms_add_sample(1 + chn, defle_wf_out, defle_wf_out);
            }
        } else {
            waveforms_add_sample(1 + chn, 0, 0);
        }
    }
    int M1 = 0;
    if (M1 & 3)
        sid.M[0x1B] = (int) wfout >> 8;
    sid.M[0x1C] = sid.SIDct->ch[2].envcnt;

    double ctf = ((double) (sid.M[0x15] & 7)) / 8 + sid.M[0x16] + 0.2;
    double reso;
    if (sid.g.model == 8580) {
        ctf = 1 - exp(ctf * sid.g.ctfr);
        reso = pow(2, ((double) (4 - (double) (sid.M[0x17] >> 4)) / 8));
    } else {
        if (ctf < 24)
            ctf = 0.035;
        else
            ctf = 1 - 1.263 * exp(ctf * sid.g.ctf_ratio_6581);
        reso = (sid.M[0x17] > 0x5F) ? 8.0 / (double) (sid.M[0x17] >> 4) : 1.41;
    }

    double tmp = flin + sid.SIDct->pbp * reso + sid.SIDct->plp;
    if (sid.M[0x18] & HP)
        output -= tmp;
    tmp = sid.SIDct->pbp - tmp * ctf;
    sid.SIDct->pbp = tmp;
    if (sid.M[0x18] & BP)
        output -= tmp;
    tmp = sid.SIDct->plp + tmp * ctf;
    sid.SIDct->plp = tmp;
    if (sid.M[0x18] & LP)
        output += tmp;
    return (output / SID_OUT_SCALE) * (sid.M[0x18] & 0xF);
}

void dSID_setMuteMask(int mute_mask) {
    sid.mute_mask = mute_mask;
}

float dSID_getVolume(int channel) {
    if ((sid.M[0x18] & 0xF) == 0)
        return 0;
    return sid.SIDct[0].ch[channel].envcnt / 256.0f;
}
