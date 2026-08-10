# raw note entry

in some cases, you might find it limiting to enter notes purely by note names. it usually occurs when the Furnace octave range (C-(-5) to B-9) is not sufficient, or when you need to play exact frequencies.

Furnace 0.7 introduces raw frequency/period notes. this allows you to directly enter frequency/period values that will be written to the chip's pitch registers as-is.

# how to use raw notes?

place the cursor somewhere in the note column of a channel. enter edit mode and hit the Minus (`-`) key (by default).

this will replace the note cell with an hexadecimal number, where you can enter a raw frequency/period value with the 0-9 and A-F keys.

once you enter the last digit, the note will be previewed.

pressing the Minus key on a raw note will make it return to normal mode.

# warning!

the following effects are unavailable in raw frequency/period mode:

- `00xy` (arpeggio)
- `04xy` (vibrato)
- `E1xy` (quick pitch slide up)
- `E2xy` (quick pitch slide down)
- `E5xx` (pitch)

the arp macro is ignored as well.

the good news is that the pitch macro is available. you may use that instead.

Furnace will not attempt to clamp frequency values when using the pitch macro. you must take care to ensure the value doesn't overflow!

bear in mind that chips differ in how their registers represent frequency:
- some are accumulators ("frequency"), where higher values will mean higher pitch.
- others are divisors ("periods"), where higher values result in lower frequencies.
- certain chips are in linear space (such as OPM, OPZ and MSM5232) where the value represents a note with a pitch offset.
- MultiPCM, OPN, OPL and OPLL chips use a two-part value:
  - the upper bits represent Block, selecting the octave.
  - the lower bits are the F-Num, which is the frequency that will be shifted left by the Block.
- SAA1099 uses a floating-point period value with a 3-bit exponent, 8-bit mantissa and no sign bit.
- POKEY is in periodic space but some shapes produce different sounds (or nothing at all) depending on the value.
  - additionally, two channels may be combined into a 16-bit register...

there are tons of other weird situations but I'm not gonna document them right now as I gotta work on the rest of the software. sorry.

good luck!
