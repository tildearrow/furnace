#include "upd1771c.h"

#include <string.h>

/*
  Each of the 8 waveforms have been extracted from the uPD1771c-017 internal
  ROM, from offset 0x1fd (start of first waveform) to offset 0x2fc (end of
  last waveform).
  (note: given test mode dumping offset non-clarity it may be 0x200-0x2ff)
  The waveforms are stored in an 8-bit sign-magnitude format, so if in ROM the
  upper bit is 0x80, invert the lower 7 bits to get the 2's complement result
  seen here.
  Note that only the last 4 waveforms appear to have been intended for use as
  waveforms; the first four look as if they're playing back a piece of code as
  wave data.
*/
const signed char WAVEFORMS[8][32]={
{   0,   0,-123,-123, -61, -23, 125, 107,  94,  83,-128,-128,-128,   0,   0,   0,   1,   1,   1,   1,   0,   0,   0,-128,-128,-128,   0,   0,   0,   0,   0,   0},
{  37,  16,  32, -21,  32,  52,   4,   4,  33,  18,  60,  56,   0,   8,   5,  16,  65,  19,  69,  16,  -2,  19,  37,  16,  97,  19,   0,  87, 127,  -3,   1,   2},
{   0,   8,   1,  52,   4,   0,   0,  77,  81,-109,  47,  97, -83,-109,  38,  97,   0,  52,   4,   0,   1,   4,   1,  22,   2, -46,  33,  97,   0,   8, -85, -99},
{  47,  97,  40,  97,  -3,  25,  64,  17,   0,  52,  12,   5,  12,   5,  12,   5,  12,   5,  12,   5,   8,   4,-114,  19,   0,  52,-122,  21,   2,   5,   0,   8},
{ -52, -96,-118,-128,-111, -74, -37,  -5,  31,  62,  89, 112, 127, 125, 115,  93,  57,  23,   0, -16,  -8,  15,  37,  54,  65,  70,  62,  54,  43,  31,  19,   0},
{ -81,-128, -61,  13,  65,  93, 127,  47,  41,  44,  52,  55,  56,  58,  58,  34,   0,  68,  76,  72,  61, 108,  55,  29,  32,  39,  43,  49,  50,  51,  51,   0},
{ -21, -45, -67, -88,-105,-114,-122,-128,-123,-116,-103, -87, -70, -53, -28,  -9,  22,  46,  67,  86, 102, 114, 123, 125, 127, 117, 104,  91,  72,  51,  28,   0},
{ -78,-118,-128,-102, -54,  -3,  40,  65,  84,  88,  84,  80,  82,  88,  94, 103, 110, 119, 122, 125, 122, 122, 121, 123, 125, 126, 127, 127, 125, 118,  82,   0}
};


#define NOISE_SIZE 255


const unsigned char noise_tbl[]=
{
	0x1c,0x86,0x8a,0x8f,0x98,0xa1,0xad,0xbe,0xd9,0x8a,0x66,0x4d,0x40,0x33,0x2b,0x23,
	0x1e,0x8a,0x90,0x97,0xa4,0xae,0xb8,0xd6,0xec,0xe9,0x69,0x4a,0x3e,0x34,0x2d,0x27,
	0x24,0x24,0x89,0x8e,0x93,0x9c,0xa5,0xb0,0xc1,0xdd,0x40,0x36,0x30,0x29,0x27,0x24,
	0x8b,0x90,0x96,0x9e,0xa7,0xb3,0xc4,0xe1,0x25,0x21,0x8a,0x8f,0x93,0x9d,0xa5,0xb2,
	0xc2,0xdd,0xdd,0x98,0xa2,0xaf,0xbf,0xd8,0xfd,0x65,0x4a,0x3c,0x31,0x2b,0x24,0x22,
	0x1e,0x87,0x8c,0x91,0x9a,0xa3,0xaf,0xc0,0xdb,0xbe,0xd9,0x8c,0x66,0x4d,0x40,0x34,
	0x2c,0x24,0x1f,0x88,0x90,0x9a,0xa4,0xb2,0xc2,0xda,0xff,0x67,0x4d,0x3d,0x34,0x2d,
	0x26,0x24,0x20,0x89,0x8e,0x93,0x9c,0xa5,0xb1,0xc2,0xde,0xc1,0xda,0xff,0x67,0x4d,
	0x3d,0x33,0x2d,0x26,0x24,0x20,0x89,0x8e,0x93,0x9c,0xa5,0xb1,0xc2,0xdd,0xa3,0xb0,
	0xc0,0xd9,0xfe,0x66,0x4b,0x3c,0x32,0x2b,0x24,0x23,0x1e,0x88,0x8d,0x92,0x9b,0xa4,
	0xb0,0xc1,0xdc,0xad,0xbe,0xda,0x22,0x20,0x1c,0x85,0x8a,0x8f,0x98,0xa1,0xad,0xbe,
	0xda,0x20,0x1b,0x85,0x8d,0x97,0xa1,0xaf,0xbf,0xd8,0xfd,0x64,0x49,0x3a,0x30,0x2a,
	0x23,0x21,0x1d,0x86,0x8b,0x91,0x9a,0xa2,0xae,0xc0,0xdb,0x33,0x2b,0x24,0x1f,0x88,
	0x90,0x9a,0xa4,0xb2,0xc2,0xda,0xff,0x67,0x4c,0x3e,0x33,0x2d,0x25,0x24,0x1f,0x89,
	0x8e,0x93,0x9c,0xa5,0xb1,0xc2,0xde,0x85,0x8e,0x98,0xa2,0xb0,0xc0,0xd9,0xfe,0x64,
	0x4b,0x3b,0x31,0x2a,0x23,0x22,0x1e,0x88,0x8c,0x91,0x9b,0xa3,0xaf,0xc1,0xdc,0xdc
};


