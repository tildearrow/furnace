#include <map>
#include <string>
#include "locale.h"

#include "russian.h"

int getPluralIndexRussian(int n)
{
    return (n%10==1 && n%100!=11) ? 0 : ((n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20)) ? 1 : 2);
    //here you can provide plural forms indices based on the integer.
    //you can find one-liners for common languages here:
    //https://www.gnu.org/software/gettext/manual/html_node/Plural-forms.html
    //these would need some adaptation to work in this code
}

class DivLocale;

void DivLocale::addTranslationsRussian()
{
    strings["%d apple"].plurals[0] = "%d яблоко";
    strings["%d apple"].plurals[1] = "%d яблока";
    strings["%d apple"].plurals[2] = "%d яблок";

    //MENU BAR ITEMS

    strings["File##menubar"].plurals[0] = "Файл##menubar";
    strings["file##menubar"].plurals[0] = "файл##menubar";
    strings["Edit##menubar"].plurals[0] = "Правка##menubar";
    strings["edit##menubar"].plurals[0] = "правка##menubar";
    strings["Settings##menubar"].plurals[0] = "Настройки##menubar";
    strings["settings##menubar"].plurals[0] = "настройки##menubar";
    strings["Window##menubar"].plurals[0] = "Окно##menubar";
    strings["window##menubar"].plurals[0] = "окно##menubar";
    strings["Help##menubar"].plurals[0] = "Справка##menubar";
    strings["help##menubar"].plurals[0] = "справка##menubar";

    //WINDOW NAMES

    strings["Settings###Settings"].plurals[0] = "Настройки###Settings";
    strings["Pattern###Pattern"].plurals[0] = "Паттерны###Pattern";
    strings["Orders###Orders"].plurals[0] = "Матр. патт.###Orders";
    strings["Statistics###Statistics"].plurals[0] = "Статистика###Statistics";
    strings["Song Info###Song Information"].plurals[0] = "О треке###Song Information";
    strings["Subsongs###Subsongs"].plurals[0] = "Подпесни###Subsongs";
    strings["About Furnace###About Furnace"].plurals[0] = "О Furnace###About Furnace";
    strings["Channels###Channels"].plurals[0] = "Каналы###Channels";
    strings["Oscilloscope (per-channel)###Oscilloscope (per-channel)"].plurals[0] = "Осц-фы (отд. кан.)###Oscilloscope (per-channel)";
    strings["Clock###Clock"].plurals[0] = "Часы###Clock";
    strings["Compatibility Flags###Compatibility Flags"].plurals[0] = "Флаги совм.###Compatibility Flags";
    strings["Instruments###Instruments"].plurals[0] = "Инструменты###Instruments";
    strings["Wavetables###Wavetables"].plurals[0] = "Волн. табл.###Wavetables";
    strings["Debug###Debug"].plurals[0] = "Отладка###Debug";
    strings["Samples###Samples"].plurals[0] = "Сэмплы###Samples";
    strings["MobileEdit###MobileEdit"].plurals[0] = "Моб. меню ред.###MobileEdit";
    strings["Mobile Controls###Mobile Controls"].plurals[0] = "Моб. элем. упр.###Mobile Controls";
    strings["Mobile Menu###Mobile Menu"].plurals[0] = "Моб. меню###Mobile Menu";
    strings["Play/Edit Controls###Play/Edit Controls"].plurals[0] = "Упр. ред./воспр.###Play/Edit Controls";
    strings["Play Controls###Play Controls"].plurals[0] = "Упр. воспр.###Play Controls";
    strings["Edit Controls###Edit Controls"].plurals[0] = "Упр. ред.###Edit Controls";
    strings["Effect List###Effect List"].plurals[0] = "Спис. эффектов###Effect List";
    strings["Find/Replace###Find/Replace"].plurals[0] = "Найти/Заменить###Find/Replace";
    strings["Grooves###Grooves"].plurals[0] = "Ритм-паттерны###Grooves";
    strings["Instrument Editor###Instrument Editor"].plurals[0] = "Ред. инструментов###Instrument Editor";
    strings["Log Viewer###Log Viewer"].plurals[0] = "Просмотр логов###Log Viewer";
    strings["Mixer###Mixer"].plurals[0] = "Микшер###Mixer";
    strings["OrderSel###OrderSel"].plurals[0] = "Выб. матр. пат.###OrderSel";
    strings["Oscilloscope###Oscilloscope"].plurals[0] = "Осциллограф###Oscilloscope";
    strings["Pattern Manager###Pattern Manager"].plurals[0] = "Упр. паттернами###Pattern Manager";
    strings["Input Pad###Input Pad"].plurals[0] = "Панель ввода###Input Pad";
    strings["Piano###Piano"].plurals[0] = "Клав. пианино###Piano";
    strings["Register View###Register View"].plurals[0] = "Просм. регистров###Register View";
    strings["Sample Editor###Sample Editor"].plurals[0] = "Ред. сэмплов###Sample Editor";
    strings["Song Comments###Song Comments"].plurals[0] = "Комм. трека###Song Comments";
    strings["Speed###Speed"].plurals[0] = "Скорость###Speed";
    strings["Spoiler###Spoiler"].plurals[0] = "Спойлер###Spoiler";
    strings["Chip Manager###Chip Manager"].plurals[0] = "Упр. чипами###Chip Manager";
    strings["Volume Meter###Volume Meter"].plurals[0] = "Изм. громкости###Volume Meter";
    strings["Wavetable Editor###Wavetable Editor"].plurals[0] = "Ред. волн. таблиц###Wavetable Editor";
    strings["Oscilloscope (X-Y)###Oscilloscope (X-Y)"].plurals[0] = "Осциллограф (X-Y)###Oscilloscope (X-Y)";

    //EFFECT LIST

    //common (non-chip-specific) effects

    strings["00xy: Arpeggio"].plurals[0] = "00xy: Арпеджио";
    
    //MACRO EDITOR

    //macro names

    strings["Volume"].plurals[0] = "Громкость";
    strings["Gain"].plurals[0] = "Усиление";

    //bitfield macros strings

    strings["hold"].plurals[0] = "удержание";
    strings["alternate"].plurals[0] = "изм. направл.";
    strings["direction"].plurals[0] = "направление";
    strings["enable"].plurals[0] = "вкл.";

    strings["right"].plurals[0] = "правый";
    strings["left"].plurals[0] = "левый";
    strings["rear right"].plurals[0] = "задний правый";
    strings["rear left"].plurals[0] = "задний левый";

    //macro hover notes

    strings["exponential"].plurals[0] = "экспоненциальное";
    strings["linear"].plurals[0] = "линейное";
    strings["direct"].plurals[0] = "прямое";

    strings["Release"].plurals[0] = "Затухание";
    strings["Loop"].plurals[0] = "Цикл";

    strings["Fixed"].plurals[0] = "Абсолютное";
    strings["Relative"].plurals[0] = "Относительное";

    strings["HP/K2, HP/K2"].plurals[0] = "ФВЧ/K2, ФВЧ/K2";
    strings["HP/K2, LP/K1"].plurals[0] = "ФВЧ/K2, ФНЧ/K1";
    strings["LP/K2, LP/K2"].plurals[0] = "ФНЧ/K2, ФНЧ/K2";
    strings["LP/K2, LP/K1"].plurals[0] = "ФНЧ/K2, ФНЧ/K1";

    strings["Saw"].plurals[0] = "Пила";
    strings["Square"].plurals[0] = "Меандр";
    strings["Triangle"].plurals[0] = "Треугольная волна";
    strings["Random"].plurals[0] = "Шум";

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
    strings["Press key..."].plurals[0] = "Нажмите клавишу...";
    strings["Do you want to save your settings?"].plurals[0] = "Вы хотите сохранить свои настройки?";

    //general section
    strings["General"].plurals[0] = "Основные";

    strings["Program"].plurals[0] = "Программа";
    strings["Render backend"].plurals[0] = "Библиотека отрисовки";
    strings["you may need to restart Furnace for this setting to take effect."].plurals[0] = "возможно, вам потребуется перезапустить Furnace, чтобы эта настрока применилась.";
    strings["Render driver"].plurals[0] = "Драйвер отрисовки";
    strings["Automatic"].plurals[0] = "Выбирать автоматически";
    strings["Late render clear"].plurals[0] = "Запаздывающая очистка буфера отрисовщика";
    strings["calls rend->clear() after rend->present(). might reduce UI latency by one frame in some drivers."].plurals[0] = "вызывает rend->clear() после rend->present(). может устранить запаздывание отрисовки интерфейса на один кадр для некоторых драйверов.";
    strings["Power-saving mode"].plurals[0] = "Режим энергосбережения";
    strings["saves power by lowering the frame rate to 2fps when idle.\nmay cause issues under Mesa drivers!"].plurals[0] = "уменьшает энергопотребление при помощи уменьшения частоты отрисовки до двух кадров в секунду в режиме ожидания.\nможет приводить к проблемам на драйверах Mesa!";
    strings["Disable threaded input (restart after changing!)"].plurals[0] = "Отключить обработку нажатий для превью инструмента в отдельном потоке (перезагрузите программу после изменения!)";
    strings["threaded input processes key presses for note preview on a separate thread (on supported platforms), which reduces latency.\nhowever, crashes have been reported when threaded input is on. enable this option if that is the case."].plurals[0] = "обработка нажатий клавиш для превью инструмента происходит в отдельном потоке (на поддерживаемых платформах), что позволяет уменьшить задержку ввода.\nтем не менее, есть сообщения о вылетах программы при выключённой настройке. включите её, если у вас программа вылетает.";
    strings["Enable event delay"].plurals[0] = "Включить задержку событий";
    strings["may cause issues with high-polling-rate mice when previewing notes."].plurals[0] = "может привести к проблемам во время превью инструмента, если подключена мышь с большой частотой обновления.";
    strings["Per-channel oscilloscope threads"].plurals[0] = "Потоки исполнения осциллографов для отдельных каналов";
    strings["you're being silly, aren't you? that's enough."].plurals[0] = "может, хватит уже хернёй страдать? этого достаточно.";
    strings["what are you doing? stop!"].plurals[0] = "ты чё делаешь? хватит!";
    strings["it is a bad idea to set this number higher than your CPU core count (%d)!"].plurals[0] = "не рекомендуется выставлять здесь значение, большее количества ядер вашего ЦП (%d)!";

    strings["File"].plurals[0] = "Файл";
    strings["Use system file picker"].plurals[0] = "Использовать диалоговое окно выбора файлов ОС";
    strings["Number of recent files"].plurals[0] = "Количество недавних файлов";
    strings["Compress when saving"].plurals[0] = "Сжимать сохраняемые файлы";
    strings["use zlib to compress saved songs."].plurals[0] = "использовать библиотеку zlib для сжатия сохраняемых модулей.";
    strings["Save unused patterns"].plurals[0] = "Сохранять неиспользуемые паттерны";
    strings["Use new pattern format when saving"].plurals[0] = "Использовать новый формат сохранения паттернов";
    strings["use a packed format which saves space when saving songs.\ndisable if you need compatibility with older Furnace and/or tools\nwhich do not support this format."].plurals[0] = "использовать сжатый формат сохранения паттернов, что позволяет уменьшить размер файла с модулем.\nотключите, если нужна совместимость со старыми версиями Furnace и/или другими программами,\nкоторые не поддерживают старый формат.";
    strings["Don't apply compatibility flags when loading .dmf"].plurals[0] = "Не применять флаги совместимости при загрузке .dmf";
    strings["do not report any issues arising from the use of this option!"].plurals[0] = "не жалуйтесь на проблемы, которые возникнут после включения этой настройки!";
    strings["Play after opening song:"].plurals[0] = "Проигрывание модуля после его загрузки:";
    strings["No##pol0"].plurals[0] = "Нет##pol0";
    strings["Only if already playing##pol1"].plurals[0] = "Только если до этого уже играл##pol1";
    strings["Yes##pol0"].plurals[0] = "Да##pol0";
    strings["Audio export loop/fade out time:"].plurals[0] = "Количество циклов проигрывания и время затухания при экспорте аудио:";
    strings["Set to these values on start-up:##fot0"].plurals[0] = "Выставить эти значения при запуске:##fot0";
    strings["Loops"].plurals[0] = "Циклы";
    strings["Fade out (seconds)"].plurals[0] = "Затухание (в секундах)";
    strings["Remember last values##fot1"].plurals[0] = "Запоминать предыдущие значения##fot1";
    strings["Store instrument name in .fui"].plurals[0] = "Сохранять название инструмента в файле .fui";
    strings["when enabled, saving an instrument will store its name.\nthis may increase file size."].plurals[0] = "При включении имя инструмента будет сохраняться в файле.\nэто может увеличить размер файла.";
    strings["Load instrument name from .fui"].plurals[0] = "Загружать имя инструмента из файла .fui";
    strings["when enabled, loading an instrument will use the stored name (if present).\notherwise, it will use the file name."].plurals[0] = "При включении имя инструмента будет загружаться из файла (при наличии имени в файле).\nВ противном случае будет использоваться имя файла.";

    strings["New Song"].plurals[0] = "Новая композиция";
    strings["Initial system:"].plurals[0] = "Система по умолчанию:";
    strings["Current system"].plurals[0] = "Текущая";
    strings["Randomize"].plurals[0] = "Выбрать случайно";
    strings["Reset to defaults"].plurals[0] = "Устан. по умолчанию";
    strings["Name"].plurals[0] = "Название";
    strings["Invert"].plurals[0] = "Обр.";
    strings["Volume"].plurals[0] = "Громкость";
    strings["Panning"].plurals[0] = "Панорамирование";
    strings["Front/Rear"].plurals[0] = "Передн./задн.";
    strings["Configure"].plurals[0] = "Настроить";
    strings["When creating new song:"].plurals[0] = "При создании новой композиции:";
    strings["Display system preset selector##NSB0"].plurals[0] = "Отобразить окно выбора пресета системы##NSB0";
    strings["Start with initial system##NSB1"].plurals[0] = "Начать с системы по умолчанию##NSB1";
    strings["Default author name"].plurals[0] = "Имя автора по умолчанию";

    strings["Start-up"].plurals[0] = "Запуск";
    strings["Disable fade-in during start-up"].plurals[0] = "Отключить плавное появление интерфейса при запуске";
    strings["About screen party time"].plurals[0] = "Вечеринка на экране \"О программе\"";
    strings["Warning: may cause epileptic seizures."].plurals[0] = "Внимание: может вызвать эпилептические приступы.";

    strings["Behavior"].plurals[0] = "Поведение программы";
    strings["New instruments are blank"].plurals[0] = "Пустые новые инструменты";

    strings["Language"].plurals[0] = "Язык";
    strings["GUI language"].plurals[0] = "Язык интерфейса";

    //audio section

    strings["Audio"].plurals[0] = "Аудио";

    strings["Output"].plurals[0] = "Вывод";
    strings["Backend"].plurals[0] = "Интерфейс";
    strings["Driver"].plurals[0] = "Драйвер";
    strings["Automatic"].plurals[0] = "Автоматически";
    strings["you may need to restart Furnace for this setting to take effect."].plurals[0] = "возможно, вам придётся перезапустить Furnace для применения настройки.";
    strings["Device"].plurals[0] = "Устройство вывода";
    strings["<click on OK or Apply first>"].plurals[0] = "=<сначала нажмите на кнопки \"ОК\" или \"Применить\">";
    strings["ALERT - TRESPASSER DETECTED"].plurals[0] = "ВНИМАНИЕ - ОБНАРУЖЕН НАРУШИТЕЛЬ";
    strings["you have been arrested for trying to engage with a disabled combo box."].plurals[0] = "вы были арестованы за попытку взаимодействия с выключенным выпадающим списком.";
    strings["<System default>"].plurals[0] = "=<По умолчанию>";
    strings["Sample rate"].plurals[0] = "Частота дискретизации";
    strings["Outputs"].plurals[0] = "Выводы";
    strings["Channels"].plurals[0] = "Число каналов";
    strings["What?"].plurals[0] = "Что?";
    strings["Buffer size"].plurals[0] = "Размер буфера";
    strings["%d (latency: ~%.1fms)"].plurals[0] = "=%d (задержка: ~%.1f мс)";
    strings["Multi-threaded (EXPERIMENTAL)"].plurals[0] = "Многопоточность (ЭКСПЕРИМЕНТАЛЬНАЯ)";
    strings["runs chip emulation on separate threads.\nmay increase performance when using heavy emulation cores.\n\nwarnings:\n- experimental!\n- only useful on multi-chip songs."].plurals[0] = "исполняет эмуляторы чипов в отдельных потоках.\nможет повысить производительность при использовании тяжёлых эмуляторов.\n\nвнимание:\n- экспериментальная функция!\n- полезна только для композиций, использующих несколько чипов.";
    strings["Number of threads"].plurals[0] = "Количество потоков";
    strings["that's the limit!"].plurals[0] = "это предел!";
    strings["it is a VERY bad idea to set this number higher than your CPU core count (%d)!"].plurals[0] = "это ОЧЕНЬ плохая идея - устанавливать это значение большим, чем колчество ядер ЦП (%d)!";
    strings["Low-latency mode"].plurals[0] = "Режим малой задержки";
    strings["reduces latency by running the engine faster than the tick rate.\nuseful for live playback/jam mode.\n\nwarning: only enable if your buffer size is small (10ms or less)."].plurals[0] = "уменьшает задержку, исполняя код движка трекера быстрее, чем указано в настройках.\nполезно для режима исполнения в реальном времени.\n\nвнимание: включайте только если размер вашего аудиобуфера мал (10 мс или меньше).";
    strings["Force mono audio"].plurals[0] = "Принудительно сводить в моно";
    strings["Exclusive mode"].plurals[0] = "Исключительный режим";
    strings["want: %d samples @ %.0fHz (%d %s)"].plurals[0] = "запрошено: %d сэмплов @ %.0fHz (%d %s)";
    strings["channel"].plurals[0] = "канал";
    strings["channel"].plurals[1] = "канала";
    strings["channel"].plurals[2] = "каналов";
    strings["got: %d samples @ %.0fHz (%d %s)"].plurals[0] = "получено: %d сэмплов @ %.0fHz (%d %s)";

    strings["Mixing"].plurals[0] = "Микширование";
    strings["Quality"].plurals[0] = "Качество";
    strings["Software clipping"].plurals[0] = "Программное ограничение сигнала";
    strings["DC offset correction"].plurals[0] = "Коррекция смещения пост. составляющей";

    strings["Metronome"].plurals[0] = "Метроном";
    strings["Volume##MetroVol"].plurals[0] = "Громкость метронома##MetroVol";

    strings["Sample preview"].plurals[0] = "Превью сэмпла";

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

    //interface selection

    //appearance section

    strings["Appearance"].plurals[0] = "Внешний вид";

    strings["Scaling"].plurals[0] = "Масштаб";
    strings["Automatic UI scaling factor"].plurals[0] = "Автоматическое масштабирование интерфейса";
    strings["UI scaling factor"].plurals[0] = "Масштаб интерфейса";
    strings["Icon size"].plurals[0] = "Размер иконок";

    strings["Text"].plurals[0] = "Текст";
    strings["Font renderer"].plurals[0] = "Отрисовщик шрифта";
    strings["Main font"].plurals[0] = "Основной шрифт";
    strings["Size##MainFontSize"].plurals[0] = "Размер##MainFontSize";
    strings["Header font"].plurals[0] = "Шрифт заголовков";
    strings["Size##HeadFontSize"].plurals[0] = "Размер##HeadFontSize";
    strings["Pattern font"].plurals[0] = "Шрифт паттернов";
    strings["Size##PatFontSize"].plurals[0] = "Размер##PatFontSize";
    strings["Anti-aliased fonts"].plurals[0] = "Сглаживание шрифтов";
    strings["Support bitmap fonts"].plurals[0] = "Поддерживать растровые шрифты";
    strings["Hinting:"].plurals[0] = "Хинтование";
    strings["Off (soft)##fh0"].plurals[0] = "Нет (слабое)##fh0";
    strings["Slight##fh1"].plurals[0] = "Небольшое##fh1";
    strings["Normal##fh2"].plurals[0] = "Нормальное##fh2";
    strings["Full (hard)##fh3"].plurals[0] = "Полное (жёсткое)##fh3";
    strings["Auto-hinter:"].plurals[0] = "Автоматическое хинтирование";
    strings["Disable##fah0"].plurals[0] = "Отключить##fah0";
    strings["Enable##fah1"].plurals[0] = "Включить##fah1";
    strings["Force##fah2"].plurals[0] = "Принудительное##fah2";
    strings["Display Japanese characters"].plurals[0] = "Отображать японские символы (вкл. иероглифы)";
    strings["Only toggle this option if you have enough graphics memory.\n"
            "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
            "このオプションは、十分なグラフィックメモリがある場合にのみ切り替えてください。\n"
            "これは、Dear ImGuiにダイナミックフォントアトラスが実装されるまでの一時的な解決策です。"].plurals[0] = 
            
            "Включайте эту настройку только в случае наличия достаточного количества графической памяти.\n"
            "Это временное решение, поскольку пока Dear ImGui не поддерживает динамический атлас шрифтов.\n\n"
            "このオプションは、十分なグラフィックメモリがある場合にのみ切り替えてください。\n"
            "これは、Dear ImGuiにダイナミックフォントアトラスが実装されるまでの一時的な解決策です。";
    strings["Display Chinese (Simplified) characters"].plurals[0] = "Отображать китайские иероглифы (упрощённые)";
    strings["Only toggle this option if you have enough graphics memory.\n"
            "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
            "请在确保你有足够的显存后再启动此设定\n"
            "这是一个在ImGui实现动态字体加载之前的临时解决方案"].plurals[0] = 
            
            "Включайте эту настройку только в случае наличия достаточного количества графической памяти.\n"
            "Это временное решение, поскольку пока Dear ImGui не поддерживает динамический атлас шрифтов.\n\n"
            "请在确保你有足够的显存后再启动此设定\n"
            "这是一个在ImGui实现动态字体加载之前的临时解决方案";
    strings["Display Chinese (Traditional) characters"].plurals[0] = "Отображать китайские иероглифы (традиционные)";
    strings["Only toggle this option if you have enough graphics memory.\n"
            "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
            "請在確保你有足夠的顯存后再啟動此設定\n"
            "這是一個在ImGui實現動態字體加載之前的臨時解決方案"].plurals[0] = 
            
            "Включайте эту настройку только в случае наличия достаточного количества графической памяти.\n"
            "Это временное решение, поскольку пока Dear ImGui не поддерживает динамический атлас шрифтов.\n\n"
            "請在確保你有足夠的顯存后再啟動此設定\n"
            "這是一個在ImGui實現動態字體加載之前的臨時解決方案";
    strings["Display Korean characters"].plurals[0] = "Отображать корейские иероглифы";
    strings["Only toggle this option if you have enough graphics memory.\n"
            "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
            "그래픽 메모리가 충분한 경우에만 이 옵션을 선택하십시오.\n"
            "이 옵션은 Dear ImGui에 동적 글꼴 아틀라스가 구현될 때까지 임시 솔루션입니다."].plurals[0] = 
            
            "Включайте эту настройку только в случае наличия достаточного количества графической памяти.\n"
            "Это временное решение, поскольку пока Dear ImGui не поддерживает динамический атлас шрифтов.\n\n"
            "그래픽 메모리가 충분한 경우에만 이 옵션을 선택하십시오.\n"
            "이 옵션은 Dear ImGui에 동적 글꼴 아틀라스가 구현될 때까지 임시 솔루션입니다.";

    strings["Program"].plurals[0] = "Программа";
    strings["Title bar:"].plurals[0] = "Полоса заголовка окна";
    strings["Song Name - Furnace##tbar1"].plurals[0] = "Название композиции - Furnace##tbar1";
    strings["file_name.fur - Furnace##tbar2"].plurals[0] = "название_файла.fur - Furnace##tbar2";
    strings["/path/to/file.fur - Furnace##tbar3"].plurals[0] = "/путь/к/файлу.fur - Furnace##tbar3";
    strings["Display system name on title bar"].plurals[0] = "Отображать название чипа/системы в полосе заголовка окна";
    strings["Display chip names instead of \"multi-system\" in title bar"].plurals[0] = "Отображать названия чипов/систем вместо \"мульти-система\" в полосе заголовка окна";
    strings["Status bar:"].plurals[0] = "Строка состояния:";
    strings["Cursor details##sbar0"].plurals[0] = "Информация о выделенном элементе##sbar0";
    strings["File path##sbar1"].plurals[0] = "Путь к файлу##sbar1";
    strings["Cursor details or file path##sbar2"].plurals[0] = "Информация о выделенном элементе или путь к файлу##sbar2";
    strings["Nothing##sbar3"].plurals[0] = "Ничего##sbar3";
    strings["Export options layout:"].plurals[0] = "Вид настроек экспорта:";
    strings["Sub-menus in File menu##eol0"].plurals[0] = "Подпункты в меню \"Файл\"##eol0";
    strings["Modal window with tabs##eol1"].plurals[0] = "Модальное окно с вкладками##eol1";
    strings["Modal windows with options in File menu##eol2"].plurals[0] = "Модальное окно с настройками в меню \"Файл\"";
    strings["Capitalize menu bar"].plurals[0] = "Названия пунктов в горизонтальном меню с большой буквы";
    strings["Display add/configure/change/remove chip menus in File menu"].plurals[0] = "Отображать в меню \"Файл\" пункты: добавить/настроить/изменить/убрать чип";

    strings["Orders"].plurals[0] = "Матрица паттернов";
    strings["Highlight channel at cursor in Orders"].plurals[0] = "Выделить в матрице паттернов канал, на котором находится курсор";
    strings["Orders row number format:"].plurals[0] = "Формат отображения номера строки матрицы паттернов:";
    strings["Decimal##orbD"].plurals[0] = "Десятеричный##orbD";
    strings["Hexadecimal##orbH"].plurals[0] = "Шестнадцатеричный##orbH";

    strings["Pattern"].plurals[0] = "Паттерн";
    strings["Center pattern view"].plurals[0] = "Центрировать отображаемые паттерны внутри окна";
    strings["Overflow pattern highlights"].plurals[0] = "Продолжать полосы подсветки строк паттернов за пределы самих паттернов";
    strings["Display previous/next pattern"].plurals[0] = "Отображать предыдущий/следующий паттерн";
    strings["Pattern row number format:"].plurals[0] = "Формат отображения номера строки паттерна:";
    strings["Decimal##prbD"].plurals[0] = "Десятеричный##prbD";
    strings["Hexadecimal##prbH"].plurals[0] = "Шестнадцатеричный##prbH";
    strings["Pattern view labels:"].plurals[0] = "Маркировка ячеек в паттерне";
    strings["Note off (3-char)"].plurals[0] = "\"Отпускание клавиши\" (резкое) (3 символа)";
    strings["Note release (3-char)"].plurals[0] = "\"Отпускание клавиши\" (с включением фазы затухания огибающей) (3 символа)";
    strings["Macro release (3-char)"].plurals[0] = "\"Отпускание клавиши\" для макросов (3 символа)";
    strings["Empty field (3-char)"].plurals[0] = "Пустая ячейка (3 символа)";
    strings["Empty field (2-char)"].plurals[0] = "Пустая ячейка (2 символа)";
    strings["Pattern view spacing after:"].plurals[0] = "Разбивка в отображении паттерна:";
    strings["Note"].plurals[0] = "Нота";
    strings["Instrument"].plurals[0] = "Инструмент";
    strings["Volume"].plurals[0] = "Громкость";
    strings["Effect"].plurals[0] = "Индекс эффекта";
    strings["Effect value"].plurals[0] = "Параметр эффекта";
    strings["Single-digit effects for 00-0F"].plurals[0] = "Отображать одной цифрой индекс эффекта для индексов 00-0F";
    strings["Use flats instead of sharps"].plurals[0] = "Отображать бемоли вместо диезов";
    strings["Use German notation"].plurals[0] = "Использовать немецкие имена нот";

    strings["Channel"].plurals[0] = "Канал";
    strings["Channel style:"].plurals[0] = "Стиль заголовка:";
    strings["Classic##CHS0"].plurals[0] = "Классический##CHS0";
    strings["Line##CHS1"].plurals[0] = "Линия##CHS1";
    strings["Round##CHS2"].plurals[0] = "Со скруглениями##CHS2";
    strings["Split button##CHS3"].plurals[0] = "С отдельной кнопкой отключения звука##CHS3";
    strings["Square border##CHS4"].plurals[0] = "С прямоугольной границей вокруг названия##CHS4";
    strings["Round border##CHS5"].plurals[0] = "Со скруглённой границей вокруг названия##CHS5";
    strings["Channel volume bar:"].plurals[0] = "Полоска громкости в заголовке канала:";
    strings["Non##CHV0"].plurals[0] = "Нет##CHV0";
    strings["Simple##CHV1"].plurals[0] = "Простая##CHV1";
    strings["Stereo##CHV2"].plurals[0] = "Стерео##CHV2";
    strings["Real##CHV3"].plurals[0] = "Настоящая громкость##CHV3";
    strings["Real (stereo)##CHV4"].plurals[0] = "Настоящая громкость (стерео)##CHV4";
    strings["Channel feedback style:"].plurals[0] = "Подсветка заголовка канала:";
    strings["Off##CHF0"].plurals[0] = "Выкл.##CHF0";
    strings["Note##CHF1"].plurals[0] = "Начало ноты##CHF1";
    strings["Volume##CHF2"].plurals[0] = "Пропорционально громкости##CHF2";
    strings["Active##CHF3"].plurals[0] = "При активности канала##CHF3";
    strings["Channel font:"].plurals[0] = "Шрифт заголовка канала:";
    strings["Regular##CHFont0"].plurals[0] = "Обычный##CHFont0";
    strings["Monospace##CHFont1"].plurals[0] = "Моноширинный##CHFont1";
    strings["Center channel name"].plurals[0] = "Центрировать название канала";
    strings["Channel colors:"].plurals[0] = "Цвета заголовка канала";
    strings["Single##CHC0"].plurals[0] = "Единый цвет##CHC0";
    strings["Channel type##CHC1"].plurals[0] = "Согласно типу канала##CHC1";
    strings["Instrument type##CHC2"].plurals[0] = "Согласно типу инструмента##CHC2";
    strings["Channel name colors:"].plurals[0] = "Цвета названия канала:";
    strings["Single##CTC0"].plurals[0] = "Единый цвет##CTC0";
    strings["Channel type##CTC1"].plurals[0] = "Согласно типу канала##CTC1";
    strings["Instrument type##CTC2"].plurals[0] = "Согласно типу инструмента##CTC2";

    strings["Assets"].plurals[0] = "Представление ресурсов модуля";
    strings["Unified instrument/wavetable/sample list"].plurals[0] = "Единый список инструментов, волновых таблиц и сэмплов";
    strings["Horizontal instrument list"].plurals[0] = "Горизонтальный список инструментов";
    strings["Instrument list icon style:"].plurals[0] = "Стиль иконок в списке инструментов:";
    strings["None##iis0"].plurals[0] = "Не показывать##iis0";
    strings["Graphical icons##iis1"].plurals[0] = "Графические иконки##iis1";
    strings["Letter icons##iis2"].plurals[0] = "Иконки с буквами##iis2";
    strings["Colorize instrument editor using instrument type"].plurals[0] = "Изменять оттенки цветов редактора инструмента согласно типу инструмента";

    strings["Macro Editor"].plurals[0] = "Редактор макросов";
    strings["Macro editor layout:"].plurals[0] = "Компоновка редактора макросов:";
    strings["Unified##mel0"].plurals[0] = "Общий список##mel0";
    strings["Grid##mel2"].plurals[0] = "Прямоугольная сетка##mel2";
    strings["Single (with list)##mel3"].plurals[0] = "Окно редактирования одного макроса + список##mel3";
    strings["Use classic macro editor vertical slider"].plurals[0] = "Использовать классическую вертикальную полосу прокрутки";

    strings["Wave Editor"].plurals[0] = "Редактор волновых таблиц";
    strings["Use compact wave editor"].plurals[0] = "Использовать компактный редактор волновых таблиц";

    strings["FM Editor"].plurals[0] = "Редактор FM-инструментов";
    strings["FM parameter names:"].plurals[0] = "Названия параметров:";
    strings["Friendly##fmn0"].plurals[0] = "Понятные##fmn0";
    strings["Technical##fmn1"].plurals[0] = "Исходные формальные##fmn1";
    strings["Technical (alternate)##fmn2"].plurals[0] = "Исходные формальные (альтернативные)##fmn2";
    strings["Use standard OPL waveform names"].plurals[0] = "Использовать стандартные названия волн для чипов серии OPL";
    strings["FM parameter editor layout:"].plurals[0] = "Компоновка редактора FM-инструментов";
    strings["Modern##fml0"].plurals[0] = "Современная##fml0";
    strings["Compact (2x2, classic)##fml1"].plurals[0] = "Компактная (2x2, классическая)##fml1";
    strings["Compact (1x4)##fml2"].plurals[0] = "Компактная (1x4)##fml2";
    strings["Compact (4x1)##fml3"].plurals[0] = "Компактная (4x1)##fml3";
    strings["Alternate (2x2)##fml4"].plurals[0] = "Альтернативная (2x2)##fml4";
    strings["Alternate (1x4)##fml5"].plurals[0] = "Альтернативная (1x4)##fml5";
    strings["Alternate (4x1)##fml6"].plurals[0] = "Альтернативная (4x1)##fml6";
    strings["Position of Sustain in FM editor:"].plurals[0] = "Позиция параметра \"Поддержка\" в редакторе:";
    strings["Between Decay and Sustain Rate##susp0"].plurals[0] = "Между спадом и уклоном поддержки##susp0";
    strings["After Release Rate##susp1"].plurals[0] = "После уклона затухания##susp1";
    strings["Use separate colors for carriers/modulators in FM editor"].plurals[0] = "Использовать различные цвета для модулирующих и несущих операторов";
    strings["Unsigned FM detune values"].plurals[0] = "Беззнаковое значение параметра расстройки";

    strings["Statistics"].plurals[0] = "Окно статистики";
    strings["Chip memory usage unit:"].plurals[0] = "Единицы измерения использования памяти чипа:";
    strings["Bytes##MUU0"].plurals[0] = "Байты##MUU0";
    strings["Kilobytes##MUU1"].plurals[0] = "Килобайты##MUU1";

    strings["Oscilloscope##set"].plurals[0] = "Осциллограф##set";
    strings["Rounded corners"].plurals[0] = "Закруглённые края";
    strings["Border"].plurals[0] = "Граница";
    strings["Mono"].plurals[0] = "Моно";
    strings["Anti-aliased"].plurals[0] = "Со сглаживанием";
    strings["Fill entire window"].plurals[0] = "Полностью заполняет окно";
    strings["Waveform goes out of bounds"].plurals[0] = "Волна может выходить за пределы окна";

    strings["Windows"].plurals[0] = "Окна";
    strings["Rounded window corners"].plurals[0] = "Закруглённые края окон";
    strings["Rounded buttons"].plurals[0] = "Закруглённые края кнопок";
    strings["Rounded tabs"].plurals[0] = "Закруглённые края заголовков вкладок";
    strings["Rounded scrollbars"].plurals[0] = "Закруглённые края ползунка полосы прокрутки";
    strings["Rounded menu corners"].plurals[0] = "Закруглённые края выпадающих меню";
    strings["Borders around widgets"].plurals[0] = "Границы вокруг кнопок, списков и т.д.";

    strings["Misc"].plurals[0] = "Разное";
    strings["Wrap text"].plurals[0] = "Переносить текст на новую строку";
    strings["Wrap text in song/subsong comments window."].plurals[0] = "Переносить текст на новую строку в окне информации/комментариев к композиции.";
    strings["Frame shading in text windows"].plurals[0] = "Градиент в текстовых окнах";
    strings["Apply frame shading to the multiline text fields\nsuch as song/subsong info/comments."].plurals[0] = "Применять градиент в окне информации/комментариев к композиции.";
    //strings[""].plurals[0] = "";
}