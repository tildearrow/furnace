#include <math.h>
#include <random>
#include <iostream>


class S3HS_sound {
public:
    
    #include "envelove.cpp"
    #include "ram.cpp"
    std::random_device rd;
    std::mt19937 mt;
    long long Total_time = 0;
    double t1[8] = {0,0,0,0,0,0,0,0};
    double t2[8] = {0,0,0,0,0,0,0,0};
    double t3[8] = {0,0,0,0,0,0,0,0};
    double t4[8] = {0,0,0,0,0,0,0,0};
    double t5[8] = {0,0,0,0,0,0,0,0};
    double t6[8] = {0,0,0,0,0,0,0,0};
    double t7[8] = {0,0,0,0,0,0,0,0};
    double t8[8] = {0,0,0,0,0,0,0,0};
    double twt[4] = {0,0,0,0};
    double in1[4]  = {0.0,0.0,0.0,0.0};
    double in2[4]  = {0.0,0.0,0.0,0.0};
    double out1[4] = {0.0,0.0,0.0,0.0};
    double out2[4] = {0.0,0.0,0.0,0.0};
    int vols[64] = {};
    std::vector<int> gateTick = {0,0,0,0,0,0,0,0};
    std::vector<Byte> reg,regenvl,regwt;
    std::vector<int> noise;
    std::vector<std::vector<double>> sintable;
    std::vector<EnvGenerator> envl;
    EnvGenerator _envl;
    double prev = 0;
    int pcm_addr[4],pcm_len[4],pcm_loop_start[4],pcm_loop_end[4]={0,0,0,0};
    std::vector<std::vector<Byte>> pcm_ram; 

    S3HS_sound() {
    };
    
    #define S3HS_SAMPLE_FREQ 48000

    double sind(double theta) {
        return sin(theta*2*M_PI);
    }

    double modulate(double theta, int wf) {
        return sintable.at(wf).at((int)((theta*256))&0xff);
    }

    double generateFMWave(double t1, double v1, double t2, double v2, double t3, double v3, double t4, double v4, int w1, int w2, int w3, int w4) {

        double value = modulate(t1+modulate(t2+modulate(t3+modulate(t4,w4)*v4,w3)*v3,w2)*v2,w1)*v1*255*127;
        return value;

    }

    double generateHSWave(int mode, double t1, double v1, double t2, double v2, double t3, double v3, double t4, double v4, double t5, double v5, double t6, double v6, double t7, double v7, double t8, double v8, int w1, int w2, int w3, int w4, int w5, int w6, int w7, int w8) {
        double value = 0;
        switch (mode)
        {
        case 0:
            value = (modulate(t1,w1)*v1+modulate(t2,w2)*v2+modulate(t3,w3)*v3+modulate(t4,w4)*v4+
                    modulate(t5,w5)*v5+modulate(t6,w6)*v6+modulate(t7,w7)*v7+modulate(t8,w8)*v8)*255*127; //Additive
            break;
        case 1:
            value = (modulate(t1,w1)*v1-modulate(t2,w2)*v2-modulate(t3,w3)*v3-modulate(t4,w4)*v4-
                    modulate(t5,w5)*v5-modulate(t6,w6)*v6-modulate(t7,w7)*v7-modulate(t8,w8)*v8)*255*127; //Subtract
            break;
        case 2:
            value = ((modulate(t1,w1)*v1+modulate(t2,w2)*v2+modulate(t3,w3)*v3+modulate(t4,w4)*v4)*
                    (modulate(t5,w5)*v5+modulate(t6,w6)*v6+modulate(t7,w7)*v7+modulate(t8,w8)*v8))*255*127; //RingMod
            break;
        default:
            break;
        }
        
        return value;

    }

    void applyEnveloveToRegisters(std::vector<Byte> &reg, std::vector<Byte> &regenvl, int opNum, int ch, double dt) {
        ADSRConfig adsr;
        adsr.attackTime = ((double)reg.at(32+64*ch+opNum*4+0).toInt())/64;
        adsr.decayTime = ((double)reg.at(32+64*ch+opNum*4+1).toInt())/64;
        adsr.sustainLevel = ((double)reg.at(32+64*ch+opNum*4+2).toInt())/255;
        adsr.releaseTime = ((double)reg.at(32+64*ch+opNum*4+3).toInt())/64;
        if (reg.at(64*ch+0x1e).toInt() == 0 && gateTick.at(ch) == 1) {
            envl.at((size_t)(ch*8+opNum)).noteOff();
            if(opNum == 7) {
                gateTick.at(ch)=0;
            }
            
        }
        if (reg.at(64*ch+0x1e).toInt() == 1 && gateTick.at(ch) == 0) {
            envl.at((size_t)(ch*8+opNum)).reset(EnvGenerator::State::Attack); 
            if(opNum == 7) {
                gateTick.at(ch)=1;
            }
        }
        //std::cout << dt << std::endl; //envl.at((size_t)(ch*4+opNum)).m_elapsed
        vols[ch*8+opNum] = (envl.at((size_t)(ch*8+opNum)).currentLevel()*255*((double)(reg.at(ch*64+opNum+16).toInt())/255));
        envl.at((size_t)(ch*8+opNum)).update(adsr,dt);
    }

