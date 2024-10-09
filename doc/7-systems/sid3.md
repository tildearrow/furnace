# SID3

a fictional chip created by LTVA. the idea is to stay vaguely in [SID](c64.md)-like category of chips, but add a lot of features and more channels.

chip has 6 synth channels and one channel capable of playing wavetable or streamed samples.

each of synth channels has the following:

- two phase accumulator based oscillators, one for tone and one for noise LFSR clocking; frequency range is from 0.001Hz to around 15kHz at default clock speed.
- 5 waveform types which can be enabled in any combination: pulse (with 16-bit pulse width control), triangle, sawtooth, noise and so called special wave.
  - there are 58 different special waves, including all [OPL3](opl.md) and [OPZ](opz.md) waveforms, their clipped versions, cubed sawtooth and triangle variations, and more...
  - noise is generated from 30-bit LFSR. like in C64, eight of the bits are used to form 8-bit noise signal. user can adjust feedback freely, any bit can be feedback bit. some feedback bits configurations produce very short looped noise which is perceived as tone. see SID3 instrument description for notable feedback bits configurations which are automatically detected by Furnace: upon detection noise frequency is adjusted in such a way that fundamental frequency of such tonal noise becomes the note frequency the channel is currently playing (noise stays in tune).
    - 1-bit noise mode is available for [AY](ay8910.md) fans. it this mode the highest LFSR bit is read as output. by rapidly switching between usual and 1-bit noise modes one can produce interesting rattling-like percussive sound.
