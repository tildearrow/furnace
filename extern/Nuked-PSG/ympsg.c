// Copyright (C) 2021 Nuke.YKT
// License: GPLv2+
// Version 1.0.1
#include <string.h>
#include "ympsg.h"

const float ympsg_vol[17] = {
    1.0, 0.772, 0.622, 0.485, 0.382, 0.29, 0.229, 0.174, 0.132, 0.096, 0.072, 0.051, 0.034, 0.019, 0.009, 0.0, -1.059
};

static void YMPSG_WriteLatch(ympsg_t *chip)
{
    uint8_t data = chip->data;
    if (chip->data_mask)
    {
        data = 0;
    }
    if (data & 128)
    {
        chip->latch = (data >> 4) & 7;
    }
}

static void YMPSG_UpdateRegisters(ympsg_t* chip)
{
    uint8_t data = chip->data;
    if (chip->data_mask)
    {
        data = 0;
    }
    if (chip->reg_reset || (chip->write_flag_l && chip->latch == 1))
    {
        chip->volume[0] = data & 15;
        if (chip->data_mask)
        {
            chip->volume[0] = 15;
        }
    }
    if (chip->reg_reset || (chip->write_flag_l && chip->latch == 3))
    {
        chip->volume[1] = data & 15;
        if (chip->data_mask)
        {
            chip->volume[1] = 15;
        }
    }
    if (chip->reg_reset || (chip->write_flag_l && chip->latch == 4))
    {
        if ((data & 128) || chip->reg_reset)
        {
            chip->freq[2] &= 1008;
            chip->freq[2] |= data & 15;
        }
        if (!(data & 128))
        {
            chip->freq[2] &= 15;
            chip->freq[2] |= (data << 4) & 1008;
        }
    }
    if (chip->reg_reset || (chip->write_flag_l && chip->latch == 2))
    {
        if ((data & 128) || chip->reg_reset)
        {
            chip->freq[1] &= 1008;
            chip->freq[1] |= data & 15;
        }
        if (!(data & 128))
        {
            chip->freq[1] &= 15;
            chip->freq[1] |= (data << 4) & 1008;
        }
    }
    if (chip->reg_reset || (chip->write_flag_l && chip->latch == 5))
    {
        chip->volume[2] = data & 15;
        if (chip->data_mask)
        {
            chip->volume[2] = 15;
        }
    }
    if (chip->reg_reset || (chip->write_flag_l && chip->latch == 0))
    {
        if ((data & 128) || chip->reg_reset)
        {
            chip->freq[0] &= 1008;
            chip->freq[0] |= data & 15;
        }
        if (!(data & 128))
        {
            chip->freq[0] &= 15;
            chip->freq[0] |= (data << 4) & 1008;
        }
    }
    if (chip->reg_reset || (chip->write_flag_l && chip->latch == 7))
    {
        chip->volume[3] = data & 15;
        if (chip->data_mask)
        {
            chip->volume[3] = 15;
        }
    }
    if (chip->reg_reset || (chip->write_flag_l && chip->latch == 6))
    {
        chip->noise_data = data & 7;
        chip->noise_trig = 1;
    }
}

