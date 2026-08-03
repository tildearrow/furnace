# scripting function reference

all functions are defined in the `fur` table.
all functions use 0-indexing.

## generic

### `fur.version()`

arguments: none

return type: integer

returns the furnace version number

### `fur.versionStr()`

arguments: none

return type: string

returns the furnace version string

### `fur.showError()`

arguments: error text (string)

return type: none

shows an error popup

### `fur.registerMenuEntry()`

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

## pattern cursor

### `fur.getCursor()`

arguments: none

return type: 3×integer

returns the pattern cursor position (channel, column, row)

### `fur.setCursor()`

arguments: cursor channel (integer), cursor column (integer), cursor row (integer)

return type: none

sets the pattern cursor position

### `fur.getSelStart()`

arguments: none

return type: 3×integer

returns the pattern selection start position (channel, column, row)

### `fur.setSelStart()`

arguments: channel (integer), column (integer), row (integer)

return type: none

sets the pattern selection start position

### `fur.getSelEnd()`

arguments: none

return type: 3×integer

returns the pattern selection end position (channel, column, row)

### `fur.setSelEnd()`

arguments: channel (integer), column (integer), row (integer)

return type: none

sets the pattern selection end position

### `fur.getCurOrder()`

arguments: none

return type: integer

returns the current order number

### `fur.getCurRow()`

arguments: none

return type: integer

returns the current row number

## engine state

### `fur.getPlayTimeSec()`

arguments: none

return type: integer

returns the play time seconds

### `fur.getPlayTimeMicro()`

arguments: none

return type: integer

returns the play time microseconds

### `fur.getPlayTimeTicks()`

arguments: none

return type: integer

returns the play time in ticks

### `fur.isPlaying()`

arguments: none

return type: boolean

returns whether the engine is playing

### `fur.isRunning()`

arguments: none

return type: boolean

returns whether the engine is running

### `fur.isFreelance()`

arguments: none

return type: boolean

i still dont understand what freelance means

### `fur.getChanCount()`

arguments: none

return type: integer

returns the total number of channels in the song

### `fur.getCurSubSong()`

arguments: none

return type: integer

returns the current subsong number

### `fur.getEditOrder()`

arguments: none

return type: integer

returns the edit order(?)

### `fur.getCurIns()`

arguments: none

return type: integer

returns the id of the current instrument

### `fur.getCurWave()`

arguments: none

return type: integer

returns the id of the current wavetable

### `fur.getCurSample()`

arguments: none

return type: integer

returns the id of the current sample

## editing

### `fur.setCurIns()`

arguments: instrument id (integer)

return type: none

sets the current instrument

### `fur.setCurWave()`

arguments: wavetable id (integer)

return type: none

sets the current wavetable

### `fur.setCurSample()`

arguments: sample id (integer)

return type: none

sets the current sample

### `fur.getOctave()`

arguments: none

return type: integer

returns the current octave

### `fur.getEditStep()`

arguments: none

return type: integer

returns the current edit step

### `fur.getEditStepCoarse()`

arguments: none

return type: integer

returns the current coarse edit step

### `fur.getOrderEditMode()`

arguments: none

return type: integer

returns the current order edit mode

### `fur.getOrderCursor()`

arguments: none

return type: integer

returns the current order cursor position

### `fur.setOctave()`

arguments: octave (integer)

return type: none

sets the current octave

### `fur.setEditStep()`

arguments: step amount (integer)

return type: none

sets the current edit step

### `fur.setEditStepCoarse()`

arguments: step amount (integer)

return type: none

sets the current coarse edit step

### `fur.setOrderEditMode()`

arguments: mode (integer)

return type: none

sets the order editing mode

### `fur.setOrderCursor()`

arguments: cursor position (integer)

return type: none

sets the order cursor position

## song metadata

### `fur.getSongName()`

arguments: none

return type: string

returns the song name

### `fur.setSongName()`

arguments: song name (string)

return type: none

sets the song name

### `fur.getSongAuthor()`

arguments: none

return type: string

returns the song author

### `fur.setSongAuthor()`

arguments: song author (string)

return type: none

sets the song author

### `fur.getSongAlbum()`

arguments: none

return type: string

returns the song album

