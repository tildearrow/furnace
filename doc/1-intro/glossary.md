# glossary of common terms

**2-op**, **3-op**, **4-op**...: the number of FM operators used to generate a sound. more operators allow for more complex sounds.

**ADPCM**: adaptive differential pulse code modulation. this is a variety of DPCM with a more complex method of storing the amplitude differences.

**ADSR**: attack, decay, sustain and release. these are elements that comprise a basic envelope.

**algorithm**: the way in which the operators in an FM instrument interact.
- when two operators connect to the same point, their sounds are added together.
- when two operators are connected left to right, the left is the modulator and the right is the carrier sound that is modified.

**asset**: an instrument, wavetable or sample.

**bit**: a single binary on-off value.

**bitbang**: to achieve PCM sound by sending a rapid stream of volume commands to a non-PCM channel.

**bitmask**: a set of bits which represent individual single-bit toggles or groups representing small numbers. these are explained fully in the [hexadecimal primer](hex.md).

**BRR**: a lossy sample format used by the SNES. it has a fixed compression ratio; groups of 32 bytes (16 samples) are encoded in 9 bytes each.
- usually stored in .brr files.

**clipping**: when a sample or playback stream exceeds the maximum or minimum values. this can cause audible distortion.
- this often occurs when a sample is amplified too much.
- it can also occur during playback if too much sound is being added together at once. in some cases the mixer can be used to reduce the volume. if this doesn't work, the clipping is caused within the chip's own mixing, and the only solution is to reduce the volumes of the notes being played.

**clock rate**: the timing at which a chip operates, expressed as cycles per second (Hz).
- changing this may change aspects of how some chips work, most notably pitch.
- some chips cannot operate at anything other than their designed clock rate.

**cursor** (1): the marker of input focus. anything typed will happen at the cursor's location.

**cursor** (2): the pointer controlled by a mouse or similar input. clicking when the cursor(2) is in a valid area will place the cursor(1) there.

**DAC**: digital analog converter. this converts a digital representation of sound into actual output.

**.dmf**: DefleMask Module File.
- _Furnace:_ .dmf files may be read, and compatibility flags will be set to make them play as accurately as possible, but there may still be glitches.
- _Furnace:_ .dmf files may be saved, but full compatibility isn't guaranteed and many features will be missing. this isn't recommended unless absolutely necessary.

**.dmp**: DefleMask Preset. an instrument file.

**.dmw**: DefleMask Wavetable. a wavetable file.

**DPCM**: differential/delta pulse code modulation. this is a variety of PCM that stores each amplitude as its difference from the previous.

**duty cycle**: usually called _pulse width._ in a pulse wave, this is the ratio of the high part to the high and low combined.

**feedback**: in FM instruments, this adds some of an operator's output into itself to create complex harmonics.
- in the algorithm view, an operator with a circle around it is capable of feedback.

**FM**: frequency modulation. this is a method of generating sound that uses one operator's amplitude to modify another operator's frequency.
- the FM in Yamaha chips is more accurately called _phase modulation,_ which uses a different method of computation to achieve similar results.

**.ftm**: FamiTracker Module.

**.fui**: a Furnace instrument file.

**.fur**: a Furnace module file.

**.fuw**: a Furnace wavetable file.

**hard-pan**: sounds can only be panned to the center, 100% left, or 100% right. this often appears in the instrument editor's panning macro as on/off toggles for the left and right channels.

**Hz**: hertz. a unit representing divisions of one second. 1 Hz means once per second; 100 Hz means one hundred times per second. also, _kHz_ (kilohertz, one thousand per second) and _MHz_ (megahertz, one million per second).

**interpolate**: to fill in the area between two values with a smooth ramp of values in between.
- some sample-based chips can interpolate, filtering out unwanted harmonics.

**.it**: Impulse Tracker module.

**ladder effect**: an inaccurate yet common term for the DAC distortion that affects some Yamaha FM chips.

**LFO**: low frequency oscillator. a wave with a slow period (often below hearing range) used to alter other sounds.

**LFSR**: linear-feedback shift register. a method to generate pseudo-random noise that loops, also known as "periodic noise". within a sequence of on-off bits, it does math to combine the bits at specified locations called "taps", then shifts the whole sequence and adds the resulting bit on the end, guaranteeing a different state for the next pass. depending on the locations of the taps, different lengths of noise loops are generated; for short loops, this will affect their tone.