static void YMPSG_ClockInternal1(ympsg_t *chip)
{
    uint16_t freq = 0;
    uint8_t chan_sel = chip->chan_sel;
    uint8_t noise_of, noise_bit1, noise_bit2, noise_next;
    if ((chip->noise_data & 3) == 3)
    {
        noise_of = (chip->sign >> 1) & 1;
    }
    else
    {
        noise_of = chip->sign & 1;
    }
    if (chip->noise_trig_l || (chip->ic_latch2 & 1))
    {
        chip->noise = 0;
    }
    else if (noise_of && !chip->noise_of)
    {
        noise_bit1 = (chip->noise >> chip->noise_tap2) & 1;
        noise_bit2 = (chip->noise >> 12) & 1;
        noise_bit1 ^= noise_bit2;
        noise_next = ((noise_bit1 && ((chip->noise_data >> 2) & 1)) || ((chip->noise & chip->noise_size) == 0));
        chip->noise <<= 1;
        chip->noise |= noise_next;
    }
    chip->noise_of = noise_of;
    if (chip->ic_latch2 & 2)
    {
        chan_sel = 0;
    }
    if (chip->chan_sel & 1)
    {
        freq |= chip->freq[0];
    }
    if (chip->chan_sel & 2)
    {
        freq |= chip->freq[1];
    }
    if (chip->chan_sel & 4)
    {
        freq |= chip->freq[2];
    }
    if (chip->chan_sel & 8)
    {
        if ((chip->noise_data & 3) == 0)
        {
            freq |= 16;
        }
        if ((chip->noise_data & 3) == 1)
        {
            freq |= 32;
        }
        if ((chip->noise_data & 3) == 2)
        {
            freq |= 64;
        }
    }
    if (chip->chan_sel & 1)
    {
        chip->sign ^= chip->counter_of & 15;
    }
    if (chip->ic_latch2 & 2)
    {
        chip->sign = 0;
    }
    chip->counter_of <<= 1;
    if (chip->counter[chip->rot] >= freq)
    {
        chip->counter_of |= 1;
        chip->counter[chip->rot] = 0;
    }
    if (chip->ic_latch2 & 2)
    {
        chip->counter[chip->rot] = 0;
    }
    chip->counter[chip->rot]++;
    chip->counter[chip->rot] &= 1023;
    if ((chip->ic_latch2 & 1) || (chip->chan_sel & 7) != 0)
    {
        chip->chan_sel <<= 1;
    }
    else
    {
        chip->chan_sel <<= 1;
        chip->chan_sel |= 1;
    }
    chip->ic_latch2 <<= 1;
    chip->ic_latch2 |= chip->ic & 1;
    chip->noise_trig_l = chip->noise_trig;
}

static void YMPSG_ClockInternal2(ympsg_t *chip)
{
    chip->data_mask = (chip->ic_latch2 >> 1) & 1;
    chip->reg_reset = (chip->ic_latch2 >> 0) & 1;
    if (chip->noise_trig_l)
    {
        chip->noise_trig = 0;
    }
    YMPSG_UpdateRegisters(chip);

    chip->rot = (chip->rot + 1) & 3;
    chip->sign_l = chip->sign;
    chip->noise_sign_l = (chip->noise >> 14) & 1;
}

static void YMPSG_UpdateSample(ympsg_t *chip)
{
    uint32_t i;
    uint8_t sign = chip->sign & 14;
    sign |= chip->noise_sign_l;
    if (chip->test & 1)
    {
        sign |= 15;
    }
    for (i = 0; i < 4; i++)
    {
        if ((sign >> (3 - i)) & 1)
        {
            chip->volume_out[i] = chip->volume[i];
        }
        else
        {
            chip->volume_out[i] = 15;
        }
    }
}

void YMPSG_Write(ympsg_t *chip, uint8_t data)
{
    chip->data = data;
    chip->write_flag = 1;
}

uint16_t YMPSG_Read(ympsg_t *chip)
{
    uint16_t data = 0;
    uint32_t i;
    YMPSG_UpdateSample(chip);
    for (i = 0; i < 4; i++)
    {
        data |= chip->volume_out[i] << ((3 - i) * 4);
    }
    return data;
}

void YMPSG_Init(ympsg_t *chip, uint8_t real_sn)
{
    uint32_t i;
    memset(chip, 0, sizeof(ympsg_t));
    YMPSG_SetIC(chip, 1);
    chip->noise_tap2 = real_sn ? 13 : 15;
    chip->noise_size = real_sn ? 16383 : 32767;
    for (i = 0; i < 16; i++)
    {
        YMPSG_Clock(chip);
    }
    YMPSG_SetIC(chip, 0);
}

void YMPSG_SetIC(ympsg_t *chip, uint32_t ic)
{
    chip->ic = (uint8_t)ic;
}

