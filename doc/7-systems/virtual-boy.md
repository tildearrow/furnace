# Virtual Boy

a "portable" video game console made by Nintendo in the '90s.

it supposedly was the beginning of virtual reality... nah, instead it failed to sell well because you use it for 15 minutes and then you get a headache.

its sound generation chip is called Virtual Sound Unit (VSU), a wavetable chip that is a lot like PC Engine, but unlike that, the waves are twice as tall, it doesn't go too low in terms of frequency (~D-2), and the last channel (yep, it has 6 channels) is a noise one.

additionally, channel 5 offers a modulation/sweep unit. the former is similar to FDS' but has much reduced speed control.

## effects

- `10xx`: **set waveform.**
- `11xx`: **set noise length.** range is `0` to `7`.
  - only in the noise channel.
- `12xy`: **setup envelope.**
  - `x` determines whether envelope is enabled or not.
    - `0`: disabled
    - `1`: enabled
    - `3`: enabled and loop
    - yeah, the value `2` isn't useful.
  - `y` sets the speed and direction.
    - `0-7`: down
    - `8-F`: up
- `13xy`: **setup sweep.**
  - `x` sets the speed.
    - `0` and `8` are "speed 0" - sweep is ineffective.
  - `y` sets the shift (`0` to `7`).
    - `8` and higher will mute the channel.
  - only in channel 5.
- `14xy`: **setup modulation.**
  - `x` determines whether it's enabled or not.
    - 0: disabled
    - 1: enabled
    - 3: enabled and loop
    - 2 isn't useful here either.
  - `y` sets the speed.
    - `0` and `8` are "speed 0" - modulation is ineffective.
    - no, you can't really do Yamaha FM using this.
  - only in channel 5.
- `15xx`: **set modulation wave.**
  - `xx` points to a wavetable. range is `0` to `FF`.
  - this is an alternative to setting the modulation wave through the instrument.

## info

this chip uses the [Virtual Boy](../4-instrument/virtual-boy.md) instrument editor.

## chip config

the following options are available in the Chip Manager window:

- **Waveform storage mode**: selects how will waveforms be loaded.
  - Static: loads the first 5 waves only.
  - Dynamic: allows switching between more than 5 waves, but loading a new wave forces all channels off and back on again, usually with an audible click.
    - **Disable anti-phase-reset**: disables the workaround to a phase reset that occurs on all channels when the waveform of any channel is changed.
    - **I don't care about hardware**: ignores the 5-wave limitation and allows dynamic waveforms without channel resets or clicking. _note:_ this is not hardware accurate!
