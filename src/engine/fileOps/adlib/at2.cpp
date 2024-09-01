/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2024 tildearrow and contributors
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

//Adlib Tracker 2 modules import (.a2m and .a2t) by SudoMaker & Enet4
//Adapted into Furnace by LTVA

#include "../fileOpsCommon.h"

#include "deps/depack.h"
#include "deps/sixpack.h"
#include "deps/unlzh.h"
#include "deps/unlzss.h"
#include "deps/unlzw.h"

#ifdef HAVE_MOMO
#define ngettext momo_ngettext
#endif

// Macros for extracting little-endian integers from filedata
#define INT16LE(A) (int16_t)((A[0]) | (A[1] << 8))
#define UINT16LE(A) (uint16_t)((A[0]) | (A[1] << 8))
#define INT32LE(A) (int32_t)((A[0]) | (A[1] << 8) | (A[2] << 16) | (A[3] << 24))
#define UINT32LE(A) (uint32_t)((A[0]) | (A[1] << 8) | (A[2] << 16) | (A[3] << 24))

#define keyoff_flag         0x80
#define fixed_note_flag     0x90
#define pattern_loop_flag   0xe0
#define pattern_break_flag  0xf0

typedef enum {
    isPlaying = 0, isPaused, isStopped
} tPLAY_STATUS;

#define BYTE_NULL (uint8_t)(0xFFFFFFFF)

#define MIN_IRQ_FREQ        50
#define MAX_IRQ_FREQ        1000

/*
    When loading A2T/A2M, FreePascal structures (no padding and little-endian) should be emulated,
    because AdlibTracker 2 was saving structures directly from memory into the file.

    That's why:
    1) only chars are used in structs to avoid any padding or alignment (default C/C++ behaviour)
    2) ints and longs are represented as arrays of chars, little-endian order is implied
    3) static_assert is used to make sure structs have the correct size
*/

typedef struct {
    union {
        struct {
            uint8_t multipM: 4, ksrM: 1, sustM: 1, vibrM: 1, tremM : 1;
            uint8_t multipC: 4, ksrC: 1, sustC: 1, vibrC: 1, tremC : 1;
            uint8_t volM: 6, kslM: 2;
            uint8_t volC: 6, kslC: 2;
            uint8_t decM: 4, attckM: 4;
            uint8_t decC: 4, attckC: 4;
            uint8_t relM: 4, sustnM: 4;
            uint8_t relC: 4, sustnC: 4;
            uint8_t wformM: 3, : 5;
            uint8_t wformC: 3, : 5;
            uint8_t connect: 1, feedb: 3, : 4; // panning is not used here
        };
        uint8_t data[11];
    };
} tFM_INST_DATA;

static_assert(sizeof(tFM_INST_DATA) == 11, "sizeof(tFM_INST_DATA) != 11");

typedef struct {
    tFM_INST_DATA fm;
    uint8_t panning;
    int8_t  fine_tune;
    uint8_t perc_voice;
} tINSTR_DATA;

static_assert(sizeof(tINSTR_DATA) == 14, "sizeof(tINSTR_DATA) != 14");

typedef struct {
    uint8_t length;
    uint8_t speed;
    uint8_t loop_begin;
    uint8_t loop_length;
    uint8_t keyoff_pos;
    uint8_t data[255];
} tARPEGGIO_TABLE;

typedef struct {
    uint8_t length;
    uint8_t speed;
    uint8_t delay;
    uint8_t loop_begin;
    uint8_t loop_length;
    uint8_t keyoff_pos;
    int8_t data[255]; // array[1..255] of Shortint;
} tVIBRATO_TABLE;

typedef struct {
    tFM_INST_DATA fm;
    uint8_t freq_slide[2]; // int16_t
    uint8_t panning;
    uint8_t duration;
} tREGISTER_TABLE_DEF;

typedef struct {
    uint8_t length;
    uint8_t loop_begin;
    uint8_t loop_length;
    uint8_t keyoff_pos;
    uint8_t arpeggio_table;
    uint8_t vibrato_table;
    tREGISTER_TABLE_DEF data[255];
} tFMREG_TABLE;

typedef struct {
    tARPEGGIO_TABLE arpeggio;
    tVIBRATO_TABLE vibrato;
} tARPVIB_TABLE;

static_assert(sizeof(tFMREG_TABLE) == 3831, "sizeof(tFMREG_TABLE) != 3831");
static_assert(sizeof(tARPVIB_TABLE) == 521, "sizeof(tARPVIB_TABLE) != 521");

typedef struct {
    uint8_t note;
    uint8_t instr_def; // TODO: rename to 'ins'
    struct {
        uint8_t def;
        uint8_t val;
    } eff[2];
} tADTRACK2_EVENT;

static_assert(sizeof(tADTRACK2_EVENT) == 6, "sizeof(tADTRACK2_EVENT) != 6");

typedef struct {
    struct {
        struct {
            tADTRACK2_EVENT ev;
        } row[256];
    } ch[20];
} tPATTERN_DATA;

static_assert(sizeof(tPATTERN_DATA) == 20 * 256 * 6, "sizeof(tPATTERN_DATA) != 30720");

#define ef_Arpeggio            0
#define ef_FSlideUp            1
#define ef_FSlideDown          2
#define ef_TonePortamento      3
#define ef_Vibrato             4
#define ef_TPortamVolSlide     5
#define ef_VibratoVolSlide     6
#define ef_FSlideUpFine        7
#define ef_FSlideDownFine      8
#define ef_SetModulatorVol     9
#define ef_VolSlide            10
#define ef_PositionJump        11
#define ef_SetInsVolume        12
#define ef_PatternBreak        13
#define ef_SetTempo            14
#define ef_SetSpeed            15
#define ef_TPortamVSlideFine   16
#define ef_VibratoVSlideFine   17
#define ef_SetCarrierVol       18
#define ef_SetWaveform         19
#define ef_VolSlideFine        20
#define ef_RetrigNote          21
#define ef_Tremolo             22
#define ef_Tremor              23
#define ef_ArpggVSlide         24
#define ef_ArpggVSlideFine     25
#define ef_MultiRetrigNote     26
#define ef_FSlideUpVSlide      27
#define ef_FSlideDownVSlide    28
#define ef_FSlUpFineVSlide     29
#define ef_FSlDownFineVSlide   30
#define ef_FSlUpVSlF           31
#define ef_FSlDownVSlF         32
#define ef_FSlUpFineVSlF       33
#define ef_FSlDownFineVSlF     34
#define ef_Extended            35
#define ef_Extended2           36
#define ef_SetGlobalVolume     37
#define ef_SwapArpeggio        38
#define ef_SwapVibrato         39
#define ef_ForceInsVolume      40
#define ef_Extended3           41
#define ef_ExtraFineArpeggio   42
#define ef_ExtraFineVibrato    43
#define ef_ExtraFineTremolo    44
#define ef_SetCustomSpeedTab   45
#define ef_GlobalFSlideUp      46
#define ef_GlobalFSlideDown    47
#define ef_GlobalFreqSlideUpXF 48 // ef_fix2 replacement for >xx + ZFE
#define ef_GlobalFreqSlideDnXF 49 // ef_fix2 replacement for <xx + ZFE

#define ef_ex_SetTremDepth     0
#define ef_ex_SetVibDepth      1
#define ef_ex_SetAttckRateM    2
#define ef_ex_SetDecayRateM    3
#define ef_ex_SetSustnLevelM   4
#define ef_ex_SetRelRateM      5
#define ef_ex_SetAttckRateC    6
#define ef_ex_SetDecayRateC    7
#define ef_ex_SetSustnLevelC   8
#define ef_ex_SetRelRateC      9
#define ef_ex_SetFeedback      10
#define ef_ex_SetPanningPos    11
#define ef_ex_PatternLoop      12
#define ef_ex_PatternLoopRec   13
#define ef_ex_ExtendedCmd      14
#define ef_ex_cmd_MKOffLoopDi  0
#define ef_ex_cmd_MKOffLoopEn  1
#define ef_ex_cmd_TPortaFKdis  2
#define ef_ex_cmd_TPortaFKenb  3
#define ef_ex_cmd_RestartEnv   4
#define ef_ex_cmd_4opVlockOff  5
#define ef_ex_cmd_4opVlockOn   6
#define ef_ex_cmd_ForceBpmSld  7
#define ef_ex_ExtendedCmd2     15
#define ef_ex_cmd2_RSS         0
#define ef_ex_cmd2_ResetVol    1
#define ef_ex_cmd2_LockVol     2
#define ef_ex_cmd2_UnlockVol   3
#define ef_ex_cmd2_LockVP      4
#define ef_ex_cmd2_UnlockVP    5
#define ef_ex_cmd2_VSlide_mod  6
#define ef_ex_cmd2_VSlide_car  7
#define ef_ex_cmd2_VSlide_def  8
#define ef_ex_cmd2_LockPan     9
#define ef_ex_cmd2_UnlockPan   10
#define ef_ex_cmd2_VibrOff     11
#define ef_ex_cmd2_TremOff     12
#define ef_ex_cmd2_FVib_FGFS   13
#define ef_ex_cmd2_FTrm_XFGFS  14
#define ef_ex_cmd2_NoRestart   15
#define ef_ex2_PatDelayFrame   0
#define ef_ex2_PatDelayRow     1
#define ef_ex2_NoteDelay       2
#define ef_ex2_NoteCut         3
#define ef_ex2_FineTuneUp      4
#define ef_ex2_FineTuneDown    5
#define ef_ex2_GlVolSlideUp    6
#define ef_ex2_GlVolSlideDn    7
#define ef_ex2_GlVolSlideUpF   8
#define ef_ex2_GlVolSlideDnF   9
#define ef_ex2_GlVolSldUpXF    10
#define ef_ex2_GlVolSldDnXF    11
#define ef_ex2_VolSlideUpXF    12
#define ef_ex2_VolSlideDnXF    13
#define ef_ex2_FreqSlideUpXF   14
#define ef_ex2_FreqSlideDnXF   15
#define ef_ex3_SetConnection   0
#define ef_ex3_SetMultipM      1
#define ef_ex3_SetKslM         2
#define ef_ex3_SetTremoloM     3
#define ef_ex3_SetVibratoM     4
#define ef_ex3_SetKsrM         5
#define ef_ex3_SetSustainM     6
#define ef_ex3_SetMultipC      7
#define ef_ex3_SetKslC         8
#define ef_ex3_SetTremoloC     9
#define ef_ex3_SetVibratoC     10
#define ef_ex3_SetKsrC         11
#define ef_ex3_SetSustainC     12