void YMPSG_Clock(ympsg_t *chip)
{
    uint8_t prescaler2_latch;
    prescaler2_latch = chip->prescaler_2;
    chip->prescaler_2 += chip->prescaler_1;
    chip->prescaler_2 &= 1;
    chip->prescaler_1 ^= 1;
    if ((chip->ic_latch1 & 3) == 2)
    {
        chip->prescaler_1 = 0;
        chip->prescaler_2 = 0;
    }
    chip->ic_latch1 <<= 1;
    chip->ic_latch1 |= chip->ic & 1;
    YMPSG_UpdateRegisters(chip);
    chip->write_flag_l = 0;
    if (chip->write_flag)
    {
        YMPSG_WriteLatch(chip);
        chip->write_flag = 0;
        chip->write_flag_l = 1;
    }
    if (chip->prescaler_1)
    {
        if (!prescaler2_latch)
        {
            YMPSG_ClockInternal1(chip);
        }
        else
        {
            YMPSG_ClockInternal2(chip);
        }
    }
}

float YMPSG_GetOutput(ympsg_t *chip)
{
    float sample = 0.f;
    uint32_t i;
    YMPSG_UpdateSample(chip);
    if (chip->test & 1)
    {
        sample += ympsg_vol[chip->volume_out[chip->test >> 1]];
        sample += ympsg_vol[16] * 3.f;
    }
    else if (!chip->mute)
    {
        sample += ympsg_vol[chip->volume_out[0]];
        sample += ympsg_vol[chip->volume_out[1]];
        sample += ympsg_vol[chip->volume_out[2]];
        sample += ympsg_vol[chip->volume_out[3]];
    }
    else
    {
        for (i = 0; i < 4; i++)
        {
            if (!((chip->mute>>i) & 1))
                sample += ympsg_vol[chip->volume_out[i]];
        }
    }
    return sample;
}

void YMPSG_Test(ympsg_t *chip, uint16_t test)
{
    chip->test = (test >> 9) & 7;
}


void YMPSG_Generate(ympsg_t *chip, int32_t *buf)
{
    uint32_t i;
    float out;

    for (i = 0; i < 16; i++)
    {
        YMPSG_Clock(chip);

        while (chip->writebuf[chip->writebuf_cur].time <= chip->writebuf_samplecnt)
        {
            if (!chip->writebuf[chip->writebuf_cur].stat)
            {
                break;
            }
            chip->writebuf[chip->writebuf_cur].stat = 0;
            YMPSG_Write(chip, chip->writebuf[chip->writebuf_cur].data);
            chip->writebuf_cur = (chip->writebuf_cur + 1) % YMPSG_WRITEBUF_SIZE;
        }
        chip->writebuf_samplecnt++;
    }
    out = YMPSG_GetOutput(chip);
    *buf = (int32_t)(out * 8192.f);
}

void YMPSG_WriteBuffered(ympsg_t *chip, uint8_t data)
{
    uint64_t time1, time2;
    uint64_t skip;

    if (chip->writebuf[chip->writebuf_last].stat)
    {
        YMPSG_Write(chip, chip->writebuf[chip->writebuf_last].data);

        chip->writebuf_cur = (chip->writebuf_last + 1) % YMPSG_WRITEBUF_SIZE;
        skip = chip->writebuf[chip->writebuf_last].time - chip->writebuf_samplecnt;
        chip->writebuf_samplecnt = chip->writebuf[chip->writebuf_last].time;
        while (skip--)
        {
            YMPSG_Clock(chip);
        }
    }

    chip->writebuf[chip->writebuf_last].stat = 1;
    chip->writebuf[chip->writebuf_last].data = data;
    time1 = chip->writebuf_lasttime + YMPSG_WRITEBUF_DELAY;
    time2 = chip->writebuf_samplecnt;

    if (time1 < time2)
    {
        time1 = time2;
    }

    chip->writebuf[chip->writebuf_last].time = time1;
    chip->writebuf_lasttime = time1;
    chip->writebuf_last = (chip->writebuf_last + 1) % YMPSG_WRITEBUF_SIZE;
}

void YMPSG_SetMute(ympsg_t *chip, uint8_t mute)
{
    chip->mute = mute;
}
