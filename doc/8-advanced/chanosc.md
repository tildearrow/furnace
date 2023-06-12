# oscilloscope (per channel)

The "Oscilloscope (per channel)" dialog shows an individual oscilloscope for each channel during playback.

![oscilloscope per-channel configuration view](chanosc.png)

Right-clicking within the view will change it to the configuration view shown above:
- "Columns" sets the number of columns the view will be split into.
- "Size (ms)" sets what length of audio is visible in each oscilloscope.
- "Center waveform" does its best to latch to the channel's note frequency and centers the display.
- "Gradient" is presently unimplemented.
- The color selector allows setting the waveform color. Right-clicking on it pops up an option dialog:
  - Select between the square selector and the color wheel selector.
  - "Alpha bar" adds a transparency selector.
- The boxes below that are for selecting colors numerically by red-green-blue-alpha, hue-saturation-value-alpha, and HTML-style RGBA in hex.
- The OK button returns from options view to regular.
