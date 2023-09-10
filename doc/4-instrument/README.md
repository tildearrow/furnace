# instrument editor

the instrument editor always starts with this section:

![top of instrument editor](instrument-editor-top.png)

- top-left numeric dropdown: instrument selector.
- folder icon: open an instrument file.
- save icon: save current instrument as a file.
  - right-clicking gives the option to save a .dmp format DefleMask preset.
- **Name**: instrument name.
- **Type**: the system for which the instrument is intended.
  - if changed, all applicable settings and macros will remain as they are. numbers will not be adjusted.

## instrument types

the following instrument types are available:

- [SN76489/Sega PSG](psg.md) - for use with TI SN76489 and derivatives like Sega Master System's PSG.
- [FM (OPN)](fm-opn.md) - for use with YM2612, YM2203, YM2608, YM2610 and YM2610B.
- [Game Boy](game-boy.md) - for use with Game Boy APU.
- [C64](c64.md) - for use with Commodore 64 SID.
- [Generic Sample](amiga.md) for controlling Amiga and other sample channels/chips like YM2612's Channel 6 PCM mode, NES channel 5, Sega PCM, X1-010 and PC Engine's sample playback mode.
- [PC Engine](pce.md) - for use with PC Engine's wavetable synthesizer.
- [AY-3-8910/SSG](ay8910.md) - for use with AY-3-8910 PSG sound source and SSG portion in YM2610.
- [AY8930](ay8930.md) - for use with Microchip AY8930 E-PSG sound source.
- [TIA](tia.md) - for use with Atari 2600 chip.
- [SAA1099](saa.md) - for use with Philips SAA1099 PSG sound source.
- [VIC](vic.md) - for use with VIC-20 sound chip.
- [PET](pet.md) - for use with Commodore PET.
- [VRC6](vrc6.md) - for use with VRC6's PSG sound source.
- [FM (OPLL)](fm-opll.md) - for use with YM2413.
- [FM (OPL)](fm-opll.md) - for use with YM3526 (OPL), YM3812 (OPL2) and YMF262 (OPL3).
- [FDS](fds.md) - for use with Famicom Disk System sound source.
- [Virtual Boy](virtual-boy.md) - for use with Virtual Boy.
- [Namco 163](n163.md) - for use with Namco 163.
- [Konami SCC/Bubble System WSG](scc.md) - for use with Konami SCC and Wavetable portion in Bubble System's sound hardware.
- [FM (OPZ)](fm-opz.md) - for use with YM2414.
- [POKEY](pokey.md) - for use with Atari 8-bit computers and their POKEY sound source.
- [Beeper](beeper.md) - for use with PC Speaker and ZX Spectrum Beeper (SFX-like engine).
- [WonderSwan](wonderswan.md) - for use with WonderSwan's wavetable synthesizer.
- [Atari Lynx](lynx.md) - for use with Atari Lynx handheld console.
- [VERA](vera.md) - for use with Commander X16 VERA.
- [Seta/Allumer X1-010](x1_010.md) - for use with Wavetable portion in Seta/Allumer X1-010.
- [ES5506](es5506.md) - for use with Ensoniq ES5506 sound chip.
- [SNES](snes.md) - for use with SNES.
- [Sound Unit](su.md) - for use with Sound Unit chip.
- [Namco WSG](wsg.md) - for use with Namco WSG wavetable chips, including C15 and C30.
- [FM (OPM)](fm-opm.md) - for use with YM2151.
- [NES](nes.md) - for use with NES.
- [MSM6258](msm6258.md) - for use with MSM6258 sample chip.
- [MSM6295](msm6295.md) - for use with MSM6295 sample chip.
- [ADPCM-A](adpcm-a.md) - for use with ADPCM-A sample chip.
- [ADPCM-B](adpcm-b.md) - for use with ADPCM-B sample chip.
- [SegaPCM](segapcm.md) - for use with SegaPCM sample chip.
- [QSound](qsound.md) - for use with QSound sample chip.
- [YMZ280B](ymz280b.md) - for use with YMZ280B sample chip.
- [RF5C68](rf5c68.md) - for use with RF5C68 sample chip.
- [MSM5232](msm5232.md) - for use with MSM5232 PSG sound source.
- [T6W28](t6w28.md) - for use with Toshiba T6W28 PSG sound source.
- [K007232](k007232.md) - for use with K007232 sample chip.
- [GA20](ga20.md) - for use with GA20 sample chip.
- [Pokémon Mini/QuadTone](pokemini.md) - for use with these systems.
- [SM8521](sm8521.md) - for use with SM8521 chip, used in Tiger Game.com.
- [PV-1000](pv1000.md) - for use with Casio PV-1000.
- [K053260](k053260.md) - for use with K053260 sample chip.
- [TED](ted.md) - for use with Commodore Plus/4 and Commodore 16's TED chip.
- [C140](c140.md) - for use with C140 sample chip.
- [C219](c219.md) - for use with C219 sample chip.