#define EFGR_ARPVOLSLIDE 1
#define EFGR_FSLIDEVOLSLIDE 2
#define EFGR_TONEPORTAMENTO 3
#define EFGR_VIBRATO 4
#define EFGR_TREMOLO 5
#define EFGR_VIBRATOVOLSLIDE 6
#define EFGR_PORTAVOLSLIDE 7
#define EFGR_RETRIGNOTE 8

/* Data for importing A2T format */
typedef struct {
    char id[15];	// '_a2tiny_module_'
    uint8_t crc[4]; // uint32_t
    uint8_t ffver;
    uint8_t npatt;
    uint8_t tempo;
    uint8_t speed;
} A2T_HEADER;

static_assert(sizeof(A2T_HEADER) == 23, "sizeof(A2T_HEADER) != 23");

typedef struct {
    char id[10];	// '_a2module_'
    uint8_t crc[4]; // uint32_t
    uint8_t ffver;
    uint8_t npatt;
} A2M_HEADER;

static_assert(sizeof(A2M_HEADER) == 16, "sizeof(A2M_HEADER) != 16");

typedef struct {
    uint8_t len[6][2]; // uint16_t
} A2T_VARHEADER_V1234;

typedef struct {
    uint8_t common_flag;
    uint8_t len[10][2]; // uint16_t
} A2T_VARHEADER_V5678;

typedef struct {
    uint8_t common_flag;
    uint8_t patt_len[2]; // uint16_t
    uint8_t nm_tracks;
    uint8_t macro_speedup[2]; // uint16_t
    uint8_t len[20][4]; // uint32_t
} A2T_VARHEADER_V9;

typedef struct {
    uint8_t common_flag;
    uint8_t patt_len[2]; // uint16_t
    uint8_t nm_tracks;
    uint8_t macro_speedup[2]; // uint16_t
    uint8_t flag_4op;
    uint8_t lock_flags[20];
    uint8_t len[20][4]; // uint32_t
} A2T_VARHEADER_V10;

typedef struct {
    uint8_t common_flag;
    uint8_t patt_len[2]; // uint16_t
    uint8_t nm_tracks;
    uint8_t macro_speedup[2]; // uint16_t
    uint8_t flag_4op;
    uint8_t lock_flags[20];
    uint8_t len[21][4]; // uint32_t
} A2T_VARHEADER_V11;

typedef union {
    A2T_VARHEADER_V1234 v1234;
    A2T_VARHEADER_V5678 v5678;
    A2T_VARHEADER_V9    v9;
    A2T_VARHEADER_V10   v10;
    A2T_VARHEADER_V11   v11;
} A2T_VARHEADER;

static_assert(sizeof(A2T_VARHEADER_V1234) == 12, "sizeof(A2T_VARHEADER_V1234) != 12");
static_assert(sizeof(A2T_VARHEADER_V5678) == 21, "sizeof(A2T_VARHEADER_V5678) != 21");
static_assert(sizeof(A2T_VARHEADER_V9) == 86, "sizeof(A2T_VARHEADER_V9) != 86");
static_assert(sizeof(A2T_VARHEADER_V10) == 107, "sizeof(A2T_VARHEADER_V10) != 107");
static_assert(sizeof(A2T_VARHEADER_V11) == 111, "sizeof(A2T_VARHEADER_V11) != 111");
static_assert(sizeof(A2T_VARHEADER) == 111, "sizeof(A2T_VARHEADER) != 111");

// only for importing v 1,2,3,4,5,6,7,8
typedef struct {
    uint8_t note;
    uint8_t instr_def;
    uint8_t effect_def;
    uint8_t effect;
} tADTRACK2_EVENT_V1234;

// for importing v 1,2,3,4 patterns
typedef struct {
    struct {
        struct {
            tADTRACK2_EVENT_V1234 ev;
        } ch[9];
    } row[64];
} tPATTERN_DATA_V1234;

// for importing v 5,6,7,8 patterns
typedef struct {
    struct {
        struct {
            tADTRACK2_EVENT_V1234 ev;
        } row[64];
    } ch[18];
} tPATTERN_DATA_V5678;

static_assert(sizeof(tADTRACK2_EVENT_V1234) == 4, "sizeof(tADTRACK2_EVENT_V1234) != 4");
static_assert(sizeof(tPATTERN_DATA_V1234) == 2304, "sizeof(tPATTERN_DATA_V1234) != 2304");
static_assert(sizeof(tPATTERN_DATA_V5678) == 4608, "sizeof(tPATTERN_DATA_V5678) != 4608");

// Old v1234 effects
enum {
    fx_Arpeggio          = 0x00,
    fx_FSlideUp          = 0x01,
    fx_FSlideDown        = 0x02,
    fx_FSlideUpFine      = 0x03,
    fx_FSlideDownFine    = 0x04,
    fx_TonePortamento    = 0x05,
    fx_TPortamVolSlide   = 0x06,
    fx_Vibrato           = 0x07,
    fx_VibratoVolSlide   = 0x08,
    fx_SetOpIntensity    = 0x09,
    fx_SetInsVolume      = 0x0a,
    fx_PatternBreak      = 0x0b,
    fx_PatternJump       = 0x0c,
    fx_SetTempo          = 0x0d,
    fx_SetTimer          = 0x0e,
    fx_Extended          = 0x0f,
    fx_ex_DefAMdepth     = 0x00,
    fx_ex_DefVibDepth    = 0x01,
    fx_ex_DefWaveform    = 0x02,
    fx_ex_ManSlideUp     = 0x03,
    fx_ex_ManSlideDown   = 0x04,
    fx_ex_VSlideUp       = 0x05,
    fx_ex_VSlideDown     = 0x06,
    fx_ex_VSlideUpFine   = 0x07,
    fx_ex_VSlideDownFine = 0x08,
    fx_ex_RetrigNote     = 0x09,
    fx_ex_SetAttckRate   = 0x0a,
    fx_ex_SetDecayRate   = 0x0b,
    fx_ex_SetSustnLevel  = 0x0c,
    fx_ex_SetReleaseRate = 0x0d,
    fx_ex_SetFeedback    = 0x0e,
    fx_ex_ExtendedCmd    = 0x0f
};

/* Data for importing A2M format */
typedef struct {
    tFM_INST_DATA fm;
    uint8_t panning;
    int8_t  fine_tune;
} tINSTR_DATA_V1_8;

static_assert(sizeof(tINSTR_DATA_V1_8) == 13, "sizeof(tINSTR_DATA_V1_8) != 13");

typedef struct {
    char songname[43];
    char composer[43];
    char instr_names[250][33];
    tINSTR_DATA_V1_8 instr_data[250];
    uint8_t pattern_order[128];
    uint8_t tempo;
    uint8_t speed;
    uint8_t common_flag; // A2M_SONGDATA_V5678
} A2M_SONGDATA_V1_8;

static_assert(sizeof(A2M_SONGDATA_V1_8) == 11717, "sizeof(A2M_SONGDATA_V1_8) != 11717");

typedef struct {
    uint8_t num_4op;
    uint8_t idx_4op[128];
} tINS_4OP_FLAGS;

typedef uint8_t tRESERVED[1024];

typedef struct {
    uint8_t rows_per_beat;
    int8_t tempo_finetune[2]; // int16_t
} tBPM_DATA;

