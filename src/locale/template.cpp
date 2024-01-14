#include <map>
#include <string>
#include "locale.h"

#include "template.h"

int getPluralIndexTemplate(int n)
{
    return 0;
    //here you can provide plural forms indices based on the integer.
    //you can find one-liners for common languages here:
    //https://www.gnu.org/software/gettext/manual/html_node/Plural-forms.html
    //these would need some adaptation to work in this code
    //see russian.cpp file for example of adapted statement
}

class DivLocale;

void DivLocale::addTranslationsTemplate()
{
    //MENU BAR ITEMS

    strings["File##menubar"].plurals[0] = "=File##menubar";
    strings["file##menubar"].plurals[0] = "=file##menubar";
    strings["Edit##menubar"].plurals[0] = "=Edit##menubar";
    strings["edit##menubar"].plurals[0] = "=edit##menubar";
    strings["Settings##menubar"].plurals[0] = "=Settings##menubar";
    strings["settings##menubar"].plurals[0] = "=settings##menubar";
    strings["Window##menubar"].plurals[0] = "=Window##menubar";
    strings["window##menubar"].plurals[0] = "=window##menubar";
    strings["Help##menubar"].plurals[0] = "=Help##menubar";
    strings["help##menubar"].plurals[0] = "=help##menubar";

    //WINDOW NAMES

    strings["Settings###Settings"].plurals[0] = "=Settings###Settings";
    strings["Pattern###Pattern"].plurals[0] = "=Pattern###Pattern";
    strings["Orders###Orders"].plurals[0] = "=Orders###Orders";
    strings["Statistics###Statistics"].plurals[0] = "=Statistics###Statistics";
    strings["Song Info###Song Information"].plurals[0] = "=Song Info###Song Information";
    strings["Subsongs###Subsongs"].plurals[0] = "=Subsongs###Subsongs";
    strings["About Furnace###About Furnace"].plurals[0] = "=About Furnace###About Furnace";
    strings["Channels###Channels"].plurals[0] = "=Channels###Channels";
    strings["Oscilloscope (per-channel)###Oscilloscope (per-channel)"].plurals[0] = "=Oscilloscope (per-channel)###Oscilloscope (per-channel)";
    strings["Clock###Clock"].plurals[0] = "Clock###Clock";
    strings["Compatibility Flags###Compatibility Flags"].plurals[0] = "=Compatibility Flags###Compatibility Flags";
    strings["Instruments###Instruments"].plurals[0] = "=Instruments###Instruments";
    strings["Wavetables###Wavetables"].plurals[0] = "=Wavetables###Wavetables";
    strings["Debug###Debug"].plurals[0] = "=Debug###Debug";
    strings["Samples###Samples"].plurals[0] = "=Samples###Samples";
    strings["MobileEdit###MobileEdit"].plurals[0] = "=MobileEdit###MobileEdit";
    strings["Mobile Controls###Mobile Controls"].plurals[0] = "=Mobile Controls###Mobile Controls";
    strings["Mobile Menu###Mobile Menu"].plurals[0] = "=Mobile Menu###Mobile Menu";
    strings["Play/Edit Controls###Play/Edit Controls"].plurals[0] = "=Play/Edit Controls###Play/Edit Controls";
    strings["Play Controls###Play Controls"].plurals[0] = "=Play Controls###Play Controls";
    strings["Edit Controls###Edit Controls"].plurals[0] = "=Edit Controls###Edit Controls";
    strings["Effect List###Effect List"].plurals[0] = "=Effect List###Effect List";
    strings["Find/Replace###Find/Replace"].plurals[0] = "=Find/Replace###Find/Replace";
    strings["Grooves###Grooves"].plurals[0] = "=Grooves###Grooves";
    strings["Instrument Editor###Instrument Editor"].plurals[0] = "=Instrument Editor###Instrument Editor";
    strings["Log Viewer###Log Viewer"].plurals[0] = "=Log Viewer###Log Viewer";
    strings["Mixer###Mixer"].plurals[0] = "=Mixer###Mixer";
    strings["OrderSel###OrderSel"].plurals[0] = "=OrderSel###OrderSel";
    strings["Oscilloscope###Oscilloscope"].plurals[0] = "=Oscilloscope###Oscilloscope";
    strings["Pattern Manager###Pattern Manager"].plurals[0] = "=Pattern Manager###Pattern Manager";
    strings["Input Pad###Input Pad"].plurals[0] = "=Input Pad###Input Pad";
    strings["Register View###Register View"].plurals[0] = "=Register View###Register View";
    strings["Sample Editor###Sample Editor"].plurals[0] = "=Sample Editor###Sample Editor";
    strings["Song Comments###Song Comments"].plurals[0] = "=Song Comments###Song Comments";
    strings["Speed###Speed"].plurals[0] = "=Speed###Speed";
    strings["Spoiler###Spoiler"].plurals[0] = "=Spoiler###Spoiler";
    strings["Chip Manager###Chip Manager"].plurals[0] = "=Chip Manager###Chip Manager";
    strings["Volume Meter###Volume Meter"].plurals[0] = "=Volume Meter###Volume Meter";
    strings["Wavetable Editor###Wavetable Editor"].plurals[0] = "=Wavetable Editor###Wavetable Editor";
    strings["Oscilloscope (X-Y)###Oscilloscope (X-Y)"].plurals[0] = "=Oscilloscope (X-Y)###Oscilloscope (X-Y)";

    //EFFECT LIST

    //common (non-chip-specific) effects

    strings["00xy: Arpeggio"].plurals[0] = "=00xy: Arpeggio";
    
    //MACRO EDITOR

    //macro names

    strings["Volume"].plurals[0] = "=Volume";
    strings["Gain"].plurals[0] = "=Gain";

    //bitfield macros strings

    strings["hold"].plurals[0] = "=hold";
    strings["alternate"].plurals[0] = "=alternate";
    strings["direction"].plurals[0] = "=direction";
    strings["enable"].plurals[0] = "=enable";

    strings["right"].plurals[0] = "=right";
    strings["left"].plurals[0] = "=left";
    strings["rear right"].plurals[0] = "=rear right";
    strings["rear left"].plurals[0] = "=rear left";

    //macro hover notes

    strings["exponential"].plurals[0] = "=exponential";
    strings["linear"].plurals[0] = "=linear";
    strings["direct"].plurals[0] = "=direct";

    strings["Release"].plurals[0] = "=Release";
    strings["Loop"].plurals[0] = "=Loop";

    strings["Fixed"].plurals[0] = "=Fixed";
    strings["Relative"].plurals[0] = "=Relative";

    strings["HP/K2, HP/K2"].plurals[0] = "=HP/K2, HP/K2";
    strings["HP/K2, LP/K1"].plurals[0] = "=HP/K2, LP/K1";
    strings["LP/K2, LP/K2"].plurals[0] = "=LP/K2, LP/K2";
    strings["LP/K2, LP/K1"].plurals[0] = "=LP/K2, LP/K1";

    strings["Saw"].plurals[0] = "=Saw";
    strings["Square"].plurals[0] = "=Square";
    strings["Triangle"].plurals[0] = "=Triangle";
    strings["Random"].plurals[0] = "=Random";

    //keyboard hotkeys

    strings["---Global"].plurals[0] = "=---Global";
    strings["New"].plurals[0] = "=New";
    strings["Open file"].plurals[0] = "=Open file";
    strings["Restore backup"].plurals[0] = "=Restore backup";
    strings["Save file"].plurals[0] = "=Save file";
    strings["Save as"].plurals[0] = "=Save as";
    strings["Export"].plurals[0] = "=Export";
    strings["Undo"].plurals[0] = "=Undo";
    strings["Redo"].plurals[0] = "=Redo";
    strings["Redo"].plurals[0] = "=Redo";
    strings["Play/Stop (toggle)"].plurals[0] = "=Play/Stop (toggle)";
    strings["Play"].plurals[0] = "=Play";
    strings["Stop"].plurals[0] = "=Stop";
    strings["Play (from beginning)"].plurals[0] = "=Play (from beginning)";
    strings["Play (repeat pattern)"].plurals[0] = "=Play (repeat pattern)";
    strings["Play from cursor"].plurals[0] = "=Play from cursor";
    strings["Step row"].plurals[0] = "=Step row";
    strings["Octave up"].plurals[0] = "=Octave up";
    strings["Octave down"].plurals[0] = "=Octave down";
    strings["Previous instrument"].plurals[0] = "=Previous instrument";
    strings["Next instrument"].plurals[0] = "=Next instrument";
    strings["Increase edit step"].plurals[0] = "=Increase edit step";
    strings["Decrease edit step"].plurals[0] = "=Decrease edit step";
    strings["Toggle edit mode"].plurals[0] = "=Toggle edit mode";
    strings["Metronome"].plurals[0] = "=Metronome";
    strings["Toggle repeat pattern"].plurals[0] = "=Toggle repeat pattern";
    strings["Follow orders"].plurals[0] = "=Follow orders";
    strings["Follow pattern"].plurals[0] = "=Follow pattern";
    strings["Toggle full-screen"].plurals[0] = "=Toggle full-screen";
    strings["Request voice from TX81Z"].plurals[0] = "=Request voice from TX81Z";
    strings["Panic"].plurals[0] = "=Panic";
    strings["Clear song data"].plurals[0] = "=Clear song data";

    strings["Edit Controls"].plurals[0] = "=Edit Controls";
    strings["Orders"].plurals[0] = "=Orders";
    strings["Instrument List"].plurals[0] = "=Instrument List";
    strings["Instrument Editor"].plurals[0] = "=Instrument Editor";
    strings["Song Information"].plurals[0] = "=Song Information";
    strings["Speed"].plurals[0] = "=Speed";
    strings["Pattern"].plurals[0] = "=Pattern";
    strings["Wavetable List"].plurals[0] = "=Wavetable List";
    strings["Wavetable Editor"].plurals[0] = "=Wavetable Editor";
    strings["Sample List"].plurals[0] = "=Sample List";
    strings["Sample Editor"].plurals[0] = "=Sample Editor";
    strings["About"].plurals[0] = "=About";
    strings["Settings"].plurals[0] = "=Settings";
    strings["Settings"].plurals[0] = "=Settings";
    strings["Mixer"].plurals[0] = "=Mixer";
    strings["Debug Menu"].plurals[0] = "=Debug Menu";
    strings["Oscilloscope (master)"].plurals[0] = "=Oscilloscope (master)";
    strings["Volume Meter"].plurals[0] = "=Volume Meter";
    strings["Statistics"].plurals[0] = "=Statistics";
    strings["Compatibility Flags"].plurals[0] = "=Compatibility Flags";
    strings["Piano"].plurals[0] = "=Piano";
    strings["Song Comments"].plurals[0] = "=Song Comments";
    strings["Channels"].plurals[0] = "=Channels";
    strings["Pattern Manager"].plurals[0] = "=Pattern Manager";
    strings["Chip Manager"].plurals[0] = "=Chip Manager";
    strings["Register View"].plurals[0] = "=Register View";
    strings["Log Viewer"].plurals[0] = "=Log Viewer";
    strings["Effect List"].plurals[0] = "=Effect List";
    strings["Oscilloscope (per-channel)"].plurals[0] = "=Oscilloscope (per-channel)";
    strings["Subsongs"].plurals[0] = "=Subsongs";
    strings["Find/Replace"].plurals[0] = "=Find/Replace";
    strings["Clock"].plurals[0] = "=Clock";
    strings["Grooves"].plurals[0] = "=Grooves";
    strings["Oscilloscope (X-Y)"].plurals[0] = "=Oscilloscope (X-Y)";

    strings["Collapse/expand current window"].plurals[0] = "=Collapse/expand current window";
    strings["Close current window"].plurals[0] = "=Close current window";

    strings["---Pattern"].plurals[0] = "=---Pattern";
    strings["Transpose (+1)"].plurals[0] = "=Transpose (+1)";
    strings["Transpose (-1)"].plurals[0] = "=Transpose (-1)";
    strings["Transpose (+1 octave)"].plurals[0] = "=Transpose (+1 octave)";
    strings["Transpose (-1 octave)"].plurals[0] = "=Transpose (-1 octave)";
    strings["Increase values (+1)"].plurals[0] = "=Increase values (+1)";
    strings["Increase values (-1)"].plurals[0] = "=Increase values (-1)";
    strings["Increase values (+16)"].plurals[0] = "=Increase values (+16)";
    strings["Increase values (-16)"].plurals[0] = "=Increase values (-16)";
    strings["Select all"].plurals[0] = "=Select all";
    strings["Cut"].plurals[0] = "=Cut";
    strings["Copy"].plurals[0] = "=Copy";
    strings["Paste"].plurals[0] = "=Paste";
    strings["Paste Mix (foreground)"].plurals[0] = "=Paste Mix (foreground)";
    strings["Paste Mix (background)"].plurals[0] = "=Paste Mix (background)";
    strings["Paste Flood"].plurals[0] = "=Paste Flood";
    strings["Paste Overflow"].plurals[0] = "=Paste Overflow";
    strings["Move cursor up"].plurals[0] = "=Move cursor up";
    strings["Move cursor down"].plurals[0] = "=Move cursor down";
    strings["Move cursor left"].plurals[0] = "=Move cursor left";
    strings["Move cursor right"].plurals[0] = "=Move cursor right";
    strings["Move cursor up by one (override Edit Step)"].plurals[0] = "=Move cursor up by one (override Edit Step)";
    strings["Move cursor down by one (override Edit Step)"].plurals[0] = "=Move cursor down by one (override Edit Step)";
    strings["Move cursor to previous channel"].plurals[0] = "=Move cursor to previous channel";
    strings["Move cursor to next channel"].plurals[0] = "=Move cursor to next channel";
    strings["Move cursor to next channel (overflow)"].plurals[0] = "=Move cursor to next channel (overflow)";
    strings["Move cursor to previous channel (overflow)"].plurals[0] = "=Move cursor to previous channel (overflow)";
    strings["Move cursor to beginning of pattern"].plurals[0] = "=Move cursor to beginning of pattern";
    strings["Move cursor to end of pattern"].plurals[0] = "=Move cursor to end of pattern";
    strings["Move cursor up (coarse)"].plurals[0] = "=Move cursor up (coarse)";
    strings["Move cursor down (coarse)"].plurals[0] = "=Move cursor down (coarse)";
    strings["Expand selection upwards"].plurals[0] = "=Expand selection upwards";
    strings["Expand selection downwards"].plurals[0] = "=Expand selection downwards";
    strings["Expand selection to the left"].plurals[0] = "=Expand selection to the left";
    strings["Expand selection to the right"].plurals[0] = "=Expand selection to the right";
    strings["Expand selection upwards by one (override Edit Step)"].plurals[0] = "=Expand selection upwards by one (override Edit Step)";
    strings["Expand selection downwards by one (override Edit Step)"].plurals[0] = "=Expand selection downwards by one (override Edit Step)";
    strings["Expand selection to beginning of pattern"].plurals[0] = "=Expand selection to beginning of pattern";
    strings["Expand selection to end of pattern"].plurals[0] = "=Expand selection to end of pattern";
    strings["Expand selection upwards (coarse)"].plurals[0] = "=Expand selection upwards (coarse)";
    strings["Expand selection downwards (coarse)"].plurals[0] = "=Expand selection downwards (coarse)";
    strings["Delete"].plurals[0] = "=Delete";
    strings["Pull delete"].plurals[0] = "=Pull delete";
    strings["Insert"].plurals[0] = "=Insert";
    strings["Mute channel at cursor"].plurals[0] = "=Mute channel at cursor";
    strings["Solo channel at cursor"].plurals[0] = "=Solo channel at cursor";
    strings["Unmute all channels"].plurals[0] = "=Unmute all channels";
    strings["Go to next order"].plurals[0] = "=Go to next order";
    strings["Go to previous order"].plurals[0] = "=Go to previous order";
    strings["Collapse channel at cursor"].plurals[0] = "=Collapse channel at cursor";
    strings["Increase effect columns"].plurals[0] = "=Increase effect columns";
    strings["Decrease effect columns"].plurals[0] = "=Decrease effect columns";
    strings["Interpolate"].plurals[0] = "=Interpolate";
    strings["Fade"].plurals[0] = "=Fade";
    strings["Invert values"].plurals[0] = "=Invert values";
    strings["Flip selection"].plurals[0] = "=Flip selection";
    strings["Collapse rows"].plurals[0] = "=Collapse rows";
    strings["Expand rows"].plurals[0] = "=Expand rows";
    strings["Collapse pattern"].plurals[0] = "=Collapse pattern";
    strings["Expand pattern"].plurals[0] = "=Expand pattern";
    strings["Collapse song"].plurals[0] = "=Collapse song";
    strings["Expand song"].plurals[0] = "=Expand song";
    strings["Set note input latch"].plurals[0] = "=Set note input latch";
    strings["Change mobile scroll mode"].plurals[0] = "=Change mobile scroll mode";
    strings["Clear note input latch"].plurals[0] = "=Clear note input latch";

    strings["---Instrument list"].plurals[0] = "=---Instrument list";
    strings["Add"].plurals[0] = "=Add";
    strings["Duplicate"].plurals[0] = "=Duplicate";
    strings["Open"].plurals[0] = "=Open";
    strings["Open (replace current)"].plurals[0] = "=Open (replace current)";
    strings["Save"].plurals[0] = "=Save";
    strings["Save (.dmp)"].plurals[0] = "=Save (.dmp)";
    strings["Move up"].plurals[0] = "=Move up";
    strings["Move down"].plurals[0] = "=Move down";
    strings["Delete"].plurals[0] = "=Delete";
    strings["Edit"].plurals[0] = "=Edit";
    strings["Cursor up"].plurals[0] = "=Cursor up";
    strings["Cursor down"].plurals[0] = "=Cursor down";
    strings["Toggle folders/standard view"].plurals[0] = "=Toggle folders/standard view";

    strings["---Wavetable list"].plurals[0] = "=---Wavetable list";
    strings["Add"].plurals[0] = "=Add";
    strings["Duplicate"].plurals[0] = "=Duplicate";
    strings["Open"].plurals[0] = "=Open";
    strings["Open (replace current)"].plurals[0] = "=Open (replace current)";
    strings["Save"].plurals[0] = "=Save";
    strings["Save (.dmw)"].plurals[0] = "=Save (.dmw)";
    strings["Save (raw)"].plurals[0] = "=Save (raw)";
    strings["Move up"].plurals[0] = "=Move up";
    strings["Move down"].plurals[0] = "=Move down";
    strings["Delete"].plurals[0] = "=Delete";
    strings["Edit"].plurals[0] = "=Edit";
    strings["Cursor up"].plurals[0] = "=Cursor up";
    strings["Cursor down"].plurals[0] = "=Cursor down";
    strings["Toggle folders/standard view"].plurals[0] = "=Toggle folders/standard view";

    strings["---Sample list"].plurals[0] = "=---Sample list";
    strings["Add"].plurals[0] = "=Add";
    strings["Duplicate"].plurals[0] = "=Duplicate";
    strings["Open"].plurals[0] = "=Open";
    strings["Open (replace current)"].plurals[0] = "=Open (replace current)";
    strings["Import raw data"].plurals[0] = "=Import raw data";
    strings["Import raw data (replace current)"].plurals[0] = "=Import raw data (replace current)";
    strings["Save"].plurals[0] = "=Save";
    strings["Save (raw)"].plurals[0] = "=Save (raw)";
    strings["Move up"].plurals[0] = "=Move up";
    strings["Move down"].plurals[0] = "=Move down";
    strings["Delete"].plurals[0] = "=Delete";
    strings["Edit"].plurals[0] = "=Edit";
    strings["Cursor up"].plurals[0] = "=Cursor up";
    strings["Cursor down"].plurals[0] = "=Cursor down";
    strings["Preview"].plurals[0] = "=Preview";
    strings["Stop preview"].plurals[0] = "=Stop preview";
    strings["Toggle folders/standard view"].plurals[0] = "=Toggle folders/standard view";

    strings["---Sample editor"].plurals[0] = "=---Sample editor";
    strings["Edit mode: Select"].plurals[0] = "=Edit mode: Select";
    strings["Edit mode: Draw"].plurals[0] = "=Edit mode: Draw";
    strings["Cut"].plurals[0] = "=Cut";
    strings["Copy"].plurals[0] = "=Copy";
    strings["Paste"].plurals[0] = "=Paste";
    strings["Paste replace"].plurals[0] = "=Paste replace";
    strings["Paste mix"].plurals[0] = "=Paste mix";
    strings["Select all"].plurals[0] = "=Select all";
    strings["Resize"].plurals[0] = "=Resize";
    strings["Resample"].plurals[0] = "=Resample";
    strings["Amplify"].plurals[0] = "=Amplify";
    strings["Normalize"].plurals[0] = "=Normalize";
    strings["Fade in"].plurals[0] = "=Fade in";
    strings["Fade out"].plurals[0] = "=Fade out";
    strings["Apply silence"].plurals[0] = "=Apply silence";
    strings["Insert silence"].plurals[0] = "=Insert silence";
    strings["Delete"].plurals[0] = "=Delete";
    strings["Trim"].plurals[0] = "=Trim";
    strings["Reverse"].plurals[0] = "=Reverse";
    strings["Invert"].plurals[0] = "=Invert";
    strings["Signed/unsigned exchange"].plurals[0] = "=Signed/unsigned exchange";
    strings["Apply filter"].plurals[0] = "=Apply filter";
    strings["Crossfade loop points"].plurals[0] = "=Crossfade loop points";
    strings["Preview sample"].plurals[0] = "=Preview sample";
    strings["Stop sample preview"].plurals[0] = "=Stop sample preview";
    strings["Zoom in"].plurals[0] = "=Zoom in";
    strings["Zoom out"].plurals[0] = "=Zoom out";
    strings["Toggle auto-zoom"].plurals[0] = "=Toggle auto-zoom";
    strings["Create instrument from sample"].plurals[0] = "=Create instrument from sample";
    strings["Set loop to selection"].plurals[0] = "=Set loop to selection";
    strings["Create wavetable from selection"].plurals[0] = "=Create wavetable from selection";

    strings["---Orders"].plurals[0] = "=---Orders";
    strings["Previous order"].plurals[0] = "=Previous order";
    strings["Next order"].plurals[0] = "=Next order";
    strings["Cursor left"].plurals[0] = "=Cursor left";
    strings["Cursor right"].plurals[0] = "=Cursor right";
    strings["Increase value"].plurals[0] = "=Increase value";
    strings["Decrease value"].plurals[0] = "=Decrease value";
    strings["Switch edit mode"].plurals[0] = "=Switch edit mode";
    strings["Toggle alter entire row"].plurals[0] = "=Toggle alter entire row";
    strings["Add"].plurals[0] = "=Add";
    strings["Duplicate"].plurals[0] = "=Duplicate";
    strings["Deep clone"].plurals[0] = "=Deep clone";
    strings["Duplicate to end of song"].plurals[0] = "=Duplicate to end of song";
    strings["Deep clone to end of song"].plurals[0] = "=Deep clone to end of song";
    strings["Remove"].plurals[0] = "=Remove";
    strings["Move up"].plurals[0] = "=Move up";
    strings["Move down"].plurals[0] = "=Move down";
    strings["Replay"].plurals[0] = "=Replay";


    //SETTINGS
    //TODO: all the "const char*" at the top of `settings.cpp`?

    //lists
    strings["<Use system font>"].plurals[0] = "=<Use system font>";
    strings["<Custom...>"].plurals[0] = "=<Custom...>";

    strings["Mono"].plurals[0] = "=Mono";
    strings["Stereo"].plurals[0] = "=Stereo";
    strings["Quadraphonic"].plurals[0] = "=Quadraphonic";
    strings["5.1 Surround"].plurals[0] = "=5.1 Surround";
    strings["7.1 Surround"].plurals[0] = "=7.1 Surround";
    strings["What?"].plurals[0] = "=What?";

    strings["High"].plurals[0] = "=High";
    strings["Low"].plurals[0] = "=Low";

    strings["ASAP (C++ port)"].plurals[0] = "=ASAP (C++ port)";

    strings["KIOCSOUND on /dev/tty1"].plurals[0] = "=KIOCSOUND on /dev/tty1";
    strings["KIOCSOUND on standard output"].plurals[0] = "=KIOCSOUND on standard output";

    strings["Disabled/custom"].plurals[0] = "=Disabled/custom";
    strings["Two octaves (0 is C-4, F is D#5)"].plurals[0] = "=Two octaves (0 is C-4, F is D#5)";
    strings["Raw (note number is value)"].plurals[0] = "=Raw (note number is value)";
    strings["Two octaves alternate (lower keys are 0-9, upper keys are A-F)"].plurals[0] = "=Two octaves alternate (lower keys are 0-9, upper keys are A-F)";
    strings["Use dual control change (one for each nibble)"].plurals[0] = "=Use dual control change (one for each nibble)";
    strings["Use 14-bit control change"].plurals[0] = "=Use 14-bit control change";
    strings["Use single control change (imprecise)"].plurals[0] = "=Use single control change (imprecise)";

    strings["--select--"].plurals[0] = "=--select--";
    strings["???"].plurals[0] = "=???";
    strings["Note Off"].plurals[0] = "=Note Off";
    strings["Note On"].plurals[0] = "=Note On";
    strings["Aftertouch"].plurals[0] = "=Aftertouch";
    strings["Control"].plurals[0] = "=Control";
    strings["Program"].plurals[0] = "=Program";
    strings["ChanPressure"].plurals[0] = "=ChanPressure";
    strings["Pitch Bend"].plurals[0] = "=Pitch Bend";
    strings["SysEx"].plurals[0] = "=SysEx";

    strings["Any"].plurals[0] = "=Any";

    strings["Instrument"].plurals[0] = "=Instrument";
    strings["Volume"].plurals[0] = "=Volume";
    strings["Effect 1 type"].plurals[0] = "=Effect 1 type";
    strings["Effect 1 value"].plurals[0] = "=Effect 1 value";
    strings["Effect 2 type"].plurals[0] = "=Effect 2 type";
    strings["Effect 2 value"].plurals[0] = "=Effect 2 value";
    strings["Effect 3 type"].plurals[0] = "=Effect 3 type";
    strings["Effect 3 value"].plurals[0] = "=Effect 3 value";
    strings["Effect 4 type"].plurals[0] = "=Effect 4 type";
    strings["Effect 4 value"].plurals[0] = "=Effect 4 value";
    strings["Effect 5 type"].plurals[0] = "=Effect 5 type";
    strings["Effect 5 value"].plurals[0] = "=Effect 5 value";
    strings["Effect 6 type"].plurals[0] = "=Effect 6 type";
    strings["Effect 6 value"].plurals[0] = "=Effect 6 value";
    strings["Effect 7 type"].plurals[0] = "=Effect 7 type";
    strings["Effect 7 value"].plurals[0] = "=Effect 7 value";
    strings["Effect 8 type"].plurals[0] = "=Effect 8 type";
    strings["Effect 8 value"].plurals[0] = "=Effect 8 value";

    //keybind prompt
    strings["Press key..."].plurals[0] = "=Press key...";

    strings["Do you want to save your settings?"].plurals[0] = "=Do you want to save your settings?";

    //general section
    strings["General"].plurals[0] = "=General";

    strings["Program"].plurals[0] = "=Program";
    strings["Render backend"].plurals[0] = "=Render backend";
    strings["you may need to restart Furnace for this setting to take effect."].plurals[0] = "=you may need to restart Furnace for this setting to take effect.";
    strings["Render driver"].plurals[0] = "=Render driver";
    strings["Automatic"].plurals[0] = "=Automatic";
    strings["Late render clear"].plurals[0] = "=Late render clear";
    strings["calls rend->clear() after rend->present(). might reduce UI latency by one frame in some drivers."].plurals[0] = "=calls rend->clear() after rend->present(). might reduce UI latency by one frame in some drivers.";
    strings["Power-saving mode"].plurals[0] = "=Power-saving mode";
    strings["saves power by lowering the frame rate to 2fps when idle.\nmay cause issues under Mesa drivers!"].plurals[0] = "=saves power by lowering the frame rate to 2fps when idle.\nmay cause issues under Mesa drivers!";
    strings["Disable threaded input (restart after changing!)"].plurals[0] = "=Disable threaded input (restart after changing!)";
    strings["threaded input processes key presses for note preview on a separate thread (on supported platforms), which reduces latency.\nhowever, crashes have been reported when threaded input is on. enable this option if that is the case."].plurals[0] = "=threaded input processes key presses for note preview on a separate thread (on supported platforms), which reduces latency.\nhowever, crashes have been reported when threaded input is on. enable this option if that is the case.";
    strings["Enable event delay"].plurals[0] = "=Enable event delay";
    strings["may cause issues with high-polling-rate mice when previewing notes."].plurals[0] = "=may cause issues with high-polling-rate mice when previewing notes.";
    strings["Per-channel oscilloscope threads"].plurals[0] = "=Per-channel oscilloscope threads";
    strings["you're being silly, aren't you? that's enough."].plurals[0] = "=you're being silly, aren't you? that's enough.";
    strings["what are you doing? stop!"].plurals[0] = "=what are you doing? stop!";
    strings["it is a bad idea to set this number higher than your CPU core count (%d)!"].plurals[0] = "=it is a bad idea to set this number higher than your CPU core count (%d)!";

    strings["File"].plurals[0] = "=File";
    strings["Use system file picker"].plurals[0] = "=Use system file picker";
    strings["Number of recent files"].plurals[0] = "=Number of recent files";
    strings["Compress when saving"].plurals[0] = "=Compress when saving";
    strings["use zlib to compress saved songs."].plurals[0] = "=use zlib to compress saved songs.";
    strings["Save unused patterns"].plurals[0] = "=Save unused patterns";
    strings["Use new pattern format when saving"].plurals[0] = "=Use new pattern format when saving";
    strings["use a packed format which saves space when saving songs.\ndisable if you need compatibility with older Furnace and/or tools\nwhich do not support this format."].plurals[0] = "=use a packed format which saves space when saving songs.\ndisable if you need compatibility with older Furnace and/or tools\nwhich do not support this format.";
    strings["Don't apply compatibility flags when loading .dmf"].plurals[0] = "=Don't apply compatibility flags when loading .dmf";
    strings["do not report any issues arising from the use of this option!"].plurals[0] = "=do not report any issues arising from the use of this option!";
    strings["Play after opening song:"].plurals[0] = "=Play after opening song:";
    strings["No##pol0"].plurals[0] = "=No##pol0";
    strings["Only if already playing##pol1"].plurals[0] = "=Only if already playing##pol1";
    strings["Yes##pol0"].plurals[0] = "=Yes##pol0";
    strings["Audio export loop/fade out time:"].plurals[0] = "=Audio export loop/fade out time:";
    strings["Set to these values on start-up:##fot0"].plurals[0] = "=Set to these values on start-up:##fot0";
    strings["Loops"].plurals[0] = "=Loops";
    strings["Fade out (seconds)"].plurals[0] = "=Fade out (seconds)";
    strings["Remember last values##fot1"].plurals[0] = "=Remember last values##fot1";
    strings["Store instrument name in .fui"].plurals[0] = "=Store instrument name in .fui";
    strings["when enabled, saving an instrument will store its name.\nthis may increase file size."].plurals[0] = "=when enabled, saving an instrument will store its name.\nthis may increase file size.";
    strings["Load instrument name from .fui"].plurals[0] = "=Load instrument name from .fui";
    strings["when enabled, loading an instrument will use the stored name (if present).\notherwise, it will use the file name."].plurals[0] = "=when enabled, loading an instrument will use the stored name (if present).\notherwise, it will use the file name.";

    strings["New Song"].plurals[0] = "=New Song";
    strings["Initial system:"].plurals[0] = "=Initial system:";
    strings["Current system"].plurals[0] = "=Current system";
    strings["Randomize"].plurals[0] = "=Randomize";
    strings["Reset to defaults"].plurals[0] = "=Reset to defaults";
    strings["Name"].plurals[0] = "=Name";
    strings["Invert"].plurals[0] = "=Invert";
    strings["Volume"].plurals[0] = "=Volume";
    strings["Panning"].plurals[0] = "=Panning";
    strings["Front/Rear"].plurals[0] = "=Front/Rear";
    strings["Configure"].plurals[0] = "=Configure";
    strings["When creating new song:"].plurals[0] = "=When creating new song:";
    strings["Display system preset selector##NSB0"].plurals[0] = "=Display system preset selector##NSB0";
    strings["Start with initial system##NSB1"].plurals[0] = "=Start with initial system##NSB1";
    strings["Default author name"].plurals[0] = "=Default author name";

    strings["Start-up"].plurals[0] = "=Start-up";
    strings["Disable fade-in during start-up"].plurals[0] = "=Disable fade-in during start-up";
    strings["About screen party time"].plurals[0] = "=About screen party time";
    strings["Warning: may cause epileptic seizures."].plurals[0] = "=Warning: may cause epileptic seizures.";

    strings["Behavior"].plurals[0] = "=Behavior";
    strings["New instruments are blank"].plurals[0] = "=New instruments are blank";

    strings["Language"].plurals[0] = "=Language";
    strings["GUI language"].plurals[0] = "=GUI language";

    //audio section

    strings["Audio"].plurals[0] = "=Audio";

    strings["Output"].plurals[0] = "=Output";
    strings["Backend"].plurals[0] = "=Backend";
    strings["Driver"].plurals[0] = "=Driver";
    strings["Automatic"].plurals[0] = "=Automatic";
    strings["you may need to restart Furnace for this setting to take effect."].plurals[0] = "=you may need to restart Furnace for this setting to take effect.";
    strings["Device"].plurals[0] = "=Device";
    strings["<click on OK or Apply first>"].plurals[0] = "=<click on OK or Apply first>";
    strings["ALERT - TRESPASSER DETECTED"].plurals[0] = "=ALERT - TRESPASSER DETECTED";
    strings["you have been arrested for trying to engage with a disabled combo box."].plurals[0] = "=you have been arrested for trying to engage with a disabled combo box.";
    strings["<System default>"].plurals[0] = "=<System default>";
    strings["Sample rate"].plurals[0] = "=Sample rate";
    strings["Outputs"].plurals[0] = "=Outputs";
    strings["Channels"].plurals[0] = "=Channels";
    strings["What?"].plurals[0] = "=What?";
    strings["Buffer size"].plurals[0] = "=Buffer size";
    strings["%d (latency: ~%.1fms)"].plurals[0] = "=%d (latency: ~%.1fms)";
    strings["Multi-threaded (EXPERIMENTAL)"].plurals[0] = "=Multi-threaded (EXPERIMENTAL)";
    strings["runs chip emulation on separate threads.\nmay increase performance when using heavy emulation cores.\n\nwarnings:\n- experimental!\n- only useful on multi-chip songs."].plurals[0] = "=runs chip emulation on separate threads.\nmay increase performance when using heavy emulation cores.\n\nwarnings:\n- experimental!\n- only useful on multi-chip songs.";
    strings["Number of threads"].plurals[0] = "=Number of threads";
    strings["that's the limit!"].plurals[0] = "=that's the limit!";
    strings["it is a VERY bad idea to set this number higher than your CPU core count (%d)!"].plurals[0] = "=it is a VERY bad idea to set this number higher than your CPU core count (%d)!";
    strings["Low-latency mode"].plurals[0] = "=Low-latency mode";
    strings["reduces latency by running the engine faster than the tick rate.\nuseful for live playback/jam mode.\n\nwarning: only enable if your buffer size is small (10ms or less)."].plurals[0] = "=reduces latency by running the engine faster than the tick rate.\nuseful for live playback/jam mode.\n\nwarning: only enable if your buffer size is small (10ms or less).";
    strings["Force mono audio"].plurals[0] = "=Force mono audio";
    strings["Exclusive mode"].plurals[0] = "=Exclusive mode";
    strings["want: %d samples @ %.0fHz (%d %s)"].plurals[0] = "=want: %d samples @ %.0fHz (%d %s)";
    strings["channel"].plurals[0] = "=channel";
    strings["channels"].plurals[0] = "=channels";
    strings["got: %d samples @ %.0fHz (%d %s)"].plurals[0] = "=got: %d samples @ %.0fHz (%d %s)";

    strings["Mixing"].plurals[0] = "=Mixing";
    strings["Quality"].plurals[0] = "=Quality";
    strings["Software clipping"].plurals[0] = "=Software clipping";
    strings["DC offset correction"].plurals[0] = "=DC offset correction";

    strings["Metronome"].plurals[0] = "=Metronome";

    strings["Sample preview"].plurals[0] = "=Sample preview";

    //MIDI section

    strings["MIDI"].plurals[0] = "=MIDI";
    strings["MIDI input"].plurals[0] = "=MIDI input";
    strings["<disabled>"].plurals[0] = "=<disabled>";
    strings["Re-scan MIDI devices"].plurals[0] = "=Re-scan MIDI devices";
    strings["Note input"].plurals[0] = "=Note input";
    strings["Velocity input"].plurals[0] = "=Velocity input";
    strings["Map MIDI channels to direct channels"].plurals[0] = "=Map MIDI channels to direct channels";
    strings["Program change pass-through"].plurals[0] = "=Program change pass-through";
    strings["Map Yamaha FM voice data to instruments"].plurals[0] = "=Map Yamaha FM voice data to instruments";
    strings["Program change is instrument selection"].plurals[0] = "=Program change is instrument selection";
    strings["Listen to MIDI clock"].plurals[0] = "=Listen to MIDI clock";
    strings["Listen to MIDI time code"].plurals[0] = "=Listen to MIDI time code";
    strings["Value input style"].plurals[0] = "=Value input style";
    strings["Control##valueCCS"].plurals[0] = "=Control##valueCCS";
    strings["CC of upper nibble##valueCC1"].plurals[0] = "=CC of upper nibble##valueCC1";
    strings["MSB CC##valueCC1"].plurals[0] = "=MSB CC##valueCC1";
    strings["CC of lower nibble##valueCC2"].plurals[0] = "=CC of lower nibble##valueCC2";
    strings["LSB CC##valueCC2"].plurals[0] = "=LSB CC##valueCC2";
    strings["Per-column control change"].plurals[0] = "=Per-column control change";
    strings["Control##valueCCS"].plurals[0] = "=Control##valueCCS";
    strings["CC of upper nibble##valueCC1"].plurals[0] = "=CC of upper nibble##valueCC1";
    strings["MSB CC##valueCC1"].plurals[0] = "=MSB CC##valueCC1";
    strings["CC of lower nibble##valueCC2"].plurals[0] = "=CC of lower nibble##valueCC2";
    strings["LSB CC##valueCC2"].plurals[0] = "=LSB CC##valueCC2";
    strings["Volume curve"].plurals[0] = "=Volume curve";
    strings["Actions:"].plurals[0] = "=Actions:";
    strings["(learning! press a button or move a slider/knob/something on your device.)"].plurals[0] = "=(learning! press a button or move a slider/knob/something on your device.)";
    strings["Type"].plurals[0] = "=Type";
    strings["Channel"].plurals[0] = "=Channel";
    strings["Note/Control"].plurals[0] = "=Note/Control";
    strings["Velocity/Value)"].plurals[0] = "=Velocity/Value)";
    strings["Action"].plurals[0] = "=Action";
    strings["Any"].plurals[0] = "=Any";
    strings["--none--"].plurals[0] = "=--none--";
    strings["waiting...##BLearn"].plurals[0] = "=waiting...##BLearn";
    strings["Learn##BLearn"].plurals[0] = "=Learn##BLearn";

    strings["MIDI output"].plurals[0] = "=MIDI output";
    strings["<disabled>"].plurals[0] = "=<disabled>";
    strings["Output mode:"].plurals[0] = "=Output mode:";
    strings["Off (use for TX81Z)"].plurals[0] = "=Off (use for TX81Z)";
    strings["Melodic"].plurals[0] = "=Melodic";
    strings["Light Show (use for Launchpad)"].plurals[0] = "=Light Show (use for Launchpad)";
    strings["Send Program Change"].plurals[0] = "=Send Program Change";
    strings["Send MIDI clock"].plurals[0] = "=Send MIDI clock";
    strings["Send MIDI timecode"].plurals[0] = "=Send MIDI timecode";
    strings["Timecode frame rate:"].plurals[0] = "=Timecode frame rate:";
    strings["Closest to Tick Rate"].plurals[0] = "=Closest to Tick Rate";
    strings["Film (24fps)"].plurals[0] = "=Film (24fps)";
    strings["PAL (25fps)"].plurals[0] = "=PAL (25fps)";
    strings["NTSC drop (29.97fps)"].plurals[0] = "=NTSC drop (29.97fps)";
    strings["NTSC non-drop (30fps)"].plurals[0] = "=NTSC non-drop (30fps)";

    //emulation section

    strings["Emulation"].plurals[0] = "=Emulation";
    strings["Cores"].plurals[0] = "=Cores";
    strings["System"].plurals[0] = "=System";
    strings["Playback Core(s)"].plurals[0] = "=Playback Core(s)";
    strings["used for playback"].plurals[0] = "=used for playback";
    strings["Render Core(s)"].plurals[0] = "=Render Core(s)";
    strings["used in audio export"].plurals[0] = "=used in audio export";
    strings["PC Speaker strategy"].plurals[0] = "=PC Speaker strategy";
    strings["Sample ROMs:"].plurals[0] = "=Sample ROMs:";
    strings["OPL4 YRW801 path"].plurals[0] = "=OPL4 YRW801 path";
    strings["MultiPCM TG100 path"].plurals[0] = "=MultiPCM TG100 path";
    strings["MultiPCM MU5 path"].plurals[0] = "=MultiPCM MU5 path";

    //keyboard section

    strings["Keyboard"].plurals[0] = "=Keyboard";
    strings["Import"].plurals[0] = "=Import";
    strings["Export"].plurals[0] = "=Export";
    strings["Reset defaults"].plurals[0] = "=Reset defaults";
    strings["Are you sure you want to reset the keyboard settings?"].plurals[0] = "=Are you sure you want to reset the keyboard settings?";
    strings["Global hotkeys"].plurals[0] = "=Global hotkeys";
    strings["Window activation"].plurals[0] = "=Window activation";
    strings["Note input"].plurals[0] = "=Note input";
    strings["Key"].plurals[0] = "=Key";
    strings["Type"].plurals[0] = "=Type";
    strings["Value"].plurals[0] = "=Value";
    strings["Remove"].plurals[0] = "=Remove";
    strings["Macro release##SNType_%d"].plurals[0] = "=Macro release##SNType_%d";
    strings["Note release##SNType_%d"].plurals[0] = "=Note release##SNType_%d";
    strings["Note off##SNType_%d"].plurals[0] = "=Note off##SNType_%d";
    strings["Note##SNType_%d"].plurals[0] = "=Note##SNType_%d";
    strings["Add..."].plurals[0] = "=Add...";
    strings["Pattern"].plurals[0] = "=Pattern";
    strings["keysPattern"].plurals[0] = "=keysPattern";
    strings["Instrument list"].plurals[0] = "=Instrument list";
    strings["Wavetable list"].plurals[0] = "=Wavetable list";
    strings["Sample list"].plurals[0] = "=Sample list";
    strings["Orders"].plurals[0] = "=Orders";
    strings["Sample editor"].plurals[0] = "=Sample editor";


    strings["TESTTEXT"].plurals[0] = "=TESTTEXT";

    //interface selection

    //appearance section

    strings["Appearance"].plurals[0] = "=Appearance";

    strings["Scaling"].plurals[0] = "=Scaling";
    strings["Automatic UI scaling factor"].plurals[0] = "=Automatic UI scaling factor";
    strings["UI scaling factor"].plurals[0] = "=UI scaling factor";
    strings["Icon size"].plurals[0] = "=Icon size";

    strings["Text"].plurals[0] = "=Text";
    strings["Font renderer"].plurals[0] = "=Font renderer";
    strings["Main font"].plurals[0] = "=Main font";
    strings["Size##MainFontSize"].plurals[0] = "=Size##MainFontSize";
    strings["Header font"].plurals[0] = "=Header font";
    strings["Size##HeadFontSize"].plurals[0] = "=Size##HeadFontSize";
    strings["Pattern font"].plurals[0] = "=Pattern font";
    strings["Size##PatFontSize"].plurals[0] = "=Size##PatFontSize";
    strings["Anti-aliased fonts"].plurals[0] = "=Anti-aliased fonts";
    strings["Support bitmap fonts"].plurals[0] = "=Support bitmap fonts";
    strings["Hinting:"].plurals[0] = "=Hinting:";
    strings["Off (soft)##fh0"].plurals[0] = "=Off (soft)##fh0";
    strings["Slight##fh1"].plurals[0] = "=Slight##fh1";
    strings["Normal##fh2"].plurals[0] = "=Normal##fh2";
    strings["Full (hard)##fh3"].plurals[0] = "=Full (hard)##fh3";
    strings["Auto-hinter:"].plurals[0] = "=Auto-hinter:";
    strings["Disable##fah0"].plurals[0] = "=Disable##fah0";
    strings["Enable##fah1"].plurals[0] = "=Enable##fah1";
    strings["Force##fah2"].plurals[0] = "=Force##fah2";
    strings["Display Japanese characters"].plurals[0] = "=Display Japanese characters";
    strings["Only toggle this option if you have enough graphics memory.\n"
            "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
            "このオプションは、十分なグラフィックメモリがある場合にのみ切り替えてください。\n"
            "これは、Dear ImGuiにダイナミックフォントアトラスが実装されるまでの一時的な解決策です。"].plurals[0] = 
            
            "=Only toggle this option if you have enough graphics memory.\n"
            "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
            "このオプションは、十分なグラフィックメモリがある場合にのみ切り替えてください。\n"
            "これは、Dear ImGuiにダイナミックフォントアトラスが実装されるまでの一時的な解決策です。";
    strings["Display Chinese (Simplified) characters"].plurals[0] = "=Display Chinese (Simplified) characters";
    strings["Only toggle this option if you have enough graphics memory.\n"
            "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
            "请在确保你有足够的显存后再启动此设定\n"
            "这是一个在ImGui实现动态字体加载之前的临时解决方案"].plurals[0] = 
            
            "=Only toggle this option if you have enough graphics memory.\n"
            "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
            "请在确保你有足够的显存后再启动此设定\n"
            "这是一个在ImGui实现动态字体加载之前的临时解决方案";
    strings["Display Chinese (Traditional) characters"].plurals[0] = "=Display Chinese (Traditional) characters";
    strings["Only toggle this option if you have enough graphics memory.\n"
            "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
            "請在確保你有足夠的顯存后再啟動此設定\n"
            "這是一個在ImGui實現動態字體加載之前的臨時解決方案"].plurals[0] = 
            
            "=Only toggle this option if you have enough graphics memory.\n"
            "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
            "請在確保你有足夠的顯存后再啟動此設定\n"
            "這是一個在ImGui實現動態字體加載之前的臨時解決方案";
    strings["Display Korean characters"].plurals[0] = "=Display Korean characters";
    strings["Only toggle this option if you have enough graphics memory.\n"
            "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
            "그래픽 메모리가 충분한 경우에만 이 옵션을 선택하십시오.\n"
            "이 옵션은 Dear ImGui에 동적 글꼴 아틀라스가 구현될 때까지 임시 솔루션입니다."].plurals[0] = 
            
            "=Only toggle this option if you have enough graphics memory.\n"
            "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
            "그래픽 메모리가 충분한 경우에만 이 옵션을 선택하십시오.\n"
            "이 옵션은 Dear ImGui에 동적 글꼴 아틀라스가 구현될 때까지 임시 솔루션입니다.";

    strings["Program"].plurals[0] = "=Program";
    strings["Title bar:"].plurals[0] = "=Title bar:";
    strings["Song Name - Furnace##tbar1"].plurals[0] = "=Song Name - Furnace##tbar1";
    strings["file_name.fur - Furnace##tbar2"].plurals[0] = "=file_name.fur - Furnace##tbar2";
    strings["/path/to/file.fur - Furnace##tbar3"].plurals[0] = "=/path/to/file.fur - Furnace##tbar3";
    strings["Display system name on title bar"].plurals[0] = "=Display system name on title bar";
    strings["Display chip names instead of \"multi-system\" in title bar"].plurals[0] = "=Display chip names instead of \"multi-system\" in title bar";
    strings["Status bar:"].plurals[0] = "=Status bar:";
    strings["Cursor details##sbar0"].plurals[0] = "=Cursor details##sbar0";
    strings["File path##sbar1"].plurals[0] = "=File path##sbar1";
    strings["Cursor details or file path##sbar2"].plurals[0] = "=Cursor details or file path##sbar2";
    strings["Nothing##sbar3"].plurals[0] = "=Nothing##sbar3";
    strings["Export options layout:"].plurals[0] = "=Export options layout:";
    strings["Sub-menus in File menu##eol0"].plurals[0] = "=Sub-menus in File menu##eol0";
    strings["Modal window with tabs##eol1"].plurals[0] = "=Modal window with tabs##eol1";
    strings["Modal windows with options in File menu##eol2"].plurals[0] = "=Modal windows with options in File menu##eol2";
    strings["Capitalize menu bar"].plurals[0] = "=Capitalize menu bar";
    strings["Display add/configure/change/remove chip menus in File menu"].plurals[0] = "=Display add/configure/change/remove chip menus in File menu";

    strings["Orders"].plurals[0] = "=Orders";
    strings["Highlight channel at cursor in Orders"].plurals[0] = "=Highlight channel at cursor in Orders";
    strings["Orders row number format:"].plurals[0] = "=Orders row number format:";
    strings["Decimal##orbD"].plurals[0] = "=Decimal##orbD";
    strings["Hexadecimal##orbH"].plurals[0] = "=Hexadecimal##orbH";

    strings["Pattern"].plurals[0] = "=Pattern";
    strings["Center pattern view"].plurals[0] = "=Center pattern view";
    strings["Overflow pattern highlights"].plurals[0] = "=Overflow pattern highlights";
    strings["Display previous/next pattern"].plurals[0] = "=Display previous/next pattern";
    strings["Pattern row number format:"].plurals[0] = "=Pattern row number format:";
    strings["Decimal##prbD"].plurals[0] = "=Decimal##prbD";
    strings["Hexadecimal##prbH"].plurals[0] = "=Hexadecimal##prbH";
    strings["Pattern view labels:"].plurals[0] = "=Pattern view labels:";
    strings["Note off (3-char)"].plurals[0] = "=Note off (3-char)";
    strings["Note release (3-char)"].plurals[0] = "=Note release (3-char)";
    strings["Macro release (3-char)"].plurals[0] = "=Macro release (3-char)";
    strings["Empty field (3-char)"].plurals[0] = "=Empty field (3-char)";
    strings["Empty field (2-char)"].plurals[0] = "=Empty field (2-char)";
    strings["Pattern view spacing after:"].plurals[0] = "=Pattern view spacing after:";
    strings["Note"].plurals[0] = "=Note";
    strings["Instrument"].plurals[0] = "=Instrument";
    strings["Volume"].plurals[0] = "=Volume";
    strings["Effect"].plurals[0] = "=Effect";
    strings["Effect value"].plurals[0] = "=Effect value";
    strings["Single-digit effects for 00-0F"].plurals[0] = "=Single-digit effects for 00-0F";
    strings["Use flats instead of sharps"].plurals[0] = "=Use flats instead of sharps";
    strings["Use German notation"].plurals[0] = "=Use German notation";

    strings["Channel"].plurals[0] = "=Channel";
    strings["Channel style:"].plurals[0] = "=Channel style:";
    strings["Classic##CHS0"].plurals[0] = "=Classic##CHS0";
    strings["Line##CHS1"].plurals[0] = "=Line##CHS1";
    strings["Round##CHS2"].plurals[0] = "=Round##CHS2";
    strings["Split button##CHS3"].plurals[0] = "=Split button##CHS3";
    strings["Square border##CHS4"].plurals[0] = "=Square border##CHS4";
    strings["Round border##CHS5"].plurals[0] = "=Round border##CHS5";
    strings["Channel volume bar:"].plurals[0] = "=Channel volume bar:";
    strings["Non##CHV0"].plurals[0] = "=Non##CHV0";
    strings["Simple##CHV1"].plurals[0] = "=Simple##CHV1";
    strings["Stereo##CHV2"].plurals[0] = "=Stereo##CHV2";
    strings["Real##CHV3"].plurals[0] = "=Real##CHV3";
    strings["Real (stereo)##CHV4"].plurals[0] = "=Real (stereo)##CHV4";
    strings["Channel feedback style:"].plurals[0] = "=Channel feedback style:";
    strings["Off##CHF0"].plurals[0] = "=Off##CHF0";
    strings["Note##CHF1"].plurals[0] = "=Note##CHF1";
    strings["Volume##CHF2"].plurals[0] = "=Volume##CHF2";
    strings["Active##CHF3"].plurals[0] = "=Active##CHF3";
    strings["Channel font:"].plurals[0] = "=Channel font:";
    strings["Regular##CHFont0"].plurals[0] = "=Regular##CHFont0";
    strings["Monospace##CHFont1"].plurals[0] = "=Monospace##CHFont1";
    strings["Center channel name"].plurals[0] = "=Center channel name";
    strings["Channel colors:"].plurals[0] = "=Channel colors:";
    strings["Single##CHC0"].plurals[0] = "=Single##CHC0";
    strings["Channel type##CHC1"].plurals[0] = "=Channel type##CHC1";
    strings["Instrument type##CHC2"].plurals[0] = "=Instrument type##CHC2";
    strings["Channel name colors:"].plurals[0] = "=Channel name colors:";
    strings["Single##CTC0"].plurals[0] = "=Single##CTC0";
    strings["Channel type##CTC1"].plurals[0] = "=Channel type##CTC1";
    strings["Instrument type##CTC2"].plurals[0] = "=Instrument type##CTC2";

    strings["Assets"].plurals[0] = "=Assets";
    strings["Unified instrument/wavetable/sample list"].plurals[0] = "=Unified instrument/wavetable/sample list";
    strings["Horizontal instrument list"].plurals[0] = "=Horizontal instrument list";
    strings["Instrument list icon style:"].plurals[0] = "=Instrument list icon style:";
    strings["None##iis0"].plurals[0] = "=None##iis0";
    strings["Graphical icons##iis1"].plurals[0] = "=Graphical icons##iis1";
    strings["Letter icons##iis2"].plurals[0] = "=Letter icons##iis2";
    strings["Colorize instrument editor using instrument type"].plurals[0] = "=Colorize instrument editor using instrument type";

    strings["Macro Editor"].plurals[0] = "=Macro Editor";
    strings["Macro editor layout:"].plurals[0] = "=Macro editor layout:";
    strings["Unified##mel0"].plurals[0] = "=Unified##mel0";
    strings["Grid##mel2"].plurals[0] = "=Grid##mel2";
    strings["Single (with list)##mel3"].plurals[0] = "=Single (with list)##mel3";
    strings["Use classic macro editor vertical slider"].plurals[0] = "=Use classic macro editor vertical slider";

    strings["Wave Editor"].plurals[0] = "=Wave Editor";
    strings["Use compact wave editor"].plurals[0] = "=Use compact wave editor";

    strings["FM Editor"].plurals[0] = "=FM Editor";
    strings["FM parameter names:"].plurals[0] = "=FM parameter names:";
    strings["Friendly##fmn0"].plurals[0] = "=Friendly##fmn0";
    strings["Technical##fmn1"].plurals[0] = "=Technical##fmn1";
    strings["Technical (alternate)##fmn2"].plurals[0] = "=Technical (alternate)##fmn2";
    strings["Use standard OPL waveform names"].plurals[0] = "=Use standard OPL waveform names";
    strings["FM parameter editor layout:"].plurals[0] = "=FM parameter editor layout:";
    strings["Modern##fml0"].plurals[0] = "=Modern##fml0";
    strings["Compact (2x2, classic)##fml1"].plurals[0] = "=Compact (2x2, classic)##fml1";
    strings["Compact (1x4)##fml2"].plurals[0] = "=Compact (1x4)##fml2";
    strings["Compact (4x1)##fml3"].plurals[0] = "=Compact (4x1)##fml3";
    strings["Alternate (2x2)##fml4"].plurals[0] = "=Alternate (2x2)##fml4";
    strings["Alternate (1x4)##fml5"].plurals[0] = "=Alternate (1x4)##fml5";
    strings["Alternate (4x1)##fml6"].plurals[0] = "=Alternate (4x1)##fml6";
    strings["Position of Sustain in FM editor:"].plurals[0] = "=Position of Sustain in FM editor:";
    strings["Between Decay and Sustain Rate##susp0"].plurals[0] = "=Between Decay and Sustain Rate##susp0";
    strings["After Release Rate##susp1"].plurals[0] = "=After Release Rate##susp1";
    strings["Use separate colors for carriers/modulators in FM editor"].plurals[0] = "=Use separate colors for carriers/modulators in FM editor";
    strings["Unsigned FM detune values"].plurals[0] = "=Unsigned FM detune values";

    strings["Statistics"].plurals[0] = "=Statistics";
    strings["Chip memory usage unit:"].plurals[0] = "=Chip memory usage unit:";
    strings["Bytes##MUU0"].plurals[0] = "=Bytes##MUU0";
    strings["Kilobytes##MUU1"].plurals[0] = "=Kilobytes##MUU1";

    strings["Oscilloscope##set"].plurals[0] = "=Oscilloscope##set";
    strings["Rounded corners"].plurals[0] = "=Rounded corners";
    strings["Border"].plurals[0] = "=Border";
    strings["Mono"].plurals[0] = "=Mono";
    strings["Anti-aliased"].plurals[0] = "=Anti-aliased";
    strings["Fill entire window"].plurals[0] = "=Fill entire window";
    strings["Waveform goes out of bounds"].plurals[0] = "=Waveform goes out of bounds";

    strings["Windows"].plurals[0] = "=Windows";
    strings["Rounded window corners"].plurals[0] = "=Rounded window corners";
    strings["Rounded buttons"].plurals[0] = "=Rounded buttons";
    strings["Rounded tabs"].plurals[0] = "=Rounded tabs";
    strings["Rounded scrollbars"].plurals[0] = "=Rounded scrollbars";
    strings["Rounded menu corners"].plurals[0] = "=Rounded menu corners";
    strings["Borders around widgets"].plurals[0] = "=Borders around widgets";

    strings["Misc"].plurals[0] = "=Misc";
    strings["Wrap text"].plurals[0] = "=Wrap text";
    strings["Wrap text in song/subsong comments window."].plurals[0] = "=Wrap text in song/subsong comments window.";
    strings["Frame shading in text windows"].plurals[0] = "=Frame shading in text windows";
    strings["Apply frame shading to the multiline text fields\nsuch as song/subsong info/comments."].plurals[0] = "=Apply frame shading to the multiline text fields\nsuch as song/subsong info/comments.";
    //strings[""].plurals[0] = "";

    // color section

}
