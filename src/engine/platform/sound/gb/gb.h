#ifndef GB_h
#define GB_h
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "gb_struct_def.h"

#include "apu.h"
#include "timing.h"

#define GB_STRUCT_VERSION 13

#define CGB (gb->model&GB_MODEL_CGB_FAMILY)

#define GB_MODEL_FAMILY_MASK 0xF00
#define GB_MODEL_DMG_FAMILY 0x000
#define GB_MODEL_MGB_FAMILY 0x100
#define GB_MODEL_CGB_FAMILY 0x200
#define GB_MODEL_PAL_BIT 0x40
#define GB_MODEL_NO_SFC_BIT 0x80

#define GB_MODEL_PAL_BIT_OLD 0x1000
#define GB_MODEL_NO_SFC_BIT_OLD 0x2000

#if __clang__
#define unrolled _Pragma("unroll")
#elif __GNUC__ >= 8
#define unrolled _Pragma("GCC unroll 8")
#else
#define unrolled
#endif

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define GB_BIG_ENDIAN
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define GB_LITTLE_ENDIAN
#else
#error Unable to detect endianess
#endif

#ifdef GB_BIG_ENDIAN
#define LE16(x) __builtin_bswap16(x)
#define LE32(x) __builtin_bswap32(x)
#define LE64(x) __builtin_bswap64(x)
#define BE16(x) (x)
#define BE32(x) (x)
#define BE64(x) (x)
#else
#define LE16(x) (x)
#define LE32(x) (x)
#define LE64(x) (x)
#define BE16(x) __builtin_bswap16(x)
#define BE32(x) __builtin_bswap32(x)
#define BE64(x) __builtin_bswap64(x)
#endif

#if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 8)
#define __builtin_bswap16(x) ({ typeof(x) _x = (x); _x >> 8 | _x << 8; })
#endif

typedef struct {
    struct {
        uint8_t r, g, b;
    } colors[5];
} GB_palette_t;

extern const GB_palette_t GB_PALETTE_GREY;
extern const GB_palette_t GB_PALETTE_DMG;
extern const GB_palette_t GB_PALETTE_MGB;
extern const GB_palette_t GB_PALETTE_GBL;

typedef union {
    struct {
        uint8_t seconds;
        uint8_t minutes;
        uint8_t hours;
        uint8_t days;
        uint8_t high;
    };
    struct {
        uint8_t seconds;
        uint8_t minutes;
        uint8_t hours:5;
        uint8_t weekday:3;
        uint8_t weeks;
    } tpp1;
    uint8_t data[5];
} GB_rtc_time_t;

typedef enum {
    // GB_MODEL_DMG_0 = 0x000,
    // GB_MODEL_DMG_A = 0x001,
    GB_MODEL_DMG_B = 0x002,
    // GB_MODEL_DMG_C = 0x003,
    GB_MODEL_SGB = 0x004,
    GB_MODEL_SGB_NTSC = GB_MODEL_SGB,
    GB_MODEL_SGB_PAL = GB_MODEL_SGB | GB_MODEL_PAL_BIT,
    GB_MODEL_SGB_NTSC_NO_SFC = GB_MODEL_SGB | GB_MODEL_NO_SFC_BIT,
    GB_MODEL_SGB_NO_SFC = GB_MODEL_SGB_NTSC_NO_SFC,
    GB_MODEL_SGB_PAL_NO_SFC = GB_MODEL_SGB | GB_MODEL_NO_SFC_BIT | GB_MODEL_PAL_BIT,
    // GB_MODEL_MGB = 0x100,
    GB_MODEL_SGB2 = 0x101,
    GB_MODEL_SGB2_NO_SFC = GB_MODEL_SGB2 | GB_MODEL_NO_SFC_BIT,
    // GB_MODEL_CGB_0 = 0x200,
    // GB_MODEL_CGB_A = 0x201,
    // GB_MODEL_CGB_B = 0x202,
    GB_MODEL_CGB_C = 0x203,
    // GB_MODEL_CGB_D = 0x204,
    GB_MODEL_CGB_E = 0x205,
    GB_MODEL_AGB = 0x206,
    GB_MODEL_AGB_NATIVE = 0x226,
} GB_model_t;

