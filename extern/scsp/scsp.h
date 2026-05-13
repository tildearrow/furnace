/*

	SCSP (YMF292-F) header
*/

#ifndef _SCSP_H_
#define _SCSP_H_

#define MAX_SCSP	(2)

#define COMBINE_DATA(varptr)	(*(varptr) = (*(varptr) & mem_mask) | (data & ~mem_mask))

// convert AO types
typedef int8 data8_t;
typedef int16 data16_t;
typedef int32 data32_t;
typedef int offs_t;

/*
    SCSP features 32 programmable slots
    that can generate FM and PCM (from ROM/RAM) sound
*/

//SLOT PARAMETERS
#define KEYONEX(slot)		((slot->udata.data[0x0]>>0x0)&0x1000)
#define KEYONB(slot)		((slot->udata.data[0x0]>>0x0)&0x0800)
#define SBCTL(slot)		((slot->udata.data[0x0]>>0x9)&0x0003)
#define SSCTL(slot)		((slot->udata.data[0x0]>>0x7)&0x0003)
#define LPCTL(slot)		((slot->udata.data[0x0]>>0x5)&0x0003)
#define PCM8B(slot)		((slot->udata.data[0x0]>>0x0)&0x0010)

#define SA(slot)		(((slot->udata.data[0x0]&0xF)<<16)|(slot->udata.data[0x1]))

#define LSA(slot)		(slot->udata.data[0x2])

#define LEA(slot)		(slot->udata.data[0x3])

#define D2R(slot)		((slot->udata.data[0x4]>>0xB)&0x001F)
#define D1R(slot)		((slot->udata.data[0x4]>>0x6)&0x001F)
#define EGHOLD(slot)		((slot->udata.data[0x4]>>0x0)&0x0020)
#define AR(slot)		((slot->udata.data[0x4]>>0x0)&0x001F)

#define LPSLNK(slot)		((slot->udata.data[0x5]>>0x0)&0x4000)
#define KRS(slot)		((slot->udata.data[0x5]>>0xA)&0x000F)
#define DL(slot)		((slot->udata.data[0x5]>>0x5)&0x001F)
#define RR(slot)		((slot->udata.data[0x5]>>0x0)&0x001F)

#define STWINH(slot)		((slot->udata.data[0x6]>>0x0)&0x0200)
#define SDIR(slot)		((slot->udata.data[0x6]>>0x0)&0x0100)
#define TL(slot)		((slot->udata.data[0x6]>>0x0)&0x00FF)

#define MDL(slot)		((slot->udata.data[0x7]>>0xC)&0x000F)
#define MDXSL(slot)		((slot->udata.data[0x7]>>0x6)&0x003F)
#define MDYSL(slot)		((slot->udata.data[0x7]>>0x0)&0x003F)

#define OCT(slot)		((slot->udata.data[0x8]>>0xB)&0x000F)
#define FNS(slot)		((slot->udata.data[0x8]>>0x0)&0x03FF)

#define LFORE(slot)		((slot->udata.data[0x9]>>0x0)&0x8000)
#define LFOF(slot)		((slot->udata.data[0x9]>>0xA)&0x001F)
#define PLFOWS(slot)		((slot->udata.data[0x9]>>0x8)&0x0003)
#define PLFOS(slot)		((slot->udata.data[0x9]>>0x5)&0x0007)
#define ALFOWS(slot)		((slot->udata.data[0x9]>>0x3)&0x0003)
#define ALFOS(slot)		((slot->udata.data[0x9]>>0x0)&0x0007)

#define ISEL(slot)		((slot->udata.data[0xA]>>0x3)&0x000F)
#define IMXL(slot)		((slot->udata.data[0xA]>>0x0)&0x0007)

#define DISDL(slot)		((slot->udata.data[0xB]>>0xD)&0x0007)
#define DIPAN(slot)		((slot->udata.data[0xB]>>0x8)&0x001F)
#define EFSDL(slot)		((slot->udata.data[0xB]>>0x5)&0x0007)
#define EFPAN(slot)		((slot->udata.data[0xB]>>0x0)&0x001F)

typedef enum {ATTACK,DECAY1,DECAY2,RELEASE} _STATE;
struct _EG
{
	int volume;	//
	_STATE state;
	int step;
	//step vals
	int AR;		//Attack
	int D1R;	//Decay1
	int D2R;	//Decay2
	int RR;		//Release

	int DL;		//Decay level
	UINT8 EGHOLD;
	UINT8 LPLINK;
};

struct _LFO
{
	unsigned short phase;
	UINT32 phase_step;
	int *table;
	int *scale;
};

struct _SLOT
{
	union
	{
		UINT16 data[0x10];	//only 0x1a bytes used
		UINT8 datab[0x20];
	} udata;
	UINT8 active;	//this slot is currently playing
	UINT8 *base;		//samples base address
	UINT32 cur_addr;	//current play address (24.8)
	UINT32 nxt_addr;	//next play address
	UINT32 step;		//pitch step (24.8)
	UINT8 Backwards;	//the wave is playing backwards
	struct _EG EG;			//Envelope
	struct _LFO PLFO;		//Phase LFO
	struct _LFO ALFO;		//Amplitude LFO
	int slot;
	signed short Prev;	//Previous sample (for interpolation)
};

