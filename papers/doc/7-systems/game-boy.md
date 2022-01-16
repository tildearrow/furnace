# Game Boy

the Nintendo Game Boy is one of the most successful portable game systems ever made.

with stereo sound, two pulse channels, a wave channel and a noise one it packed some serious punch.

# effects

- `10xx`: change wave.
- `11xx`: set noise length. `xx` may be one of:
  - 0: long
  - 1: short
- `12xx`: set duty cycle (from 0 to 3).
- `13xy`: setup sweep (pulse channels only).
  - `x` is the time.
  - `y` is the shift.
  - set to 0 to disable it.
- `14xx`: set sweep direction. 0 is up and 1 is down.
