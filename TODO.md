# to-do for ES5506

- envelope shape
- reversed playing flag in instrument/macro/commands
- transwave synthesizer (like ensoniq synths - 12 bit command and macro)

# to-do for 0.6pre1

- implement Defle slide bug when using E1xy/E2xy and repeating origin note (requires format change)

# to-do for 0.6pre2 (as this requires new data structures)

- rewrite the system name detection function anyway
  - this involves the addition of a new "system" field in the song (which solves the problem)
  - songs made in older versions will go through old system name detection for compatibility
- Game Boy envelope macro/sequence
- volume commands should work on Game Boy
- ability to customize `OFF`, `===` and `REL`
- stereo separation control for AY