typedef struct {
    char songname[43];
    char composer[43];
    char instr_names[255][43];
    tINSTR_DATA instr_data[255];
    tFMREG_TABLE fmreg_table[255];
    tARPVIB_TABLE arpvib_table[255];
    uint8_t pattern_order[128];
    uint8_t tempo;
    uint8_t speed;
    uint8_t common_flag;
    uint8_t patt_len[2];           // uint16_t
    uint8_t nm_tracks;
    uint8_t macro_speedup[2];      // uint16_t
    uint8_t flag_4op;              // A2M_SONGDATA_V10
    uint8_t lock_flags[20];        // A2M_SONGDATA_V10
    char pattern_names[128][43];   // A2M_SONGDATA_V11
    bool dis_fmreg_col[255][28];   // A2M_SONGDATA_V11
    tINS_4OP_FLAGS ins_4op_flags;  // A2M_SONGDATA_V12_13
    tRESERVED reserved_data;       // A2M_SONGDATA_V12_13
    tBPM_DATA bpm_data;            // A2M_SONGDATA_V14
} A2M_SONGDATA_V9_14;

static_assert(sizeof(A2M_SONGDATA_V9_14) == 1138338, "sizeof(A2M_SONGDATA_V9_14) != 1138338");

/* Player data */

typedef struct {
    tINSTR_DATA instr_data;
    uint8_t vibrato;
    uint8_t arpeggio;
    tFMREG_TABLE *fmreg;
    uint32_t dis_fmreg_cols;
} tINSTR_DATA_EXT;

typedef struct {
    char            songname[43];        // pascal String[42];
    char            composer[43];        // pascal String[42];
    char            instr_names[255][43];// array[1..255] of String[42];
    uint8_t         pattern_order[0x80]; // array[0..0x7f] of Byte;
    uint8_t         tempo;
    uint8_t         speed;
    uint8_t         common_flag;
    uint16_t        patt_len;
    uint8_t         nm_tracks;
    uint16_t        macro_speedup;
    uint8_t         flag_4op;
    uint8_t         lock_flags[20];

    char pattern_names[128][43]; //Furnace addition
} tSONGINFO;

typedef struct {
    uint16_t freq;
    uint8_t speed;
} tPORTA_TABLE;

typedef struct {
    uint8_t state, note, add1, add2;
} tARPGG_TABLE;

typedef struct {
    int8_t pos;
    uint8_t volM, volC;
} tTREMOR_TABLE;

typedef struct {
    uint8_t pos, dir, speed, depth;
    bool fine;
} tVIBRTREM_TABLE;

typedef struct {
    uint8_t def, val;
} tEFFECT_TABLE;

typedef struct {
    uint16_t fmreg_pos,
         arpg_pos,
         vib_pos;
    uint8_t
         fmreg_duration,
         arpg_count,
         vib_count,
         vib_delay,
         fmreg_ins, // fmreg_table
         arpg_table,
         vib_table,
         arpg_note;
    bool vib_paused;
    uint16_t vib_freq;
} tCH_MACRO_TABLE;

typedef struct {
    tFM_INST_DATA fmpar_table[20]; // TODO: rename to 'fm'
    bool volume_lock[20];
    bool vol4op_lock[20];
    bool peak_lock[20];
    bool pan_lock[20];
    uint8_t modulator_vol[20];
    uint8_t carrier_vol[20];
    // note/instr_def - memorized across rows
    // effects - change each row
    tADTRACK2_EVENT event_table[20];
    uint8_t voice_table[20];
    uint16_t freq_table[20];
    uint16_t zero_fq_table[20];
    tEFFECT_TABLE effect_table[2][20];
    uint8_t fslide_table[2][20];
    tEFFECT_TABLE glfsld_table[2][20];
    tPORTA_TABLE porta_table[2][20];
    bool portaFK_table[20];
    tARPGG_TABLE arpgg_table[2][20];
    tVIBRTREM_TABLE vibr_table[2][20];
    tVIBRTREM_TABLE trem_table[2][20];
    uint8_t retrig_table[2][20];
    tTREMOR_TABLE tremor_table[2][20];
    uint8_t panning_table[20];
    tEFFECT_TABLE last_effect[2][20];
    uint8_t volslide_type[20];
    uint8_t notedel_table[20];
    uint8_t notecut_table[20];
    int8_t ftune_table[20];
    bool keyoff_loop[20];
    uint8_t loopbck_table[20];
    uint8_t loop_table[20][256];
    bool reset_chan[20];
    tCH_MACRO_TABLE macro_table[20];
} tCHDATA;

typedef struct {
    unsigned int count;
    size_t size;
    tINSTR_DATA_EXT *instruments;
} tINSTR_INFO;

typedef struct {
    int patterns, rows, channels;
    size_t size;
    tADTRACK2_EVENT *events;
} tEVENTS_INFO;

typedef struct _4op_data {
    uint32_t mode: 1, conn: 3, ch1: 4, ch2: 4, ins1: 8, ins2: 8;
} t4OP_DATA;

void a2t_depack(unsigned char *src, int srcsize, unsigned char *dst, int dstsize, int ffver)
{
    switch (ffver) {
    case 1:
    case 5: // sixpack
        sixdepak((unsigned short *)src, (unsigned char *)dst, srcsize, dstsize);
        break;
    case 2:
    case 6: // lzw
        LZW_decompress(src, dst, srcsize, dstsize);
        break;
    case 3:
    case 7: // lzss
        LZSS_decompress(src, dst, srcsize, dstsize);
        break;
    case 4:
    case 8: // unpacked
        if (dstsize <= srcsize)
            memcpy(dst, src, srcsize);
        break;
    case 9:
    case 10:
    case 11: // apack (aPlib)
        aP_depack(src, dst, srcsize, dstsize);
        break;
    case 12:
    case 13:
    case 14: // lzh
        LZH_decompress(src, dst, srcsize, dstsize);
        break;
    }
}

bool is_data_empty(char *data, unsigned int size)
{
    while (size--) {
        if (*(char *)data++)
            return false;
    }

    return true;
}

int findEmptyEffectSlot(short* data)
{
    for(int i = 0; i < DIV_MAX_EFFECTS; i++)
    {
        if(data[4 + i * 2] == -1)
        {
            return i;
        }
    }

    return -1;
}

