# tuning samples

loading a new sample into Furnace is easy... but getting it transposed and tuned to match the song can be tricky at first.

it's important to remember that the "Hz" and "Note" as shown in the sample editor are unrelated to the note heard in the sample itself. a sample shown as having a "Note" of C-4 will use a sample rate of 4181, even though it may contain a note played at a different pitch than C.

for this example, we'll use a sample of a note played at E and recorded at 22050Hz.

- if needed, use the "Create instrument from sample" button to make an instrument to use in the track.
- calculate the semitone difference in Hz between the note your recorded sample is playing and C. in this example, the nearest C is 4 semitones down from E.
- set "Note" to 4 semitones lower than it shows. in this case, it starts at `F-6`, so set it to `C#6`.
	- or, use a pitch calculator like https://www.omnicalculator.com/other/semitone. input Frequency 1 as 22050Hz, input -4 semitones, and receive a Frequency 2 of 17501.10Hz. enter that value into "Hz".
- now, using the "Preview sample" button should play the note at C. entering an E in the pattern will now play it at or near E.
- if the sample still sounds out of tune, adjust "Hz" or "Fine" to bring it in line.

if notes seem "capped" – for example, playing anything over D-6 sounds like a D-6 – those notes exceed the maximum sample playback rate for the chip. the only solution is to use "Resample" to change the sample to a lower rate.
