## v0.1:
LTVA:
- Macros are now dynamically allocated, adds unexpected bugs but improves RAM usage. Instrument and each FM op can have up to 32 macros each, although yet no system exists that uses that many. The same dynamic allocation, although simplified, is done for hardware sequences and note table which is used in sample map
- Fixed annoying one-frame glitch in macros menu (when you open instrument the first time; pay the attention to the area where macro names are displayed)
- remove "guru mode" (always expose full color settings)
- throw a warning and refuse to load instrument/song file if encountered instrument is of unknown type
- remove the ability to **save** instrument in old format (useless and afaik never even used in the program)
- CSM variants for YM2203, YM2608, YM2610 and YM2610B chips
- Hide FM chips variations (ext3 ch, drums mode, CSM) into spoilers to reduce FM chips tab clutter in chips manager
- Do the same thing in presets menu
- Make a menu in settings->general where you select default chips a proper menu just like in chip manager
- Move useful cheat codes into debug->settings and delete the ridiculous one (that just forces shitty padding for all UI elements)
- Add Ensoniq ES5503 system
- Add filter on/off macro for C64 instrument
- Apply some imgui patch so text in song/subsong info/comments window actually wraps (and add a setting to toggle the wrap)
- cap max arrow size in arp and pitch macros so arrow (which emerges if you scroll arp/pitch macro vertically and the values go out of bounds) doesn't freak out in grid macro layout
- apply frame shading to song/subsong info text fields (and a setting to toggle it)
- refactor instrument editor GUI code - now each instrument is drawn by separate function instead of big mess that was there before
- remove the ability to make pattern length 0 by repeatedly applying "collapse song"
- Add localization code and Russian translation
- Break compatibility with tildearrow Furnace because of PowerNoise and ES5503 instrument ID conflicts: you can load tildearrow's Furnace modules just fine, inst IDs will be converted, but the format (module and instrument) magic was changed so you won't be able to load Furnace-B modules in original Furnace.

freq-mod:
- 86-PCM streo DAC fix
- Remove basic mode, tutorial and intro
- Remove certian compatibility flags from the GUI (notably, old Furnace compatibility)

Electric Keet:
- "Furnace-B" name
- The invaluable and enormous tiresome work of placing localization macros all over the GUI code
