# Nuked-OPN2
High accuracy Yamaha YM3438(OPN2) emulator.

The YM3438 is a CMOS variant of the YM2612 used in Sega MegaDrive(Genesis) and FM Towns.

Genesis Plus GX fork with this core integrated is available here: https://github.com/nukeykt/Genesis-Plus-GX

# Features:
- Based on YM3438 die shot reverse engineering and thus provides very high emulation accuracy.

- Cycle-accurate.

- Undocumented registers/features emulation.
- SSG-EG, CSM mode emulation.
- Compatible with the YM2612.

# API documention
```
void OPN2_Reset(ym3438_t *chip) - Reset emulated chip
void OPN2_Clock(ym3438_t *chip, Bit32s *buffer) - Advances emulated chip state by 1 internal clock(6 master clocks). Writes signed 9-bit MOL, MOR pin states to buffer. 
void OPN2_Write(ym3438_t *chip, Bit32u port, Bit8u data) - Write 8-bit data to port.
void OPN2_SetTestPin(ym3438_t *chip, Bit32u value) - Set TEST pin value.
Bit32u OPN2_ReadTestPin(ym3438_t *chip) - Read TEST pin value.
Bit32u OPN2_ReadIRQPin(ym3438_t *chip) - Read IRQ pin value.
Bit8u OPN2_Read(ym3438_t *chip, Bit32u port) - Read chip status.
```

# Samples
Sonic the Hedgehog:
https://youtu.be/ImmKy_-pJ8g

Sega CD BIOS v1.10:
https://youtu.be/s-8ASMbtojQ