### `fur.setSongAlbum()`

arguments: song album (string)

return type: none

sets the song album

### `fur.getSongSysName()`

arguments: none

return type: string

returns the song system name

### `fur.setSongSysName()`

arguments: system name (string)

return type: none

sets the song system name

### `fur.getSongTuning()`

arguments: none

return type: number

returns the song tuning

### `fur.setSongTuning()`

arguments: A-4 frequency (number)

return type: none

sets the song tuning

### `fur.getSongComments()`

arguments: none

return type: string

returns the song comments

### `fur.setSongComments()`

arguments: comment (string)

return type: none

sets the song comments

### `fur.getSubSongName()`

arguments: (optional) subsong id (integer)

return type: string

returns the subsong name

if subsong id not given, assumes the current subsong

### `fur.setSubSongName()`

arguments: (optional) subsong id (integer), name (string)

return type: none

sets the subsong name

if subsong id not given, assumes the current subsong

### `fur.getSubSongComments()`

arguments: (optional) subsong id (integer)

return type: string

returns the subsong comments

if subsong id not given, assumes the current subsong

### `fur.setSubSongComments()`

arguments: (optional) subsong id (integer), comments (string)

return type: none

sets the subsong comments

if subsong id not given, assumes the current subsong

### `fur.getSongRate()`

arguments: (optional) subsong id (integer)

return type: number

returns the subsong tick rate

if subsong id not given, assumes the current subsong

### `fur.setSongRate()`

arguments: (optional) subsong id (integer), tick rate (number)

return type: none

sets the subsong tick rate

if subsong id not given, assumes the current subsong

### `fur.getSongVirtualTempo()`

arguments: (optional) subsong id (integer)

return type: 2×number

returns the subsong virtual tempo (numerator, denominator)

if subsong id not given, assumes the current subsong

### `fur.setSongVirtualTempo()`

arguments: (optional) subsong id (integer), numerator (integer), denominator (integer)

return type: none

sets the subsong virtual tempo

if subsong id not given, assumes the current subsong

### `fur.getSongHighlights()`

arguments: (optional) subsong id (integer)

return type: 2×number

returns the subsong pattern highlights

if subsong id not given, assumes the current subsong

### `fur.setSongHighlights()`

arguments: (optional) subsong id (integer), 1st highlight (integer), 2nd highlight (integer)

return type: none

sets the subsong pattern highlights

if subsong id not given, assumes the current subsong

### `fur.getSongSpeeds()`

arguments: (optional) subsong id (integer)

return type: table (array)

returns the subsong speeds

if subsong id not given, assumes the current subsong

### `fur.setSongSpeeds()`

arguments: (optional) subsong id (integer), speeds array (table)

return type: none

sets the subsong speeds

if subsong id not given, assumes the current subsong

### `fur.getSongLength()`

arguments: (optional) subsong id (integer)

return type: integer

returns the subsong length

if subsong id not given, assumes the current subsong

### `fur.setSongLength()`

arguments: (optional) subsong id (integer), length (number)

return type: none

sets the subsong length

if subsong id not given, assumes the current subsong

### `fur.getPatLength()`

arguments: (optional) subsong id (integer)

return type: integer

returns the subsong pattern length

if subsong id not given, assumes the current subsong

### `fur.setPatLength()`

arguments: (optional) subsong id (integer), length (number)

return type: none

sets the subsong pattern length

if subsong id not given, assumes the current subsong

## assets

### `fur.createIns()`

arguments: none

return type: integer, nil on failure

creates a new instrument and returs the id of the new instrument

if a new instrument could not be created, instead returns nil

### `fur.deleteIns()`

arguments: (optional) instrument id (number)

return type: none

deletes an instrument

if instrument id not given, assumes current instrument

### `fur.createWave()`

arguments: none

return type: integer, nil on failure

creates a new wavetable and returs the id of the new wavetable

if a new wavetable could not be created, instead returns nil

### `fur.deleteWave()`

arguments: (optional) wavetable id (number)

return type: none

deletes an wavetable

if wavetable id not given, assumes current wavetable

### `fur.getWaveWidth()`

arguments: (optional) wavetable id (integer)

return type: integer, nil on failure

returns the wavetable width

