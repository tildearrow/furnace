# wavetable editor

Wavetable synthizers, in context of Furnace, are sound sources that operate on extremely short n-bit PCM streams. By extremely shit, no more than 256 bytes. this amount of space is nowhere near enough to store an actual sampled sound, it allows certain amount of freedom to define a waveform shape. As of Furnace 0.5.5, wavetable editor affects PC Engine  and channel 3 of Game Boy.

Furnace's wavetable editor is rather simple, you can draw the waveform using mouse or by pasting an MML bit stream in the input field. Maximum wave width (length) is 256 bytes, and maximum wave height (depth) is 256. NOTE: both Game Boy and PCE can handle max 32 byte waveforms as of now, width 16-level height for GB and 32-level height for PCE. If larger wave will be defined for these two systems, it will be squashed to fit in the constrains of the system.  
