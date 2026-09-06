; this file defines an SNES APU.

; DSP registers

dsp_LVOL=$00
dsp_RVOL=$01
dsp_FREQL=$02
dsp_FREQH=$03
dsp_SRCN=$04
dsp_AD=$05
dsp_SR=$06
dsp_GAIN=$07
dsp_ENVX=$08
dsp_OUTX=$09
dsp_MVOL_L=$0C
dsp_MVOL_R=$1C
dsp_EVOL_L=$2C
dsp_EVOL_R=$3C
dsp_KON=$4C
dsp_KOF=$5C
dsp_FLG=$6C
dsp_ENDX=$7C
dsp_EFB=$0D
dsp_PMON=$2D
dsp_NON=$3D
dsp_EON=$4D
dsp_DIR=$5D
dsp_ESA=$6D
dsp_EDL=$7D
dsp_COEF=$0F

; machine registers

spc_boot=$F0
spc_ctrl=$F1
spc_dspAddr=$F2
spc_dspData=$F3
spc_port0=$F4
spc_port1=$F5
spc_port2=$F6
spc_port3=$F7
spc_timer0=$FA
spc_timer1=$FB
spc_timer2=$FC
spc_timerRead0=$FD
spc_timerRead1=$FE
spc_timerRead2=$FF

; memory map

.MEMORYMAP
DEFAULTSLOT 1
SLOT 0 START $0000 SIZE $200
SLOT 1 START $0200 SIZE $FDC0
SLOT 2 START $FFC0 SIZE $40
; pseudo-slots for file header
SLOT 3 START $10000 SIZE $100
SLOT 4 START $10100 SIZE $100
.ENDME

; map for an SPC file.

.ROMBANKMAP
BANKSTOTAL 5
BANKSIZE $100 ; SPC file header
BANKS 1
BANKSIZE $200 ; zero page/stack
BANKS 1
BANKSIZE $FDC0 ; RAM
BANKS 1
BANKSIZE $40 ; ROM (bootloader)
BANKS 1
BANKSIZE $100 ; SPC extra
BANKS 1
.ENDRO

