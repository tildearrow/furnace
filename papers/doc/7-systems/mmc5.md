# Nintendo MMC5

a mapper chip which made NES cartridges exceeding 1MB possible.

it has two pulse channels which are very similar to the ones found in the NES, but lacking the sweep unit.

additionally, it offers an 8-bit DAC which can be used to play samples. only one game is known to use it, though.

# effects

- `12xx`: set duty cycle or noise mode of channel.
  - may be 0-3 for the pulse channels.