    std::vector<std::vector<std::vector<int16_t>>> AudioCallBack(int len)
    {
        int i;
        std::vector<int16_t> __frames(len,0);
        std::vector<std::vector<int16_t>> _frames(12,__frames);
        std::vector<std::vector<std::vector<int16_t>>> frames(2,_frames);
        int framesize = len;
        reg = ram_peek2array(ram,0x400000,512);
        regwt = ram_peek2array(ram,0x400200,192);

        for (int ch=0;ch<4;ch++) {
            if(regwt.at(48*ch+3).toInt() == 0) {
                pcm_addr[ch] = regwt.at(16+48*ch+0).toInt()*65536+regwt.at(16+48*ch+1).toInt()*256+regwt.at(16+48*ch+2).toInt();
                pcm_len[ch] = regwt.at(16+48*ch+3).toInt()*65536+regwt.at(16+48*ch+4).toInt()*256+regwt.at(16+48*ch+5).toInt();
                pcm_loop_start[ch] = regwt.at(16+48*ch+6).toInt()*65536+regwt.at(16+48*ch+7).toInt()*256+regwt.at(16+48*ch+8).toInt();
                //pcm_loop_end[ch] = regwt.at(12+64*ch+9).toInt()*65536+regwt.at(12+64*ch+10).toInt()*256+regwt.at(12+64*ch+11).toInt();
                //std::cout << pcm_addr[ch] << std::endl;
                //std::cout << pcm_len[ch] << std::endl;
            }
        }
        
        for(int ch=0; ch < 8; ch++) {
            for (int opNum=0; opNum < 8; opNum++) {
                applyEnveloveToRegisters(reg,regenvl,opNum,ch,((double)len/(double)S3HS_SAMPLE_FREQ));
            }
        }
        for (i = 0; i < framesize; i++) {
            double result[12] = {0};
            for(int ch=0; ch < 8; ch++) {
                int addr = 64*ch;
                double f1 = ((double)reg.at(addr+0).toInt()*256+reg.at(addr+1).toInt());
                t1[ch] = t1[ch] + (f1/S3HS_SAMPLE_FREQ);
                t2[ch] = t2[ch] + f1*(((double)reg.at(addr+2).toInt()*256+reg.at(addr+3).toInt())/4096)/S3HS_SAMPLE_FREQ;
                t3[ch] = t3[ch] + f1*(((double)reg.at(addr+4).toInt()*256+reg.at(addr+5).toInt())/4096)/S3HS_SAMPLE_FREQ;
                t4[ch] = t4[ch] + f1*(((double)reg.at(addr+6).toInt()*256+reg.at(addr+7).toInt())/4096)/S3HS_SAMPLE_FREQ;
                t5[ch] = t5[ch] + f1*(((double)reg.at(addr+8).toInt()*256+reg.at(addr+9).toInt())/4096)/S3HS_SAMPLE_FREQ;
                t6[ch] = t6[ch] + f1*(((double)reg.at(addr+10).toInt()*256+reg.at(addr+11).toInt())/4096)/S3HS_SAMPLE_FREQ;
                t7[ch] = t7[ch] + f1*(((double)reg.at(addr+12).toInt()*256+reg.at(addr+13).toInt())/4096)/S3HS_SAMPLE_FREQ;
                t8[ch] = t8[ch] + f1*(((double)reg.at(addr+14).toInt()*256+reg.at(addr+15).toInt())/4096)/S3HS_SAMPLE_FREQ;
                double v1 = (double)(vols[ch*8+0])/256;
                double v2 = (double)(vols[ch*8+1])/256;
                double v3 = (double)(vols[ch*8+2])/256;
                double v4 = (double)(vols[ch*8+3])/256;
                double v5 = (double)(vols[ch*8+4])/256;
                double v6 = (double)(vols[ch*8+5])/256;
                double v7 = (double)(vols[ch*8+6])/256;
                double v8 = (double)(vols[ch*8+7])/256;
                int w1 = reg.at(addr+24).toInt()>>4;
                int w2 = reg.at(addr+24).toInt()&0xf;
                int w3 = reg.at(addr+25).toInt()>>4;
                int w4 = reg.at(addr+25).toInt()&0xf;
                int w5 = reg.at(addr+26).toInt()>>4;
                int w6 = reg.at(addr+26).toInt()&0xf;
                int w7 = reg.at(addr+27).toInt()>>4;
                int w8 = reg.at(addr+27).toInt()&0xf;
                int mode = reg.at(addr+0x1c).toInt();
                result[ch] += generateHSWave(mode,t1[ch],v1,t2[ch],v2,t3[ch],v3,t4[ch],v4,t5[ch],v5,t6[ch],v6,t7[ch],v7,t8[ch],v8,w1,w2,w3,w4,w5,w6,w7,w8);
                //std::cout << v1 << std::endl;
            }
            
            for(int ch=0; ch<4; ch++) {
                double ft = ((double)regwt.at(ch*48+0).toInt()*256+regwt.at(ch*48+1).toInt());
                twt[ch] = twt[ch] + (ft/S3HS_SAMPLE_FREQ)*32;
                double vt = ((double)regwt.at(ch*48+2).toInt())/255;
                int val = 0;
                if (regwt.at(ch*48+3).toInt() == 1) {
                    val = regwt.at(16+48*ch+((int)twt[ch]%32)).toInt();
                    if ((int)(twt[ch]*2)%2 == 0) {
                        val /= 16;
                    } else {
                        val %= 16;
                    }
                    val *= 16;
                } else if(regwt.at(ch*48+3).toInt() == 2) {
                    val = noise.at(((int)twt[ch]%65536))*255;
                } else if(regwt.at(ch*48+3).toInt() == 3) {
                    val = noise.at(((int)twt[ch]%64))*255;
                } else if(regwt.at(ch*48+3).toInt() == 0) {
                    if (pcm_addr[ch]+(int)twt[ch] > pcm_len[ch] && pcm_loop_start[ch] < pcm_len[ch] && pcm_loop_start[ch] != 0) {
                        val = ram_peek(ram,pcm_addr[ch]+((int)twt[ch]%(pcm_len[ch]-pcm_loop_start[ch]))+(pcm_addr[ch]-pcm_loop_start[ch])).toInt();
                    } else {
                        val = ram_peek(ram,std::min(pcm_addr[ch]+(int)twt[ch],pcm_len[ch])).toInt();
                    }
                    //std::cout << twt[ch] << std::endl;
                }
                val -= 128;
                /**double omega = 2.0 * 3.14159265 * ((double)regwt.at(ch*48+5).toInt()+1)*32 / S3HS_SAMPLE_FREQ;
                double alpha = sin(omega) / (2.0 * 1.5);
                double a0 =  1.0 + alpha;
                double a1 = -2.0 * cos(omega);
                double a2 =  1.0 - alpha;
                double b0 = (1.0 - cos(omega)) / 2.0;
                double b1 =  1.0 - cos(omega);
                double b2 = (1.0 - cos(omega)) / 2.0;
                double output = b0/a0*(double)val+b1/a0*in1[ch]+b2/a0*in2[ch]-a1/a0*out1[ch]-a2/a0*out2[ch];
                in2[ch]  = in1[ch];
                in1[ch]  = val; 
                out2[ch] = out1[ch];     
                out1[ch] = output; 
                if (regwt.at(ch*48+5).toInt() == 0) {
                    result[ch+8] += (double)(val)*255*vt;
                } else {
                    result[ch+8] += std::min(std::max((double)output*255*vt,-32768.0),32767.0);
                }**/
                result[ch+8] += (double)(val)*255*vt;
            }
            
            for(int ch=0; ch<12; ch++) {
                int panL, panR;
                if (ch < 8) {
                    panL = reg.at(0x1d+64*ch).toInt()>>4;
                    panR = reg.at(0x1d+64*ch).toInt()&0xf;
                } else {
                    panL = regwt.at(0x07+48*(ch-8)).toInt()>>4;
                    panR = regwt.at(0x07+48*(ch-8)).toInt()&0xf;
                }
                if (panL == 0 && panR == 0) {
                    panL = 15;
                    panR = 15;
                }
                frames[0][ch][i] = (int16_t)std::min(std::max(result[ch]*((double)(panL)/15),-32768.0),32767.0);
                frames[1][ch][i] = (int16_t)std::min(std::max(result[ch]*((double)(panR)/15),-32768.0),32767.0);
            }
        }

        reg.clear();
        regenvl.clear();
        regwt.clear();
        Total_time++;
        return frames;
    }

    void initSound() {
        
        envl.resize(64,_envl);
        noise.resize(65536,0);
        std::vector<double> _sintable;
        _sintable.resize(256,0);
        sintable.resize(16,_sintable);
        for (int i=0;i<65536;i++) {
            noise[i] = mt()%2;
        }
        for (int i=0; i<256; i++) {
            sintable.at(0).at(i) = sind((double)i/256);
            sintable.at(1).at(i) = std::max(0.0,sind((double)i/256));
            sintable.at(4).at(i) = i<128?(double)i/64-1.0:(double)i/-64+3.0;
            sintable.at(5).at(i) = (double)i/128-1.0;
            if (i<128) {
                sintable.at(2).at(i) = sind((double)i/128);
                sintable.at(3).at(i) = 1.0;
            } else {
                sintable.at(3).at(i) = -1.0;
            }
        }
        
        for (int addr=400000;addr<0x400400;addr++) {
            ram_poke(ram,addr,0x00);
        }
    }

    void resetGate(int ch) {
        for (int i=0;i<8;i++) {
            envl.at((size_t)(ch*8+i)).reset(EnvGenerator::State::Attack); 
        }
    }

    void wtSync(int ch) {
        twt[ch]=0;
    }
    
};