**macro**: a sequence of values automatically applied while a note plays.

**noise bass**: the technique of using a PSG's periodic noise generator with a very short period to create low-frequency sounds.

**normalize**: to adjust the volume of a sample so it is as loud as possible without adding distortion from clipping.

**operator**: in FM, a single oscillator that interacts with other oscillators to generate sound.

**oscillator**: a sine wave or other basic waveform used as sound or to alter sound.

**PCM**: pulse code modulation. a stream of data that represents sound as a rapid sequence of amplitudes.

**period**: the length of a repeating waveform. as frequency rises, the period shortens.

**periodic noise**: an approximation of random noise generated algorithmically with an LFSR.
- the period is the number of values generated until the algorithm repeats itself.

**phase reset**: to restart a waveform at its initial value.
- for FM instruments, this restarts the volume envelope also.

**PSG**: programmable sound generator. any sound chip is a PSG, though the term is often used to specifically refer to chips that produce only simple waveforms and noise.

**pulse wave**: a waveform with a period consisting of only two amplitudes, high and low. also known as a rectangular wave.

**pulse width**: sometimes called _duty cycle._ in a pulse wave, this is the ratio of the high part to the high and low combined.

**release**: the part of a note that plays after it's no longer held, or the part of a macro the plays after it stops looping. usually applies at key off.

**resample**: to convert a sample to a different playback rate.
- this is a "lossy" process; it usually loses some amount of audio quality. the results can't be converted back into the original rate without further loss of quality.
- resampling to a lower rate reduces the amount of memory required, but strips away higher frequencies in the sound.
- resampling to a higher rate cannot recover missing frequencies and may add unwanted harmonics along with greater memory requirements.

**raw**: a sample or wavetable file without a header. when loading such a file, the format must be set properly or it will be a mess.

**register**: a memory location within a sound chip. "register view" shows all the relevant memory of all chips in use.

**.s3m**: ScreamTracker 3 Module.

**sample** (1): a digitally recorded sound. usually stored as some variant of PCM.
- these can take up a lot of room depending on length and sample rate, thus older systems tend to use short, lower quality samples.

**sample** (2): a single value taken from a digitally recorded sound. a sample(1) is made up of samples(2).

**signed**: a digital representation of a number that may be negative or positive.
- if an imported raw sample sounds recognizable but heavily distorted, it's likely to be unsigned interpreted as signed or vice-versa.

**software mixing**: mixing multiple channels of sound down to a single stream to be sent to a PCM channel.
- this puts a heavy load on the CPU of the host system, so it was rarely used in games.
- _Furnace:_ this is used for DualPCM and QuadTone.

**square wave**: a wave consisting of only two values, high and low, with equal durations within the wave's period.
- this is equivalent to a pulse wave with a duty of 50%.

**supersaw**: a sound made up of multiple saw waves at slightly different frequencies to achieve a chorusing effect.

**tap**: a specified bit location within an LFSR.

**tick rate**: the number of times per second that the sound engine moves forward. all notes and effects are quantized to this rate.
- this usually corresponds to the frame rate the system uses for video, approximately 60 for NTSC and 50 for PAL.

**unsigned**: a digital representation of a number that can only be positive.
- if an imported raw sample sounds recognizable but heavily distorted, it's likely to be signed interpreted as unsigned or vice-versa.

**.vgm**: Video Game Music. a file containing the log of data sent to a sound chip during sound playback.
- saving to a .vgm file may be compared to "converting text to outlines" or similar irreversible processes. the results cannot be loaded back into the tracker.
- different versions of the VGM format have different capabilities, with trade-offs. older versions may lack chips or features; newer versions may not be compatible with some software.
- samples are stored uncompressed. PCM streams (such as DualPCM) can quickly take up a huge amount of space.

**waveform**: a very short period of repeating sound.
- the most basic waveform is a sine wave. others include triangle, pulse, saw, and the like.

**wavetable** (1): a very short looping sample.

**wavetable** (2): an ordered group of wavetables(1) used in sequence within a single instrument.

**.xm**: eXtended Module. the file format of songs made with FastTracker 2.

**.zsm**: ZSound Music. a VGM-like file meant specifically for the Commander X16 computer.
