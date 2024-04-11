# using samples with limited playback rates

some sample-based chips have a limited number of available sample playback rates. when working with these chips, notes entered in the pattern editor will play back at the closest available rate... which might be perfect, or might be several semitones off. the solution is to prepare samples to work around this.

for example: using the NES, a `C-4` note in the PCM channel means the associated sample will play back at a rate of 8363Hz. let's say we want to use a slap bass sample recorded at a rate of 22050Hz in which the audible pitch is A-2. let's also say that when we put a `C-4` note in the tracker, we want to hear the bass play at what sounds like C-3, transposed three semitones higher than the recorded pitch.

here's how to make this example work:

- load up the sample and open it in the sample editor.
- the Note selector will show "F-6"; add the three semitones mentioned above to make it "G#6". the Hz will change to 26217.
- use the Resample button. in the pop-up dialog, type in `8363`, then click Resample.
- select the instrument from the instrument list, and in the pattern editor, enter a `C-4` in the PCM channel. it should sound like a slap bass playing a C-3 note.

the NES PCM frequency table shows the sixteen notes can be played. if a `D-4` is entered, the slap bass will be heard at D-3 as desired. what if we want to hear a C#3, though?

- load up the original sample in a new slot and open it in the sample editor.
- the Note selector will show "F-6"; this time we add four semitones to make it "A-6". the Hz will change to 27776.
- just like before, use the Resample button. in the little pop-up, type in `8363` (yes, the NES's C-4 rate), then click Resample.
- select the instrument from the instrument list and open it in the instrument editor. turn on "Use sample map".
- in the leftmost column, find C#5. click in the next column and enter the number of the second sample. in the next column to the right, click the "C#4" and hit the key for C-4 to change it.
- in the pattern editor, enter a `C#4`; it should sound like a C#3!
- try adding another entry in the sample map so the note D#4 plays the second sample at D-4.