enum {
    GB_REGISTER_AF,
    GB_REGISTER_BC,
    GB_REGISTER_DE,
    GB_REGISTER_HL,
    GB_REGISTER_SP,
    GB_REGISTERS_16_BIT /* Count */
};

/* Todo: Actually use these! */
enum {
    GB_CARRY_FLAG = 16,
    GB_HALF_CARRY_FLAG = 32,
    GB_SUBTRACT_FLAG = 64,
    GB_ZERO_FLAG = 128,
};

typedef enum {
    GB_BORDER_SGB,
    GB_BORDER_NEVER,
    GB_BORDER_ALWAYS,
} GB_border_mode_t;

enum {
    /* Joypad and Serial */
    GB_IO_JOYP       = 0x00, // Joypad (R/W)
    GB_IO_SB         = 0x01, // Serial transfer data (R/W)
    GB_IO_SC         = 0x02, // Serial Transfer Control (R/W)

    /* Missing */

    /* Timers */
    GB_IO_DIV        = 0x04, // Divider Register (R/W)
    GB_IO_TIMA       = 0x05, // Timer counter (R/W)
    GB_IO_TMA        = 0x06, // Timer Modulo (R/W)
    GB_IO_TAC        = 0x07, // Timer Control (R/W)

    /* Missing */

    GB_IO_IF         = 0x0f, // Interrupt Flag (R/W)

    /* Sound */
    GB_IO_NR10       = 0x10, // Channel 1 Sweep register (R/W)
    GB_IO_NR11       = 0x11, // Channel 1 Sound length/Wave pattern duty (R/W)
    GB_IO_NR12       = 0x12, // Channel 1 Volume Envelope (R/W)
    GB_IO_NR13       = 0x13, // Channel 1 Frequency lo (Write Only)
    GB_IO_NR14       = 0x14, // Channel 1 Frequency hi (R/W)
    /* NR20 does not exist */
    GB_IO_NR21       = 0x16, // Channel 2 Sound Length/Wave Pattern Duty (R/W)
    GB_IO_NR22       = 0x17, // Channel 2 Volume Envelope (R/W)
    GB_IO_NR23       = 0x18, // Channel 2 Frequency lo data (W)
    GB_IO_NR24       = 0x19, // Channel 2 Frequency hi data (R/W)
    GB_IO_NR30       = 0x1a, // Channel 3 Sound on/off (R/W)
    GB_IO_NR31       = 0x1b, // Channel 3 Sound Length
    GB_IO_NR32       = 0x1c, // Channel 3 Select output level (R/W)
    GB_IO_NR33       = 0x1d, // Channel 3 Frequency's lower data (W)
    GB_IO_NR34       = 0x1e, // Channel 3 Frequency's higher data (R/W)
    /* NR40 does not exist */
    GB_IO_NR41       = 0x20, // Channel 4 Sound Length (R/W)
    GB_IO_NR42       = 0x21, // Channel 4 Volume Envelope (R/W)
    GB_IO_NR43       = 0x22, // Channel 4 Polynomial Counter (R/W)
    GB_IO_NR44       = 0x23, // Channel 4 Counter/consecutive, Inital (R/W)
    GB_IO_NR50       = 0x24, // Channel control / ON-OFF / Volume (R/W)
    GB_IO_NR51       = 0x25, // Selection of Sound output terminal (R/W)
    GB_IO_NR52       = 0x26, // Sound on/off

    /* Missing */

    GB_IO_WAV_START  = 0x30, // Wave pattern start
    GB_IO_WAV_END    = 0x3f, // Wave pattern end

    /* Graphics */
    GB_IO_LCDC       = 0x40, // LCD Control (R/W)
    GB_IO_STAT       = 0x41, // LCDC Status (R/W)
    GB_IO_SCY        = 0x42, // Scroll Y (R/W)
    GB_IO_SCX        = 0x43, // Scroll X (R/W)
    GB_IO_LY         = 0x44, // LCDC Y-Coordinate (R)
    GB_IO_LYC        = 0x45, // LY Compare (R/W)
    GB_IO_DMA        = 0x46, // DMA Transfer and Start Address (W)
    GB_IO_BGP        = 0x47, // BG Palette Data (R/W) - Non CGB Mode Only
    GB_IO_OBP0       = 0x48, // Object Palette 0 Data (R/W) - Non CGB Mode Only
    GB_IO_OBP1       = 0x49, // Object Palette 1 Data (R/W) - Non CGB Mode Only
    GB_IO_WY         = 0x4a, // Window Y Position (R/W)
    GB_IO_WX         = 0x4b, // Window X Position minus 7 (R/W)
    // Has some undocumented compatibility flags written at boot.
    // Unfortunately it is not readable or writable after boot has finished, so research of this
    // register is quite limited. The value written to this register, however, can be controlled
    // in some cases.
    GB_IO_KEY0 = 0x4c,