void convertAT2effect(unsigned short at2Eff, short* data, int version)
{
    int eff = at2Eff >> 8;
    int param = at2Eff & 0xff;
    int paramUpperNibble = (at2Eff & 0xff) >> 4;
    int paramLowerNibble = at2Eff & 0xf;

    int emptyEffSlot = findEmptyEffectSlot(data);

    if(version < 5)
    {
        switch(at2Eff >> 8)
        {
            case fx_Arpeggio:
            {
                break;
            }
            
            default: break;
        }
        return;
    }

    switch(at2Eff >> 8)
    {
        case ef_Arpeggio:
        case ef_FSlideUp:
        case ef_FSlideDown:
        case ef_TonePortamento:
        case ef_Vibrato:
        case ef_VolSlide:
        case ef_PositionJump:
        case ef_PatternBreak:
        {
            data[4 + emptyEffSlot * 2] = eff;
            data[5 + emptyEffSlot * 2] = param;
            break;
        }
        case ef_TPortamVolSlide:
        {
            data[4 + emptyEffSlot * 2] = 0x06;
            data[5 + emptyEffSlot * 2] = param;
            break;
        }
        case ef_VibratoVolSlide:
        {
            data[4 + emptyEffSlot * 2] = 0x05;
            data[5 + emptyEffSlot * 2] = param;
            break;
        }
        case ef_FSlideUpFine:
        {
            data[4 + emptyEffSlot * 2] = 0xf1;
            data[5 + emptyEffSlot * 2] = param;
            break;
        }
        case ef_FSlideDownFine:
        {
            data[4 + emptyEffSlot * 2] = 0xf2;
            data[5 + emptyEffSlot * 2] = param;
            break;
        }
        case ef_SetModulatorVol:
        {
            data[4 + emptyEffSlot * 2] = 0x12;
            data[5 + emptyEffSlot * 2] = 63 - param;
            break;
        }
        case ef_SetInsVolume:
        {
            //data[4 + emptyEffSlot * 2] = 0x13;
            //data[5 + emptyEffSlot * 2] = 63 - param; //todo: adapt to volume column??

            data[3] = param;
            break;
        }
        case ef_SetTempo:
        {
            data[4 + emptyEffSlot * 2] = 0xC0;
            data[5 + emptyEffSlot * 2] = param;
            break;
        }
        case ef_SetSpeed:
        {
            data[4 + emptyEffSlot * 2] = 0x09;
            data[5 + emptyEffSlot * 2] = param;
            break;
        }
        case ef_TPortamVSlideFine:
        {
            //todo
            break;
        }
        case ef_VibratoVSlideFine:
        {
            //todo
            break;
        }
        case ef_SetCarrierVol:
        {
            data[4 + emptyEffSlot * 2] = 0x13;
            data[5 + emptyEffSlot * 2] = 63 - param;
            break;
        }
        case ef_SetWaveform:
        {
            if(paramUpperNibble != 0xf)
            {
                data[4 + emptyEffSlot * 2] = 0x2A;
                data[5 + emptyEffSlot * 2] = (2 << 4) | paramUpperNibble;
            }
            if(paramLowerNibble != 0xf)
            {
                emptyEffSlot = findEmptyEffectSlot(data);

                data[4 + emptyEffSlot * 2] = 0x2A;
                data[5 + emptyEffSlot * 2] = (1 << 4) | paramUpperNibble;
            }
            break;
        }
        case ef_VolSlideFine:
        {
            if(paramUpperNibble != 0)
            {
                data[4 + emptyEffSlot * 2] = 0xf3;
                data[5 + emptyEffSlot * 2] = paramUpperNibble;
                return;
            }
            if(paramLowerNibble != 0)
            {
                data[4 + emptyEffSlot * 2] = 0xf4;
                data[5 + emptyEffSlot * 2] = paramLowerNibble;
                return;
            }
            break;
        }
        case ef_RetrigNote:
        {
            data[4 + emptyEffSlot * 2] = 0x0C;
            data[5 + emptyEffSlot * 2] = param;
            break;
        }
        case ef_Tremolo:
        {
            data[4 + emptyEffSlot * 2] = 0x07;
            data[5 + emptyEffSlot * 2] = param;
            break;
        }
        case ef_Tremor:
        {
            //todo
            break;
        }
        case ef_ArpggVSlide:
        {
            data[4 + emptyEffSlot * 2] = 0x0A;
            data[5 + emptyEffSlot * 2] = param;

            //emptyEffSlot = findEmptyEffectSlot(data);

            //data[4 + emptyEffSlot * 2] = eff;
            //data[5 + emptyEffSlot * 2] = param;
            break;
        }
        case ef_ArpggVSlideFine:
        {
            if(paramUpperNibble != 0)
            {
                data[4 + emptyEffSlot * 2] = 0xf3;
                data[5 + emptyEffSlot * 2] = paramUpperNibble;
                return;
            }
            if(paramLowerNibble != 0)
            {
                data[4 + emptyEffSlot * 2] = 0xf4;
                data[5 + emptyEffSlot * 2] = paramLowerNibble;
                return;
            }

            //emptyEffSlot = findEmptyEffectSlot(data);

            //data[4 + emptyEffSlot * 2] = eff;
            //data[5 + emptyEffSlot * 2] = param;
            break;
        }
        case ef_MultiRetrigNote:
        {
            //todo
            break;
        }
        case ef_FSlideUpVSlide:
        {
            data[4 + emptyEffSlot * 2] = 0x0A;
            data[5 + emptyEffSlot * 2] = param;

            //emptyEffSlot = findEmptyEffectSlot(data);

            //data[4 + emptyEffSlot * 2] = eff;
            //data[5 + emptyEffSlot * 2] = param;
            break;
        }
        case ef_FSlideDownVSlide:
        {
            data[4 + emptyEffSlot * 2] = 0x0A;
            data[5 + emptyEffSlot * 2] = param;

            //emptyEffSlot = findEmptyEffectSlot(data);

            //data[4 + emptyEffSlot * 2] = eff;
            //data[5 + emptyEffSlot * 2] = param;
            break;
        }
        case ef_FSlUpFineVSlide:
        {
            if(paramUpperNibble != 0)
            {
                data[4 + emptyEffSlot * 2] = 0xf3;
                data[5 + emptyEffSlot * 2] = paramUpperNibble;
                return;
            }
            if(paramLowerNibble != 0)
            {
                data[4 + emptyEffSlot * 2] = 0xf4;
                data[5 + emptyEffSlot * 2] = paramLowerNibble;
                return;
            }

            //emptyEffSlot = findEmptyEffectSlot(data);

            //data[4 + emptyEffSlot * 2] = eff;
            //data[5 + emptyEffSlot * 2] = param;
            break;
        }
        case ef_FSlDownFineVSlide:
        {
            if(paramUpperNibble != 0)
            {
                data[4 + emptyEffSlot * 2] = 0xf3;
                data[5 + emptyEffSlot * 2] = paramUpperNibble;
                return;
            }
            if(paramLowerNibble != 0)
            {
                data[4 + emptyEffSlot * 2] = 0xf4;
                data[5 + emptyEffSlot * 2] = paramLowerNibble;
                return;
            }

            //emptyEffSlot = findEmptyEffectSlot(data);

            //data[4 + emptyEffSlot * 2] = eff;
            //data[5 + emptyEffSlot * 2] = param;
            break;
        }
        case ef_FSlUpVSlF:
        {
            if(paramUpperNibble != 0)
            {
                data[4 + emptyEffSlot * 2] = 0xf3;
                data[5 + emptyEffSlot * 2] = paramUpperNibble;
                return;
            }
            if(paramLowerNibble != 0)
            {
                data[4 + emptyEffSlot * 2] = 0xf4;
                data[5 + emptyEffSlot * 2] = paramLowerNibble;
                return;
            }

            //emptyEffSlot = findEmptyEffectSlot(data);

            //data[4 + emptyEffSlot * 2] = eff;
            //data[5 + emptyEffSlot * 2] = param;
            break;
        }
        case ef_FSlDownVSlF:
        {
            if(paramUpperNibble != 0)
            {
                data[4 + emptyEffSlot * 2] = 0xf3;
                data[5 + emptyEffSlot * 2] = paramUpperNibble;
                return;
            }
            if(paramLowerNibble != 0)
            {
                data[4 + emptyEffSlot * 2] = 0xf4;
                data[5 + emptyEffSlot * 2] = paramLowerNibble;
                return;
            }

            //emptyEffSlot = findEmptyEffectSlot(data);

            //data[4 + emptyEffSlot * 2] = eff;
            //data[5 + emptyEffSlot * 2] = param;
            break;
        }
        case ef_FSlUpFineVSlF:
        {
            if(paramUpperNibble != 0)
            {
                data[4 + emptyEffSlot * 2] = 0xf3;
                data[5 + emptyEffSlot * 2] = paramUpperNibble;
                return;
            }
            if(paramLowerNibble != 0)
            {
                data[4 + emptyEffSlot * 2] = 0xf4;
                data[5 + emptyEffSlot * 2] = paramLowerNibble;
                return;
            }

            //emptyEffSlot = findEmptyEffectSlot(data);

            //data[4 + emptyEffSlot * 2] = eff;
            //data[5 + emptyEffSlot * 2] = param;
            break;
        }
        case ef_FSlDownFineVSlF:
        {
            if(paramUpperNibble != 0)
            {
                data[4 + emptyEffSlot * 2] = 0xf3;
                data[5 + emptyEffSlot * 2] = paramUpperNibble;
                return;
            }
            if(paramLowerNibble != 0)
            {
                data[4 + emptyEffSlot * 2] = 0xf4;
                data[5 + emptyEffSlot * 2] = paramLowerNibble;
                return;
            }

            //emptyEffSlot = findEmptyEffectSlot(data);

            //data[4 + emptyEffSlot * 2] = eff;
            //data[5 + emptyEffSlot * 2] = param;
            break;
        }
        case ef_Extended:
        {
            switch(paramUpperNibble)
            {
                case ef_ex_SetTremDepth:
                {
                    //todo
                    break;
                }
                case ef_ex_SetVibDepth:
                {
                    data[4 + emptyEffSlot * 2] = 0xE4;
                    data[5 + emptyEffSlot * 2] = paramLowerNibble;
                    break;
                }
                case ef_ex_SetAttckRateM:
                {
                    data[4 + emptyEffSlot * 2] = 0x1A;
                    data[5 + emptyEffSlot * 2] = paramLowerNibble;
                    break;
                }
                case ef_ex_SetDecayRateM:
                {
                    data[4 + emptyEffSlot * 2] = 0x57;
                    data[5 + emptyEffSlot * 2] = paramLowerNibble;
                    break;
                }
                case ef_ex_SetSustnLevelM:
                {
                    data[4 + emptyEffSlot * 2] = 0x51;
                    data[5 + emptyEffSlot * 2] = (1 << 4) | paramLowerNibble;
                    break;
                }
                case ef_ex_SetRelRateM:
                {
                    data[4 + emptyEffSlot * 2] = 0x52;
                    data[5 + emptyEffSlot * 2] = (1 << 4) | paramLowerNibble;
                    break;
                }
                case ef_ex_SetAttckRateC:
                {
                    data[4 + emptyEffSlot * 2] = 0x1B;
                    data[5 + emptyEffSlot * 2] = paramLowerNibble;
                    break;
                }
                case ef_ex_SetDecayRateC:
                {
                    data[4 + emptyEffSlot * 2] = 0x58;
                    data[5 + emptyEffSlot * 2] = paramLowerNibble;
                    break;
                }
                case ef_ex_SetSustnLevelC:
                {
                    data[4 + emptyEffSlot * 2] = 0x51;
                    data[5 + emptyEffSlot * 2] = (2 << 4) | paramLowerNibble;
                    break;
                }
                case ef_ex_SetRelRateC:
                {
                    data[4 + emptyEffSlot * 2] = 0x52;
                    data[5 + emptyEffSlot * 2] = (2 << 4) | paramLowerNibble;
                    break;
                }
                case ef_ex_SetFeedback:
                {
                    data[4 + emptyEffSlot * 2] = 0x11;
                    data[5 + emptyEffSlot * 2] = paramLowerNibble;
                    break;
                }
                case ef_ex_SetPanningPos:
                {
                    data[4 + emptyEffSlot * 2] = 0x88;
                    
                    if(paramLowerNibble == 0)
                    {
                        data[5 + emptyEffSlot * 2] = 0x80;
                    }
                    if(paramLowerNibble == 1)
                    {
                        data[5 + emptyEffSlot * 2] = 0;
                    }
                    if(paramLowerNibble == 2)
                    {
                        data[5 + emptyEffSlot * 2] = 0xFF;
                    }
                    break;
                }
                case ef_ex_PatternLoop:
                {
                    //todo
                    break;
                }
                case ef_ex_PatternLoopRec:
                {
                    //todo
                    break;
                }
                case ef_ex_ExtendedCmd2:
                {
                    switch(paramLowerNibble)
                    {
                        case ef_ex_cmd2_RSS:
                        {
                            data[4 + emptyEffSlot * 2] = 0xEC; //todo check if true
                            data[5 + emptyEffSlot * 2] = 0;
                        }
                        default: break;
                    }
                }

                default: break;
            }
            break;
        }
        case ef_Extended2:
        {
            switch(paramUpperNibble)
            {
                case ef_ex2_NoteDelay:
                {
                    data[4 + emptyEffSlot * 2] = 0xED;
                    data[5 + emptyEffSlot * 2] = paramLowerNibble;
                    break;
                }
                case ef_ex2_NoteCut:
                {
                    data[4 + emptyEffSlot * 2] = 0xEC;
                    data[5 + emptyEffSlot * 2] = paramLowerNibble;
                    break;
                }
                default: break;
            }
        }
        case ef_Extended3:
        {
            switch(paramUpperNibble)
            {
                case ef_ex3_SetConnection:
                {
                    //todo
                    break;
                }
                case ef_ex3_SetMultipM:
                {
                    data[4 + emptyEffSlot * 2] = 0x16;
                    data[5 + emptyEffSlot * 2] = (1 << 4) | paramLowerNibble;
                    break;
                }
                case ef_ex3_SetKslM:
                {
                    data[4 + emptyEffSlot * 2] = 0x54;
                    data[5 + emptyEffSlot * 2] = (1 << 4) | paramLowerNibble;
                    break;
                }
                case ef_ex3_SetTremoloM:
                {
                    data[4 + emptyEffSlot * 2] = 0x50;
                    data[5 + emptyEffSlot * 2] = (1 << 4) | paramLowerNibble;
                    break;
                }
                case ef_ex3_SetVibratoM:
                {
                    data[4 + emptyEffSlot * 2] = 0x53;
                    data[5 + emptyEffSlot * 2] = (1 << 4) | paramLowerNibble;
                    break;
                }
                case ef_ex3_SetKsrM:
                {
                    data[4 + emptyEffSlot * 2] = 0x5B;
                    data[5 + emptyEffSlot * 2] = (1 << 4) | paramLowerNibble;
                    break;
                }
                case ef_ex3_SetSustainM:
                {
                    data[4 + emptyEffSlot * 2] = 0x55;
                    data[5 + emptyEffSlot * 2] = (1 << 4) | paramLowerNibble;
                    break;
                }
                case ef_ex3_SetMultipC:
                {
                    data[4 + emptyEffSlot * 2] = 0x16;
                    data[5 + emptyEffSlot * 2] = (2 << 4) | paramLowerNibble;
                    break;
                }
                case ef_ex3_SetKslC:
                {
                    data[4 + emptyEffSlot * 2] = 0x54;
                    data[5 + emptyEffSlot * 2] = (2 << 4) | paramLowerNibble;
                    break;
                }
                case ef_ex3_SetTremoloC:
                {
                    data[4 + emptyEffSlot * 2] = 0x50;
                    data[5 + emptyEffSlot * 2] = (2 << 4) | paramLowerNibble;
                    break;
                }
                case ef_ex3_SetVibratoC:
                {
                    data[4 + emptyEffSlot * 2] = 0x53;
                    data[5 + emptyEffSlot * 2] = (2 << 4) | paramLowerNibble;
                    break;
                }
                case ef_ex3_SetKsrC:
                {
                    data[4 + emptyEffSlot * 2] = 0x5B;
                    data[5 + emptyEffSlot * 2] = (2 << 4) | paramLowerNibble;
                    break;
                }
                case ef_ex3_SetSustainC:
                {
                    data[4 + emptyEffSlot * 2] = 0x55;
                    data[5 + emptyEffSlot * 2] = (2 << 4) | paramLowerNibble;
                    break;
                }
                default: break;
            }
        }

        default: break;
    }
}

