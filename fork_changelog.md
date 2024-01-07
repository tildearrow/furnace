## v0.1:
LTVA:
- Macros are now dynamically allocated, adds unexpected bugs but improves RAM usage. Instrument and each FM op can have up to 32 macros each, although yet no system exists that uses that many. The same dynamic allocation, although simplified, is done for hardware sequences and note table which is used in sample map
- Fixed annoying one-frame glitch in macros menu (when you open instrument the first time; pay the attention to the area where macro names are displayed)
- Added options to make tabs borders round or sharp
- remove "guru mode" (always expose full color settings)
- throw a warning and refuse to load instrument/song file if encountered instrument is of unknown type
- remove the ability to **save** instrument in old format (useless and afaik never even used in the program)
- CSM variants for YM2203, YM2608, YM2610 and YM2610B chips
- Hide FM chips variations (ext3 ch, drums mode, CSM) into spoilers to reduce FM chips tab clutter in chips manager
- Do the same thing in presets menu
- Remove ability to save `.dmf` files
- Add ESFM system. The code is courtesy of @Kagamiin, I just added per-op arpeggio and pitch macros which Kagamiin actually adopted in tilde's Furnace PR. And, of course, I edited the code to match new macro system.
- Make a menu in settings->general where you select default chips a proper menu just like in chip manager
- Move useful cheat codes into debug->settings and delete the ridiculous one (that just forces shitty padding for all UI elements)
- Add Ensoniq ES5503 system
- Add filter on/off macro for C64 instrument
- Apply some imgui patch so text in song/subsong info/comments window actually wraps (and add a setting to toggle the wrap)
- cap max arrow size in arp and pitch macros so arrow (which emerges if you scroll arp/pitch macro vertically and the values go out of bounds) doesn't freak out in grid macro layout
- apply frame shading to song/subsong info text fields (and a setting to toggle it)

freq-mod:
- 86-PCM streo DAC fix
- Remove basic mode, tutorial and intro

Electric Keet:
- "Furnace-B" name