uint8_t upd1771c_packets[16];
uint8_t upd1771c_mode;
uint32_t upd1771c_pos;
uint8_t upd1771c_off;
uint8_t upd1771c_posc;
uint8_t upd1771c_wave;
uint8_t upd1771c_vol;
uint8_t upd1771c_period;
uint8_t upd1771c_npos;
void upd1771c_reset() {
    memset(upd1771c_packets,0,16);
    upd1771c_mode = 0;
    upd1771c_pos = 0;
    upd1771c_posc = 0;
    upd1771c_wave = 0;
    upd1771c_vol = 0;
    upd1771c_period = 0;
    upd1771c_off = 0;
    upd1771c_npos = 0;
}

void upd1771c_write_packet(uint8_t ind, uint8_t val) {
    upd1771c_packets[ind&15] = val;
    switch (upd1771c_packets[0]) {
        case 1:
            if (ind == 3) {
                upd1771c_mode = 1;
                upd1771c_wave = (upd1771c_packets[1]&0xe0)>>5;
                upd1771c_off = 0; //?
                upd1771c_period = upd1771c_packets[2];
                upd1771c_vol = upd1771c_packets[3]&0x1f;
            }
            break;
        case 2:
            if (ind == 3) {
                upd1771c_mode = 2;
                upd1771c_wave = (upd1771c_packets[1]&0xe0)>>5;
                upd1771c_off = upd1771c_packets[1]&0x1f;
                upd1771c_period = upd1771c_packets[2]<0x20?0x20:upd1771c_packets[2];
                upd1771c_vol = upd1771c_packets[3]&0x1f;
            }
            break;
        default:
        case 0:
            upd1771c_mode = 0;
            break;
    }
}

int upd1771c_repsamp = 0;

void upd1771c_sound_set_clock(unsigned int clock, unsigned int divi) {
    upd1771c_repsamp = divi;
}

int16_t upd1771c_sound_stream_update() {
    int16_t s = 0;
    for (int i = 0; i < upd1771c_repsamp; i++) {
        s = 0;
        switch (upd1771c_mode) {
            case 2:
                s = ((int16_t)WAVEFORMS[upd1771c_wave][upd1771c_posc])*upd1771c_vol;
                upd1771c_pos++;
                if (upd1771c_pos >= upd1771c_period) {
                    upd1771c_pos=0;
                    upd1771c_posc++;
                    if (upd1771c_posc == 32)
                        upd1771c_posc = upd1771c_off;
                }
                break;
            case 1:
                upd1771c_pos++;
                if (upd1771c_pos >= ((((uint32_t)upd1771c_period) + 1)*128)) {
                    upd1771c_pos=0;
                    upd1771c_posc++;
                    if (upd1771c_posc == NOISE_SIZE)
                        upd1771c_posc = 0;
                }
                s = ((int16_t)(noise_tbl[upd1771c_posc]))*upd1771c_vol;
                // inaccurate noise mixing :/
                // s |= (upd1771c_npos&128)?127*upd1771c_vol:0;
                break;
            case 0:
            default:
                break;
        }
    }
    return s;
}