bool AT2ReadPatterns(DivSubSong* s, SafeReader& reader, int version, unsigned int* len, int patterns, tSONGINFO& songInfo, int ssss)
{
    size_t posBegin = reader.tell();

    switch (version) 
    {
        case 1:
        case 2:
        case 3:
        case 4: // [4][16][64][9][4]
        {
            tPATTERN_DATA_V1234 *old = (tPATTERN_DATA_V1234 *)calloc(16, sizeof(*old));

            //memset(adsr_carrier, false, sizeof(adsr_carrier));

            for (int i = 0; i < 4; i++) 
            {
                if (!len[i+ssss]) continue;

                //if (len[i+ssss] > size) return INT_MAX;
                if (len[i+ssss] > reader.size() - reader.tell())
                {
                    free(old);
                    //lastError = "incomplete songdata";
                    //delete[] file;
                    return false;
                }

                posBegin = reader.tell();

                unsigned char* temp = new unsigned char[len[i+ssss]];
                reader.read((void*)temp, len[i+ssss]);
                a2t_depack(temp, len[i+ssss], (unsigned char *)old, 16 * sizeof (*old), version);
                delete[] temp;

                //a2t_depack(src, len[i+ssss], (char *)old, 16 * sizeof (*old), version);

                for (int p = 0; p < 16; p++) 
                { // pattern
                    if (i * 8 + p >= patterns)
                            break;
                    for (int r = 0; r < 64; r++) // row
                    for (int c = 0; c < 9; c++) 
                    { // channel
                        tADTRACK2_EVENT_V1234 *src = &old[p].row[r].ch[c].ev;
                        //tADTRACK2_EVENT *dst = get_event_p(i * 16 + p, c, r);

                        //convert_v1234_event(src, c);

                        DivPattern* pat = s->pat[c].getPattern(i * 16 + p, true);
                        uint8_t note = src->note;

                        if(note == 255)
                        {
                            pat->data[r][0]=101; //key off
                        }
                        else if(note <= 96 && note != 0)
                        {
                            note -= 1;

                            pat->data[r][0]=((note)%12);
                            pat->data[r][1]=(note)/12;

                            if(note % 12 == 0)
                            {
                                pat->data[r][0] = 12; //what the fuck?
                                pat->data[r][1]--;
                            }
                        }
                        
                        if(src->instr_def > 0 && src->instr_def < 129)
                        {
                            pat->data[r][2] = src->instr_def - 1; //instrument

                            if(pat->data[r][0] != -1)
                            {
                                pat->data[r][3] = 0x3f; //force max volume on each new note?
                            }
                        }

                        if(src->effect_def != 0 || (src->effect_def == 0 && src->effect != 0))
                        {
                            convertAT2effect(((unsigned short)src->effect_def << 8) | src->effect, &pat->data[r][0], version);

                            //pat->data[r][4] = src->effect_def;
                            //pat->data[r][5] = src->effect;
                        }

                        //dst->note = src->note;
                        //dst->instr_def = src->instr_def;
                        //dst->eff[0].def = src->effect_def;
                        //dst->eff[0].val = src->effect;
                    }
                }

                //src += len[i+s];
                reader.seek(posBegin + len[i+ssss], SEEK_SET);
                //size -= len[i+s];
                //retval += len[i+s];
            }

            free(old);
            break;
        }
        case 5:
        case 6:
        case 7:
        case 8: // [8][8][18][64][4]
        {
            tPATTERN_DATA_V5678 *old = (tPATTERN_DATA_V5678 *)calloc(8, sizeof(*old));

            for (int i = 0; i < 8; i++) {
                if (!len[i+ssss]) continue;

                if (len[i+ssss] > reader.size() - reader.tell())
                {
                    free(old);
                    //lastError = "incomplete songdata";
                    //delete[] file;
                    return false;
                }

                posBegin = reader.tell();

                unsigned char* temp = new unsigned char[len[i+ssss]];
                reader.read((void*)temp, len[i+ssss]);
                a2t_depack(temp, len[i+ssss], (unsigned char *)old, 8 * sizeof (*old), version);
                delete[] temp;
                //a2t_depack(src, len[i+ssss], (char *)old, 8 * sizeof (*old), version);

                for (int p = 0; p < 8; p++) { // pattern
                    if (i * 8 + p >= patterns)
                        break;
                    for (int c = 0; c < 18; c++) // channel
                    for (int r = 0; r < 64; r++) { // row
                        tADTRACK2_EVENT_V1234 *src = &old[p].ch[c].row[r].ev;
                        //tADTRACK2_EVENT *dst = get_event_p(i * 8 + p, c, r);

                        //dst->note = src->note;
                        //dst->instr_def = src->instr_def;
                        //dst->eff[0].def = src->effect_def;
                        //dst->eff[0].val = src->effect;

                        DivPattern* pat = s->pat[c].getPattern(i * 8 + p, true);
                        uint8_t note = src->note;

                        if(note == 255)
                        {
                            pat->data[r][0]=101; //key off
                        }
                        else if(note <= 96 && note != 0)
                        {
                            note -= 1;

                            pat->data[r][0]=((note)%12);
                            pat->data[r][1]=(note)/12;

                            if(note % 12 == 0)
                            {
                                pat->data[r][0] = 12; //what the fuck?
                                pat->data[r][1]--;
                            }
                        }
                        
                        if(src->instr_def > 0 && src->instr_def < 129)
                        {
                            pat->data[r][2] = src->instr_def - 1; //instrument

                            if(pat->data[r][0] != -1)
                            {
                                pat->data[r][3] = 0x3f; //force max volume on each new note?
                            }
                        }

                        if(src->effect_def != 0 || (src->effect_def == 0 && src->effect != 0))
                        {
                            convertAT2effect(((unsigned short)src->effect_def << 8) | src->effect, &pat->data[r][0], version);

                            //pat->data[r][4] = src->effect_def;
                            //pat->data[r][5] = src->effect;
                        }
                    }
                }

                //src += len[i+s];
                reader.seek(posBegin + len[i+ssss], SEEK_SET);
                //size -= len[i+s];
                //retval += len[i+s];
            }

            free(old);
            break;
        }
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14: // [16][8][20][256][6]
        {
            tPATTERN_DATA *old = (tPATTERN_DATA *)calloc(8, sizeof(*old));

            // 16 groups of 8 patterns
            for (int i = 0; i < 16; i++) 
            {
                if (!len[i+ssss]) continue;
                //if (len[i+ssss] > size) return INT_MAX;
                if (len[i+ssss] > reader.size() - reader.tell())
                {
                    free(old);
                    //lastError = "incomplete songdata";
                    //delete[] file;
                    return false;
                }

                posBegin = reader.tell();

                unsigned char* temp = new unsigned char[len[i+ssss]];
                reader.read((void*)temp, len[i+ssss]);
                a2t_depack(temp, len[i+ssss], (unsigned char *)old, 8 * sizeof (*old), version);
                delete[] temp;
                //a2t_depack(src, len[i+ssss], (char *)old, 8 * sizeof (*old), version);
                //src += len[i+ssss];
                //size -= len[i+ssss];
                reader.seek(posBegin + len[i+ssss], SEEK_SET);
                //retval += len[i+ssss];

                for (int p = 0; p < 8; p++) 
                { // pattern
                    if (i * 8 + p >= patterns)
                            break;

                    for (int c = 0; c < songInfo.nm_tracks; c++) // channel
                    for (int r = 0; r < songInfo.patt_len; r++) { // row
                        //tADTRACK2_EVENT *dst = get_event_p(i * 8 + p, c, r);
                        tADTRACK2_EVENT *src = &old[p].ch[c].row[r].ev;
                        //*dst = *src; // copy struct
                        DivPattern* pat = s->pat[c].getPattern(i * 8 + p, true);
                        uint8_t note = src->note;

                        if(note == 255)
                        {
                            pat->data[r][0]=101; //key off
                        }
                        else if(note <= 96 && note != 0)
                        {
                            note -= 1;

                            pat->data[r][0]=((note)%12);
                            pat->data[r][1]=(note)/12;

                            if(note % 12 == 0)
                            {
                                pat->data[r][0] = 12; //what the fuck?
                                pat->data[r][1]--;
                            }
                        }
                        
                        if(src->instr_def > 0 && src->instr_def < 129)
                        {
                            pat->data[r][2] = src->instr_def - 1; //instrument

                            if(pat->data[r][0] != -1)
                            {
                                pat->data[r][3] = 0x3f; //force max volume on each new note?
                            }
                        }

                        for(int effe = 0; effe < 2; effe++)
                        {
                            if(src->eff[effe].def != 0 || (src->eff[effe].def == 0 && src->eff[effe].val != 0))
                            {
                                convertAT2effect(((unsigned short)src->eff[effe].def << 8) | src->eff[effe].val, &pat->data[r][0], version);
                                //pat->data[r][4 + effe*2] = src->eff[effe].def;
                                //pat->data[r][5 + effe*2] = src->eff[effe].val;
                            }
                        }
                    }
                }
            }

            free(old);
            break;
        }
    }

    return true;
}

