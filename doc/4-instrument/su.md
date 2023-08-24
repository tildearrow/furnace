# Sound Unit instrument editor

## Sound Unit tab
- **Use sample**: Enable playing back samples on this channel.
- **Switch roles of frequency and phase reset timer**: Whether the pitch of the channels should be the speed at which the channel phase resets, or it should be the absolute pitch on the channel.
- **Initial Sample**: Select the sample you want to use here.
- **Use sample map**: Enables the sample map on this channel.

## Macros tab
- **Volume**: Standard linear volume macro.
- **Arpeggio**: Coarse pitch macro, in half steps.
- **Waveform**: Select the waveform to use on this channel.
  - 0: Pulse
  - 1: Saw
  - 2: Sine
  - 3: Triangle
  - 4: Noise
  - 5: Short noise
  - 6: Sine (XOR)
  - 7: Saw (XOR)
- **Panning**: Signed 8-bit panning macro. 0 is absolute center, 127 is fully right, -128 is fully left.
- **Pitch** Fine pitch macro.
- **Phase reset**: When this bit is set, it will phase reset the channel.
- **Cutoff**: Filter strength on this channel.
- **Resonance**: How much to boost the highest allowed frequency in the filter.
- **Control**:
  - Band pass: Allow only the frequencies in a range.
  - High pass: Allow only the specified frequency in the cutoff macro up to the highest possible frequency.
  - Low pass: Allow every frequency from the lowest that it can produce up to the filter cutoff's value. Opposite of the high pass filter.
  - Ring mod: Modulate this channel by the output of the next one. The next channel must have ring mod off for it to make sound.
- **Phase Reset Timer**: How fast to phase reset this channel, Alternatively, if the "Switch roles of frequency and phase reset timer" checkbox is ticked, then this macro becomes the absolute frequency of this channel.
