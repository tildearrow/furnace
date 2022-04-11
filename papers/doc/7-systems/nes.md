# NES

the console from Nintendo that plays Super Mario Bros. and helped revive the agonizing video game market in the US during mid-80s.

also known as Famicom. It is a five-channel PSG: first two channels play pulse wave with three different duty cycles, third is a fixed-volume triangle channel, fourth is a noise channel (can work in both pseudo-random and periodic modes) and  fifth is a (D)PCM sample channel

# effects

- `12xx`: set duty cycle or noise mode of channel.
  - may be 0-3 for the pulse channels and 0-1 for the noise channel.
- `13xy`: setup sweep up.
  - `x` is the time.
  - `y` is the shift.
  - set to 0 to disable it.
- `14xy`: setup sweep down.
  - `x` is the time.
  - `y` is the shift.
  - set to 0 to disable it.