bool DivEngine::loadAT2(unsigned char* file, size_t len) 
{
    SafeReader reader=SafeReader(file,len);
    warnings="";

    try 
    {
        bool isA2t = false;

        if(memcmp(file,DIV_A2T_MAGIC, 14) == 0)
        {
            isA2t = true;
        }

        DivSong ds;
        ds.subsong.push_back(new DivSubSong);
        DivSubSong* s = ds.subsong[0];
        ds.systemLen = 1;
        ds.system[0] = DIV_SYSTEM_OPL3;

        int version = 0;

        unsigned int len[22] = { 0 };

        int patterns = 0;

        tSONGINFO songInfo;
        memset((void*)&songInfo, 0, sizeof(tSONGINFO));

        if(isA2t) //a2t
        {
            logI("a2t");

            A2T_HEADER header;
            unsigned char* hacky = (unsigned char*)&header;
            for(int i = 0; i < (int)sizeof(A2T_HEADER); i++)
            {
                hacky[i] = reader.readC();
            }

            logI("version %d", header.ffver);
            logI("tempo %d", header.tempo);
            logI("speed %d", header.speed);

            version = header.ffver;

            songInfo.patt_len = 64;
            songInfo.nm_tracks = 18;
            songInfo.tempo = header.tempo;
            songInfo.speed = header.speed;

            patterns = header.npatt;

            size_t posBegin = reader.tell();

            A2T_VARHEADER varheader;
            hacky = (unsigned char*)&varheader;
            for(int i = 0; i < (int)sizeof(A2T_VARHEADER); i++)
            {
                hacky[i] = reader.readC();
            }

            switch (version)
            {
                case 1:
                case 2:
                case 3:
                case 4:
                    //if (sizeof(A2T_VARHEADER_V1234) > size)
                        //return INT_MAX;
                    for (int i = 0; i < 6; i++)
                    {
                        len[i] = UINT16LE(varheader.v1234.len[i]);
                        //len[i] = UINT16LE(varheader->v1234.len[i]);
                    }

                    reader.seek(posBegin + sizeof(A2T_VARHEADER_V1234), SEEK_SET);
                    
                    break;
                    //return sizeof(A2T_VARHEADER_V1234);
                case 5:
                case 6:
                case 7:
                case 8:
                    //if (sizeof(A2T_VARHEADER_V5678) > size)
                        //return INT_MAX;
                    songInfo.common_flag = varheader.v5678.common_flag;
                    for (int i = 0; i < 10; i++)
                    {
                        len[i] = UINT16LE(varheader.v5678.len[i]);
                        //len[i] = UINT16LE(varheader->v5678.len[i]);
                    }

                    reader.seek(posBegin + sizeof(A2T_VARHEADER_V5678), SEEK_SET);
                    //return sizeof(A2T_VARHEADER_V5678);
                    break;
                case 9:
                    //if (sizeof(A2T_VARHEADER_V9) > size)
                        //return INT_MAX;
                    songInfo.common_flag = varheader.v9.common_flag;
                    songInfo.patt_len = UINT16LE(varheader.v9.patt_len);
                    songInfo.nm_tracks = varheader.v9.nm_tracks;
                    songInfo.macro_speedup = UINT16LE(varheader.v9.macro_speedup);
                    for (int i = 0; i < 20; i++)
                    {
                        len[i] = UINT32LE(varheader.v9.len[i]);
                        //len[i] = UINT32LE(varheader->v9.len[i]);
                    }

                    reader.seek(posBegin + sizeof(A2T_VARHEADER_V9), SEEK_SET);
                    //return sizeof(A2T_VARHEADER_V9);
                    break;
                case 10:
                    //if (sizeof(A2T_VARHEADER_V10) > size)
                        //return INT_MAX;
                    songInfo.common_flag = varheader.v10.common_flag;
                    songInfo.patt_len = UINT16LE(varheader.v10.patt_len);
                    songInfo.nm_tracks = varheader.v10.nm_tracks;
                    songInfo.macro_speedup = UINT16LE(varheader.v10.macro_speedup);
                    songInfo.flag_4op = varheader.v10.flag_4op;
                    for (int i = 0; i < 20; i++)
                    {
                        songInfo.lock_flags[i] = varheader.v10.lock_flags[i];
                    }
                    for (int i = 0; i < 20; i++)
                    {
                        len[i] = UINT32LE(varheader.v10.len[i]);
                    }

                    reader.seek(posBegin + sizeof(A2T_VARHEADER_V10), SEEK_SET);
                    //return sizeof(A2T_VARHEADER_V10);
                    break;
                case 11:
                case 12:
                case 13:
                case 14:
                    //if (sizeof(A2T_VARHEADER_V11) > size)
                        //return INT_MAX;
                    songInfo.common_flag = varheader.v11.common_flag;
                    songInfo.patt_len = UINT16LE(varheader.v11.patt_len);
                    songInfo.nm_tracks = varheader.v11.nm_tracks;
                    songInfo.macro_speedup = UINT16LE(varheader.v11.macro_speedup);
                    songInfo.flag_4op = varheader.v11.flag_4op;
                    for (int i = 0; i < 20; i++)
                    {
                        songInfo.lock_flags[i] = varheader.v10.lock_flags[i];
                    }
                    for (int i = 0; i < 21; i++)
                    {
                        len[i] = UINT32LE(varheader.v11.len[i]);
                    }

                    reader.seek(posBegin + sizeof(A2T_VARHEADER_V11), SEEK_SET);
                    //return sizeof(A2T_VARHEADER_V11);
                    break;
            }

            s->ordersLen = songInfo.nm_tracks;
            s->patLen = songInfo.patt_len;
        }

        if(!isA2t) //a2m
        {
            logI("a2m");

            A2M_HEADER header;
            unsigned char* hacky = (unsigned char*)&header;
            for(int i = 0; i < (int)sizeof(A2M_HEADER); i++)
            {
                hacky[i] = reader.readC();
            }

            logI("version %d", header.ffver);
            logI("num patterns %d", header.npatt);
            logI("id %s", header.id);

            version = header.ffver;
            
            //s->ordersLen = songInfo.nm_tracks;
            songInfo.patt_len = 64;
            songInfo.nm_tracks = 18;

            patterns = header.npatt;

            int lensize = 0;
            int maxblock = (version < 5 ? header.npatt / 16 : header.npatt / 8) + 1;

            if (version < 5) lensize = 5;         // 1,2,3,4 - uint16_t len[5];
            else if (version < 9) lensize = 9;    // 5,6,7,8 - uint16_t len[9];
            else lensize = 17;                  // 9,10,11 - uint32_t len[17];

            size_t posBegin = reader.tell();

            if (version >= 1 && version <= 8) { // 1 - 8
                //if (lensize * sizeof(uint16_t) > len + sizeof(A2M_HEADER)) return INT_MAX;
                if (lensize * sizeof(uint16_t) > reader.size() - reader.tell())
                {
                    lastError = _("Incomplete songdata!");
                    delete[] file;
                    return false;
                }

                // skip possible rubbish (MARIO.A2M)
                for (int i = 0; (i < lensize) && (i <= maxblock); i++)
                {
                    len[i] = reader.readS(); //UINT16LE(src16[i]);
                    logI("len 16bit %d %d", i, len[i]);
                }

                reader.seek(posBegin + lensize * sizeof(uint16_t), SEEK_SET);
                //return lensize * sizeof(uint16_t);
            } else if (version >= 9 && version <= 14) { // 9 - 14
                //if (lensize * sizeof(uint32_t) > len + sizeof(A2M_HEADER)) return INT_MAX;
                if (lensize * sizeof(uint32_t) > reader.size() - reader.tell())
                {
                    lastError = _("Incomplete songdata!");
                    delete[] file;
                    return false;
                }

                for (int i = 0; i < lensize; i++)
                {
                    len[i] = reader.readI(); //UINT32LE(src32[i]);
                    logI("len 32bit %d %d", i, len[i]);
                }

                reader.seek(posBegin + lensize * sizeof(uint32_t), SEEK_SET);
            }
        }

        if(!isA2t) //a2m, a2m_read_songdata
        {
            if (len[0] > reader.size() - reader.tell())
            {
                lastError = _("Incomplete songdata!");
                delete[] file;
                return false;
            }

            if (version < 9) 
            {    // 1 - 8
                //if (len[0] > reader.size()) return INT_MAX;
                A2M_SONGDATA_V1_8 *data = (A2M_SONGDATA_V1_8 *)calloc(1, sizeof(*data));
                unsigned char* temp = new unsigned char[len[0]];
                reader.read((void*)temp, len[0]);
                a2t_depack(temp, len[0], (unsigned char *)data, sizeof (*data), version);
                delete[] temp;

                memcpy(songInfo.songname, data->songname + 1, 42);
                memcpy(songInfo.composer, data->composer + 1, 42);

                // Calculate the real number of used instruments
                int count = 250;
                while (count && is_data_empty((char *)&data->instr_data[count - 1], sizeof(tINSTR_DATA_V1_8)))
                    count--;

                //instruments_allocate(count);
                ds.ins.reserve(count);

                for (int i = 0; i < 250; i++)
                {
                    memcpy(songInfo.instr_names[i], data->instr_names[i] + 1, 32);
                }

                for (int i = 0; i < count; i++) //instrument import
                {
                    ds.ins.push_back(new DivInstrument());
                    //instrument_import_v1_8(i + 1, &data->instr_data[i]);
                    tINSTR_DATA_V1_8 *instr_s = &data->instr_data[i];
                    DivInstrument* ins = ds.ins[i];

                    ins->name = songInfo.instr_names[i];
                    ins->type = DIV_INS_OPL;

                    ins->fm.op[0].mult = instr_s->fm.multipM;
                    ins->fm.op[0].ksr = instr_s->fm.ksrM;
                    ins->fm.op[0].sus = instr_s->fm.sustM;
                    ins->fm.op[0].vib = instr_s->fm.vibrM;
                    ins->fm.op[0].am = instr_s->fm.tremM;
                    ins->fm.op[0].tl = instr_s->fm.volM;
                    ins->fm.op[0].ksl = instr_s->fm.kslM;
                    ins->fm.op[0].ar = instr_s->fm.attckM;
                    ins->fm.op[0].dr = instr_s->fm.decM;
                    ins->fm.op[0].sl = instr_s->fm.sustnM;
                    ins->fm.op[0].rr = instr_s->fm.relM;
                    ins->fm.op[0].ws = instr_s->fm.wformM;

                    ins->fm.op[1].mult = instr_s->fm.multipC;
                    ins->fm.op[1].ksr = instr_s->fm.ksrC;
                    ins->fm.op[1].sus = instr_s->fm.sustC;
                    ins->fm.op[1].vib = instr_s->fm.vibrC;
                    ins->fm.op[1].am = instr_s->fm.tremC;
                    ins->fm.op[1].tl = instr_s->fm.volC;
                    ins->fm.op[1].ksl = instr_s->fm.kslC;
                    ins->fm.op[1].ar = instr_s->fm.attckC;
                    ins->fm.op[1].dr = instr_s->fm.decC;
                    ins->fm.op[1].sl = instr_s->fm.sustnC;
                    ins->fm.op[1].rr = instr_s->fm.relC;
                    ins->fm.op[1].ws = instr_s->fm.wformC;

                    ins->fm.fb = instr_s->fm.feedb;
                    ins->fm.alg = instr_s->fm.connect;

                    //panning (0=C,1=L,2=R)

                    ins->std.panLMacro.len = 1;
                    
                    if(instr_s->panning == 0)
                    {
                        ins->std.panLMacro.val[0] = 3;
                    }
                    if(instr_s->panning == 1)
                    {
                        ins->std.panLMacro.val[0] = 2;
                    }
                    if(instr_s->panning == 2)
                    {
                        ins->std.panLMacro.val[0] = 1;
                    }
                    //todo: finetune
                }

                memcpy(songInfo.pattern_order, data->pattern_order, 128);

                songInfo.tempo = data->tempo;
                songInfo.speed = data->speed;

                if (version > 4) { // 5 - 8
                    songInfo.common_flag = data->common_flag;
                }

                free(data);
            }
            else 
            {    // 9 - 14
                A2M_SONGDATA_V9_14 *data = (A2M_SONGDATA_V9_14 *)calloc(1, sizeof(*data));
                unsigned char* temp = new unsigned char[len[0]];
                reader.read((void*)temp, len[0]);
                a2t_depack(temp, len[0], (unsigned char *)data, sizeof (*data), version);
                delete[] temp;

                memcpy(songInfo.songname, data->songname + 1, 42);
                memcpy(songInfo.composer, data->composer + 1, 42);

                // Calculate the real number of used instruments
                int count = 255;
                while (count && is_data_empty((char *)&data->instr_data[count - 1], sizeof(tINSTR_DATA)))
                    count--;

                ds.ins.reserve(count);

                for (int i = 0; i < 255; i++)
                {
                    memcpy(songInfo.instr_names[i], data->instr_names[i] + 1, 32);
                }

                songInfo.common_flag = data->common_flag;

                for (int i = 0; i < count; i++) 
                {
                    //instrument_import(i + 1, &data->instr_data[i]);
                    ds.ins.push_back(new DivInstrument());
                    //instrument_import_v1_8(i + 1, &data->instr_data[i]);
                    tINSTR_DATA* instr_s = &data->instr_data[i];
                    DivInstrument* ins = ds.ins[i];

                    // Instrument arpegio/vibrato references
                    //tINSTR_DATA_EXT *dst = get_instr(i + 1, data->instr_data);
                    //assert(dst);
                    //dst->arpeggio = data->fmreg_table[i].arpeggio_table;
                    //dst->vibrato = data->fmreg_table[i].vibrato_table;

                    ins->name = songInfo.instr_names[i];
                    ins->type = DIV_INS_OPL;

                    ins->fm.op[0].mult = instr_s->fm.multipM;
                    ins->fm.op[0].ksr = instr_s->fm.ksrM;
                    ins->fm.op[0].sus = instr_s->fm.sustM;
                    ins->fm.op[0].vib = instr_s->fm.vibrM;
                    ins->fm.op[0].am = instr_s->fm.tremM;
                    ins->fm.op[0].tl = instr_s->fm.volM;
                    ins->fm.op[0].ksl = instr_s->fm.kslM;
                    ins->fm.op[0].ar = instr_s->fm.attckM;
                    ins->fm.op[0].dr = instr_s->fm.decM;
                    ins->fm.op[0].sl = instr_s->fm.sustnM;
                    ins->fm.op[0].rr = instr_s->fm.relM;
                    ins->fm.op[0].ws = instr_s->fm.wformM;

                    ins->fm.op[1].mult = instr_s->fm.multipC;
                    ins->fm.op[1].ksr = instr_s->fm.ksrC;
                    ins->fm.op[1].sus = instr_s->fm.sustC;
                    ins->fm.op[1].vib = instr_s->fm.vibrC;
                    ins->fm.op[1].am = instr_s->fm.tremC;
                    ins->fm.op[1].tl = instr_s->fm.volC;
                    ins->fm.op[1].ksl = instr_s->fm.kslC;
                    ins->fm.op[1].ar = instr_s->fm.attckC;
                    ins->fm.op[1].dr = instr_s->fm.decC;
                    ins->fm.op[1].sl = instr_s->fm.sustnC;
                    ins->fm.op[1].rr = instr_s->fm.relC;
                    ins->fm.op[1].ws = instr_s->fm.wformC;

                    ins->fm.fb = instr_s->fm.feedb;
                    ins->fm.alg = instr_s->fm.connect;
                    //todo: finetune, percvoice

                    //panning (0=C,1=L,2=R)

                    ins->std.panLMacro.len = 1;
                    
                    if(instr_s->panning == 0)
                    {
                        ins->std.panLMacro.val[0] = 3;
                    }
                    if(instr_s->panning == 1)
                    {
                        ins->std.panLMacro.val[0] = 2;
                    }
                    if(instr_s->panning == 2)
                    {
                        ins->std.panLMacro.val[0] = 1;
                    }
                }

                // Allocate fmreg macro tables
                //fmreg_table_allocate(count, data->fmreg_table);

                // Allocate arpeggio/vibrato macro tables
                //arpvib_tables_allocate(255, data->arpvib_table);

                memcpy(songInfo.pattern_order, data->pattern_order, 128);

                songInfo.tempo = data->tempo;
                songInfo.speed = data->speed;
                songInfo.common_flag = data->common_flag;
                songInfo.patt_len = UINT16LE(data->patt_len);
                songInfo.nm_tracks = data->nm_tracks;
                songInfo.macro_speedup = UINT16LE(data->macro_speedup);

                // v10
                songInfo.flag_4op = data->flag_4op;
                memcpy(songInfo.lock_flags, data->lock_flags, sizeof(data->lock_flags));

                // v11
                // NOTE: not used anywhere
                memcpy(songInfo.pattern_names, data->pattern_names, 128 * 43);

                //disabled_fmregs_import(count, (bool (*)[28])data->dis_fmreg_col);

                // v12-13
                // NOTE: not used anywhere
                //songinfo->ins_4op_flags.num_4op = data->ins_4op_flags.num_4op;
                //memcpy(songinfo->ins_4op_flags.idx_4op, data->ins_4op_flags.idx_4op, 128);
                //memcpy(songinfo->reserved_data, data->reserved_data, 1024);

                // v14
                // NOTE: not used anywhere
                //songinfo->bpm_data.rows_per_beat = data->bpm_data.rows_per_beat;
                //songinfo->bpm_data.tempo_finetune = INT16LE(data->bpm_data.tempo_finetune);

                free(data);
            }

            s->hz = songInfo.tempo;
            s->speeds.val[0] = songInfo.speed;
            s->patLen = songInfo.patt_len;
            
            logI("tempo %d", songInfo.tempo);
            logI("speed %d", songInfo.speed);
            logI("pat length %d", songInfo.patt_len);
            logI("nm tracks %d", songInfo.nm_tracks);

            s->name = songInfo.songname;
            ds.name = songInfo.songname;
            ds.composer = songInfo.composer;

            s->ordersLen = 128;
            
            for(int i = 0; i < 128; i++)
            {
                if(songInfo.pattern_order[i] > 0x7f)
                {
                    s->ordersLen = i;
                    break;
                }
            }

            for(int j = 0; j < s->ordersLen; j++)
            {
                for(int i = 0; i < songInfo.nm_tracks; i++)
                {
                    s->orders.ord[i][j] = songInfo.pattern_order[j];
                }
            }
        }

        /*
        ┌─────┬────────────────────────────┐
        │ BiT │ SWiTCH                     │
        ├─────┼────────────────────────────┤
        │  0  │ update speed               │
        │  1  │ track volume lock          │
        │  2  │ volume peak lock           │
        │  3  │ tremolo depth              │
        │  4  │ vibrato depth              │
        │  5  │ track panning lock         │
        │  6  │ percussion track extension │
        │  7  │ volume scaling             │
        └─────┴────────────────────────────┘
        */

        if(songInfo.common_flag & 64)
        {
            ds.system[0] = DIV_SYSTEM_OPL3_DRUMS;
        }

        if(!isA2t) //a2m, a2m_read_patterns
        {
            if(AT2ReadPatterns(s, reader, version, len, patterns, songInfo, 1) == false)
            {
                lastError = _("Incomplete pattern data!");
                delete[] file;
                return false;
            }

            if(version >= 11)
            {
                for(int i = 0; i < patterns; i++)
                {
                    for(int j = 0; j < songInfo.nm_tracks; j++)
                    {
                        DivPattern* pat = s->pat[j].getPattern(i, true);
                        pat->name = (const char*)&songInfo.pattern_names[i][1]; //skip 1st symbol bc it seems to hold weird special byte?
                    }
                }
            }
        }

        //s->makePatUnique(); //needed for non-continuous to continuous effects conversion

        //todo effects conversion

        //s->optimizePatterns(); //if after converting effects we still have some duplicates
        //s->rearrangePatterns();

        logI("macro speedup %d", songInfo.macro_speedup);

        if(songInfo.macro_speedup > 1)
        {
        #ifdef HAVE_LOCALE
            warnings += fmt::sprintf(ngettext("In this module macros execution speed is %d time larger than engine rate. Conversion may be inaccurate.",
                "In this module macros execution speed is %d times larger than engine rate. Conversion may be inaccurate.\n",songInfo.macro_speedup),songInfo.macro_speedup);
        #else
            warnings += fmt::sprintf(_GN("In this module macros execution speed is %d time larger than engine rate. Conversion may be inaccurate.",
                "In this module macros execution speed is %d times larger than engine rate. Conversion may be inaccurate.\n",songInfo.macro_speedup),songInfo.macro_speedup);
        #endif
        }

        ds.insLen = ds.ins.size();
        ds.sampleLen = ds.sample.size();
        ds.waveLen = ds.wave.size();

        ds.systemName = _("OPL3");

        if(songInfo.common_flag & 64)
        {
            ds.systemName = _("OPL3 in drums mode");
        }

        ds.linearPitch = 0;
        ds.pitchMacroIsLinear = false;

        if (active) quitDispatch();
        BUSY_BEGIN_SOFT;
        saveLock.lock();
        song.unload();
        song=ds;
        changeSong(0);
        recalcChans();
        saveLock.unlock();
        BUSY_END;

        if (active) 
        {
            initDispatch();
            BUSY_BEGIN;
            renderSamples();
            reset();
            BUSY_END;
        }
    }
    catch (EndOfFileException& e) 
    {
        logE("premature end of file!");
        lastError="incomplete file";
        delete[] file;
        return false;
    }
    delete[] file;
    return true;
}