    /* General CGB features */
    GB_IO_KEY1       = 0x4d, // CGB Mode Only - Prepare Speed Switch

    /* Missing */

    GB_IO_VBK        = 0x4f, // CGB Mode Only - VRAM Bank
    GB_IO_BANK       = 0x50, // Write to disable the BIOS mapping

    /* CGB DMA */
    GB_IO_HDMA1      = 0x51, // CGB Mode Only - New DMA Source, High
    GB_IO_HDMA2      = 0x52, // CGB Mode Only - New DMA Source, Low
    GB_IO_HDMA3      = 0x53, // CGB Mode Only - New DMA Destination, High
    GB_IO_HDMA4      = 0x54, // CGB Mode Only - New DMA Destination, Low
    GB_IO_HDMA5      = 0x55, // CGB Mode Only - New DMA Length/Mode/Start

    /* IR */
    GB_IO_RP         = 0x56, // CGB Mode Only - Infrared Communications Port

    /* Missing */

    /* CGB Paletts */
    GB_IO_BGPI       = 0x68, // CGB Mode Only - Background Palette Index
    GB_IO_BGPD       = 0x69, // CGB Mode Only - Background Palette Data
    GB_IO_OBPI       = 0x6a, // CGB Mode Only - Sprite Palette Index
    GB_IO_OBPD       = 0x6b, // CGB Mode Only - Sprite Palette Data
    GB_IO_OPRI       = 0x6c, // Affects object priority (X based or index based)

    /* Missing */

    GB_IO_SVBK       = 0x70, // CGB Mode Only - WRAM Bank
    GB_IO_UNKNOWN2   = 0x72, // (00h) - Bit 0-7 (Read/Write)
    GB_IO_UNKNOWN3   = 0x73, // (00h) - Bit 0-7 (Read/Write)
    GB_IO_UNKNOWN4   = 0x74, // (00h) - Bit 0-7 (Read/Write) - CGB Mode Only
    GB_IO_UNKNOWN5   = 0x75, // (8Fh) - Bit 4-6 (Read/Write)
    GB_IO_PCM_12     = 0x76, // Channels 1 and 2 amplitudes
    GB_IO_PCM_34     = 0x77, // Channels 3 and 4 amplitudes
    GB_IO_UNKNOWN8   = 0x7F, // Unknown, write only
};

typedef enum {
    GB_LOG_BOLD = 1,
    GB_LOG_DASHED_UNDERLINE = 2,
    GB_LOG_UNDERLINE = 4,
    GB_LOG_UNDERLINE_MASK =  GB_LOG_DASHED_UNDERLINE | GB_LOG_UNDERLINE
} GB_log_attributes;

typedef enum {
    GB_BOOT_ROM_DMG0,
    GB_BOOT_ROM_DMG,
    GB_BOOT_ROM_MGB,
    GB_BOOT_ROM_SGB,
    GB_BOOT_ROM_SGB2,
    GB_BOOT_ROM_CGB0,
    GB_BOOT_ROM_CGB,
    GB_BOOT_ROM_AGB,
} GB_boot_rom_t;

typedef enum {
    GB_RTC_MODE_SYNC_TO_HOST,
    GB_RTC_MODE_ACCURATE,
} GB_rtc_mode_t;

#define LCDC_PERIOD 70224
#define CPU_FREQUENCY 0x400000
#define SGB_NTSC_FREQUENCY (21477272 / 5)
#define SGB_PAL_FREQUENCY (21281370 / 5)
#define DIV_CYCLES (0x100)
#define INTERNAL_DIV_CYCLES (0x40000)

#if !defined(MIN)
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#if !defined(MAX)
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

