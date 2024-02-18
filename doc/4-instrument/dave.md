# Dave instrument editor

the Dave instrument editor consists of these macros:

- **Volume**: volume sequence.
- **Arpeggio**: pitch sequence.
- **Noise Freq**: set noise frequency source.
  - 0: fixed frequency (~62.5KHz)
  - 1: channel 1
  - 2: channel 2
  - 3: channel 3
- **Waveform**: select waveform or noise length.
  - 0: square
  - 1: bass
  - 2: buzz
  - 3: reed
  - 4: noise
  - for noise channel the range is 0 to 3.
- **Panning (left)**: output level for left channel.
- **Panning (right)**: output level for right channel.
- **Pitch**: fine pitch.
- **Phase Reset**: trigger restart of waveform.
  - does not apply for noise channel.
- **Control**: set channel parameters.
  - **low pass (noise)**: enable low-pass filter. only in noise channel.
  - **swap counters (noise)**: enable swap counters mode. only in noise channel.
  - **ring mod**: enable ring mod with channel+2.
  - **high pass**: enable high-pass filter with the next channel.