if wavetable id not given, assumes current wavetable

if wavetable id is invalid, instead returns nil

### `fur.setWaveWidth()`

arguments: (optional) wavetable id (integer), width (number)

return type: none

sets the wavetable width

if wavetable id not given, assumes current wavetable

### `fur.getWaveHeight()`

arguments: (optional) wavetable id (integer)

return type: integer, nil on failure

returns the wavetable height

if wavetable id not given, assumes current wavetable

if wavetable id is invalid, instead returns nil

### `fur.setWaveHeight()`

arguments: (optional) wavetable id (integer), height (number)

return type: none

sets the wavetable height

if wavetable id not given, assumes current wavetable

### `fur.getWaveData()`

arguments: (optional) wavetable id (integer), data index (number)

return type: integer, nil on failure

returns the wavetable data value at the given index

if wavetable id not given, assumes current wavetable

if wavetable id is invalid, instead returns nil

### `fur.setWaveData()`

arguments: (optional) wavetable id (integer), data index (number), data value (number)

return type: none

sets the wavetable data value at the given index

if wavetable id not given, assumes current wavetable

### `fur.createSample()`

arguments: none

return type: integer, nil on failure

creates a new sample and returs the id of the new sample

if a new sample could not be created, instead returns nil

### `fur.deleteSample()`

arguments: (optional) sample id (number)

return type: none

deletes an sample

if sample id not given, assumes current sample

### `fur.getSampleLength()`

arguments: (optional) sample id (integer)

return type: integer, nil on failure

returns the sample length (in samples)

if sample id not given, assumes current sample

if sample id is invalid, instead returns nil

### `fur.setSampleLength()`

arguments: (optional) sample id (integer), length (number)

return type: none

sets the sample length (in samples)

if sample id not given, assumes current sample

### `fur.getSampleSize()`

arguments: (optional) sample id (integer)

return type: integer, nil on failure

returns the sample size (in bytes)

if sample id not given, assumes current sample

if sample id is invalid, instead returns nil

### `fur.getSampleType()`

arguments: (optional) sample id (integer)

return type: integer, nil on failure

returns the sample type

if sample id not given, assumes current sample

if sample id is invalid, instead returns nil

### `fur.setSampleType()`

arguments: (optional) sample id (integer), length (number)

return type: none

sets the sample type

if sample id not given, assumes current sample

### `fur.getSampleLoop()`

arguments: (optional) sample id (integer)

return type: boolean, 3×integer, 4×nil on failure

returns the sample loop parameters (enables, start, end, mode)

if sample id not given, assumes current sample

if sample id is invalid, instead returns nil

### `fur.setSampleLoop()`

arguments: (optional) sample id (integer), enabled (boolean), start (number), end (number), mode (number)

return type: none

sets the sample loop parameters

if sample id not given, assumes current sample

### `fur.getSampleRate()`

arguments: (optional) sample id (integer)

return type: integer, nil on failure

returns the sample sample rate

if sample id not given, assumes current sample

if sample id is invalid, instead returns nil

### `fur.setSampleRate()`

arguments: (optional) sample id (integer), rate (number)

return type: none

sets the sample sample rate

if sample id not given, assumes current sample

### `fur.getSampleData()`

arguments: (optional) sample id (integer), position (number)

return type: integer, nil on failure

returns the sample data

for 8 and 16-bit samples, returns the value at that position

for other types, returns the raw byte at that offset

if sample id not given, assumes current sample

if sample id is invalid, instead returns nil

### `fur.setSampleData()`

arguments: (optional) sample id (integer), position (number), value (number)

return type: none

sets the sample data

for 8 and 16-bit samples, sets the value at that position

for other types, sets the raw byte at that offset

if sample id not given, assumes current sample

### `fur.isSampleEditable()`

arguments: (optional) sample id (integer)

return type: boolean, nil on failure

returns whether the sample is editable

editable samples are the ones of either 8 or 16-bit type

if sample id not given, assumes current sample

if sample id is invalid, instead returns nil

### `fur.isSampleEditable()`

arguments: (optional) sample id (integer)

return type: none

renders the samples of a sample

if sample id not given, assumes current sample

if sample id is invalid, instead returns nil

## orders

## pattern

## dialogs

