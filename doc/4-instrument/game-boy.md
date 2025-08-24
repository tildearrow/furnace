# Game Boy instrument editor

the Game Boy instrument editor consists of three tabs: one controlling envelope of sound channels, another for the wave synth and macro tab containing several macros.

## Game Boy

- **Use software envelope**: switch to volume macro instead of envelope.
  - this exploits a bug in the Game Boy sound hardware in order to have software envelopes ("zombie mode", explained more in the [Game Boy system documentation](../7-systems/game-boy.md)).
  - only a couple emulators have accurate reproduction of this bug.
- **Initialize envelope on every note**: forces a volume reset on each new note.
- **Volume**: initial channel volume (0 to 15).
- **Length**: envelope decay/attack duration (0 to 7)
- **Sound Length**: cuts off channel after specified length, overriding the Length value.

- **Direction**: up makes the envelope an attack. down makes it decay.
  - note: for attack to have effect, start at a lower volume.

- **Hardware Sequence**: this allows you to define a sequence of hardware envelope changes for creating complex envelopes. see the next section for more information.

### hardware sequence

Furnace provides a sequencer for the hardware envelope. this way you can define timed envelope changes which may be used for simulating ADSR, adding simple release, and other things.

the sequence consists of a list of "commands".

the `+` button adds a new command, which may be one of the following:

- **Envelope**: sets envelope values and retriggers note. it is highly recommended to have this as the first command.
- **Sweep**: sets sweep parameters. only works on the first channel.
- **Wait**: waits a specific number of ticks.
- **Wait for Release**: waits until the note is released with `===` or `REL`.
- **Loop**: goes to a previous position in the sequence.
- **Loop until Release**: same as Loop, but doesn't have effect after releasing the note.

each command in the sequence is represented in three columns:

- **Tick**: the tick this command will execute, followed by position in the sequence.
- **Command**: the command and its parameters.
- **Move/Remove**: allows you to move the command, or remove it.

## Wavetable

this allows you to enable and configure the Furnace wavetable synthesizer. see [this page](wavesynth.md) for more information.

notes:
- only for Wave channel.
- on Game Boy, using the wave synth may result in clicking and/or phase resets. by default Furnace attempts to mitigate this problem though, but some clicking may still be audible.

## Macros

- **Volume**: volume sequence.
  - note: this only appears if "Use software envelope" is checked.
- **Arpeggio**: pitch sequence.
- **Duty/Noise**: pulse wave duty cycle or noise mode sequence.
- **Waveform**: channel 3 wavetable sequence.
- **Panning**: output for left and right channels.
- **Pitch**: fine pitch.
- **Phase Reset**: trigger restart of waveform.
