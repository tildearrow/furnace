# Philips SAA1099 instrument editor

The SAA1099 instrument editor consists of these macros:

- **Volume**: volume sequence
- **Arpeggio**: pitch sequence
- **Duty/Noise**: noise generator frequency
- **Waveform**: selector between tone and noise
- **Panning (left)**: output level for left channel
- **Panning (right)**: output level for right channel
- **Pitch**: fine pitch
- **Envelope**: envelope generator settings:
  enable: enables the envelope generator
  N/A: has no effect
  fixed: toggles whether to use a fixed frequency or lock to the frequency of channel 2 or 5
  resolution: increases the envelope generator pitch resolution
  direction: inverts the waveform around Y axis
  cut: cuts the waveform (producing saw wave out of tri wave)
  loop: toggles wheteher evelope is one-off or constantly looping
  mirror: sets whether the right output will mirror the left one
