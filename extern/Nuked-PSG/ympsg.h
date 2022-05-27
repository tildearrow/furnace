// Copyright (C) 2021 Nuke.YKT
// License: GPLv2+
// Version 1.0.1
#ifndef _YMPSG_H_
#define _YMPSG_H_
#include <stdint.h>

#define YMPSG_WRITEBUF_SIZE 2048
#define YMPSG_WRITEBUF_DELAY 8

typedef struct _ympsg_writebuf {
    uint64_t time;
    uint8_t stat;
    uint8_t data;
} ympsg_writebuf;

typedef struct {
    // IO
    uint8_t data;
    uint8_t latch;
    uint8_t write_flag;
    uint8_t write_flag_l;

    uint8_t prescaler_1;
    uint8_t prescaler_2;
    uint8_t prescaler_2_l;
    uint8_t reset_latch;
    uint8_t ic;
    uint8_t ic_latch1;
    uint8_t ic_latch2;
    uint8_t data_mask;
    uint8_t reg_reset;
    uint8_t volume[4];
    uint16_t freq[3];
    uint8_t noise_data;
    uint8_t noise_of;
    uint8_t noise_trig;
    uint8_t noise_trig_l;
    uint8_t rot;

    uint8_t chan_sel;

    uint16_t counter[4];
    uint8_t counter_of;
    uint8_t sign;
    uint8_t sign_l;
    uint8_t noise_sign_l;
    uint16_t noise;
    uint8_t noise_tap2;
    uint16_t noise_size;
    uint8_t test;
    uint8_t volume_out[4];

    //
    uint64_t writebuf_samplecnt;
    uint32_t writebuf_cur;
    uint32_t writebuf_last;
    uint64_t writebuf_lasttime;
    ympsg_writebuf writebuf[YMPSG_WRITEBUF_SIZE];

    uint8_t mute;
} ympsg_t;


void YMPSG_Write(ympsg_t *chip, uint8_t data);
uint16_t YMPSG_Read(ympsg_t *chip);
void YMPSG_Init(ympsg_t *chip, uint8_t real_sn);
void YMPSG_SetIC(ympsg_t *chip, uint32_t ic);
void YMPSG_Clock(ympsg_t *chip);
float YMPSG_GetOutput(ympsg_t *chip);
void YMPSG_Test(ympsg_t *chip, uint16_t test);


void YMPSG_Generate(ympsg_t *chip, int32_t *buf);
void YMPSG_WriteBuffered(ympsg_t *chip, uint8_t data);

void YMPSG_SetMute(ympsg_t *chip, uint8_t mute);

#endif
