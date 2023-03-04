/**********************************************************************************************
 *
 *   SGS-Thomson Microelectronics M114S/M114A/M114AF Digital Sound Generator
 *   by Steve Ellenoff
 *   09/02/2004
 *
 *   Thanks to R.Belmont for Support & Agreeing to read through that nasty data sheet with me..
 *   Big thanks to Destruk for help in tracking down the data sheet.. Could have never done it
 *   without it!!
 *
 *
 *   Code based largely on Aaron Gile's BSMT2000 driver.
 **********************************************************************************************/

#ifndef M114S_H
#define M114S_H
#if !defined(__GNUC__) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || (__GNUC__ >= 4)	// GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include <stdint.h>

// by default output all 4 channels because Furnace supports that
#define M114S_OUTPUT_CHANNELS 4 // HW uses 4, but if set to 16 instead, we simply output each (internal) channel independently (which is not how the real chip works, as that one mixes the 16 down to 4 outputs)

#define M114S_CHANNELS			16				// Chip has 16 internal channels for sound output


/* struct describing a single table */
struct M114STable
{
		uint8_t			reread;						/* # of times to re-read each byte from table */
		//uint32_t			position;					/* current reading position for table */
		uint32_t			start_address;				/* start address (offset into ROM) for table */
		//uint32_t			stop_address;				/* stop address (offset into ROM) for table */
		uint16_t			length;						/* length in bytes of the table */
		uint16_t			total_length;				/* total length in bytes of the table (including repetitions) */
};

/* struct describing the registers for a single channel */
struct M114SChannelRegs
{
		uint8_t			atten;							/* Attenuation Register */
		uint8_t			outputs;						/* Output Pin Register */
		uint8_t			table1_addr;					/* Table 1 MSB Starting Address Register */
		uint8_t			table2_addr;					/* Table 2 MSB Starting Address Register */
		uint8_t			table_len;						/* Table Length Register */
		uint8_t			read_meth;						/* Read Method Register */
		uint8_t			interp;							/* Interpolation Register */
		bool			env_enable;						/* Envelope Enable Register */
		bool			oct_divisor;					/* Octave Divisor Register */
		uint8_t			frequency;						/* Frequency Register */
};

/* struct describing a single playing channel */
struct M114SChannel
{
	/* registers */
		struct M114SChannelRegs regs;					/* register data for the channel */
	/* internal state */
		bool			active;							/* is the channel active */
		int16_t			output[4096];					/* Holds output samples mixed from table 1 & 2 */
		uint32_t			outpos;							/* Index into output samples */
		int				prev_volume;					/* Holds the last volume code set for the channel */
		int				current_volume;					/* Volume to apply to each sample output */
		int				target_volume;					/* Target to match for volume envelope */
		int				step_rate_volume_env;			/* Volume step for volume envelope */
		//int				end_of_table;					/* End of Table Flag */
		struct M114STable table1;						/* Table 1 Data */
		struct M114STable table2;						/* Table 2 Data */
		uint32_t			incr;							/* Current Sample Rate/Step size increase to play back table */
};

/* struct describing the entire M114S chip */
struct M114SChip
{
	/* vars to be reset when the chip is reset */
	int							bytes_read;					/* # of bytes read */
	int							channel;					/* Which channel is being programmed via the data bus */
	struct M114SChannelRegs		tempch_regs;				/* temporary channel register data for gathering the data programming */
	struct M114SChannel			channels[M114S_CHANNELS];	/* All the chip's internal channels */
	int							channel_volume[4];			/* scale for the HW/chip outputs */
	int8_t *						region_base;				/* pointer to the base of the ROM region */
	struct M114Sinterface*		intf;						/* Pointer to the interface */
	double						reset_cycles;				/* # of cycles that must pass between programming bytes to auto reset the chip */
	int							cpu_num;					/* # of the cpu controlling the M114S */
	bool 						is_M114A;					/* M114A 4MHz or M114AF 6MHz */
};

struct M114Sinterface
{
        int num;                             /* total number of chips */
        int baseclock[MAX_M114S];            /* input clock - Allowed values are 4Mhz & 6Mhz only! */
        int region[MAX_M114S];               /* memory region where the sample ROM lives */
        int mixing_level[MAX_M114S][4];      /* master volume, one for each output sample */
        int cpunum[MAX_M114S];               /* # of the cpu controlling the M114S */
};

int M114S_sh_start(const struct MachineSound *msound);
void M114S_sh_stop(void);
void M114S_sh_reset(void);

void m114s_data_write(struct M114SChip* chip, data8_t data);
void m114s_update(struct M114SChip* chip, int16_t **buffer, int samples);

#endif
