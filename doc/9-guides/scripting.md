# scripting

scripts allow you to extend the capabilities of furnace by using code written in the [Lua](https://lua.org) programming language.

## guidelines

- **scripts shall not modufy the `fur` table**
- **scripts shall not depend on other scripts, nor interfere with them**

  to ensure scripts do not interfere with each other, each script may:
  - store its data inside a table
  - prefix its expression names (the prefix shall be unique to a script)

## API reference

### functions

all functions are defined in the `fur` table.
all functions use 0-indexing.

#### general functions

- `fur.version()`

  arguments: none

  return type: integer

  returns the furnace version number

- `fur.versionStr()`

  arguments: none

  return type: string

  returns the furnace version string

- `fur.showError()`

  arguments: error text (string)

  return type: none

  shows an error popup

- `fur.inspect()`

  arguments: any

  return type: string

  returns the input as a string

- `fur.logV()`

  arguments: log message (string)

  return type: none

  appends a trace log

- `fur.logD()`

  arguments: log message (string)

  return type: none

  appends a debug log

- `fur.logI()`

  arguments: log message (string)

  return type: none

  appends an info log

- `fur.logW()`

  arguments: log message (string)

  return type: none

  appends a warning log

- `fur.logE()`

  arguments: log message (string)

  return type: none

  appends an error log

#### cursor functions

- `fur.cursor.getPos()`

  arguments: none

  return type: 3×integer

  returns the pattern cursor position (channel, column, row)

- `fur.cursor.setPos()`

  arguments: cursor channel (integer), cursor column (integer), cursor row (integer)

  return type: none

  sets the pattern cursor position

- `fur.cursor.getSelStart()`

  arguments: none

  return type: 3×integer

  returns the pattern selection start position (channel, column, row)

- `fur.cursor.setSelStart()`

  arguments: channel (integer), column (integer), row (integer)

  return type: none

  sets the pattern selection start position

- `fur.cursor.getSelEnd()`

  arguments: none

  return type: 3×integer

  returns the pattern selection end position (channel, column, row)

- `fur.cursor.setSelEnd()`

  arguments: channel (integer), column (integer), row (integer)

  return type: none

  sets the pattern selection end position

#### engine state functions

- `fur.engine.getCurOrder()`

  arguments: none

  return type: integer

  returns the current order number

- `fur.engine.getCurRow()`

  arguments: none

  return type: integer

  returns the current row number

- `fur.engine.getPlayTimeSec()`

  arguments: none

  return type: integer

  returns the play time seconds

- `fur.engine.getPlayTimeMicro()`

  arguments: none

  return type: integer

  returns the play time microseconds

- `fur.engine.getPlayTimeTicks()`

  arguments: none

  return type: integer

  returns the play time in ticks

- `fur.engine.isPlaying()`

  arguments: none

  return type: boolean

  returns whether the engine is playing

- `fur.engine.isRunning()`

  arguments: none

  return type: boolean

  returns whether the engine is running

- `fur.engine.isFreelance()`

  arguments: none

  return type: boolean

  i still dont understand what freelance means

- `fur.engine.getChanCount()`

  arguments: none

  return type: integer

  returns the total number of channels in the song

- `fur.engine.getCurSubSong()`

  arguments: none

  return type: integer

  returns the current subsong number

#### interface functions

- `fur.interface.getEditOrder()`

  arguments: none

  return type: integer

  returns the edit order(?)

- `fur.interface.getCurIns()`

  arguments: none

  return type: integer

  returns the id of the current instrument

- `fur.interface.getCurWave()`

  arguments: none

  return type: integer

  returns the id of the current wavetable

- `fur.interface.getCurSample()`

  arguments: none

  return type: integer

  returns the id of the current sample

- `fur.interface.setCurIns()`

  arguments: instrument id (integer)

  return type: none

  sets the current instrument

- `fur.interface.setCurWave()`

  arguments: wavetable id (integer)

  return type: none

  sets the current wavetable

- `fur.interface.setCurSample()`

  arguments: sample id (integer)

  return type: none

  sets the current sample

- `fur.interface.getOctave()`

  arguments: none

  return type: integer

  returns the current octave

- `fur.interface.getEditStep()`

  arguments: none

  return type: integer

  returns the current edit step

- `fur.interface.getEditStepCoarse()`

  arguments: none

  return type: integer

  returns the current coarse edit step

- `fur.interface.getOrderEditMode()`

  arguments: none

  return type: integer

  returns the current order edit mode

- `fur.interface.getOrderCursor()`

  arguments: none

  return type: integer

  returns the current order cursor position

- `fur.interface.setOctave()`

  arguments: octave (integer)

  return type: none

  sets the current octave

- `fur.interface.setEditStep()`

  arguments: step amount (integer)

  return type: none

  sets the current edit step

- `fur.interface.setEditStepCoarse()`

  arguments: step amount (integer)

  return type: none

  sets the current coarse edit step

- `fur.interface.setOrderEditMode()`

  arguments: mode (integer)

  return type: none

  sets the order editing mode

- `fur.interface.setOrderCursor()`

  arguments: cursor position (integer)

  return type: none

  sets the order cursor position

- `fur.interface.registerMenuEntry()`

  arguments: menu name (string), menu entry name (string), menu function (function)

  return type: none

  binds a lua function to a menu entry in the program

  if the menu name doesn't exist, a new menu will appear

  example:
  ```lua
  -- define a function to call from a menu
  function showHello()
    fur.showError("Hello!")
  end
  -- add the menu entry ("hello...") to the "Help" menu, which will call the showHello function
  fur.registerMenuEntry("Help", "hello...", showHello)
  ```

#### song metadata

- `fur.song.getName()`

  arguments: none

  return type: string

  returns the song name

- `fur.song.setName()`

  arguments: song name (string)

  return type: none

  sets the song name

- `fur.song.getAuthor()`

  arguments: none

  return type: string

  returns the song author

- `fur.song.setAuthor()`

  arguments: song author (string)

  return type: none

  sets the song author

- `fur.song.getAlbum()`

  arguments: none

  return type: string

  returns the song album

- `fur.song.setAlbum()`

  arguments: song album (string)

  return type: none

  sets the song album

- `fur.song.getSysName()`

  arguments: none

  return type: string

  returns the song system name

- `fur.song.setSysName()`

  arguments: system name (string)

  return type: none

  sets the song system name

- `fur.song.getTuning()`

  arguments: none

  return type: number

  returns the song tuning

- `fur.song.setTuning()`

  arguments: A-4 frequency (number)

  return type: none

  sets the song tuning

- `fur.song.getComments()`

  arguments: none

  return type: string

  returns the song comments

- `fur.song.setComments()`

  arguments: comment (string)

  return type: none

  sets the song comments

#### subsong metadata

- `fur.subsong.getName()`

  arguments: (optional) subsong id (integer)

  return type: string

  returns the subsong name

  if subsong id not given, assumes the current subsong

- `fur.subsong.setName()`

  arguments: (optional) subsong id (integer), name (string)

  return type: none

  sets the subsong name

  if subsong id not given, assumes the current subsong

- `fur.subsong.getComments()`

  arguments: (optional) subsong id (integer)

  return type: string

  returns the subsong comments

  if subsong id not given, assumes the current subsong

- `fur.subsong.setComments()`

  arguments: (optional) subsong id (integer), comments (string)

  return type: none

  sets the subsong comments

  if subsong id not given, assumes the current subsong

- `fur.subsong.getRate()`

  arguments: (optional) subsong id (integer)

  return type: number

  returns the subsong tick rate

  if subsong id not given, assumes the current subsong

- `fur.subsong.setRate()`

  arguments: (optional) subsong id (integer), tick rate (number)

  return type: none

  sets the subsong tick rate

  if subsong id not given, assumes the current subsong

- `fur.subsong.getVirtualTempo()`

  arguments: (optional) subsong id (integer)

  return type: 2×number

  returns the subsong virtual tempo (numerator, denominator)

  if subsong id not given, assumes the current subsong

- `fur.subsong.setVirtualTempo()`

  arguments: (optional) subsong id (integer), numerator (integer), denominator (integer)

  return type: none

  sets the subsong virtual tempo

  if subsong id not given, assumes the current subsong

- `fur.subsong.getHighlights()`

  arguments: (optional) subsong id (integer)

  return type: 2×number

  returns the subsong pattern highlights

  if subsong id not given, assumes the current subsong

- `fur.subsong.setHighlights()`

  arguments: (optional) subsong id (integer), 1st highlight (integer), 2nd highlight (integer)

  return type: none

  sets the subsong pattern highlights

  if subsong id not given, assumes the current subsong

- `fur.subsong.getSpeeds()`

  arguments: (optional) subsong id (integer)

  return type: table (array)

  returns the subsong speeds

  if subsong id not given, assumes the current subsong

- `fur.subsong.setSpeeds()`

  arguments: (optional) subsong id (integer), speeds array (table)

  return type: none

  sets the subsong speeds

  if subsong id not given, assumes the current subsong

- `fur.subsong.getLength()`

  arguments: (optional) subsong id (integer)

  return type: integer

  returns the subsong length

  if subsong id not given, assumes the current subsong

- `fur.subsong.setLength()`

  arguments: (optional) subsong id (integer), length (number)

  return type: none

  sets the subsong length

  if subsong id not given, assumes the current subsong

- `fur.getPatLength()`

  arguments: (optional) subsong id (integer)

  return type: integer

  returns the subsong pattern length

  if subsong id not given, assumes the current subsong

- `fur.setPatLength()`

  arguments: (optional) subsong id (integer), length (number)

  return type: none

  sets the subsong pattern length

  if subsong id not given, assumes the current subsong

#### instrument manipulation

- `fur.instrument.create()`

  arguments: none

  return type: integer, nil on failure

  creates a new instrument and returs the id of the new instrument

  if a new instrument could not be created, instead returns nil

- `fur.instrument.delete()`

  arguments: (optional) instrument id (integer)

  return type: none

  deletes an instrument

  if instrument id not given, assumes current instrument

- `fur.instrument.setData()`

  arguments: (optional) instrument id (integer), instrument feature code (integer), instrument feature data (table)

  return type: none

  sets instrument parameters

  if instrument id not given, assumes current instrument

  for the data format, see [appendix A](#appendix-a-instrument-data-format)

  for a list of valid instrument feature codes, see the [constants below](#instrument)

- `fur.instrument.getData()`

  arguments: (optional) instrument id (integer)

  return type: table

  returns instrument parameters

  if instrument id not given, assumes current instrument

  for the data format, see [appendix A](#appendix-a-instrument-data-format)

- `fur.instrument.setMacroData()`

  arguments: (optional) instrument id (integer), instrument macro code (integer), instrument macro data (table)

  return type: none

  sets instrument macro parameters

  if instrument id not given, assumes current instrument

  for the data format, see [appendix B](#appendix-b-instrument-macro-format)

  for a list of valid instrument macro codes, see the [constants below](#instrument)

- `fur.instrument.getMacroData()`

  arguments: (optional) instrument id (integer)

  return type: table

  returns instrument macro parameters

  if instrument id not given, assumes current instrument

  for the data format, see [appendix B](#appendix-b-instrument-macro-format)

#### wave manipulation

- `fur.wave.create()`

  arguments: none

  return type: integer, nil on failure

  creates a new wavetable and returs the id of the new wavetable

  if a new wavetable could not be created, instead returns nil

- `fur.wave.delete()`

  arguments: (optional) wavetable id (number)

  return type: none

  deletes an wavetable

  if wavetable id not given, assumes current wavetable

- `fur.wave.getWidth()`

  arguments: (optional) wavetable id (integer)

  return type: integer, nil on failure

  returns the wavetable width

  if wavetable id not given, assumes current wavetable

  if wavetable id is invalid, instead returns nil

- `fur.wave.setWidth()`

  arguments: (optional) wavetable id (integer), width (number)

  return type: none

  sets the wavetable width

  if wavetable id not given, assumes current wavetable

- `fur.wave.getHeight()`

  arguments: (optional) wavetable id (integer)

  return type: integer, nil on failure

  returns the wavetable height

  if wavetable id not given, assumes current wavetable

  if wavetable id is invalid, instead returns nil

- `fur.wave.setHeight()`

  arguments: (optional) wavetable id (integer), height (number)

  return type: none

  sets the wavetable height

  if wavetable id not given, assumes current wavetable

- `fur.wave.getData()`

  arguments: (optional) wavetable id (integer), data index (number)

  return type: integer, nil on failure

  returns the wavetable data value at the given index

  if wavetable id not given, assumes current wavetable

  if wavetable id is invalid, instead returns nil

- `fur.wave.setData()`

  arguments: (optional) wavetable id (integer), data index (number), data value (number)

  return type: none

  sets the wavetable data value at the given index

  if wavetable id not given, assumes current wavetable

#### sample manipulation

- `fur.sample.create()`

  arguments: none

  return type: integer, nil on failure

  creates a new sample and returs the id of the new sample

  if a new sample could not be created, instead returns nil

- `fur.sample.delete()`

  arguments: (optional) sample id (number)

  return type: none

  deletes an sample

  if sample id not given, assumes current sample

- `fur.sample.getLength()`

  arguments: (optional) sample id (integer)

  return type: integer, nil on failure

  returns the sample length (in samples)

  if sample id not given, assumes current sample

  if sample id is invalid, instead returns nil

- `fur.sample.setLength()`

  arguments: (optional) sample id (integer), length (number)

  return type: none

  sets the sample length (in samples)

  if sample id not given, assumes current sample

- `fur.sample.getSize()`

  arguments: (optional) sample id (integer)

  return type: integer, nil on failure

  returns the sample size (in bytes)

  if sample id not given, assumes current sample

  if sample id is invalid, instead returns nil

- `fur.sample.getType()`

  arguments: (optional) sample id (integer)

  return type: integer, nil on failure

  returns the sample type

  if sample id not given, assumes current sample

  if sample id is invalid, instead returns nil

- `fur.sample.setType()`

  arguments: (optional) sample id (integer), length (number)

  return type: none

  sets the sample type

  if sample id not given, assumes current sample

- `fur.sample.getLoop()`

  arguments: (optional) sample id (integer)

  return type: boolean, 3×integer, 4×nil on failure

  returns the sample loop parameters (enables, start, end, mode)

  if sample id not given, assumes current sample

  if sample id is invalid, instead returns nil

- `fur.sample.setLoop()`

  arguments: (optional) sample id (integer), enabled (boolean), start (number), end (number), mode (number)

  return type: none

  sets the sample loop parameters

  if sample id not given, assumes current sample

- `fur.sample.getRate()`

  arguments: (optional) sample id (integer)

  return type: integer, nil on failure

  returns the sample sample rate

  if sample id not given, assumes current sample

  if sample id is invalid, instead returns nil

- `fur.sample.setRate()`

  arguments: (optional) sample id (integer), rate (number)

  return type: none

  sets the sample sample rate

  if sample id not given, assumes current sample

- `fur.sample.getData()`

  arguments: (optional) sample id (integer), position (number)

  return type: integer, nil on failure

  returns the sample data

  for 8 and 16-bit samples, returns the value at that position

  for other types, returns the raw byte at that offset

  if sample id not given, assumes current sample

  if sample id is invalid, instead returns nil

- `fur.sample.setData()`

  arguments: (optional) sample id (integer), position (number), value (number)

  return type: none

  sets the sample data

  for 8 and 16-bit samples, sets the value at that position

  for other types, sets the raw byte at that offset

  if sample id not given, assumes current sample

- `fur.sample.isEditable()`

  arguments: (optional) sample id (integer)

  return type: boolean, nil on failure

  returns whether the sample is editable

  editable samples are the ones of either 8 or 16-bit type

  if sample id not given, assumes current sample

  if sample id is invalid, instead returns nil

- `fur.sample.render()`

  arguments: (optional) sample id (integer)

  return type: none

  renders the samples of a sample

  if sample id not given, assumes current sample

  if sample id is invalid, instead returns nil

#### order manipulation

- `fur.order.get()`

  arguments: (optional) subsong id (integer), channel (integer), order (integer)

  return type: integer

  returns the pattern number of a channel at the order

- `fur.order.set()`

  arguments: (optional) subsong id (integer), channel (integer), order (integer), pattern number (integer)

  return type: none

  sets the pattern number of a channel at the order

#### pattern manipulation

- `fur.pattern.get()`

  arguments: (optional) subsong id (integer), (optional) order number (integer), channel (integer), row (integer), column (integer)

  return type: integer

  returns the pattern data

- `fur.pattern.set()`

  arguments: (optional) subsong id (integer), (optional) order number (integer), channel (integer), row (integer), column (integer), data (integer)

  return type: none

  sets the pattern data

- `fur.pattern.getDirect()`

  arguments: (optional) subsong id (integer), (optional) pattern id (integer), channel (integer), row (integer), column (integer)

  return type: integer

  returns the pattern data

- `fur.pattern.setDirect()`

  arguments: (optional) subsong id (integer), (optional) pattern id (integer), channel (integer), row (integer), column (integer), data (integer)

  return type: none

  sets the pattern data

- `fur.pattern.addinputCallback()`

  arguments: callback function (function)

  return type: none

  adds a function which shall run when a pattern is edited

- `fur.pattern.columnEffect()`

  arguments: effect column (integer)

  return type: integer

  returns the column number of an effect column

  effect column may be in range 0-7 inclusive

- `fur.pattern.columnEffectVal()`

  arguments: effect value column (integer)

  return type: integer

  returns the column number of an effect value column

  effect value column may be in range 0-7 inclusive

#### dialogs

- `fur.dialog.new()`

  arguments: dialog title (string)

  return type: none

  begins a new dialog

- `fur.dialog.itemInt()`

  arguments: item label (string), (optional) default value (integer), (optional) minimum value (integer), (optional) maximum value (integer)

  return type: none

  adds a integer input to the dialog

  if no default value given, it's set to 0

  the default minimum and maximum values are 0 and 100 respectively

- `fur.dialog.itemFloat()`

  arguments: item label (string), (optional) default value (number), (optional) minimum value (number), (optional) maximum value (number)

  return type: none

  adds a float input to the dialog

  if no default value given, it's set to 0

  the default minimum and maximum values are 0 and 100 respectively

- `fur.dialog.itemString()`

  arguments: item label (string), (optional) default value (string)

  return type: none

  adds a string input to the dialog

- `fur.dialog.itemCheckbox()`

  arguments: item label (string), (optional) default value (boolean)

  return type: none

  adds a checkbox input to the dialog

- `fur.dialog.show()`

  arguments: dialog accept function (function)

  return type: none

  shows the dialog

- `fur.dialog.getItems()`

  arguments: none

  return type: (any)

  returns the values from the dialog items, in order

### constants

#### pattern

- `fur.pattern.columnNote`

  type: integer

  value: `0`

  the index of a note column

- `fur.pattern.columnIns`

  type: integer

  value: `1`

  the index of an instrument column

- `fur.pattern.columnVol`

  type: integer

  value: `2`

  the index of a volume column

- `fur.pattern.columnRaw`

  type: integer

  value: `21`

  special index for accessing raw frequency values

- `fur.pattern.noteOff`

  type: integer

  value: `253`

  the internal value of a `OFF` note

- `fur.pattern.noteRel`

  type: integer

  value: `254`

  the internal value of a `===` note (note release)

- `fur.pattern.macroRel`

  type: integer

  value: `255`

  the internal value of a `REL` note (macro release)

- `fur.pattern.noteRaw`

  type: integer

  value: `251`

  the internal value of a note which denotes whether the row contains a raw frequency value

#### instrument

- `fur.instrument.featureFM`
- `fur.instrument.featureGB`
- `fur.instrument.featureC64`
- `fur.instrument.featureAmiga`
- `fur.instrument.featureX1`
- `fur.instrument.feature163`
- `fur.instrument.featureFDS`
- `fur.instrument.featureMultiPCM`
- `fur.instrument.featureWaveSynth`
- `fur.instrument.featureSoundUnit`
- `fur.instrument.featureES5506`
- `fur.instrument.featureSNES`
- `fur.instrument.featureESFM`
- `fur.instrument.featurePowerNoise`
- `fur.instrument.featureSID2`
- `fur.instrument.featureSID3`
- `fur.instrument.featureKlattsch`

  type: integer

  value: 0-16

  constants denoting the instrument features

- `fur.instrument.macroVolume`
- `fur.instrument.macroArp`
- `fur.instrument.macroDuty`
- `fur.instrument.macroWave`
- `fur.instrument.macroPitch`
- `fur.instrument.macroEx1`
- `fur.instrument.macroEx2`
- `fur.instrument.macroEx3`
- `fur.instrument.macroAlg`
- `fur.instrument.macroFeedback`
- `fur.instrument.macroFMS`
- `fur.instrument.macroAMS`
- `fur.instrument.macroPanLeft`
- `fur.instrument.macroPanRight`
- `fur.instrument.macroPhaseReset`
- `fur.instrument.macroEx4`
- `fur.instrument.macroEx5`
- `fur.instrument.macroEx6`
- `fur.instrument.macroEx7`
- `fur.instrument.macroEx8`
- `fur.instrument.macroEx9`
- `fur.instrument.macroEx10`

  type: integer

  value: 0-21

  constants denoting the instrument macro codes

## Appendix A: instrument data format

```
  {
    fm=(table),
    ...
  }
```
blank features return a `nil` table

- `fm` table:
  ```
  {
    alg=(integer),
    feedback=(integer),
    fms=(integer),
    ams=(integer),
    fms2=(integer),
    ams2=(integer),
    opllPreset=(integer),
    block=(integer),
    fixedDrums=(integer),
    kickFreq=(integer),
    snareHatFreq=(integer),
    tomTopFreq=(integer),
    op=(array of tables)
  }
  ```
  - `op` table
    ```
    {
      enable=(boolean),
      am=(integer),
      ar=(integer),
      dr=(integer),
      mult=(integer),
      rr=(integer),
      sl=(integer)
      tl=(integer),
      dt2=(integer),
      rs=(integer),
      dt=(integer),
      d2r=(integer),
      ssgEnv=(integer),
      dam=(integer),
      dvb=(integer),
      egt=(integer),
      ksl=(integer),
      sus=(integer),
      vib=(integer),
      ws=(integer),
      ksr=(integer),
      kvs=(integer),
    }
    ```

## Appendix B: instrument macro format

```
{
  volume=(table),
  arp=(table),
  duty=(table),
  wave=(table),
  pitch=(table),
  ex1=(table),
  ex2=(table),
  ex3=(table),
  alg=(table),
  feedback=(table),
  fms=(table),
  ams=(table),
  panLeft=(table),
  panRight=(table),
  phaseReset=(table),
  ex4=(table),
  ex5=(table),
  ex6=(table),
  ex7=(table),
  ex8=(table),
  ex9=(table),
  ex10=(table),
}
```

all macro entries use the same format:

```
{
  open=(boolean),
  mode=(integer),
  delay=(integer),
  instantRelease=(boolean),
  speed=(integer),
  loop=(integer),
  release=(integer),
  -- if the macro type is sequence
  values=(array of integers),
  -- if the macro type is ADSR
  envelope=(table),
  -- if the macro type is LFO
  lfo=(table),
}
```

`values` may contain up to 255 integers

- `envelope` table
  ```
  {
    bottom=(integer),
    top=(integer),
    attack=(integer),
    hold=(integer),
    decay=(integer),
    sustain=(integer),
    susTime=(integer),
    susDecay=(integer),
    release=(integer),
  },
  ```

- `lfo` table
  ```
  {
    bottom=(integer),
    top=(integer),
    speed=(integer),
    waveform=(integer),
    phase=(integer),
  },
  ```