#define MEM4B(scsp)		((scsp->udata.data[0]>>0x0)&0x0200)
#define DAC18B(scsp)		((scsp->udata.data[0]>>0x0)&0x0100)
#define MVOL(scsp)		((scsp->udata.data[0]>>0x0)&0x000F)
#define RBL(scsp)		((scsp->udata.data[1]>>0x7)&0x0003)
#define RBP(scsp)		((scsp->udata.data[1]>>0x0)&0x003F)
#define MOFULL(scsp)   		((scsp->udata.data[2]>>0x0)&0x1000)
#define MOEMPTY(scsp)		((scsp->udata.data[2]>>0x0)&0x0800)
#define MIOVF(scsp)		((scsp->udata.data[2]>>0x0)&0x0400)
#define MIFULL(scsp)		((scsp->udata.data[2]>>0x0)&0x0200)
#define MIEMPTY(scsp)		((scsp->udata.data[2]>>0x0)&0x0100)

#define SCILV0(scsp)    	((scsp->udata.data[0x24/2]>>0x0)&0xff)
#define SCILV1(scsp)    	((scsp->udata.data[0x26/2]>>0x0)&0xff)
#define SCILV2(scsp)    	((scsp->udata.data[0x28/2]>>0x0)&0xff)

#define SCIEX0	0
#define SCIEX1	1
#define SCIEX2	2
#define SCIMID	3
#define SCIDMA	4
#define SCIIRQ	5
#define SCITMA	6
#define SCITMB	7

//the DSP Context
struct _SCSPDSP
{
//Config
	UINT16 *SCSPRAM;
	UINT32 SCSPRAM_LENGTH;
	UINT32 RBP;	//Ring buf pointer
	UINT32 RBL;	//Delay ram (Ring buffer) size in words

//context

	INT16 COEF[64];		//16 bit signed
	UINT16 MADRS[32];	//offsets (in words), 16 bit
	UINT16 MPRO[128*4];	//128 steps 64 bit
	INT32 TEMP[128];	//TEMP regs,24 bit signed
	INT32 MEMS[32];	//MEMS regs,24 bit signed
	UINT32 DEC;

//input
	INT32 MIXS[16];	//MIXS, 24 bit signed
	INT16 EXTS[2];	//External inputs (CDDA)    16 bit signed

//output
	INT16 EFREG[16];	//EFREG, 16 bit signed

	int Stopped;
	int LastStep;
};

void SCSPDSP_Init(struct _SCSPDSP *DSP);
void SCSPDSP_SetSample(struct _SCSPDSP *DSP, INT32 sample, INT32 SEL, INT32 MXL);
void SCSPDSP_Step(struct _SCSPDSP *DSP);
void SCSPDSP_Start(struct _SCSPDSP *DSP);

struct _SCSP
{
	union
	{
		UINT16 data[0x30/2];
		UINT8 datab[0x30];
	} udata;
	struct _SLOT Slots[32];
	signed short RINGBUF[64];
	unsigned char BUFPTR;
	#if FM_DELAY
	signed short DELAYBUF[FM_DELAY];
	unsigned char DELAYPTR;
	#endif
	unsigned char *SCSPRAM;
	UINT32 SCSPRAM_LENGTH;
	char Master;
	void (*Int68kCB)(int irq);

	UINT32 IrqTimA;
	UINT32 IrqTimBC;
	UINT32 IrqMidi;

	UINT8 MidiOutW,MidiOutR;
	UINT8 MidiStack[16];
	UINT8 MidiW,MidiR;

	int LPANTABLE[0x10000];
	int RPANTABLE[0x10000];

	int TimPris[3];
	int TimCnt[3];

	// DMA stuff
	UINT32 scsp_dmea;
	UINT16 scsp_drga;
	UINT16 scsp_dtlg;

	int ARTABLE[64], DRTABLE[64];

	struct _SCSPDSP DSP;
};

extern struct _SCSP SCSP;

struct SCSPinterface
{
	int num;
	void *region[MAX_SCSP];
	int mixing_level[MAX_SCSP];			/* volume */
	void (*irq_callback[MAX_SCSP])(int state);	/* irq callback */
};

void *scsp_start(const void *config);
void SCSP_Update(void *param, INT16 **inputs, stereo_sample_t *sample);

#define READ16_HANDLER(name)	data16_t name(offs_t offset, data16_t mem_mask)
#define WRITE16_HANDLER(name)	void     name(offs_t offset, data16_t data, data16_t mem_mask)

// SCSP register access
READ16_HANDLER( SCSP_0_r );
WRITE16_HANDLER( SCSP_0_w );
READ16_HANDLER( SCSP_1_r );
WRITE16_HANDLER( SCSP_1_w );

// MIDI I/O access (used for comms on Model 2/3)
WRITE16_HANDLER( SCSP_MidiIn );
READ16_HANDLER( SCSP_MidiOutR );

#endif