- 5 waveform mixing modes: 8580 SID (C64's combined waves; mode does bitwise AND with noise and special wave), bitwise AND, bitwise OR, bitwise XOR and sum of oscillators' signals.
- hard sync between channels. each channel can have any other channel as sync source, even itself.
- ring modulation between channels. each channel can have any other channel as modulation source, even itself. when you self-modulate, you effectively square the signal, but the behavior is a bit different.
- phase modulation between channels. each channel can have any other channel as modulation source, even itself. when you self-modulate, you have an effect similar to enabling strong feedback. channel output after filters is used as modulation source.
- ADSR envelope with sustain rate setting (how fast sound decays when envelope is in sustain phase).
- 4 independent filters. each filter has its own cutoff, resonance, output volume, mode, on/off and distortion setting. each filter can be connected to channel's ADSR output. each filter's output can be routed to the final channel output. each filter output can be connected to each filter's input (full connection matrix).
  - distortion is a simple asymmetrical distortion with hyperbolic tangent function for positive half of the wave and exponential function for negative half.
  - several filters can be chained for flexible subtractive synth or to increase filter's slope (which is 12 dB/octave for a single filter).
  - multiple filter modes can be selected simultaneously. for example, selecting both "low" and "high" results in a bandstop (notch) filter.
- adjustable feedback. feedback saves two previous channel's outputs and adds them to accumulator on the next step before computing waveform signal.
- fine control over left and right channel panning.
- left and right channels' signals can be inverted to create simple "surround" sound.

ADSR can be reset to the start of attack phase. phase of tone and noise oscillators can also be reset, and with noise oscillator reset noise LFSR is also reset to initial state.

wave channel has all these features, except, obviously, waveform generation stage, as well as feedback and noise generator.

## effects

- `1xxx`: **set filter 1 cutoff.** `xxx` range is `000` to `FFF`.
- `2xxx`: **set filter 2 cutoff.** `xxx` range is `000` to `FFF`.
- `3xxx`: **set filter 3 cutoff.** `xxx` range is `000` to `FFF`.
- `4xxx`: **set filter 4 cutoff.** `xxx` range is `000` to `FFF`.
- `5xxx`: **set duty cycle.** `xxx` range is `000` to `FFF`.
- `60xx`: **change wave.** lower 5 bits specify the wave:
  - `bit 0`: triangle
  - `bit 1`: saw
  - `bit 2`: pulse
  - `bit 3`: noise
  - `bit 4`: special wave
- `61xx`: **change special wave.** `xx` range is `00` to `39`.
- `62xx`: **modulation control.** lower 3 bits control the modulation:
  - `bit 0`: ring modulation
  - `bit 1`: oscillator sync
  - `bit 2`: phase modulation
- `63xy`: **reset duty cycle**:
  - if `x` is not 0: on new note
  - if `y` is not 0: now
- `64xx`: **set ring modulation source channel.** `xx` range is `00` to `07` where `07` means self-modulation and lower values specify a source channel.
- `65xx`: **set hard sync source channel.** `xx` is `00` to `06`.
- `66xx`: **set phase modulation source channel.** `xx` is `00` to `06`.
- `67xx`: **set attack.** `xx` range is `00` to `FF`.
- `68xx`: **set decay.** `xx` range is `00` to `FF`.
- `69xx`: **set sustain level.** `xx` range is `00` to `FF`.
- `6Axx`: **set sustain rate.** `xx` range is `00` to `FF`.
- `6Bxx`: **set release.** `xx` range is `00` to `FF`.
- `6Cxx`: **set waveform mix mode.** `xx` range is `00` to `04`.
- `6Dxx`: **set noise LFSR feedback bits (lower byte).** `xx` range is `00` to `FF`.
- `6Exx`: **set noise LFSR feedback bits (medium byte).** `xx` range is `00` to `FF`.
- `6Fxx`: **set noise LFSR feedback bits (higher byte).** `xx` range is `00` to `FF`.
- `70xx`: **set noise LFSR feedback bits (highest bits).** `xx` range is `00` to `3F`.
- `71xx`: **set filter 1 resonance.** `xx` range is `00` to `FF`.
- `72xx`: **set filter 2 resonance.** `xx` range is `00` to `FF`.
- `73xx`: **set filter 3 resonance.** `xx` range is `00` to `FF`.
- `74xx`: **set filter 4 resonance.** `xx` range is `00` to `FF`.
- `75xx`: **set noise/wave channel mode.** `xx` range is `00` to `01`. on synth channels `00` sets usual noise mode and `01` sets 1-bit noise mode. on wave channel `00` sets wavetable mode and `01` sets streamed PCM sample playback mode.
- `76xx`: **set filter 1 output volume.** `xx` range is `00` to `FF`.
- `77xx`: **set filter 2 output volume.** `xx` range is `00` to `FF`.
- `78xx`: **set filter 3 output volume.** `xx` range is `00` to `FF`.
- `79xx`: **set filter 4 output volume.** `xx` range is `00` to `FF`.
- `7Axx`: **set filter 1 distortion.** `xx` range is `00` to `FF`.
- `7Bxx`: **set filter 2 distortion.** `xx` range is `00` to `FF`.
- `7Cxx`: **set filter 3 distortion.** `xx` range is `00` to `FF`.
- `7Dxx`: **set filter 4 distortion.** `xx` range is `00` to `FF`.
- `7Exx`: **set feedback.** `xx` range is `00` to `FF`.
- `7Fxx`: **channel inversion control.** lower 2 bits control the channel signal inversion:
  - `bit 0`: invert right channel
  - `bit 1`: invert left channel
- `A0xy`: **set filter mode.** `x` is the filter (`0-3`), and lower 3 bits of `y` control the mode:
  - `bit 0`: low pass
  - `bit 1`: band pass
  - `bit 2`: high pass
- `A1xy`: **set filter connection.** `x` is the filter (`0-3`), and lower 2 bits of `y` control the connection:
  - `bit 0`: connect filter input to channel's ADSR output
  - `bit 1`: connect filter's output to final channel output
- `A2xy`: **set filter connection matrix row.** `x` is the filter (`0-3`), and lower 4 bits of `y` control the inter-filter connections:
  - `bit 0`: connect filter input to filter 1 output
  - `bit 1`: connect filter input to filter 2 output
  - `bit 2`: connect filter input to filter 3 output
  - `bit 3`: connect filter input to filter 4 output
- `A3xy`: **enable filter.** `x` is the filter (`0-3`), `y` is either `0` (filter disabled) or `1` (filter enabled).
- `A4xx`: **pulse width slide up.** `xx` is speed. `A400` stops the slide.
- `A5xx`: **pulse width slide down.** `xx` is speed. `A500` stops the slide.
- `A6xx`: **filter 1 cutoff slide up.** `xx` is speed. `A600` stops the slide.
- `A7xx`: **filter 1 cutoff slide down.** `xx` is speed. `A700` stops the slide.
- `A8xx`: **filter 2 cutoff slide up.** `xx` is speed. `A800` stops the slide.
- `A9xx`: **filter 2 cutoff slide down.** `xx` is speed. `A900` stops the slide.
- `AAxx`: **filter 3 cutoff slide up.** `xx` is speed. `AA00` stops the slide.
- `ABxx`: **filter 3 cutoff slide down.** `xx` is speed. `AB00` stops the slide.
- `ACxx`: **filter 4 cutoff slide up.** `xx` is speed. `AC00` stops the slide.
- `ADxx`: **filter 4 cutoff slide down.** `xx` is speed. `AD00` stops the slide.
- `AExx`: **tone phase reset.** `xx` is the tick on which the phase reset happens.
- `AFxx`: **noise phase reset.** `xx` is the tick on which the phase reset happens.
- `B0xx`: **envelope reset.** `xx` is the tick on which the envelope reset happens.
- `B1xy`: **filter cutoff scaling control.** `x` is the filter (`0-3`), and lower 2 bits of `y` control the scaling:
  - `bit 0`: enable cutoff scaling
  - `bit 1`: inverse cutoff scaling
- `B2xy`: **filter resonance scaling control.** `x` is the filter (`0-3`), and lower 2 bits of `y` control the scaling:
  - `bit 0`: enable resonance scaling
  - `bit 1`: inverse resonance scaling

## info

this chip uses the [SID3](../4-instrument/sid3.md) instrument editor.

## chip config

the following options are available in the Chip Manager window:

- **Quarter clock speed**: make chip run on quarter the default clock rate (1MHz is default). this lowers CPU load almost 4 times at the cost of filters becoming unstable or having different timbre at high cutoff and resonance settings. option affects the chip only in playback mode. when you render module into audio file, option is not applied.
