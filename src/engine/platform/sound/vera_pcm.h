// Commander X16 Emulator
// Copyright (c) 2020 Frank van den Hoef
// All rights reserved. License: 2-clause BSD

#pragma once

#include <stdint.h>
#include <stdbool.h>

struct VERA_PCM {
  uint8_t fifo[4096 - 1]; // Actual hardware FIFO is 4kB, but you can only use 4095 bytes.
  unsigned fifo_wridx;
  unsigned fifo_rdidx;
  unsigned fifo_cnt;
  
  uint8_t ctrl;
  uint8_t rate;
  
  int16_t cur_l, cur_r;
  uint8_t phase;
};


void    pcm_reset(struct VERA_PCM* pcm);
void    pcm_write_ctrl(struct VERA_PCM* pcm, uint8_t val);
uint8_t pcm_read_ctrl(struct VERA_PCM* pcm);
void    pcm_write_rate(struct VERA_PCM* pcm, uint8_t val);
uint8_t pcm_read_rate(struct VERA_PCM* pcm);
void    pcm_write_fifo(struct VERA_PCM* pcm, uint8_t val);
void    pcm_render(struct VERA_PCM* pcm, int16_t* buf_l, int16_t* buf_r, unsigned num_samples);
bool    pcm_is_fifo_almost_empty(struct VERA_PCM* pcm);