# macros

Macros are incredibly versatile tools for automating instrument parameters.

After creating an instrument, open the Instrument Editor and select the "Macros" tab. There may be multiple macro tabs to control individual FM operators and such.

![macro view](macroview.png)

The very first numeric entry sets the visible width of the bars in sequence-type macros. The scrollbar affects the view of all macros at once. There's a matching scrollbar at the bottom underneath all the macros.

Each macro has two buttons on the left.
- Macro type (explained below).
- Timing editor, which pops up a small dialog:
  - Step Length (ticks): Determines how many ticks pass before each change of value. Default is 1.
  - Delay: Delays the start of the macro until this many ticks have passed. Default is 0.
  - The button is highlighted if either of these is set differently from default.

## macro types

Every macro can be defined though one of three methods, selectable with the leftmost button under the macro type label:

- ![](macro-button-seq.png) **Sequence:** displayed as a bar graph, this is a sequence of numeric values.
- ![](macro-button-ADSR.png) **ADSR:** this is a traditional ADSR envelope, defined by the rate of increase and decrease of value over time.
- ![](macro-button-LFO.png) **LFO:** the Low Frequency Oscillator generates a repeating wave of values.

Some macros are "bitmap" style. They represent a number of "bits" that can be toggled individually, and the values listed represent the sum of which bits are turned on.

### sequence

![sequence macro editor](macro-seq.png)

The number between the macro type label and the macro type button is the macro length in steps. The `-` and `+` buttons change the length of the macro. Start out by adding at least a few steps.

The values of the macro can be drawn in the "bar graph" box.

Just beneath the box is a shorter bar that controls looping.
- Click to set the start point of a loop; the end point is the last value or release point. It appears as half-height bars. Right-click to remove the loop.
- Shift-click to set the release point. When played, the macro will hold here until the note is released. It appears as a full-height bar. Right-click to remove the release point.

Finally, the sequence of values can be directly edited in the text box at the bottom.
- The loop start is entered as a `|`.
- The release point is entered as a `/`.
- In arpeggio macros, a value starting with a `@` is an absolute note (instead of a relative shift). No matter the note entered in the pattern, `@` values will be played at that exact note. This is especially useful for noise instruments with preset periods.

### ADSR

![ADSR macro editor](macro-ADSR.png)

- **Bottom** and **Top** determine the range of outputs generated by the macro. (Bottom can be larger than Top to invert the envelope!) All outputs will be between these two values.
- Attack, Decay, Sustain, SusDecay, and Release accept inputs between 0 to 255. These are scaled to the distance between Bottom and Top.
- **Attack** is how much the value moves toward Top with each tick.
- **Hold** sets how many ticks to stay at Top before Decay.
- **Decay** is how much the value moves to the Sustain level.
- **Sustain** is how far from Bottom the value stays while the note is held.
- **SusTime** is how many ticks to stay at Sustain until SusDecay.
- **SusDecay** is how much the value moves toward Bottom with each tick while the note is held.
- **Release** is how much the value moves toward Bottom with each tick after the note is released.

![macro ADSR chart](macro-ADSRchart.png)

### LFO

![LFO macro editor](macro-LFO.png)

- **Bottom** and **Top** determine the range of values generated by the macro. (Bottom can be larger than Top to invert the waveform!)
- **Speed** is how quickly the values change - the frequency of the oscillator.
- **Phase** is which part of the waveform the macro will start at, measured in 1/1024 increments.
- **Shape** is the waveform used. Triangle is the default, and Saw and Square are exactly as they say.



# wavetable

This tab appears for PC Engine, FDS, Namco WSG, and other wavetable-based instruments.

![wavetable tab](wavetable.png)

When **Enable synthesizer** is off, the wavetable used for the instrument may be selected by creating a Waveform macro with a single value.

To use the wavetable synthesizer, refer to [the wavetable synthesizer section](wavesynth.md).


# sample

This tab appears for Generic PCM DAC, Amiga and SNES.

![sample tab](sample-map.png)

see the [Generic Sample section](sample.md) for more information.