typedef void (*GB_vblank_callback_t)(GB_gameboy_t *gb);
typedef void (*GB_log_callback_t)(GB_gameboy_t *gb, const char *string, GB_log_attributes attributes);
typedef char *(*GB_input_callback_t)(GB_gameboy_t *gb);
typedef uint32_t (*GB_rgb_encode_callback_t)(GB_gameboy_t *gb, uint8_t r, uint8_t g, uint8_t b);
typedef void (*GB_infrared_callback_t)(GB_gameboy_t *gb, bool on);
typedef void (*GB_rumble_callback_t)(GB_gameboy_t *gb, double rumble_amplitude);
typedef void (*GB_serial_transfer_bit_start_callback_t)(GB_gameboy_t *gb, bool bit_to_send);
typedef bool (*GB_serial_transfer_bit_end_callback_t)(GB_gameboy_t *gb);
typedef void (*GB_update_input_hint_callback_t)(GB_gameboy_t *gb);
typedef void (*GB_joyp_write_callback_t)(GB_gameboy_t *gb, uint8_t value);
typedef void (*GB_icd_pixel_callback_t)(GB_gameboy_t *gb, uint8_t row);
typedef void (*GB_icd_hreset_callback_t)(GB_gameboy_t *gb);
typedef void (*GB_icd_vreset_callback_t)(GB_gameboy_t *gb);
typedef void (*GB_boot_rom_load_callback_t)(GB_gameboy_t *gb, GB_boot_rom_t type);

struct GB_breakpoint_s;
struct GB_watchpoint_s;

typedef struct {
    uint8_t pixel; // Color, 0-3
    uint8_t palette; // Palette, 0 - 7 (CGB); 0-1 in DMG (or just 0 for BG)
    uint8_t priority; // Sprite priority – 0 in DMG, OAM index in CGB
    bool bg_priority; // For sprite FIFO – the BG priority bit. For the BG FIFO – the CGB attributes priority bit
} GB_fifo_item_t;

#define GB_FIFO_LENGTH 16
typedef struct {
    GB_fifo_item_t fifo[GB_FIFO_LENGTH];
    uint8_t read_end;
    uint8_t write_end;
} GB_fifo_t;

typedef struct {
    uint32_t magic;
    uint8_t track_count;
    uint8_t first_track;
    uint16_t load_address;
    uint16_t init_address;
    uint16_t play_address;
    uint16_t sp;
    uint8_t TMA;
    uint8_t TAC;
    char title[32];
    char author[32];
    char copyright[32];
} GB_gbs_header_t;

typedef struct {
    uint8_t track_count;
    uint8_t first_track;
    char title[33];
    char author[33];
    char copyright[33];
} GB_gbs_info_t;

/* When state saving, each section is dumped independently of other sections.
   This allows adding data to the end of the section without worrying about future compatibility.
   Some other changes might be "safe" as well.
   This struct is not packed, but dumped sections exclusively use types that have the same alignment in both 32 and 64
   bit platforms. */

/* minimal GB_gameboy_s struct with just the APU */
struct GB_gameboy_s {
        /* CPU and General Hardware Flags*/
        GB_model_t model;
        bool cgb_mode;
        bool cgb_double_speed;
        bool halted;
        bool stopped;

        uint8_t io_registers[0x80];
        GB_UNIT(div);
        uint16_t div_counter;
        uint8_t tima_reload_state;
        GB_apu_t apu;
        GB_apu_output_t apu_output;
};

#ifndef __printflike
/* Missing from Linux headers. */
#define __printflike(fmtarg, firstvararg) \
__attribute__((__format__ (__printf__, fmtarg, firstvararg)))
#endif

typedef enum {
    GB_DIRECT_ACCESS_ROM,
    GB_DIRECT_ACCESS_RAM,
    GB_DIRECT_ACCESS_CART_RAM,
    GB_DIRECT_ACCESS_VRAM,
    GB_DIRECT_ACCESS_HRAM,
    GB_DIRECT_ACCESS_IO, /* Warning: Some registers can only be read/written correctly via GB_memory_read/write. */
    GB_DIRECT_ACCESS_BOOTROM,
    GB_DIRECT_ACCESS_OAM,
    GB_DIRECT_ACCESS_BGP,
    GB_DIRECT_ACCESS_OBP,
    GB_DIRECT_ACCESS_IE,
} GB_direct_access_t;

#ifdef __cplusplus
}
#endif

#endif /* GB_h */
