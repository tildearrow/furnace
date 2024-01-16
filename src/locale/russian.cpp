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
    // everything in a string after the ## or ### must remain as is
    // example: Sparkles!##sgab1 means the second instance of "Sparkles!"
    //   in `src/gui/about.cpp`.

    strings["%d apple"].plurals[0] = "%d яблоко";
    strings["%d apple"].plurals[1] = "%d яблока";
    strings["%d apple"].plurals[2] = "%d яблок";

    //src/gui/about.cpp

    strings["About Furnace###About Furnace"].plurals[0] = "=About Furnace";

    strings["and Furnace-B developers##sgab"].plurals[0] = "=and Furnace-B developers";
    strings["are proud to present##sgab"].plurals[0] = "=are proud to present";
    strings["the biggest multi-system chiptune tracker!##sgab"].plurals[0] = "=the biggest multi-system chiptune tracker!";
    strings["featuring DefleMask song compatibility.##sgab"].plurals[0] = "=featuring DefleMask song compatibility.";

    strings["> CREDITS <##sgab"].plurals[0] = "=> CREDITS <";
    strings["-- program --##sgab"].plurals[0] = "=-- program --";
    strings["A M 4 N (intro tune)##sgab"].plurals[0] = "=A M 4 N (intro tune)";
    strings["-- graphics/UI design --##sgab"].plurals[0] = "=-- graphics/UI design --";
    strings["-- documentation --##sgab"].plurals[0] = "=-- documentation --";
    strings["-- demo songs --##sgab"].plurals[0] = "=-- demo songs --";
    strings["-- additional feedback/fixes --##sgab"].plurals[0] = "=-- additional feedback/fixes --";

    strings["powered by:##sgab"].plurals[0] = "=powered by:";
    strings["Dear ImGui by Omar Cornut##sgab"].plurals[0] = "=Dear ImGui by Omar Cornut";
    strings["SDL2 by Sam Lantinga##sgab"].plurals[0] = "=SDL2 by Sam Lantinga";
    strings["zlib by Jean-loup Gailly##sgab"].plurals[0] = "=zlib by Jean-loup Gailly";
    strings["and Mark Adler##sgab"].plurals[0] = "=and Mark Adler";
    strings["libsndfile by Erik de Castro Lopo##sgab"].plurals[0] = "=libsndfile by Erik de Castro Lopo";
    strings["Portable File Dialogs by Sam Hocevar##sgab"].plurals[0] = "=Portable File Dialogs by Sam Hocevar";
    strings["Native File Dialog by Frogtoss Games##sgab"].plurals[0] = "=Native File Dialog by Frogtoss Games";
    strings["Weak-JACK by x42##sgab"].plurals[0] = "=Weak-JACK by x42";
    strings["RtMidi by Gary P. Scavone##sgab"].plurals[0] = "=RtMidi by Gary P. Scavone";
    strings["FFTW by Matteo Frigo and Steven G. Johnson##sgab"].plurals[0] = "=FFTW by Matteo Frigo and Steven G. Johnson";
    strings["backward-cpp by Google##sgab"].plurals[0] = "=backward-cpp by Google";
    strings["adpcm by superctr##sgab"].plurals[0] = "=adpcm by superctr";
    strings["Nuked-OPL3/OPLL/OPM/OPN2/PSG by nukeykt##sgab"].plurals[0] = "=Nuked-OPL3/OPLL/OPM/OPN2/PSG by nukeykt";
    strings["YM3812-LLE, YMF262-LLE and YMF276-LLE by nukeykt##sgab"].plurals[0] = "=YM3812-LLE, YMF262-LLE and YMF276-LLE by nukeykt";
    strings["ymfm by Aaron Giles##sgab"].plurals[0] = "=ymfm by Aaron Giles";
    strings["MAME SN76496 by Nicola Salmoria##sgab"].plurals[0] = "=MAME SN76496 by Nicola Salmoria";
    strings["MAME AY-3-8910 by Couriersud##sgab"].plurals[0] = "=MAME AY-3-8910 by Couriersud";
    strings["with AY8930 fixes by Eulous, cam900 and Grauw##sgab"].plurals[0] = "=with AY8930 fixes by Eulous, cam900 and Grauw";
    strings["MAME SAA1099 by Juergen Buchmueller and Manuel Abadia##sgab"].plurals[0] = "=MAME SAA1099 by Juergen Buchmueller and Manuel Abadia";
    strings["MAME Namco WSG by Nicola Salmoria and Aaron Giles##sgab"].plurals[0] = "=MAME Namco WSG by Nicola Salmoria and Aaron Giles";
    strings["MAME RF5C68 core by Olivier Galibert and Aaron Giles##sgab"].plurals[0] = "=MAME RF5C68 core by Olivier Galibert and Aaron Giles";
    strings["MAME MSM5232 core by Jarek Burczynski and Hiromitsu Shioya##sgab"].plurals[0] = "=MAME MSM5232 core by Jarek Burczynski and Hiromitsu Shioya";
    strings["MAME MSM6258 core by Barry Rodewald##sgab"].plurals[0] = "=MAME MSM6258 core by Barry Rodewald";
    strings["MAME YMZ280B core by Aaron Giles##sgab"].plurals[0] = "=MAME YMZ280B core by Aaron Giles";
    strings["MAME GA20 core by Acho A. Tang and R. Belmont##sgab"].plurals[0] = "=MAME GA20 core by Acho A. Tang and R. Belmont";
    strings["MAME SegaPCM core by Hiromitsu Shioya and Olivier Galibert##sgab"].plurals[0] = "=MAME SegaPCM core by Hiromitsu Shioya and Olivier Galibert";
    strings["SAASound by Dave Hooper and Simon Owen##sgab"].plurals[0] = "=SAASound by Dave Hooper and Simon Owen";
    strings["SameBoy by Lior Halphon##sgab"].plurals[0] = "=SameBoy by Lior Halphon";
    strings["Mednafen PCE, WonderSwan, T6W28 and Virtual Boy audio cores##sgab"].plurals[0] = "=Mednafen PCE, WonderSwan, T6W28 and Virtual Boy audio cores";
    strings["SNES DSP core by Blargg##sgab"].plurals[0] = "=SNES DSP core by Blargg";
    strings["puNES (NES, MMC5 and FDS) by FHorse##sgab"].plurals[0] = "=puNES (NES, MMC5 and FDS) by FHorse";
    strings["NSFPlay (NES and FDS) by Brad Smith and Brezza##sgab"].plurals[0] = "=NSFPlay (NES and FDS) by Brad Smith and Brezza";
    strings["reSID by Dag Lem##sgab"].plurals[0] = "=reSID by Dag Lem";
    strings["reSIDfp by Dag Lem, Antti Lankila##sgab"].plurals[0] = "=reSIDfp by Dag Lem, Antti Lankila";
    strings["and Leandro Nini##sgab"].plurals[0] = "=and Leandro Nini";
    strings["dSID by DefleMask Team based on jsSID##sgab"].plurals[0] = "=dSID by DefleMask Team based on jsSID";
    strings["Stella by Stella Team##sgab"].plurals[0] = "=Stella by Stella Team";
    strings["QSound emulator by superctr and Valley Bell##sgab"].plurals[0] = "=QSound emulator by superctr and Valley Bell";
    strings["VICE VIC-20 sound core by Rami Rasanen and viznut##sgab"].plurals[0] = "=VICE VIC-20 sound core by Rami Rasanen and viznut";
    strings["VICE TED sound core by Andreas Boose, Tibor Biczo##sgab"].plurals[0] = "=VICE TED sound core by Andreas Boose, Tibor Biczo";
    strings["and Marco van den Heuvel##sgab"].plurals[0] = "=and Marco van den Heuvel";
    strings["VERA sound core by Frank van den Hoef##sgab"].plurals[0] = "=VERA sound core by Frank van den Hoef";
    strings["mzpokeysnd POKEY emulator by Michael Borisov##sgab"].plurals[0] = "=mzpokeysnd POKEY emulator by Michael Borisov";
    strings["ASAP POKEY emulator by Piotr Fusik##sgab"].plurals[0] = "=ASAP POKEY emulator by Piotr Fusik";
    strings["ported by laoo to C++##sgab"].plurals[0] = "=ported by laoo to C++";
    strings["vgsound_emu (second version, modified version) by cam900##sgab"].plurals[0] = "=vgsound_emu (second version, modified version) by cam900";
    strings["SM8521 emulator (modified version) by cam900##sgab"].plurals[0] = "=SM8521 emulator (modified version) by cam900";
    strings["D65010G031 emulator (modified version) by cam900##sgab"].plurals[0] = "=D65010G031 emulator (modified version) by cam900";
    strings["Namco C140/C219 emulator (modified version) by cam900##sgab"].plurals[0] = "=Namco C140/C219 emulator (modified version) by cam900";
    strings["ESFMu emulator by Kagamiin~##sgab"].plurals[0] = "=ESFMu emulator by Kagamiin~";

    strings["greetings to:##sgab"].plurals[0] = "=greetings to:";
    strings["NEOART Costa Rica##sgab"].plurals[0] = "=NEOART Costa Rica";
    strings["Xenium Demoparty##sgab"].plurals[0] = "=Xenium Demoparty";
    strings["all members of Deflers of Noice!##sgab"].plurals[0] = "=all members of Deflers of Noice!";

    strings["copyright © 2021-2023 tildearrow##sgab"].plurals[0] = "=copyright © 2021-2023 tildearrow";
    strings["(and contributors).##sgab"].plurals[0] = "=(and contributors).";
    strings["licensed under GPLv2+! see##sgab"].plurals[0] = "=licensed under GPLv2+! see";
    strings["LICENSE for more information.##sgab"].plurals[0] = "=LICENSE for more information.";

    strings["help Furnace grow:##sgab"].plurals[0] = "=help Furnace grow:";
    strings["help Furnace-B:##sgab"].plurals[0] = "=help Furnace-B:";

    strings["contact tildearrow at:##sgab"].plurals[0] = "=contact tildearrow at:";

    strings["disclaimer:##sgab"].plurals[0] = "=disclaimer:";
    strings["despite the fact this program works##sgab"].plurals[0] = "=despite the fact this program works";
    strings["with the .dmf file format, it is NOT##sgab"].plurals[0] = "=with the .dmf file format, it is NOT";
    strings["affiliated with Delek or DefleMask in##sgab"].plurals[0] = "=affiliated with Delek or DefleMask in";
    strings["any way, nor it is a replacement for##sgab"].plurals[0] = "=any way, nor it is a replacement for";
    strings["the original program.##sgab"].plurals[0] = "=the original program.";

    strings["it also comes with ABSOLUTELY NO WARRANTY.##sgab"].plurals[0] = "=it also comes with ABSOLUTELY NO WARRANTY.";

    strings["thanks to all contributors/bug reporters!##sgab"].plurals[0] = "=thanks to all contributors/bug reporters!";

    //src/gui/channels.cpp

    strings["Channels###Channels"].plurals[0] = "=Channels###Channels";
    strings["Pat##sgch"].plurals[0] = "=Pat";
    strings["Osc##sgch"].plurals[0] = "=Osc";
    strings["Swap##sgch"].plurals[0] = "=Swap";
    strings["Name##sgch"].plurals[0] = "=Name";
    strings["Show in pattern##sgch"].plurals[0] = "=Show in pattern";
    strings["Show in per-channel oscilloscope##sgch"].plurals[0] = "=Show in per-channel oscilloscope";
    strings["%s #%d\n(drag to swap channels)##sgch"].plurals[0] = "=%s #%d\n(drag to swap channels)";

    //src/gui/chanOsc.cpp

    strings["None (0%)##sgco"].plurals[0] = "=None (0%)";
    strings["None (50%)##sgco"].plurals[0] = "=None (50%)";
    strings["None (100%)##sgco"].plurals[0] = "=None (100%)";
    strings["Frequency##sgco"].plurals[0] = "=Frequency";
    strings["Volume##sgco"].plurals[0] = "=Volume";
    strings["Channel##sgco"].plurals[0] = "=Channel";
    strings["Brightness##sgco"].plurals[0] = "=Brightness";
    strings["Note Trigger##sgco"].plurals[0] = "=Note Trigger";
    strings["Off##sgco"].plurals[0] = "=Off";
    strings["Mode 1##sgco"].plurals[0] = "=Mode 1";
    strings["Mode 2##sgco"].plurals[0] = "=Mode 2";
    strings["Mode 3##sgco"].plurals[0] = "=Mode 3";

    strings["Oscilloscope (per-channel)###Oscilloscope (per-channel)"].plurals[0] = "=Oscilloscope (per-channel)###Oscilloscope (per-channel)";

    strings["Columns##sgco"].plurals[0] = "=Columns";
    strings["Size (ms)##sgco"].plurals[0] = "=Size (ms)";
    strings["Automatic columns##sgco"].plurals[0] = "=Automatic columns";
    strings["Center waveform##sgco"].plurals[0] = "=Center waveform";
    strings["Randomize phase on note##sgco"].plurals[0] = "=Randomize phase on note";
    strings["Amplitude##sgco"].plurals[0] = "=Amplitude";
    strings["Gradient##sgco"].plurals[0] = "=Gradient";
    strings["Color##sgco0"].plurals[0] = "=Color";
    strings["Distance##sgco"].plurals[0] = "=Distance";
    strings["Spread##sgco"].plurals[0] = "=Spread";
    strings["Remove##sgco"].plurals[0] = "=Remove";
    strings["Background##sgco"].plurals[0] = "=Background";
    strings["X Axis##AxisX"].plurals[0] = "=X Axis##AxisX";
    strings["Y Axis##AxisY"].plurals[0] = "=Y Axis##AxisY";
    strings["Color##sgco1"].plurals[0] = "=Color";
    strings["Text format:##sgco"].plurals[0] = "=Text format:";

    strings["format guide:\n"
            "- %c: channel name\n"
            "- %C: channel short name\n"
            "- %d: channel number (starting from 0)\n"
            "- %D: channel number (starting from 1)\n"
            "- %n: channel note\n"
            "- %i: instrument name\n"
            "- %I: instrument number (decimal)\n"
            "- %x: instrument number (hex)\n"
            "- %s: chip name\n"
            "- %p: chip part number\n"
            "- %S: chip ID\n"
            "- %v: volume (decimal)\n"
            "- %V: volume (percentage)\n"
            "- %b: volume (hex)\n"
            "- %%: percent sign##sgco"].plurals[0] = 

            "=format guide:\n"
            "=- %c: channel name\n"
            "=- %C: channel short name\n"
            "=- %d: channel number (starting from 0)\n"
            "=- %D: channel number (starting from 1)\n"
            "=- %n: channel note\n"
            "=- %i: instrument name\n"
            "=- %I: instrument number (decimal)\n"
            "=- %x: instrument number (hex)\n"
            "=- %s: chip name\n"
            "=- %p: chip part number\n"
            "=- %S: chip ID\n"
            "=- %v: volume (decimal)\n"
            "=- %V: volume (percentage)\n"
            "=- %b: volume (hex)\n"
            "=- %%: percent sign";

    strings["Text color##sgco"].plurals[0] = "=Text color";
    strings["Error!##sgco"].plurals[0] = "=Error!";
    strings["\nquiet##sgco"].plurals[0] = "=\nquiet";

    //src/gui/clock.cpp

    strings["Clock###Clock"].plurals[0] = "=Clock###Clock";

    //src/gui/dataList.cpp

    strings["Bug!##sgdl"].plurals[0] = "=Bug!";
    strings["Unknown##sgdl"].plurals[0] = "=Unknown";
    strings["duplicate##sgdl0"].plurals[0] = "=duplicate";
    strings["replace...##sgdl0"].plurals[0] = "=replace...";
    strings["save##sgdl0"].plurals[0] = "=save";
    strings["save (.dmp)##sgdl"].plurals[0] = "=save (.dmp)";
    strings["delete##sgdl0"].plurals[0] = "=delete";
    strings["%.2X: <INVALID>##sgdl"].plurals[0] = "=%.2X: <INVALID>";
    strings["- None -##sgdl"].plurals[0] = "=- None -";
    strings["out of memory for this sample!##sgdl"].plurals[0] = "=out of memory for this sample!";
    strings["make instrument##sgdl"].plurals[0] = "=make instrument";
    strings["duplicate##sgdl1"].plurals[0] = "=duplicate";
    strings["replace...##sgdl1"].plurals[0] = "=replace...";
    strings["save##sgdl1"].plurals[0] = "=save";
    strings["delete##sgdl1"].plurals[0] = "=delete";
    strings["Instruments###Instruments"].plurals[0] = "=Instruments###Instruments";
    strings["Add##sgdl0"].plurals[0] = "=Add";
    strings["Duplicate##sgdl2"].plurals[0] = "=Duplicate";
    strings["Open##sgdl0"].plurals[0] = "=Open";
    strings["replace instrument...##sgdl"].plurals[0] = "=replace instrument...";
    strings["load instrument from TX81Z##sgdl"].plurals[0] = "=load instrument from TX81Z";
    strings["replace wavetable...##sgdl"].plurals[0] = "=replace wavetable...";
    strings["replace sample...##sgdl"].plurals[0] = "=replace sample...";
    strings["import raw sample...##sgdl"].plurals[0] = "=import raw sample...";
    strings["import raw sample (replace)...##sgdl"].plurals[0] = "=import raw sample (replace)...";
    strings["replace...##sgdl2"].plurals[0] = "=replace...";
    strings["load from TX81Z##sgdl"].plurals[0] = "=load from TX81Z";
    strings["Open (insert; right-click to replace)##sgdl"].plurals[0] = "=Open (insert; right-click to replace)";
    strings["Save##sgdl2"].plurals[0] = "=Save";
    strings["save instrument as .dmp...##sgdl"].plurals[0] = "=save instrument as .dmp...";
    strings["save wavetable as .dmw...##sgdl"].plurals[0] = "=save wavetable as .dmw...";
    strings["save raw wavetable...##sgdl"].plurals[0] = "=save raw wavetable...";
    strings["save raw sample...##sgdl"].plurals[0] = "=save raw sample...";
    strings["save as .dmp...##sgdl"].plurals[0] = "=save as .dmp...";
    strings["Toggle folders/standard view##sgdl0"].plurals[0] = "=Toggle folders/standard view";
    strings["Move up##sgdl0"].plurals[0] = "=Move up";
    strings["Move down##sgdl0"].plurals[0] = "=Move down";
    strings["Create##sgdl0"].plurals[0] = "=Create";
    strings["New folder##sgdl0"].plurals[0] = "=New folder";
    strings["Preview (right click to stop)##sgdl0"].plurals[0] = "=Preview (right click to stop)";
    strings["Delete##sgdl2"].plurals[0] = "=Delete";
    strings["Instruments##sgdl"].plurals[0] = "=Instruments";
    strings["<uncategorized>##sgdl0"].plurals[0] = "=<uncategorized>";
    strings["rename...##sgdl0"].plurals[0] = "=rename...";
    strings["delete##sgdl3"].plurals[0] = "=delete";
    strings["Wavetables##sgdl"].plurals[0] = "=Wavetables";
    strings["Samples##sgdl"].plurals[0] = "=Samples";
    strings["Wavetables###Wavetables"].plurals[0] = "=Wavetables###Wavetables";
    strings["Add##sgdl2"].plurals[0] = "=Add";
    strings["Duplicate##sgdl3"].plurals[0] = "=Duplicate";
    strings["Open##sgdl1"].plurals[0] = "=Open";
    strings["replace...##sgdl3"].plurals[0] = "=replace...";
    strings["Save##sgdl3"].plurals[0] = "=Save";
    strings["save as .dmw...##sgdl"].plurals[0] = "=save as .dmw...";
    strings["save raw...##sgdl0"].plurals[0] = "=save raw...";
    strings["Toggle folders/standard view##sgdl1"].plurals[0] = "=Toggle folders/standard view";
    strings["Move up##sgdl1"].plurals[0] = "=Move up";
    strings["Move down##sgdl1"].plurals[0] = "=Move down";
    strings["Create##sgdl1"].plurals[0] = "=Create";
    strings["New folder##sgdl1"].plurals[0] = "=New folder";
    strings["Delete##sgdl4"].plurals[0] = "=Delete";
    strings["Samples###Samples"].plurals[0] = "=Samples###Samples";
    strings["Add##sgdl3"].plurals[0] = "=Add";
    strings["Duplicate##sgdl4"].plurals[0] = "=Duplicate";
    strings["Open##sgdl2"].plurals[0] = "=Open";
    strings["replace...##sgdl4"].plurals[0] = "=replace...";
    strings["import raw...##sgdl"].plurals[0] = "=import raw...";
    strings["import raw (replace)...##sgdl"].plurals[0] = "=import raw (replace)...";
    strings["Save##sgdl4"].plurals[0] = "=Save";
    strings["save raw...##sgdl1"].plurals[0] = "=save raw...";
    strings["Toggle folders/standard view##sgdl2"].plurals[0] = "=Toggle folders/standard view";
    strings["Move up##sgdl2"].plurals[0] = "=Move up";
    strings["Move down##sgdl2"].plurals[0] = "=Move down";
    strings["Create##sgdl2"].plurals[0] = "=Create";
    strings["New folder##sgdl2"].plurals[0] = "=New folder";
    strings["Preview (right click to stop)##sgdl1"].plurals[0] = "=Preview (right click to stop)";
    strings["Delete##sgdl5"].plurals[0] = "=Delete";
    strings["<uncategorized>##sgdl1"].plurals[0] = "=<uncategorized>";
    strings["rename...##sgdl1"].plurals[0] = "=rename...";
    strings["delete##sgdl6"].plurals[0] = "=delete";
    strings["rename...##sgdl2"].plurals[0] = "=rename...";
    strings["delete##sgdl7"].plurals[0] = "=delete";

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

    //MENU BARS

    strings["new...##sggu"].plurals[0] = "=new...";
    strings["Unsaved changes! Save changes before creating a new song?##sggu"].plurals[0] = "=Unsaved changes! Save changes before creating a new song?";
    strings["open...##sggu"].plurals[0] = "=open...";
    strings["Unsaved changes! Save changes before opening another file?##sggu"].plurals[0] = "=Unsaved changes! Save changes before opening another file?";
    strings["open recent##sggu"].plurals[0] = "=open recent";
    strings["Unsaved changes! Save changes before opening file?##sggu"].plurals[0] = "=Unsaved changes! Save changes before opening file?";
    strings["Error while loading file! (%s)##sggu"].plurals[0] = "=Error while loading file! (%s)";
    strings["nothing here yet##sggu"].plurals[0] = "=nothing here yet";
    strings["clear history##sggu"].plurals[0] = "=clear history";
    strings["Are you sure you want to clear the recent file list?##sggu"].plurals[0] = "=Are you sure you want to clear the recent file list?";
    strings["save##sggu"].plurals[0] = "=save";
    strings["Error while saving file! (%s)##sggu"].plurals[0] = "=Error while saving file! (%s)";
    strings["save as...##sggu"].plurals[0] = "=save as...";
    strings["export audio...##sggu"].plurals[0] = "=export audio...";
    strings["export VGM...##sggu"].plurals[0] = "=export VGM...";
    strings["export ZSM...##sggu"].plurals[0] = "=export ZSM...";
    strings["export Amiga validation data...##sggu"].plurals[0] = "=export Amiga validation data...";
    strings["export text...##sggu"].plurals[0] = "=export text...";
    strings["export command stream...##sggu"].plurals[0] = "=export command stream...";
    strings["export...##sggu"].plurals[0] = "=export...";
    strings["manage chips##sggu"].plurals[0] = "=manage chips";
    strings["add chip...##sggu"].plurals[0] = "=add chip...";
    strings["cannot add chip! (##sggu"].plurals[0] = "=cannot add chip! (";
    strings["configure chip...##sggu"].plurals[0] = "=configure chip...##sggu";
    strings["change chip...##sggu"].plurals[0] = "=change chip...";
    strings["Preserve channel positions##sggu0"].plurals[0] = "=Preserve channel positions";
    strings["remove chip...##sggu"].plurals[0] = "=remove chip...";
    strings["Preserve channel positions##sggu1"].plurals[0] = "=Preserve channel positions";
    strings["cannot remove chip! (##sggu"].plurals[0] = "=cannot remove chip! (";
    strings["restore backup##sggu"].plurals[0] = "=restore backup";
    strings["Unsaved changes! Save before quitting?##sggu"].plurals[0] = "=Unsaved changes! Save before quitting?";
    
    strings["undo##sggu"].plurals[0] = "=undo";
    strings["redo##sggu"].plurals[0] = "=redo";
    strings["cut##sggu"].plurals[0] = "=cut";
    strings["copy##sggu"].plurals[0] = "=copy";
    strings["paste##sggu0"].plurals[0] = "=paste";
    strings["paste special...##sggu"].plurals[0] = "=paste special...##sggu";
    strings["paste mix##sggu"].plurals[0] = "=paste mix";
    strings["paste mix (background)##sggu"].plurals[0] = "=paste mix (background)";
    strings["paste with ins (foreground)##sggu"].plurals[0] = "=paste with ins (foreground)";
    strings["no instruments available##sggu0"].plurals[0] = "=no instruments available";
    strings["paste with ins (background)##sggu"].plurals[0] = "=paste with ins (background)";
    strings["no instruments available##sggu1"].plurals[0] = "=no instruments available";
    strings["paste flood##sggu"].plurals[0] = "=paste flood";
    strings["paste overflow##sggu"].plurals[0] = "=paste overflow";
    strings["delete##sggu0"].plurals[0] = "=delete";
    strings["select all##sggu"].plurals[0] = "=select all";
    strings["operation mask...##sggu"].plurals[0] = "=operation mask...";
    strings["delete##sggu1"].plurals[0] = "=delete";
    strings["pull delete##sggu"].plurals[0] = "=pull delete";
    strings["insert##sggu"].plurals[0] = "=insert";
    strings["paste##sggu1"].plurals[0] = "=paste";
    strings["transpose (note)##sggu"].plurals[0] = "=transpose (note)";
    strings["transpose (value)##sggu"].plurals[0] = "=transpose (value)";
    strings["interpolate##sggu"].plurals[0] = "=interpolate";
    strings["fade##sggu"].plurals[0] = "=fade";
    strings["invert values##sggu"].plurals[0] = "=invert values";
    strings["scale##sggu"].plurals[0] = "=scale";
    strings["randomize##sggu"].plurals[0] = "=randomize";
    strings["flip##sggu"].plurals[0] = "=flip";
    strings["collapse/expand##sggu"].plurals[0] = "=collapse/expand";
    strings["input latch##sggu"].plurals[0] = "=input latch";
    strings["&&: selected instrument\n..: no instrument##sggu"].plurals[0] = "=&&: selected instrument\n..: no instrument";
    strings["Set##sggu"].plurals[0] = "=Set";
    strings["Reset##sggu"].plurals[0] = "=Reset";
    strings["note up##sggu"].plurals[0] = "=note up";
    strings["note down##sggu"].plurals[0] = "=note down";
    strings["octave up##sggu"].plurals[0] = "=octave up";
    strings["octave down##sggu"].plurals[0] = "=octave down";
    strings["values up##sggu"].plurals[0] = "=values up";
    strings["values down##sggu"].plurals[0] = "=values down";
    strings["values up (+16)##sggu"].plurals[0] = "=values up (+16)";
    strings["values down (-16)##sggu"].plurals[0] = "=values down (-16)";
    strings["transpose##sggu"].plurals[0] = "=transpose";
    strings["Notes##sggu"].plurals[0] = "=Notes";
    strings["Values##sggu"].plurals[0] = "=Values";
    strings["interpolate##sggu"].plurals[0] = "=interpolate";
    strings["change instrument...##sggu"].plurals[0] = "=change instrument...";
    strings["no instruments available##sggu"].plurals[0] = "=no instruments available";
    strings["gradient/fade...##sggu"].plurals[0] = "=gradient/fade...";
    strings["Start##sggu"].plurals[0] = "=Start";
    strings["End##sggu"].plurals[0] = "=End";
    strings["Nibble mode##sggu0"].plurals[0] = "=Nibble mode";
    strings["Go ahead##sggu"].plurals[0] = "=Go ahead";
    strings["scale...##sggu"].plurals[0] = "=scale...";
    strings["Scale##sggu"].plurals[0] = "=Scale";
    strings["randomize...##sggu"].plurals[0] = "=randomize...";
    strings["Minimum##sggu"].plurals[0] = "=Minimum";
    strings["Maximum##sggu"].plurals[0] = "=Maximum";
    strings["Nibble mode##sggu1"].plurals[0] = "=Nibble mode";
    strings["Randomize##sggu"].plurals[0] = "=Randomize";
    strings["invert values##sggu"].plurals[0] = "=invert values";
    strings["flip selection##sggu"].plurals[0] = "=flip selection";
    strings["collapse/expand amount##CollapseAmount"].plurals[0] = "=collapse/expand amount##CollapseAmount";
    strings["collapse##sggu"].plurals[0] = "=collapse";
    strings["expand##sggu"].plurals[0] = "=expand";
    strings["collapse pattern##sggu"].plurals[0] = "=collapse pattern";
    strings["expand pattern##sggu"].plurals[0] = "=expand pattern";
    strings["collapse song##sggu"].plurals[0] = "=collapse song";
    strings["expand song##sggu"].plurals[0] = "=expand song";
    strings["find/replace##sggu"].plurals[0] = "=find/replace";
    strings["clear...##sggu"].plurals[0] = "=clear...";

    strings["full screen##sggu"].plurals[0] = "=full screen";
    strings["lock layout##sggu"].plurals[0] = "=lock layout";
    strings["visualizer##sggu"].plurals[0] = "=visualizer";
    strings["reset layout##sggu"].plurals[0] = "=reset layout";
    strings["Are you sure you want to reset the workspace layout?##sggu"].plurals[0] = "=Are you sure you want to reset the workspace layout?";
    strings["switch to mobile view##sggu"].plurals[0] = "=switch to mobile view";
    strings["settings...##sggu"].plurals[0] = "=settings...";

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
    //popups
    strings["Warning###Warning"].plurals[0] = "=Warning###Warning";
    strings["Error###Error"].plurals[0] = "=Error###Error";
    strings["Select Instrument###Select Instrument"].plurals[0] = "=Select Instrument###Select Instrument";
    strings["Import Raw Sample###Import Raw Sample"].plurals[0] = "=Import Raw Sample###Import Raw Sample";
    strings["Rendering...###Rendering..."].plurals[0] = "=Rendering...###Rendering...";
    strings["New Song###New Song"].plurals[0] = "=New Song###New Song";

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


    //src/gui/settings.cpp


    strings["<Use system font>##sgse0"].plurals[0] = "=<Use system font>";
    strings["<Custom...>##sgse0"].plurals[0] = "=<Custom...>";
    strings["<Use system font>##sgse1"].plurals[0] = "=<Use system font>";
    strings["<Custom...>##sgse1"].plurals[0] = "=<Custom...>";
    strings["<Use system font>##sgse2"].plurals[0] = "=<Use system font>";
    strings["<Custom...>##sgse2"].plurals[0] = "=<Custom...>";
    strings["Mono##sgse0"].plurals[0] = "=Mono";
    strings["Stereo##sgse"].plurals[0] = "=Stereo";
    strings["What?##sgse0"].plurals[0] = "=What?";
    strings["Quadraphonic##sgse"].plurals[0] = "=Quadraphonic";
    strings["What?##sgse1"].plurals[0] = "=What?";
    strings["5.1 Surround##sgse"].plurals[0] = "=5.1 Surround";
    strings["What?##sgse2"].plurals[0] = "=What?";
    strings["7.1 Surround##sgse"].plurals[0] = "=7.1 Surround";
    strings["High##sgse"].plurals[0] = "=High";
    strings["Low##sgse"].plurals[0] = "=Low";
    strings["ASAP (C++ port)##sgse"].plurals[0] = "=ASAP (C++ port)";
    strings["KIOCSOUND on /dev/tty1##sgse"].plurals[0] = "=KIOCSOUND on /dev/tty1";
    strings["KIOCSOUND on standard output##sgse"].plurals[0] = "=KIOCSOUND on standard output";
    strings["Disabled/custom##sgse0"].plurals[0] = "=Disabled/custom";
    strings["Raw (note number is value)##sgse"].plurals[0] = "=Raw (note number is value)";
    strings["Two octaves alternate (lower keys are 0-9, upper keys are A-F)##sgse"].plurals[0] = "=Two octaves alternate (lower keys are 0-9, upper keys are A-F)";
    strings["Use dual control change (one for each nibble)##sgse0"].plurals[0] = "=Use dual control change (one for each nibble)";
    strings["Use 14-bit control change##sgse0"].plurals[0] = "=Use 14-bit control change";
    strings["Use single control change (imprecise)##sgse0"].plurals[0] = "=Use single control change (imprecise)";
    strings["Disabled/custom##sgse1"].plurals[0] = "=Disabled/custom";
    strings["Use dual control change (one for each nibble)##sgse1"].plurals[0] = "=Use dual control change (one for each nibble)";
    strings["Use 14-bit control change##sgse1"].plurals[0] = "=Use 14-bit control change";
    strings["Use single control change (imprecise)##sgse1"].plurals[0] = "=Use single control change (imprecise)";
    strings["--select--##sgse"].plurals[0] = "=--select--";
    strings["Note Off##sgse"].plurals[0] = "=Note Off";
    strings["Note On##sgse"].plurals[0] = "=Note On";
    strings["Aftertouch##sgse"].plurals[0] = "=Aftertouch";
    strings["Control##sgse"].plurals[0] = "=Control";
    strings["Program##sgse0"].plurals[0] = "=Program";
    strings["ChanPressure##sgse"].plurals[0] = "=ChanPressure";
    strings["Pitch Bend##sgse"].plurals[0] = "=Pitch Bend";
    strings["SysEx##sgse"].plurals[0] = "=SysEx";
    strings["Instrument##sgse0"].plurals[0] = "=Instrument";
    strings["Volume##sgse0"].plurals[0] = "=Volume";
    strings["Effect 1 type##sgse"].plurals[0] = "=Effect 1 type";
    strings["Effect 1 value##sgse"].plurals[0] = "=Effect 1 value";
    strings["Effect 2 type##sgse"].plurals[0] = "=Effect 2 type";
    strings["Effect 2 value##sgse"].plurals[0] = "=Effect 2 value";
    strings["Effect 3 type##sgse"].plurals[0] = "=Effect 3 type";
    strings["Effect 3 value##sgse"].plurals[0] = "=Effect 3 value";
    strings["Effect 4 type##sgse"].plurals[0] = "=Effect 4 type";
    strings["Effect 4 value##sgse"].plurals[0] = "=Effect 4 value";
    strings["Effect 5 type##sgse"].plurals[0] = "=Effect 5 type";
    strings["Effect 5 value##sgse"].plurals[0] = "=Effect 5 value";
    strings["Effect 6 type##sgse"].plurals[0] = "=Effect 6 type";
    strings["Effect 6 value##sgse"].plurals[0] = "=Effect 6 value";
    strings["Effect 7 type##sgse"].plurals[0] = "=Effect 7 type";
    strings["Effect 7 value##sgse"].plurals[0] = "=Effect 7 value";
    strings["Effect 8 type##sgse"].plurals[0] = "=Effect 8 type";
    strings["Effect 8 value##sgse"].plurals[0] = "=Effect 8 value";

    strings["Press key...##sgse"].plurals[0] = "Нажмите клавишу...";
    strings["Settings###Settings"].plurals[0] = "=Settings###Settings";
    strings["Do you want to save your settings?##sgse"].plurals[0] = "Вы хотите сохранить свои настройки?";

    strings["General##sgse"].plurals[0] = "Основные";
    strings["Program##sgse1"].plurals[0] = "Программа";
    strings["Render backend##sgse"].plurals[0] = "Библиотека отрисовки";
    strings["you may need to restart Furnace for this setting to take effect.##sgse0"].plurals[0] = "возможно, вам потребуется перезапустить Furnace, чтобы эта настрока применилась.";
    strings["Render driver##sgse"].plurals[0] = "Драйвер отрисовки";
    strings["Automatic##sgse0"].plurals[0] = "Выбирать автоматически";
    strings["Automatic##sgse1"].plurals[0] = "Выбирать автоматически";
    strings["you may need to restart Furnace for this setting to take effect.##sgse1"].plurals[0] = "возможно, вам потребуется перезапустить Furnace, чтобы эта настрока применилась.";
    strings["Late render clear##sgse"].plurals[0] = "Запаздывающая очистка буфера отрисовщика";
    strings["calls rend->clear() after rend->present(). might reduce UI latency by one frame in some drivers.##sgse"].plurals[0] = "вызывает rend->clear() после rend->present(). может устранить запаздывание отрисовки интерфейса на один кадр для некоторых драйверов.";
    strings["Power-saving mode##sgse"].plurals[0] = "Режим энергосбережения";
    strings["saves power by lowering the frame rate to 2fps when idle.\nmay cause issues under Mesa drivers!##sgse"].plurals[0] = "уменьшает энергопотребление при помощи уменьшения частоты отрисовки до двух кадров в секунду в режиме ожидания.\nможет приводить к проблемам на драйверах Mesa!";
    strings["Disable threaded input (restart after changing!)##sgse"].plurals[0] = "Отключить обработку нажатий для превью инструмента в отдельном потоке (перезагрузите программу после изменения!)";
    strings["threaded input processes key presses for note preview on a separate thread (on supported platforms), which reduces latency.\nhowever, crashes have been reported when threaded input is on. enable this option if that is the case.##sgse"].plurals[0] = "обработка нажатий клавиш для превью инструмента происходит в отдельном потоке (на поддерживаемых платформах), что позволяет уменьшить задержку ввода.\nтем не менее, есть сообщения о вылетах программы при выключённой настройке. включите её, если у вас программа вылетает.";
    strings["Enable event delay##sgse"].plurals[0] = "Включить задержку событий";
    strings["may cause issues with high-polling-rate mice when previewing notes.##sgse"].plurals[0] = "может привести к проблемам во время превью инструмента, если подключена мышь с большой частотой обновления.";
    strings["Per-channel oscilloscope threads##sgse"].plurals[0] = "Потоки исполнения осциллографов для отдельных каналов";
    strings["you're being silly, aren't you? that's enough.##sgse"].plurals[0] = "может, хватит уже хернёй страдать? этого достаточно.";
    strings["what are you doing? stop!##sgse"].plurals[0] = "ты чё делаешь? хватит!";
    strings["it is a bad idea to set this number higher than your CPU core count (%d)!##sgse"].plurals[0] = "не рекомендуется выставлять здесь значение, большее количества ядер вашего ЦП (%d)!";
    strings["File##sgse"].plurals[0] = "Файл";
    strings["Use system file picker##sgse"].plurals[0] = "Использовать диалоговое окно выбора файлов ОС";
    strings["Number of recent files##sgse"].plurals[0] = "Количество недавних файлов";
    strings["Compress when saving##sgse"].plurals[0] = "Сжимать сохраняемые файлы";
    strings["use zlib to compress saved songs.##sgse"].plurals[0] = "использовать библиотеку zlib для сжатия сохраняемых модулей.";
    strings["Save unused patterns##sgse"].plurals[0] = "Сохранять неиспользуемые паттерны";
    strings["Use new pattern format when saving##sgse"].plurals[0] = "Использовать новый формат сохранения паттернов";
    strings["use a packed format which saves space when saving songs.\ndisable if you need compatibility with older Furnace and/or tools\nwhich do not support this format.##sgse"].plurals[0] = "использовать сжатый формат сохранения паттернов, что позволяет уменьшить размер файла с модулем.\nотключите, если нужна совместимость со старыми версиями Furnace и/или другими программами,\nкоторые не поддерживают старый формат.";
    strings["Don't apply compatibility flags when loading .dmf##sgse"].plurals[0] = "Не применять флаги совместимости при загрузке .dmf";
    strings["do not report any issues arising from the use of this option!##sgse"].plurals[0] = "не жалуйтесь на проблемы, которые возникнут после включения этой настройки!";
    strings["Play after opening song:##sgse"].plurals[0] = "Проигрывание модуля после его загрузки:";
    strings["No##pol0"].plurals[0] = "Нет##pol0";
    strings["Only if already playing##pol1"].plurals[0] = "Только если до этого уже играл##pol1";
    strings["Yes##pol0"].plurals[0] = "Да##pol0";
    strings["Audio export loop/fade out time:##sgse"].plurals[0] = "Количество циклов проигрывания и время затухания при экспорте аудио:";
    strings["Set to these values on start-up:##fot0"].plurals[0] = "Выставить эти значения при запуске:##fot0";
    strings["Loops##sgse"].plurals[0] = "Циклы";
    strings["Fade out (seconds)##sgse"].plurals[0] = "Затухание (в секундах)";
    strings["Remember last values##fot1"].plurals[0] = "Запоминать предыдущие значения##fot1";
    strings["Store instrument name in .fui##sgse"].plurals[0] = "Сохранять название инструмента в файле .fui";
    strings["when enabled, saving an instrument will store its name.\nthis may increase file size.##sgse"].plurals[0] = "При включении имя инструмента будет сохраняться в файле.\nэто может увеличить размер файла.";
    strings["Load instrument name from .fui##sgse"].plurals[0] = "Загружать имя инструмента из файла .fui";
    strings["when enabled, loading an instrument will use the stored name (if present).\notherwise, it will use the file name.##sgse"].plurals[0] = "При включении имя инструмента будет загружаться из файла (при наличии имени в файле).\nВ противном случае будет использоваться имя файла.";
    strings["New Song##sgse"].plurals[0] = "Новая композиция";
    strings["Initial system:##sgse"].plurals[0] = "Система по умолчанию:";
    strings["Current system##sgse"].plurals[0] = "Текущая";
    strings["Randomize##sgse"].plurals[0] = "Выбрать случайно";
    strings["Reset to defaults##sgse"].plurals[0] = "Устан. по умолчанию";
    strings["Name##sgse"].plurals[0] = "Название";
    strings["Invert##sgse0"].plurals[0] = "Обр.";
    strings["Invert##sgse1"].plurals[0] = "Обр.";
    strings["Volume##sgse1"].plurals[0] = "Громкость";
    strings["Panning##sgse"].plurals[0] = "Панорамирование";
    strings["Front/Rear##sgse"].plurals[0] = "Передн./задн.";
    strings["Configure##sgse"].plurals[0] = "Настроить";
    strings["When creating new song:##sgse"].plurals[0] = "При создании новой композиции:";
    strings["Display system preset selector##NSB0"].plurals[0] = "Отобразить окно выбора пресета системы##NSB0";
    strings["Start with initial system##NSB1"].plurals[0] = "Начать с системы по умолчанию##NSB1";
    strings["Default author name##sgse"].plurals[0] = "Имя автора по умолчанию";
    strings["Start-up##sgse"].plurals[0] = "Запуск";
    strings["Disable fade-in during start-up##sgse"].plurals[0] = "Отключить плавное появление интерфейса при запуске";
    strings["About screen party time##sgse"].plurals[0] = "Вечеринка на экране \"О программе\"";
    strings["Warning: may cause epileptic seizures.##sgse"].plurals[0] = "Внимание: может вызвать эпилептические приступы.";
    strings["Behavior##sgse"].plurals[0] = "Поведение программы";
    strings["New instruments are blank##sgse"].plurals[0] = "Пустые новые инструменты";
    strings["Language##sgse"].plurals[0] = "Язык";
    strings["GUI language##sgse"].plurals[0] = "Язык интерфейса";
    strings["Audio##sgse"].plurals[0] = "Аудио";
    strings["Output##sgse"].plurals[0] = "Вывод";
    strings["Backend##sgse"].plurals[0] = "Интерфейс";
    strings["JACK##sgse"].plurals[0] = "=JACK";
    strings["SDL##sgse"].plurals[0] = "=SDL";
    strings["PortAudio##sgse"].plurals[0] = "=PortAudio";
    strings["Driver##sgse"].plurals[0] = "Драйвер";
    strings["Automatic##sgse2"].plurals[0] = "Автоматически";
    strings["you may need to restart Furnace for this setting to take effect.##sgse2"].plurals[0] = "возможно, вам придётся перезапустить Furnace для применения настройки.";
    strings["Device##sgse"].plurals[0] = "Устройство вывода";
    strings["<click on OK or Apply first>##sgse"].plurals[0] = "<сначала нажмите на кнопки \"ОК\" или \"Применить\">";
    strings["ALERT - TRESPASSER DETECTED##sgse"].plurals[0] = "ВНИМАНИЕ - ОБНАРУЖЕН НАРУШИТЕЛЬ";
    strings["you have been arrested for trying to engage with a disabled combo box.##sgse"].plurals[0] = "вы были арестованы за попытку взаимодействия с выключенным выпадающим списком.";
    strings["<System default>##sgse0"].plurals[0] = "<По умолчанию>";
    strings["<System default>##sgse1"].plurals[0] = "<По умолчанию>";
    strings["Sample rate##sgse"].plurals[0] = "Частота дискретизации";
    strings["Outputs##sgse"].plurals[0] = "Выводы";
    strings["Channels##sgse"].plurals[0] = "Число каналов";
    strings["What?##sgse3"].plurals[0] = "Что?";
    strings["Buffer size##sgse"].plurals[0] = "Размер буфера";
    strings["%d (latency: ~%.1fms)##sgse"].plurals[0] = "=%d (задержка: ~%.1f мс)";
    strings["Multi-threaded (EXPERIMENTAL)##sgse"].plurals[0] = "Многопоточность (ЭКСПЕРИМЕНТАЛЬНАЯ)";
    strings["runs chip emulation on separate threads.\nmay increase performance when using heavy emulation cores.\n\nwarnings:\n- experimental!\n- only useful on multi-chip songs.##sgse"].plurals[0] = "исполняет эмуляторы чипов в отдельных потоках.\nможет повысить производительность при использовании тяжёлых эмуляторов.\n\nвнимание:\n- экспериментальная функция!\n- полезна только для композиций, использующих несколько чипов.";
    strings["Number of threads##sgse"].plurals[0] = "Количество потоков";
    strings["that's the limit!##sgse"].plurals[0] = "это предел!";
    strings["it is a VERY bad idea to set this number higher than your CPU core count (%d)!##sgse"].plurals[0] = "это ОЧЕНЬ плохая идея - устанавливать это значение большим, чем колчество ядер ЦП (%d)!";
    strings["Low-latency mode##sgse"].plurals[0] = "Режим малой задержки";
    strings["reduces latency by running the engine faster than the tick rate.\nuseful for live playback/jam mode.\n\nwarning: only enable if your buffer size is small (10ms or less).##sgse"].plurals[0] = "уменьшает задержку, исполняя код движка трекера быстрее, чем указано в настройках.\nполезно для режима исполнения в реальном времени.\n\nвнимание: включайте только если размер вашего аудиобуфера мал (10 мс или меньше).";
    strings["Force mono audio##sgse"].plurals[0] = "Принудительно сводить в моно";
    strings["Exclusive mode##sgse"].plurals[0] = "Исключительный режим";
    strings["want: %d samples @ %.0fHz (%d %s)##sgse"].plurals[0] = "запрошено: %d сэмплов @ %.0fHz (%d %s)";
    strings["channel##sgse"].plurals[0] = "канал";
    strings["channel##sgse"].plurals[1] = "канала";
    strings["channel##sgse"].plurals[2] = "каналов";
    strings["got: %d samples @ %.0fHz (%d %s)##sgse"].plurals[0] = "получено: %d сэмплов @ %.0fHz (%d %s)";
    strings["Mixing##sgse"].plurals[0] = "Микширование";
    strings["Quality##sgse"].plurals[0] = "Качество";
    strings["Software clipping##sgse"].plurals[0] = "Программное ограничение сигнала";
    strings["DC offset correction##sgse"].plurals[0] = "Коррекция смещения пост. составляющей";
    strings["Metronome##sgse"].plurals[0] = "Метроном";
    strings["Volume##sgse2"].plurals[0] = "Громкость метронома";
    strings["Sample preview##sgse"].plurals[0] = "Превью сэмпла";
    strings["Volume##sgse3"].plurals[0] = "Громкость";
    strings["MIDI##sgse"].plurals[0] = "=MIDI";
    strings["MIDI input##sgse0"].plurals[0] = "=MIDI input";
    strings["MIDI input##sgse1"].plurals[0] = "=MIDI input";
    strings["<disabled>##sgse0"].plurals[0] = "=<disabled>";
    strings["<disabled>##sgse1"].plurals[0] = "=<disabled>";
    strings["Re-scan MIDI devices##sgse"].plurals[0] = "=Re-scan MIDI devices";
    strings["Note input##sgse0"].plurals[0] = "=Note input";
    strings["Velocity input##sgse"].plurals[0] = "=Velocity input";
    strings["Map MIDI channels to direct channels##sgse"].plurals[0] = "=Map MIDI channels to direct channels";
    strings["Program change pass-through##sgse"].plurals[0] = "=Program change pass-through";
    strings["Map Yamaha FM voice data to instruments##sgse"].plurals[0] = "=Map Yamaha FM voice data to instruments";
    strings["Program change is instrument selection##sgse"].plurals[0] = "=Program change is instrument selection";
    strings["Listen to MIDI clock##sgse"].plurals[0] = "=Listen to MIDI clock";
    strings["Listen to MIDI time code##sgse"].plurals[0] = "=Listen to MIDI time code";
    strings["Value input style##sgse0"].plurals[0] = "=Value input style";
    strings["Value input style##sgse1"].plurals[0] = "=Value input style";
    strings["Control##valueCCS"].plurals[0] = "=Control##valueCCS";
    strings["CC of upper nibble##valueCC1"].plurals[0] = "=CC of upper nibble##valueCC1";
    strings["MSB CC##valueCC1"].plurals[0] = "=MSB CC##valueCC1";
    strings["CC of lower nibble##valueCC2"].plurals[0] = "=CC of lower nibble##valueCC2";
    strings["LSB CC##valueCC2"].plurals[0] = "=LSB CC##valueCC2";
    strings["Per-column control change##sgse"].plurals[0] = "=Per-column control change";
    strings["Control##valueCCS"].plurals[0] = "=Control##valueCCS";
    strings["CC of upper nibble##valueCC1"].plurals[0] = "=CC of upper nibble##valueCC1";
    strings["MSB CC##valueCC1"].plurals[0] = "=MSB CC##valueCC1";
    strings["CC of lower nibble##valueCC2"].plurals[0] = "=CC of lower nibble##valueCC2";
    strings["LSB CC##valueCC2"].plurals[0] = "=LSB CC##valueCC2";
    strings["Volume curve##sgse0"].plurals[0] = "=Volume curve";
    strings["Volume curve##sgse1"].plurals[0] = "=Volume curve";
    strings["Actions:##sgse"].plurals[0] = "=Actions:";
    strings["(learning! press a button or move a slider/knob/something on your device.)##sgse"].plurals[0] = "=(learning! press a button or move a slider/knob/something on your device.)";
    strings["Type##sgse0"].plurals[0] = "=Type";
    strings["Channel##sgse0"].plurals[0] = "=Channel";
    strings["Note/Control##sgse"].plurals[0] = "=Note/Control";
    strings["Velocity/Value##sgse"].plurals[0] = "=Velocity/Value";
    strings["Action##sgse"].plurals[0] = "=Action";
    strings["Any##sgse0"].plurals[0] = "=Any";
    strings["Any##sgse1"].plurals[0] = "=Any";
    strings["Any##sgse2"].plurals[0] = "=Any";
    strings["Any##sgse3"].plurals[0] = "=Any";
    strings["--none--##sgse"].plurals[0] = "=--none--";
    strings["waiting...##BLearn"].plurals[0] = "=waiting...##BLearn";
    strings["Learn##BLearn"].plurals[0] = "=Learn##BLearn";
    strings["MIDI output##sgse0"].plurals[0] = "=MIDI output";
    strings["MIDI output##sgse1"].plurals[0] = "=MIDI output";
    strings["<disabled>##sgse2"].plurals[0] = "=<disabled>";
    strings["<disabled>##sgse3"].plurals[0] = "=<disabled>";
    strings["Output mode:##sgse"].plurals[0] = "=Output mode:";
    strings["Off (use for TX81Z)##sgse"].plurals[0] = "=Off (use for TX81Z)";
    strings["Melodic##sgse"].plurals[0] = "=Melodic";
    strings["Light Show (use for Launchpad)##sgse"].plurals[0] = "=Light Show (use for Launchpad)";
    strings["Send Program Change##sgse"].plurals[0] = "=Send Program Change";
    strings["Send MIDI clock##sgse"].plurals[0] = "=Send MIDI clock";
    strings["Send MIDI timecode##sgse"].plurals[0] = "=Send MIDI timecode";
    strings["Timecode frame rate:##sgse"].plurals[0] = "=Timecode frame rate:";
    strings["Closest to Tick Rate##sgse"].plurals[0] = "=Closest to Tick Rate";
    strings["Film (24fps)##sgse"].plurals[0] = "=Film (24fps)";
    strings["PAL (25fps)##sgse"].plurals[0] = "=PAL (25fps)";
    strings["NTSC drop (29.97fps)##sgse"].plurals[0] = "=NTSC drop (29.97fps)";
    strings["NTSC non-drop (30fps)##sgse"].plurals[0] = "=NTSC non-drop (30fps)";
    strings["Emulation##sgse"].plurals[0] = "=Emulation";
    strings["Cores##sgse"].plurals[0] = "=Cores";
    strings["System##sgse"].plurals[0] = "=System";
    strings["Playback Core(s)##sgse"].plurals[0] = "=Playback Core(s)";
    strings["used for playback##sgse"].plurals[0] = "=used for playback";
    strings["Render Core(s)##sgse"].plurals[0] = "=Render Core(s)";
    strings["used in audio export##sgse"].plurals[0] = "=used in audio export";
    strings["PC Speaker strategy##sgse"].plurals[0] = "=PC Speaker strategy";
    strings["Sample ROMs:##sgse"].plurals[0] = "=Sample ROMs:";
    strings["OPL4 YRW801 path##sgse"].plurals[0] = "=OPL4 YRW801 path";
    strings["MultiPCM TG100 path##sgse"].plurals[0] = "=MultiPCM TG100 path";
    strings["MultiPCM MU5 path##sgse"].plurals[0] = "=MultiPCM MU5 path";
    strings["Keyboard##sgse0"].plurals[0] = "=Keyboard";
    strings["Keyboard##sgse1"].plurals[0] = "=Keyboard";
    strings["Import##sgse0"].plurals[0] = "=Import";
    strings["Export##sgse0"].plurals[0] = "=Export";
    strings["Reset defaults##sgse0"].plurals[0] = "=Reset defaults";
    strings["Are you sure you want to reset the keyboard settings?##sgse"].plurals[0] = "=Are you sure you want to reset the keyboard settings?";
    strings["Global hotkeys##sgse"].plurals[0] = "=Global hotkeys";
    strings["Window activation##sgse"].plurals[0] = "=Window activation";
    strings["Note input##sgse1"].plurals[0] = "=Note input";
    strings["Key##sgse"].plurals[0] = "=Key";
    strings["Type##sgse1"].plurals[0] = "=Type";
    strings["Value##sgse"].plurals[0] = "=Value";
    strings["Remove##sgse"].plurals[0] = "=Remove";
    strings["Macro release##SNType_%d"].plurals[0] = "=Macro release##SNType_%d";
    strings["Note release##SNType_%d"].plurals[0] = "=Note release##SNType_%d";
    strings["Note off##SNType_%d"].plurals[0] = "=Note off##SNType_%d";
    strings["Note##SNType_%d"].plurals[0] = "=Note##SNType_%d";
    strings["Add...##sgse"].plurals[0] = "=Add...";
    strings["Pattern##sgse0"].plurals[0] = "=Pattern";
    strings["keysPattern##sgse"].plurals[0] = "=keysPattern";
    strings["Instrument list##sgse"].plurals[0] = "=Instrument list";
    strings["Wavetable list##sgse"].plurals[0] = "=Wavetable list";
    strings["Sample list##sgse"].plurals[0] = "=Sample list";
    strings["Orders##sgse0"].plurals[0] = "=Orders";
    strings["Sample editor##sgse"].plurals[0] = "=Sample editor";
    strings["Interface##sgse0"].plurals[0] = "=Interface";
    strings["Layout##sgse"].plurals[0] = "=Layout";
    strings["Workspace layout:##sgse"].plurals[0] = "=Workspace layout:";
    strings["Import##sgse1"].plurals[0] = "=Import";
    strings["Export##sgse1"].plurals[0] = "=Export";
    strings["Reset##sgse"].plurals[0] = "=Reset";
    strings["Are you sure you want to reset the workspace layout?##sgse"].plurals[0] = "=Are you sure you want to reset the workspace layout?";
    strings["Allow docking editors##sgse"].plurals[0] = "=Allow docking editors";
    strings["Remember window position##sgse"].plurals[0] = "=Remember window position";
    strings["remembers the window's last position on start-up.##sgse"].plurals[0] = "=remembers the window's last position on start-up.";
    strings["Only allow window movement when clicking on title bar##sgse"].plurals[0] = "=Only allow window movement when clicking on title bar";
    strings["Center pop-up windows##sgse"].plurals[0] = "=Center pop-up windows";
    strings["Play/edit controls layout:##sgse"].plurals[0] = "=Play/edit controls layout:";
    strings["Classic##ecl0"].plurals[0] = "=Classic##ecl0";
    strings["Compact##ecl1"].plurals[0] = "=Compact##ecl1";
    strings["Compact (vertical)##ecl2"].plurals[0] = "=Compact (vertical)##ecl2";
    strings["Split##ecl3"].plurals[0] = "=Split##ecl3";
    strings["Position of buttons in Orders:##sgse"].plurals[0] = "=Position of buttons in Orders:";
    strings["Top##obp0"].plurals[0] = "=Top##obp0";
    strings["Left##obp1"].plurals[0] = "=Left##obp1";
    strings["Right##obp2"].plurals[0] = "=Right##obp2";
    strings["Mouse##sgse"].plurals[0] = "=Mouse";
    strings["Double-click time (seconds)##sgse"].plurals[0] = "=Double-click time (seconds)";
    strings["Don't raise pattern editor on click##sgse"].plurals[0] = "=Don't raise pattern editor on click";
    strings["Focus pattern editor when selecting instrument##sgse"].plurals[0] = "=Focus pattern editor when selecting instrument";
    strings["Note preview behavior:##sgse"].plurals[0] = "=Note preview behavior:";
    strings["Never##npb0"].plurals[0] = "=Never##npb0";
    strings["When cursor is in Note column##npb1"].plurals[0] = "=When cursor is in Note column##npb1";
    strings["When cursor is in Note column or not in edit mode##npb2"].plurals[0] = "=When cursor is in Note column or not in edit mode##npb2";
    strings["Always##npb3"].plurals[0] = "=Always##npb3";
    strings["Allow dragging selection:##sgse"].plurals[0] = "=Allow dragging selection:";
    strings["No##dms0"].plurals[0] = "=No##dms0";
    strings["Yes##dms1"].plurals[0] = "=Yes##dms1";
    strings["Yes (while holding Ctrl only)##dms2"].plurals[0] = "=Yes (while holding Ctrl only)##dms2";
    strings["Toggle channel solo on:##sgse"].plurals[0] = "=Toggle channel solo on:";
    strings["Right-click or double-click##soloA"].plurals[0] = "=Right-click or double-click##soloA";
    strings["Right-click##soloR"].plurals[0] = "=Right-click##soloR";
    strings["Double-click##soloD"].plurals[0] = "=Double-click##soloD";
    strings["Double click selects entire column##sgse"].plurals[0] = "=Double click selects entire column";
    strings["Cursor behavior##sgse"].plurals[0] = "=Cursor behavior";
    strings["Insert pushes entire channel row##sgse"].plurals[0] = "=Insert pushes entire channel row";
    strings["Pull delete affects entire channel row##sgse"].plurals[0] = "=Pull delete affects entire channel row";
    strings["Push value when overwriting instead of clearing it##sgse"].plurals[0] = "=Push value when overwriting instead of clearing it";
    strings["Effect input behavior:##sgse"].plurals[0] = "=Effect input behavior:";
    strings["Move down##eicb0"].plurals[0] = "=Move down##eicb0";
    strings["Move to effect value (otherwise move down)##eicb1"].plurals[0] = "=Move to effect value (otherwise move down)##eicb1";
    strings["Move to effect value/next effect and wrap around##eicb2"].plurals[0] = "=Move to effect value/next effect and wrap around##eicb2";
    strings["Delete effect value when deleting effect##sgse"].plurals[0] = "=Delete effect value when deleting effect";
    strings["Change current instrument when changing instrument column (absorb)##sgse"].plurals[0] = "=Change current instrument when changing instrument column (absorb)";
    strings["Remove instrument value when inserting note off/release##sgse"].plurals[0] = "=Remove instrument value when inserting note off/release";
    strings["Remove volume value when inserting note off/release##sgse"].plurals[0] = "=Remove volume value when inserting note off/release";
    strings["Cursor movement##sgse"].plurals[0] = "=Cursor movement";
    strings["Wrap horizontally:##sgse"].plurals[0] = "=Wrap horizontally:";
    strings["No##wrapH0"].plurals[0] = "=No##wrapH0";
    strings["Yes##wrapH1"].plurals[0] = "=Yes##wrapH1";
    strings["Yes, and move to next/prev row##wrapH2"].plurals[0] = "=Yes, and move to next/prev row##wrapH2";
    strings["Wrap vertically:##sgse"].plurals[0] = "=Wrap vertically:";
    strings["No##wrapV0"].plurals[0] = "=No##wrapV0";
    strings["Yes##wrapV1"].plurals[0] = "=Yes##wrapV1";
    strings["Yes, and move to next/prev pattern##wrapV2"].plurals[0] = "=Yes, and move to next/prev pattern##wrapV2";
    strings["Yes, and move to next/prev pattern (wrap around)##wrapV2"].plurals[0] = "=Yes, and move to next/prev pattern (wrap around)##wrapV2";
    strings["Cursor movement keys behavior:##sgse"].plurals[0] = "=Cursor movement keys behavior:";
    strings["Move by one##cmk0"].plurals[0] = "=Move by one##cmk0";
    strings["Move by Edit Step##cmk1"].plurals[0] = "=Move by Edit Step##cmk1";
    strings["Move cursor by edit step on delete##sgse"].plurals[0] = "=Move cursor by edit step on delete";
    strings["Move cursor by edit step on insert (push)##sgse"].plurals[0] = "=Move cursor by edit step on insert (push)";
    strings["Move cursor up on backspace-delete##sgse"].plurals[0] = "=Move cursor up on backspace-delete";
    strings["Move cursor to end of clipboard content when pasting##sgse"].plurals[0] = "=Move cursor to end of clipboard content when pasting";
    strings["Scrolling##sgse"].plurals[0] = "=Scrolling";
    strings["Change order when scrolling outside of pattern bounds:##sgse"].plurals[0] = "=Change order when scrolling outside of pattern bounds:";
    strings["No##pscroll0"].plurals[0] = "=No##pscroll0";
    strings["Yes##pscroll1"].plurals[0] = "=Yes##pscroll1";
    strings["Yes, and wrap around song##pscroll2"].plurals[0] = "=Yes, and wrap around song##pscroll2";
    strings["Cursor follows current order when moving it##sgse"].plurals[0] = "=Cursor follows current order when moving it";
    strings["applies when playback is stopped.##sgse"].plurals[0] = "=applies when playback is stopped.";
    strings["Don't scroll when moving cursor##sgse"].plurals[0] = "=Don't scroll when moving cursor";
    strings["Move cursor with scroll wheel:##sgse"].plurals[0] = "=Move cursor with scroll wheel:";
    strings["No##csw0"].plurals[0] = "=No##csw0";
    strings["Yes##csw1"].plurals[0] = "=Yes##csw1";
    strings["Inverted##csw2"].plurals[0] = "=Inverted##csw2";
    strings["Assets##sgse0"].plurals[0] = "=Assets";
    strings["Display instrument type menu when adding instrument##sgse"].plurals[0] = "=Display instrument type menu when adding instrument";
    strings["Select asset after opening one##sgse"].plurals[0] = "=Select asset after opening one";
    strings["Appearance##sgse"].plurals[0] = "Внешний вид";
    strings["Scaling##sgse"].plurals[0] = "Масштаб";
    strings["Automatic UI scaling factor##sgse"].plurals[0] = "Автоматическое масштабирование интерфейса";
    strings["UI scaling factor##sgse"].plurals[0] = "Масштаб интерфейса";
    strings["Icon size##sgse"].plurals[0] = "Размер иконок";
    strings["Text##sgse"].plurals[0] = "Текст";
    strings["Font renderer##sgse"].plurals[0] = "Отрисовщик шрифта";
    strings["Main font##sgse"].plurals[0] = "Основной шрифт";
    strings["Size##MainFontSize"].plurals[0] = "Размер##MainFontSize";
    strings["Header font##sgse"].plurals[0] = "Шрифт заголовков";
    strings["Size##HeadFontSize"].plurals[0] = "Размер##HeadFontSize";
    strings["Pattern font##sgse"].plurals[0] = "Шрифт паттернов";
    strings["Size##PatFontSize"].plurals[0] = "Размер##PatFontSize";
    strings["Anti-aliased fonts##sgse"].plurals[0] = "Сглаживание шрифтов";
    strings["Support bitmap fonts##sgse"].plurals[0] = "Поддерживать растровые шрифты";
    strings["Hinting:##sgse"].plurals[0] = "Хинтование";
    strings["Off (soft)##fh0"].plurals[0] = "Нет (слабое)##fh0";
    strings["Slight##fh1"].plurals[0] = "Небольшое##fh1";
    strings["Normal##fh2"].plurals[0] = "Нормальное##fh2";
    strings["Full (hard)##fh3"].plurals[0] = "Полное (жёсткое)##fh3";
    strings["Auto-hinter:##sgse"].plurals[0] = "Автоматическое хинтирование";
    strings["Disable##fah0"].plurals[0] = "Отключить##fah0";
    strings["Enable##fah1"].plurals[0] = "Включить##fah1";
    strings["Force##fah2"].plurals[0] = "Принудительное##fah2";
    strings["Display Japanese characters##sgse"].plurals[0] = "Отображать японские символы (вкл. иероглифы)";
    strings["Only toggle this option if you have enough graphics memory.\n"
            "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
            "このオプションは、十分なグラフィックメモリがある場合にのみ切り替えてください。\n"
            "これは、Dear ImGuiにダイナミックフォントアトラスが実装されるまでの一時的な解決策です。##sgse"].plurals[0] = 
            
            "Включайте эту настройку только в случае наличия достаточного количества графической памяти.\n"
            "Это временное решение, поскольку пока Dear ImGui не поддерживает динамический атлас шрифтов.\n\n"
            "このオプションは、十分なグラフィックメモリがある場合にのみ切り替えてください。\n"
            "これは、Dear ImGuiにダイナミックフォントアトラスが実装されるまでの一時的な解決策です。";
    strings["Display Chinese (Simplified) characters##sgse"].plurals[0] = "Отображать китайские иероглифы (упрощённые)";
    strings["Only toggle this option if you have enough graphics memory.\n"
            "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
            "请在确保你有足够的显存后再启动此设定\n"
            "这是一个在ImGui实现动态字体加载之前的临时解决方案##sgse"].plurals[0] = 
            
            "Включайте эту настройку только в случае наличия достаточного количества графической памяти.\n"
            "Это временное решение, поскольку пока Dear ImGui не поддерживает динамический атлас шрифтов.\n\n"
            "请在确保你有足够的显存后再启动此设定\n"
            "这是一个在ImGui实现动态字体加载之前的临时解决方案";
    strings["Display Chinese (Traditional) characters##sgse"].plurals[0] = "Отображать китайские иероглифы (традиционные)";
    strings["Only toggle this option if you have enough graphics memory.\n"
            "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
            "請在確保你有足夠的顯存后再啟動此設定\n"
            "這是一個在ImGui實現動態字體加載之前的臨時解決方案##sgse"].plurals[0] = 
            
            "Включайте эту настройку только в случае наличия достаточного количества графической памяти.\n"
            "Это временное решение, поскольку пока Dear ImGui не поддерживает динамический атлас шрифтов.\n\n"
            "請在確保你有足夠的顯存后再啟動此設定\n"
            "這是一個在ImGui實現動態字體加載之前的臨時解決方案";
    strings["Display Korean characters##sgse"].plurals[0] = "Отображать корейские иероглифы";
    strings["Only toggle this option if you have enough graphics memory.\n"
            "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
            "그래픽 메모리가 충분한 경우에만 이 옵션을 선택하십시오.\n"
            "이 옵션은 Dear ImGui에 동적 글꼴 아틀라스가 구현될 때까지 임시 솔루션입니다.##sgse"].plurals[0] = 
            
            "Включайте эту настройку только в случае наличия достаточного количества графической памяти.\n"
            "Это временное решение, поскольку пока Dear ImGui не поддерживает динамический атлас шрифтов.\n\n"
            "그래픽 메모리가 충분한 경우에만 이 옵션을 선택하십시오.\n"
            "이 옵션은 Dear ImGui에 동적 글꼴 아틀라스가 구현될 때까지 임시 솔루션입니다.";

    strings["Program##sgse2"].plurals[0] = "Программа";
    strings["Title bar:##sgse"].plurals[0] = "Полоса заголовка окна";
    strings["Furnace##tbar0"].plurals[0] = "Furnace##tbar0";
    strings["Song Name - Furnace##tbar1"].plurals[0] = "Название композиции - Furnace##tbar1";
    strings["file_name.fur - Furnace##tbar2"].plurals[0] = "название_файла.fur - Furnace##tbar2";
    strings["/path/to/file.fur - Furnace##tbar3"].plurals[0] = "/путь/к/файлу.fur - Furnace##tbar3";
    strings["Display system name on title bar##sgse"].plurals[0] = "Отображать название чипа/системы в полосе заголовка окна";
    strings["Display chip names instead of \"multi-system\" in title bar##sgse"].plurals[0] = "Отображать названия чипов/систем вместо \"мульти-система\" в полосе заголовка окна";
    strings["Status bar:##sgse"].plurals[0] = "Строка состояния:";
    strings["Cursor details##sbar0"].plurals[0] = "Информация о выделенном элементе##sbar0";
    strings["File path##sbar1"].plurals[0] = "Путь к файлу##sbar1";
    strings["Cursor details or file path##sbar2"].plurals[0] = "Информация о выделенном элементе или путь к файлу##sbar2";
    strings["Nothing##sbar3"].plurals[0] = "Ничего##sbar3";
    strings["Export options layout:##sgse"].plurals[0] = "Вид настроек экспорта:";
    strings["Sub-menus in File menu##eol0"].plurals[0] = "Подпункты в меню \"Файл\"##eol0";
    strings["Modal window with tabs##eol1"].plurals[0] = "Модальное окно с вкладками##eol1";
    strings["Modal windows with options in File menu##eol2"].plurals[0] = "Модальное окно с настройками в меню \"Файл\"";
    strings["Capitalize menu bar##sgse"].plurals[0] = "Названия пунктов в горизонтальном меню с большой буквы";
    strings["Display add/configure/change/remove chip menus in File menu##sgse"].plurals[0] = "Отображать в меню \"Файл\" пункты: добавить/настроить/изменить/убрать чип";
    strings["Orders##sgse1"].plurals[0] = "Матрица паттернов";
    strings["Highlight channel at cursor in Orders##sgse"].plurals[0] = "Выделить в матрице паттернов канал, на котором находится курсор";
    strings["Orders row number format:##sgse"].plurals[0] = "Формат отображения номера строки матрицы паттернов:";
    strings["Decimal##orbD"].plurals[0] = "Десятеричный##orbD";
    strings["Hexadecimal##orbH"].plurals[0] = "Шестнадцатеричный##orbH";
    strings["Pattern##sgse1"].plurals[0] = "Паттерн";
    strings["Center pattern view##sgse"].plurals[0] = "Центрировать отображаемые паттерны внутри окна";
    strings["Overflow pattern highlights##sgse"].plurals[0] = "Продолжать полосы подсветки строк паттернов за пределы самих паттернов";
    strings["Display previous/next pattern##sgse"].plurals[0] = "Отображать предыдущий/следующий паттерн";
    strings["Pattern row number format:##sgse"].plurals[0] = "Формат отображения номера строки паттерна:";
    strings["Decimal##prbD"].plurals[0] = "Десятеричный##prbD";
    strings["Hexadecimal##prbH"].plurals[0] = "Шестнадцатеричный##prbH";
    strings["Pattern view labels:##sgse"].plurals[0] = "Маркировка ячеек в паттерне";
    strings["Note off (3-char)##sgse"].plurals[0] = "\"Отпускание клавиши\" (резкое) (3 символа)";
    strings["Note release (3-char)##sgse"].plurals[0] = "\"Отпускание клавиши\" (с включением фазы затухания огибающей) (3 символа)";
    strings["Macro release (3-char)##sgse"].plurals[0] = "\"Отпускание клавиши\" для макросов (3 символа)";
    strings["Empty field (3-char)##sgse"].plurals[0] = "Пустая ячейка (3 символа)";
    strings["Empty field (2-char)##sgse"].plurals[0] = "Пустая ячейка (2 символа)";
    strings["Pattern view spacing after:##sgse"].plurals[0] = "Разбивка в отображении паттерна:";
    strings["Note##sgse"].plurals[0] = "Нота";
    strings["Instrument##sgse1"].plurals[0] = "Инструмент";
    strings["Volume##sgse4"].plurals[0] = "Громкость";
    strings["Effect##sgse"].plurals[0] = "Индекс эффекта";
    strings["Effect value##sgse"].plurals[0] = "Параметр эффекта";
    strings["Single-digit effects for 00-0F##sgse"].plurals[0] = "Отображать одной цифрой индекс эффекта для индексов 00-0F";
    strings["Use flats instead of sharps##sgse"].plurals[0] = "Отображать бемоли вместо диезов";
    strings["Use German notation##sgse"].plurals[0] = "Использовать немецкие имена нот";
    strings["Channel##sgse1"].plurals[0] = "Канал";
    strings["Channel style:##sgse"].plurals[0] = "Стиль заголовка:";
    strings["Classic##CHS0"].plurals[0] = "Классический##CHS0";
    strings["Line##CHS1"].plurals[0] = "Линия##CHS1";
    strings["Round##CHS2"].plurals[0] = "Со скруглениями##CHS2";
    strings["Split button##CHS3"].plurals[0] = "С отдельной кнопкой отключения звука##CHS3";
    strings["Square border##CHS4"].plurals[0] = "С прямоугольной границей вокруг названия##CHS4";
    strings["Round border##CHS5"].plurals[0] = "Со скруглённой границей вокруг названия##CHS5";
    strings["Channel volume bar:##sgse"].plurals[0] = "Полоска громкости в заголовке канала:";
    strings["Non##CHV0"].plurals[0] = "Нет##CHV0";
    strings["Simple##CHV1"].plurals[0] = "Простая##CHV1";
    strings["Stereo##CHV2"].plurals[0] = "Стерео##CHV2";
    strings["Real##CHV3"].plurals[0] = "Настоящая громкость##CHV3";
    strings["Real (stereo)##CHV4"].plurals[0] = "Настоящая громкость (стерео)##CHV4";
    strings["Channel feedback style:##sgse"].plurals[0] = "Подсветка заголовка канала:";
    strings["Off##CHF0"].plurals[0] = "Выкл.##CHF0";
    strings["Note##CHF1"].plurals[0] = "Начало ноты##CHF1";
    strings["Volume##CHF2"].plurals[0] = "Пропорционально громкости##CHF2";
    strings["Active##CHF3"].plurals[0] = "При активности канала##CHF3";
    strings["Channel font:##sgse"].plurals[0] = "Шрифт заголовка канала:";
    strings["Regular##CHFont0"].plurals[0] = "Обычный##CHFont0";
    strings["Monospace##CHFont1"].plurals[0] = "Моноширинный##CHFont1";
    strings["Center channel name##sgse"].plurals[0] = "Центрировать название канала";
    strings["Channel colors:##sgse"].plurals[0] = "Цвета заголовка канала";
    strings["Single##CHC0"].plurals[0] = "Единый цвет##CHC0";
    strings["Channel type##CHC1"].plurals[0] = "Согласно типу канала##CHC1";
    strings["Instrument type##CHC2"].plurals[0] = "Согласно типу инструмента##CHC2";
    strings["Channel name colors:##sgse"].plurals[0] = "Цвета названия канала:";
    strings["Single##CTC0"].plurals[0] = "Единый цвет##CTC0";
    strings["Channel type##CTC1"].plurals[0] = "Согласно типу канала##CTC1";
    strings["Instrument type##CTC2"].plurals[0] = "Согласно типу инструмента##CTC2";
    strings["Assets##sgse1"].plurals[0] = "Представление ресурсов модуля";
    strings["Unified instrument/wavetable/sample list##sgse"].plurals[0] = "Единый список инструментов, волновых таблиц и сэмплов";
    strings["Horizontal instrument list##sgse"].plurals[0] = "Горизонтальный список инструментов";
    strings["Instrument list icon style:##sgse"].plurals[0] = "Стиль иконок в списке инструментов:";
    strings["None##iis0"].plurals[0] = "Не показывать##iis0";
    strings["Graphical icons##iis1"].plurals[0] = "Графические иконки##iis1";
    strings["Letter icons##iis2"].plurals[0] = "Иконки с буквами##iis2";
    strings["Colorize instrument editor using instrument type##sgse"].plurals[0] = "Изменять оттенки цветов редактора инструмента согласно типу инструмента";
    strings["Macro Editor##sgse0"].plurals[0] = "Редактор макросов";
    strings["Macro editor layout:##sgse"].plurals[0] = "Компоновка редактора макросов:";
    strings["Unified##mel0"].plurals[0] = "Общий список##mel0";
    strings["Grid##mel2"].plurals[0] = "Прямоугольная сетка##mel2";
    strings["Single (with list)##mel3"].plurals[0] = "Окно редактирования одного макроса + список##mel3";
    strings["Use classic macro editor vertical slider##sgse"].plurals[0] = "Использовать классическую вертикальную полосу прокрутки";
    strings["Wave Editor##sgse"].plurals[0] = "Редактор волновых таблиц";
    strings["Use compact wave editor##sgse"].plurals[0] = "Использовать компактный редактор волновых таблиц";
    strings["FM Editor##sgse0"].plurals[0] = "Редактор FM-инструментов";
    strings["FM parameter names:##sgse"].plurals[0] = "Названия параметров:";
    strings["Friendly##fmn0"].plurals[0] = "Понятные##fmn0";
    strings["Technical##fmn1"].plurals[0] = "Исходные формальные##fmn1";
    strings["Technical (alternate)##fmn2"].plurals[0] = "Исходные формальные (альтернативные)##fmn2";
    strings["Use standard OPL waveform names##sgse"].plurals[0] = "Использовать стандартные названия волн для чипов серии OPL";
    strings["FM parameter editor layout:##sgse"].plurals[0] = "Компоновка редактора FM-инструментов";
    strings["Modern##fml0"].plurals[0] = "Современная##fml0";
    strings["Compact (2x2, classic)##fml1"].plurals[0] = "Компактная (2x2, классическая)##fml1";
    strings["Compact (1x4)##fml2"].plurals[0] = "Компактная (1x4)##fml2";
    strings["Compact (4x1)##fml3"].plurals[0] = "Компактная (4x1)##fml3";
    strings["Alternate (2x2)##fml4"].plurals[0] = "Альтернативная (2x2)##fml4";
    strings["Alternate (1x4)##fml5"].plurals[0] = "Альтернативная (1x4)##fml5";
    strings["Alternate (4x1)##fml6"].plurals[0] = "Альтернативная (4x1)##fml6";
    strings["Position of Sustain in FM editor:##sgse"].plurals[0] = "Позиция параметра \"Поддержка\" в редакторе:";
    strings["Between Decay and Sustain Rate##susp0"].plurals[0] = "Между спадом и уклоном поддержки##susp0";
    strings["After Release Rate##susp1"].plurals[0] = "После уклона затухания##susp1";
    strings["Use separate colors for carriers/modulators in FM editor##sgse"].plurals[0] = "Использовать различные цвета для модулирующих и несущих операторов";
    strings["Unsigned FM detune values##sgse"].plurals[0] = "Беззнаковое значение параметра расстройки";
    strings["Statistics##sgse"].plurals[0] = "Окно статистики";
    strings["Chip memory usage unit:##sgse"].plurals[0] = "Единицы измерения использования памяти чипа:";
    strings["Bytes##MUU0"].plurals[0] = "Байты##MUU0";
    strings["Kilobytes##MUU1"].plurals[0] = "Килобайты##MUU1";
    strings["Oscilloscope##set"].plurals[0] = "Осциллограф##set";
    strings["Rounded corners##sgse"].plurals[0] = "Закруглённые края";
    strings["Border##sgse"].plurals[0] = "Граница";
    strings["Mono##sgse1"].plurals[0] = "Моно";
    strings["Anti-aliased##sgse"].plurals[0] = "Со сглаживанием";
    strings["Fill entire window##sgse"].plurals[0] = "Полностью заполняет окно";
    strings["Waveform goes out of bounds##sgse"].plurals[0] = "Волна может выходить за пределы окна";
    strings["Windows##sgse"].plurals[0] = "Окна";
    strings["Rounded window corners##sgse"].plurals[0] = "Закруглённые края окон";
    strings["Rounded buttons##sgse"].plurals[0] = "Закруглённые края кнопок";
    strings["Rounded tabs##sgse"].plurals[0] = "Закруглённые края заголовков вкладок";
    strings["Rounded scrollbars##sgse"].plurals[0] = "Закруглённые края ползунка полосы прокрутки";
    strings["Rounded menu corners##sgse"].plurals[0] = "Закруглённые края выпадающих меню";
    strings["Borders around widgets##sgse"].plurals[0] = "Границы вокруг кнопок, списков и т.д.";
    strings["Misc##sgse"].plurals[0] = "Разное";
    strings["Wrap text##sgse"].plurals[0] = "Переносить текст на новую строку";
    strings["Wrap text in song/subsong comments window.##sgse"].plurals[0] = "Переносить текст на новую строку в окне информации/комментариев к композиции.";
    strings["Frame shading in text windows##sgse"].plurals[0] = "Градиент в текстовых окнах";
    strings["Apply frame shading to the multiline text fields\nsuch as song/subsong info/comments.##sgse"].plurals[0] = "Применять градиент в окне информации/комментариев к композиции.";
    strings["Color##sgse"].plurals[0] = "=Color";
    strings["Color scheme##sgse"].plurals[0] = "=Color scheme";
    strings["Import##sgse2"].plurals[0] = "=Import";
    strings["Export##sgse2"].plurals[0] = "=Export";
    strings["Reset defaults##sgse1"].plurals[0] = "=Reset defaults";
    strings["Are you sure you want to reset the color scheme?##sgse"].plurals[0] = "=Are you sure you want to reset the color scheme?";
    strings["Interface##sgse1"].plurals[0] = "=Interface";
    strings["Frame shading##sgse"].plurals[0] = "=Frame shading";
    strings["Interface (other)##sgse"].plurals[0] = "=Interface (other)";
    strings["Miscellaneous##sgse"].plurals[0] = "=Miscellaneous";
    strings["File Picker (built-in)##sgse"].plurals[0] = "=File Picker (built-in)";
    strings["Oscilloscope##sgse"].plurals[0] = "=Oscilloscope";
    strings["Wave (non-mono)##sgse"].plurals[0] = "=Wave (non-mono)";
    strings["Volume Meter##sgse"].plurals[0] = "=Volume Meter";
    strings["Orders##sgse2"].plurals[0] = "=Orders";
    strings["Envelope View##sgse"].plurals[0] = "=Envelope View";
    strings["FM Editor##sgse1"].plurals[0] = "=FM Editor";
    strings["Macro Editor##sgse1"].plurals[0] = "=Macro Editor";
    strings["Instrument Types##sgse"].plurals[0] = "=Instrument Types";
    strings["Channel##sgse2"].plurals[0] = "=Channel";
    strings["Pattern##sgse2"].plurals[0] = "=Pattern";
    strings["Sample Editor##sgse"].plurals[0] = "=Sample Editor";
    strings["Pattern Manager##sgse"].plurals[0] = "=Pattern Manager";
    strings["Piano##sgse"].plurals[0] = "=Piano";
    strings["Clock##sgse"].plurals[0] = "=Clock";
    strings["Patchbay##sgse"].plurals[0] = "=Patchbay";
    strings["Log Viewer##sgse"].plurals[0] = "=Log Viewer";

    // these are messy, but the ##CC_GUI... is required.
    strings["Button##CC_GUI_COLOR_BUTTON"].plurals[0] = "=Button##CC_GUI_COLOR_BUTTON";
    strings["Button (hovered)##CC_GUI_COLOR_BUTTON_HOVER"].plurals[0] = "=Button (hovered)##CC_GUI_COLOR_BUTTON_HOVER";
    strings["Button (active)##CC_GUI_COLOR_BUTTON_ACTIVE"].plurals[0] = "=Button (active)##CC_GUI_COLOR_BUTTON_ACTIVE";
    strings["Tab##CC_GUI_COLOR_TAB"].plurals[0] = "=Tab##CC_GUI_COLOR_TAB";
    strings["Tab (hovered)##CC_GUI_COLOR_TAB_HOVER"].plurals[0] = "=Tab (hovered)##CC_GUI_COLOR_TAB_HOVER";
    strings["Tab (active)##CC_GUI_COLOR_TAB_ACTIVE"].plurals[0] = "=Tab (active)##CC_GUI_COLOR_TAB_ACTIVE";
    strings["Tab (unfocused)##CC_GUI_COLOR_TAB_UNFOCUSED"].plurals[0] = "=Tab (unfocused)##CC_GUI_COLOR_TAB_UNFOCUSED";
    strings["Tab (unfocused and active)##CC_GUI_COLOR_TAB_UNFOCUSED_ACTIVE"].plurals[0] = "=Tab (unfocused and active)##CC_GUI_COLOR_TAB_UNFOCUSED_ACTIVE";
    strings["ImGui header##CC_GUI_COLOR_IMGUI_HEADER"].plurals[0] = "=ImGui header##CC_GUI_COLOR_IMGUI_HEADER";
    strings["ImGui header (hovered)##CC_GUI_COLOR_IMGUI_HEADER_HOVER"].plurals[0] = "=ImGui header (hovered)##CC_GUI_COLOR_IMGUI_HEADER_HOVER";
    strings["ImGui header (active)##CC_GUI_COLOR_IMGUI_HEADER_ACTIVE"].plurals[0] = "=ImGui header (active)##CC_GUI_COLOR_IMGUI_HEADER_ACTIVE";
    strings["Resize grip##CC_GUI_COLOR_RESIZE_GRIP"].plurals[0] = "=Resize grip##CC_GUI_COLOR_RESIZE_GRIP";
    strings["Resize grip (hovered)##CC_GUI_COLOR_RESIZE_GRIP_HOVER"].plurals[0] = "=Resize grip (hovered)##CC_GUI_COLOR_RESIZE_GRIP_HOVER";
    strings["Resize grip (active)##CC_GUI_COLOR_RESIZE_GRIP_ACTIVE"].plurals[0] = "=Resize grip (active)##CC_GUI_COLOR_RESIZE_GRIP_ACTIVE";
    strings["Widget background##CC_GUI_COLOR_WIDGET_BACKGROUND"].plurals[0] = "=Widget background##CC_GUI_COLOR_WIDGET_BACKGROUND";
    strings["Widget background (hovered)##CC_GUI_COLOR_WIDGET_BACKGROUND_HOVER"].plurals[0] = "=Widget background (hovered)##CC_GUI_COLOR_WIDGET_BACKGROUND_HOVER";
    strings["Widget background (active)##CC_GUI_COLOR_WIDGET_BACKGROUND_ACTIVE"].plurals[0] = "=Widget background (active)##CC_GUI_COLOR_WIDGET_BACKGROUND_ACTIVE";
    strings["Slider grab##CC_GUI_COLOR_SLIDER_GRAB"].plurals[0] = "=Slider grab##CC_GUI_COLOR_SLIDER_GRAB";
    strings["Slider grab (active)##CC_GUI_COLOR_SLIDER_GRAB_ACTIVE"].plurals[0] = "=Slider grab (active)##CC_GUI_COLOR_SLIDER_GRAB_ACTIVE";
    strings["Title background (active)##CC_GUI_COLOR_TITLE_BACKGROUND_ACTIVE"].plurals[0] = "=Title background (active)##CC_GUI_COLOR_TITLE_BACKGROUND_ACTIVE";
    strings["Checkbox/radio button mark##CC_GUI_COLOR_CHECK_MARK"].plurals[0] = "=Checkbox/radio button mark##CC_GUI_COLOR_CHECK_MARK";
    strings["Text selection##CC_GUI_COLOR_TEXT_SELECTION"].plurals[0] = "=Text selection##CC_GUI_COLOR_TEXT_SELECTION";
    strings["Line plot##CC_GUI_COLOR_PLOT_LINES"].plurals[0] = "=Line plot##CC_GUI_COLOR_PLOT_LINES";
    strings["Line plot (hovered)##CC_GUI_COLOR_PLOT_LINES_HOVER"].plurals[0] = "=Line plot (hovered)##CC_GUI_COLOR_PLOT_LINES_HOVER";
    strings["Histogram plot##CC_GUI_COLOR_PLOT_HISTOGRAM"].plurals[0] = "=Histogram plot##CC_GUI_COLOR_PLOT_HISTOGRAM";
    strings["Histogram plot (hovered)##CC_GUI_COLOR_PLOT_HISTOGRAM_HOVER"].plurals[0] = "=Histogram plot (hovered)##CC_GUI_COLOR_PLOT_HISTOGRAM_HOVER";
    strings["Table row (even)##CC_GUI_COLOR_TABLE_ROW_EVEN"].plurals[0] = "=Table row (even)##CC_GUI_COLOR_TABLE_ROW_EVEN";
    strings["Table row (odd)##CC_GUI_COLOR_TABLE_ROW_ODD"].plurals[0] = "=Table row (odd)##CC_GUI_COLOR_TABLE_ROW_ODD";

    strings["Background##CC_GUI_COLOR_BACKGROUND"].plurals[0] = "=Background##CC_GUI_COLOR_BACKGROUND";
    strings["Window background##CC_GUI_COLOR_FRAME_BACKGROUND"].plurals[0] = "=Window background##CC_GUI_COLOR_FRAME_BACKGROUND";
    strings["Sub-window background##CC_GUI_COLOR_FRAME_BACKGROUND_CHILD"].plurals[0] = "=Sub-window background##CC_GUI_COLOR_FRAME_BACKGROUND_CHILD";
    strings["Pop-up background##CC_GUI_COLOR_FRAME_BACKGROUND_POPUP"].plurals[0] = "=Pop-up background##CC_GUI_COLOR_FRAME_BACKGROUND_POPUP";
    strings["Modal backdrop##CC_GUI_COLOR_MODAL_BACKDROP"].plurals[0] = "=Modal backdrop##CC_GUI_COLOR_MODAL_BACKDROP";
    strings["Header##CC_GUI_COLOR_HEADER"].plurals[0] = "=Header##CC_GUI_COLOR_HEADER";
    strings["Text##CC_GUI_COLOR_TEXT"].plurals[0] = "=Text##CC_GUI_COLOR_TEXT";
    strings["Text (disabled)##CC_GUI_COLOR_TEXT_DISABLED"].plurals[0] = "=Text (disabled)##CC_GUI_COLOR_TEXT_DISABLED";
    strings["Title bar (inactive)##CC_GUI_COLOR_TITLE_INACTIVE"].plurals[0] = "=Title bar (inactive)##CC_GUI_COLOR_TITLE_INACTIVE";
    strings["Title bar (collapsed)##CC_GUI_COLOR_TITLE_COLLAPSED"].plurals[0] = "=Title bar (collapsed)##CC_GUI_COLOR_TITLE_COLLAPSED";
    strings["Menu bar##CC_GUI_COLOR_MENU_BAR"].plurals[0] = "=Menu bar##CC_GUI_COLOR_MENU_BAR";
    strings["Border##CC_GUI_COLOR_BORDER"].plurals[0] = "=Border##CC_GUI_COLOR_BORDER";
    strings["Border shadow##CC_GUI_COLOR_BORDER_SHADOW"].plurals[0] = "=Border shadow##CC_GUI_COLOR_BORDER_SHADOW";
    strings["Scroll bar##CC_GUI_COLOR_SCROLL"].plurals[0] = "=Scroll bar##CC_GUI_COLOR_SCROLL";
    strings["Scroll bar (hovered)##CC_GUI_COLOR_SCROLL_HOVER"].plurals[0] = "=Scroll bar (hovered)##CC_GUI_COLOR_SCROLL_HOVER";
    strings["Scroll bar (clicked)##CC_GUI_COLOR_SCROLL_ACTIVE"].plurals[0] = "=Scroll bar (clicked)##CC_GUI_COLOR_SCROLL_ACTIVE";
    strings["Scroll bar background##CC_GUI_COLOR_SCROLL_BACKGROUND"].plurals[0] = "=Scroll bar background##CC_GUI_COLOR_SCROLL_BACKGROUND";
    strings["Separator##CC_GUI_COLOR_SEPARATOR"].plurals[0] = "=Separator##CC_GUI_COLOR_SEPARATOR";
    strings["Separator (hover)##CC_GUI_COLOR_SEPARATOR_HOVER"].plurals[0] = "=Separator (hover)##CC_GUI_COLOR_SEPARATOR_HOVER";
    strings["Separator (active)##CC_GUI_COLOR_SEPARATOR_ACTIVE"].plurals[0] = "=Separator (active)##CC_GUI_COLOR_SEPARATOR_ACTIVE";
    strings["Docking preview##CC_GUI_COLOR_DOCKING_PREVIEW"].plurals[0] = "=Docking preview##CC_GUI_COLOR_DOCKING_PREVIEW";
    strings["Docking empty##CC_GUI_COLOR_DOCKING_EMPTY"].plurals[0] = "=Docking empty##CC_GUI_COLOR_DOCKING_EMPTY";
    strings["Table header##CC_GUI_COLOR_TABLE_HEADER"].plurals[0] = "=Table header##CC_GUI_COLOR_TABLE_HEADER";
    strings["Table border (hard)##CC_GUI_COLOR_TABLE_BORDER_HARD"].plurals[0] = "=Table border (hard)##CC_GUI_COLOR_TABLE_BORDER_HARD";
    strings["Table border (soft)##CC_GUI_COLOR_TABLE_BORDER_SOFT"].plurals[0] = "=Table border (soft)##CC_GUI_COLOR_TABLE_BORDER_SOFT";
    strings["Drag and drop target##CC_GUI_COLOR_DRAG_DROP_TARGET"].plurals[0] = "=Drag and drop target##CC_GUI_COLOR_DRAG_DROP_TARGET";
    strings["Window switcher (highlight)##CC_GUI_COLOR_NAV_WIN_HIGHLIGHT"].plurals[0] = "=Window switcher (highlight)##CC_GUI_COLOR_NAV_WIN_HIGHLIGHT";
    strings["Window switcher backdrop##CC_GUI_COLOR_NAV_WIN_BACKDROP"].plurals[0] = "=Window switcher backdrop##CC_GUI_COLOR_NAV_WIN_BACKDROP";

    strings["Toggle on##CC_GUI_COLOR_TOGGLE_ON"].plurals[0] = "=Toggle on##CC_GUI_COLOR_TOGGLE_ON";
    strings["Toggle off##CC_GUI_COLOR_TOGGLE_OFF"].plurals[0] = "=Toggle off##CC_GUI_COLOR_TOGGLE_OFF";
    strings["Playback status##CC_GUI_COLOR_PLAYBACK_STAT"].plurals[0] = "=Playback status##CC_GUI_COLOR_PLAYBACK_STAT";
    strings["Destructive hint##CC_GUI_COLOR_DESTRUCTIVE"].plurals[0] = "=Destructive hint##CC_GUI_COLOR_DESTRUCTIVE";
    strings["Warning hint##CC_GUI_COLOR_WARNING"].plurals[0] = "=Warning hint##CC_GUI_COLOR_WARNING";
    strings["Error hint##CC_GUI_COLOR_ERROR"].plurals[0] = "=Error hint##CC_GUI_COLOR_ERROR";

    strings["Directory##CC_GUI_COLOR_FILE_DIR"].plurals[0] = "=Directory##CC_GUI_COLOR_FILE_DIR";
    strings["Song (native)##CC_GUI_COLOR_FILE_SONG_NATIVE"].plurals[0] = "=Song (native)##CC_GUI_COLOR_FILE_SONG_NATIVE";
    strings["Song (import)##CC_GUI_COLOR_FILE_SONG_IMPORT"].plurals[0] = "=Song (import)##CC_GUI_COLOR_FILE_SONG_IMPORT";
    strings["Instrument##CC_GUI_COLOR_FILE_INSTR"].plurals[0] = "=Instrument##CC_GUI_COLOR_FILE_INSTR";
    strings["Audio##CC_GUI_COLOR_FILE_AUDIO"].plurals[0] = "=Audio##CC_GUI_COLOR_FILE_AUDIO";
    strings["Wavetable##CC_GUI_COLOR_FILE_WAVE"].plurals[0] = "=Wavetable##CC_GUI_COLOR_FILE_WAVE";
    strings["VGM##CC_GUI_COLOR_FILE_VGM"].plurals[0] = "=VGM##CC_GUI_COLOR_FILE_VGM";
    strings["ZSM##CC_GUI_COLOR_FILE_ZSM"].plurals[0] = "=ZSM##CC_GUI_COLOR_FILE_ZSM";
    strings["Font##CC_GUI_COLOR_FILE_FONT"].plurals[0] = "=Font##CC_GUI_COLOR_FILE_FONT";
    strings["Other##CC_GUI_COLOR_FILE_OTHER"].plurals[0] = "=Other##CC_GUI_COLOR_FILE_OTHER";

    strings["Border##CC_GUI_COLOR_OSC_BORDER"].plurals[0] = "=Border##CC_GUI_COLOR_OSC_BORDER";
    strings["Background (top-left)##CC_GUI_COLOR_OSC_BG1"].plurals[0] = "=Background (top-left)##CC_GUI_COLOR_OSC_BG1";
    strings["Background (top-right)##CC_GUI_COLOR_OSC_BG2"].plurals[0] = "=Background (top-right)##CC_GUI_COLOR_OSC_BG2";
    strings["Background (bottom-left)##CC_GUI_COLOR_OSC_BG3"].plurals[0] = "=Background (bottom-left)##CC_GUI_COLOR_OSC_BG3";
    strings["Background (bottom-right)##CC_GUI_COLOR_OSC_BG4"].plurals[0] = "=Background (bottom-right)##CC_GUI_COLOR_OSC_BG4";
    strings["Waveform##CC_GUI_COLOR_OSC_WAVE"].plurals[0] = "=Waveform##CC_GUI_COLOR_OSC_WAVE";
    strings["Waveform (clip)##CC_GUI_COLOR_OSC_WAVE_PEAK"].plurals[0] = "=Waveform (clip)##CC_GUI_COLOR_OSC_WAVE_PEAK";
    strings["Reference##CC_GUI_COLOR_OSC_REF"].plurals[0] = "=Reference##CC_GUI_COLOR_OSC_REF";
    strings["Guide##CC_GUI_COLOR_OSC_GUIDE"].plurals[0] = "=Guide##CC_GUI_COLOR_OSC_GUIDE";

    strings["Low##CC_GUI_COLOR_VOLMETER_LOW"].plurals[0] = "=Low##CC_GUI_COLOR_VOLMETER_LOW";
    strings["High##CC_GUI_COLOR_VOLMETER_HIGH"].plurals[0] = "=High##CC_GUI_COLOR_VOLMETER_HIGH";
    strings["Clip##CC_GUI_COLOR_VOLMETER_PEAK"].plurals[0] = "=Clip##CC_GUI_COLOR_VOLMETER_PEAK";

    strings["Order number##CC_GUI_COLOR_ORDER_ROW_INDEX"].plurals[0] = "=Order number##CC_GUI_COLOR_ORDER_ROW_INDEX";
    strings["Playing order background##CC_GUI_COLOR_ORDER_ACTIVE"].plurals[0] = "=Playing order background##CC_GUI_COLOR_ORDER_ACTIVE";
    strings["Song loop##CC_GUI_COLOR_SONG_LOOP"].plurals[0] = "=Song loop##CC_GUI_COLOR_SONG_LOOP";
    strings["Selected order##CC_GUI_COLOR_ORDER_SELECTED"].plurals[0] = "=Selected order##CC_GUI_COLOR_ORDER_SELECTED";
    strings["Similar patterns##CC_GUI_COLOR_ORDER_SIMILAR"].plurals[0] = "=Similar patterns##CC_GUI_COLOR_ORDER_SIMILAR";
    strings["Inactive patterns##CC_GUI_COLOR_ORDER_INACTIVE"].plurals[0] = "=Inactive patterns##CC_GUI_COLOR_ORDER_INACTIVE";

    strings["Envelope##CC_GUI_COLOR_FM_ENVELOPE"].plurals[0] = "=Envelope##CC_GUI_COLOR_FM_ENVELOPE";
    strings["Sustain guide##CC_GUI_COLOR_FM_ENVELOPE_SUS_GUIDE"].plurals[0] = "=Sustain guide##CC_GUI_COLOR_FM_ENVELOPE_SUS_GUIDE";
    strings["Release##CC_GUI_COLOR_FM_ENVELOPE_RELEASE"].plurals[0] = "=Release##CC_GUI_COLOR_FM_ENVELOPE_RELEASE";

    strings["Algorithm background##CC_GUI_COLOR_FM_ALG_BG"].plurals[0] = "=Algorithm background##CC_GUI_COLOR_FM_ALG_BG";
    strings["Algorithm lines##CC_GUI_COLOR_FM_ALG_LINE"].plurals[0] = "=Algorithm lines##CC_GUI_COLOR_FM_ALG_LINE";
    strings["Modulator##CC_GUI_COLOR_FM_MOD"].plurals[0] = "=Modulator##CC_GUI_COLOR_FM_MOD";
    strings["Carrier##CC_GUI_COLOR_FM_CAR"].plurals[0] = "=Carrier##CC_GUI_COLOR_FM_CAR";

    strings["SSG-EG##CC_GUI_COLOR_FM_SSG"].plurals[0] = "=SSG-EG##CC_GUI_COLOR_FM_SSG";
    strings["Waveform##CC_GUI_COLOR_FM_WAVE"].plurals[0] = "=Waveform##CC_GUI_COLOR_FM_WAVE";

    strings["Mod. accent (primary)##CC_GUI_COLOR_FM_PRIMARY_MOD"].plurals[0] = "=Mod. accent (primary)##CC_GUI_COLOR_FM_PRIMARY_MOD";
    strings["Mod. accent (secondary)##CC_GUI_COLOR_FM_SECONDARY_MOD"].plurals[0] = "=Mod. accent (secondary)##CC_GUI_COLOR_FM_SECONDARY_MOD";
    strings["Mod. border##CC_GUI_COLOR_FM_BORDER_MOD"].plurals[0] = "=Mod. border##CC_GUI_COLOR_FM_BORDER_MOD";
    strings["Mod. border shadow##CC_GUI_COLOR_FM_BORDER_SHADOW_MOD"].plurals[0] = "=Mod. border shadow##CC_GUI_COLOR_FM_BORDER_SHADOW_MOD";

    strings["Car. accent (primary##CC_GUI_COLOR_FM_PRIMARY_CAR"].plurals[0] = "=Car. accent (primary##CC_GUI_COLOR_FM_PRIMARY_CAR";
    strings["Car. accent (secondary)##CC_GUI_COLOR_FM_SECONDARY_CAR"].plurals[0] = "=Car. accent (secondary)##CC_GUI_COLOR_FM_SECONDARY_CAR";
    strings["Car. border##CC_GUI_COLOR_FM_BORDER_CAR"].plurals[0] = "=Car. border##CC_GUI_COLOR_FM_BORDER_CAR";
    strings["Car. border shadow##CC_GUI_COLOR_FM_BORDER_SHADOW_CAR"].plurals[0] = "=Car. border shadow##CC_GUI_COLOR_FM_BORDER_SHADOW_CAR";

    strings["Volume##CC_GUI_COLOR_MACRO_VOLUME"].plurals[0] = "=Volume##CC_GUI_COLOR_MACRO_VOLUME";
    strings["Pitch##CC_GUI_COLOR_MACRO_PITCH"].plurals[0] = "=Pitch##CC_GUI_COLOR_MACRO_PITCH";
    strings["Wave##CC_GUI_COLOR_MACRO_WAVE"].plurals[0] = "=Wave##CC_GUI_COLOR_MACRO_WAVE";
    strings["Other##CC_GUI_COLOR_MACRO_OTHER"].plurals[0] = "=Other##CC_GUI_COLOR_MACRO_OTHER";

    strings["FM (OPN)##CC_GUI_COLOR_INSTR_FM"].plurals[0] = "=FM (OPN)##CC_GUI_COLOR_INSTR_FM";
    strings["SN76489/Sega PSG##CC_GUI_COLOR_INSTR_STD"].plurals[0] = "=SN76489/Sega PSG##CC_GUI_COLOR_INSTR_STD";
    strings["T6W28##CC_GUI_COLOR_INSTR_T6W28"].plurals[0] = "=T6W28##CC_GUI_COLOR_INSTR_T6W28";
    strings["Game Boy##CC_GUI_COLOR_INSTR_GB"].plurals[0] = "=Game Boy##CC_GUI_COLOR_INSTR_GB";
    strings["C64##CC_GUI_COLOR_INSTR_C64"].plurals[0] = "=C64##CC_GUI_COLOR_INSTR_C64";
    strings["Amiga/Generic Sample##CC_GUI_COLOR_INSTR_AMIGA"].plurals[0] = "=Amiga/Generic Sample##CC_GUI_COLOR_INSTR_AMIGA";
    strings["PC Engine##CC_GUI_COLOR_INSTR_PCE"].plurals[0] = "=PC Engine##CC_GUI_COLOR_INSTR_PCE";
    strings["AY-3-8910/SSG##CC_GUI_COLOR_INSTR_AY"].plurals[0] = "=AY-3-8910/SSG##CC_GUI_COLOR_INSTR_AY";
    strings["AY8930##CC_GUI_COLOR_INSTR_AY8930"].plurals[0] = "=AY8930##CC_GUI_COLOR_INSTR_AY8930";
    strings["TIA##CC_GUI_COLOR_INSTR_TIA"].plurals[0] = "=TIA##CC_GUI_COLOR_INSTR_TIA";
    strings["SAA1099##CC_GUI_COLOR_INSTR_SAA1099"].plurals[0] = "=SAA1099##CC_GUI_COLOR_INSTR_SAA1099";
    strings["VIC##CC_GUI_COLOR_INSTR_VIC"].plurals[0] = "=VIC##CC_GUI_COLOR_INSTR_VIC";
    strings["PET##CC_GUI_COLOR_INSTR_PET"].plurals[0] = "=PET##CC_GUI_COLOR_INSTR_PET";
    strings["VRC6##CC_GUI_COLOR_INSTR_VRC6"].plurals[0] = "=VRC6##CC_GUI_COLOR_INSTR_VRC6";
    strings["VRC6 (saw)##CC_GUI_COLOR_INSTR_VRC6_SAW"].plurals[0] = "=VRC6 (saw)##CC_GUI_COLOR_INSTR_VRC6_SAW";
    strings["FM (OPLL)##CC_GUI_COLOR_INSTR_OPLL"].plurals[0] = "=FM (OPLL)##CC_GUI_COLOR_INSTR_OPLL";
    strings["FM (OPL)##CC_GUI_COLOR_INSTR_OPL"].plurals[0] = "=FM (OPL)##CC_GUI_COLOR_INSTR_OPL";
    strings["FDS##CC_GUI_COLOR_INSTR_FDS"].plurals[0] = "=FDS##CC_GUI_COLOR_INSTR_FDS";
    strings["Virtual Boy##CC_GUI_COLOR_INSTR_VBOY"].plurals[0] = "=Virtual Boy##CC_GUI_COLOR_INSTR_VBOY";
    strings["Namco 163##CC_GUI_COLOR_INSTR_N163"].plurals[0] = "=Namco 163##CC_GUI_COLOR_INSTR_N163";
    strings["Konami SCC##CC_GUI_COLOR_INSTR_SCC"].plurals[0] = "=Konami SCC##CC_GUI_COLOR_INSTR_SCC";
    strings["FM (OPZ)##CC_GUI_COLOR_INSTR_OPZ"].plurals[0] = "=FM (OPZ)##CC_GUI_COLOR_INSTR_OPZ";
    strings["POKEY##CC_GUI_COLOR_INSTR_POKEY"].plurals[0] = "=POKEY##CC_GUI_COLOR_INSTR_POKEY";
    strings["PC Beeper##CC_GUI_COLOR_INSTR_BEEPER"].plurals[0] = "=PC Beeper##CC_GUI_COLOR_INSTR_BEEPER";
    strings["WonderSwan##CC_GUI_COLOR_INSTR_SWAN"].plurals[0] = "=WonderSwan##CC_GUI_COLOR_INSTR_SWAN";
    strings["Lynx##CC_GUI_COLOR_INSTR_MIKEY"].plurals[0] = "=Lynx##CC_GUI_COLOR_INSTR_MIKEY";
    strings["VERA##CC_GUI_COLOR_INSTR_VERA"].plurals[0] = "=VERA##CC_GUI_COLOR_INSTR_VERA";
    strings["X1-010##CC_GUI_COLOR_INSTR_X1_010"].plurals[0] = "=X1-010##CC_GUI_COLOR_INSTR_X1_010";
    strings["ES5506##CC_GUI_COLOR_INSTR_ES5506"].plurals[0] = "=ES5506##CC_GUI_COLOR_INSTR_ES5506";
    strings["MultiPCM##CC_GUI_COLOR_INSTR_MULTIPCM"].plurals[0] = "=MultiPCM##CC_GUI_COLOR_INSTR_MULTIPCM";
    strings["SNES##CC_GUI_COLOR_INSTR_SNES"].plurals[0] = "=SNES##CC_GUI_COLOR_INSTR_SNES";
    strings["Sound Unit##CC_GUI_COLOR_INSTR_SU"].plurals[0] = "=Sound Unit##CC_GUI_COLOR_INSTR_SU";
    strings["Namco WSG##CC_GUI_COLOR_INSTR_NAMCO"].plurals[0] = "=Namco WSG##CC_GUI_COLOR_INSTR_NAMCO";
    strings["FM (OPL Drums)##CC_GUI_COLOR_INSTR_OPL_DRUMS"].plurals[0] = "=FM (OPL Drums)##CC_GUI_COLOR_INSTR_OPL_DRUMS";
    strings["FM (OPM)##CC_GUI_COLOR_INSTR_OPM"].plurals[0] = "=FM (OPM)##CC_GUI_COLOR_INSTR_OPM";
    strings["NES##CC_GUI_COLOR_INSTR_NES"].plurals[0] = "=NES##CC_GUI_COLOR_INSTR_NES";
    strings["MSM6258##CC_GUI_COLOR_INSTR_MSM6258"].plurals[0] = "=MSM6258##CC_GUI_COLOR_INSTR_MSM6258";
    strings["MSM6295##CC_GUI_COLOR_INSTR_MSM6295"].plurals[0] = "=MSM6295##CC_GUI_COLOR_INSTR_MSM6295";
    strings["ADPCM-A##CC_GUI_COLOR_INSTR_ADPCMA"].plurals[0] = "=ADPCM-A##CC_GUI_COLOR_INSTR_ADPCMA";
    strings["ADPCM-B##CC_GUI_COLOR_INSTR_ADPCMB"].plurals[0] = "=ADPCM-B##CC_GUI_COLOR_INSTR_ADPCMB";
    strings["Sega PCM##CC_GUI_COLOR_INSTR_SEGAPCM"].plurals[0] = "=Sega PCM##CC_GUI_COLOR_INSTR_SEGAPCM";
    strings["QSound##CC_GUI_COLOR_INSTR_QSOUND"].plurals[0] = "=QSound##CC_GUI_COLOR_INSTR_QSOUND";
    strings["YMZ280B##CC_GUI_COLOR_INSTR_YMZ280B"].plurals[0] = "=YMZ280B##CC_GUI_COLOR_INSTR_YMZ280B";
    strings["RF5C68##CC_GUI_COLOR_INSTR_RF5C68"].plurals[0] = "=RF5C68##CC_GUI_COLOR_INSTR_RF5C68";
    strings["MSM5232##CC_GUI_COLOR_INSTR_MSM5232"].plurals[0] = "=MSM5232##CC_GUI_COLOR_INSTR_MSM5232";
    strings["K007232##CC_GUI_COLOR_INSTR_K007232"].plurals[0] = "=K007232##CC_GUI_COLOR_INSTR_K007232";
    strings["GA20##CC_GUI_COLOR_INSTR_GA20"].plurals[0] = "=GA20##CC_GUI_COLOR_INSTR_GA20";
    strings["Pokémon Mini##CC_GUI_COLOR_INSTR_POKEMINI"].plurals[0] = "=Pokémon Mini##CC_GUI_COLOR_INSTR_POKEMINI";
    strings["SM8521##CC_GUI_COLOR_INSTR_SM8521"].plurals[0] = "=SM8521##CC_GUI_COLOR_INSTR_SM8521";
    strings["PV-1000##CC_GUI_COLOR_INSTR_PV1000"].plurals[0] = "=PV-1000##CC_GUI_COLOR_INSTR_PV1000";
    strings["K053260##CC_GUI_COLOR_INSTR_K053260"].plurals[0] = "=K053260##CC_GUI_COLOR_INSTR_K053260";
    strings["C140##CC_GUI_COLOR_INSTR_C140"].plurals[0] = "=C140##CC_GUI_COLOR_INSTR_C140";
    strings["Other/Unknown##CC_GUI_COLOR_INSTR_UNKNOWN"].plurals[0] = "=Other/Unknown##CC_GUI_COLOR_INSTR_UNKNOWN";

    strings["Single color (background)##CC_GUI_COLOR_CHANNEL_BG"].plurals[0] = "=Single color (background)##CC_GUI_COLOR_CHANNEL_BG";
    strings["Single color (text)##CC_GUI_COLOR_CHANNEL_FG"].plurals[0] = "=Single color (text)##CC_GUI_COLOR_CHANNEL_FG";
    strings["FM##CC_GUI_COLOR_CHANNEL_FM"].plurals[0] = "=FM##CC_GUI_COLOR_CHANNEL_FM";
    strings["Pulse##CC_GUI_COLOR_CHANNEL_PULSE"].plurals[0] = "=Pulse##CC_GUI_COLOR_CHANNEL_PULSE";
    strings["Noise##CC_GUI_COLOR_CHANNEL_NOISE"].plurals[0] = "=Noise##CC_GUI_COLOR_CHANNEL_NOISE";
    strings["PCM##CC_GUI_COLOR_CHANNEL_PCM"].plurals[0] = "=PCM##CC_GUI_COLOR_CHANNEL_PCM";
    strings["Wave##CC_GUI_COLOR_CHANNEL_WAVE"].plurals[0] = "=Wave##CC_GUI_COLOR_CHANNEL_WAVE";
    strings["FM operator##CC_GUI_COLOR_CHANNEL_OP"].plurals[0] = "=FM operator##CC_GUI_COLOR_CHANNEL_OP";
    strings["Muted##CC_GUI_COLOR_CHANNEL_MUTED"].plurals[0] = "=Muted##CC_GUI_COLOR_CHANNEL_MUTED";

    strings["Playhead##CC_GUI_COLOR_PATTERN_PLAY_HEAD"].plurals[0] = "=Playhead##CC_GUI_COLOR_PATTERN_PLAY_HEAD";
    strings["Editing##CC_GUI_COLOR_EDITING"].plurals[0] = "=Editing##CC_GUI_COLOR_EDITING";
    strings["Editing (will clone)##CC_GUI_COLOR_EDITING_CLONE"].plurals[0] = "=Editing (will clone)##CC_GUI_COLOR_EDITING_CLONE";
    strings["Cursor##CC_GUI_COLOR_PATTERN_CURSOR"].plurals[0] = "=Cursor##CC_GUI_COLOR_PATTERN_CURSOR";
    strings["Cursor (hovered)##CC_GUI_COLOR_PATTERN_CURSOR_HOVER"].plurals[0] = "=Cursor (hovered)##CC_GUI_COLOR_PATTERN_CURSOR_HOVER";
    strings["Cursor (clicked)##CC_GUI_COLOR_PATTERN_CURSOR_ACTIVE"].plurals[0] = "=Cursor (clicked)##CC_GUI_COLOR_PATTERN_CURSOR_ACTIVE";
    strings["Selection##CC_GUI_COLOR_PATTERN_SELECTION"].plurals[0] = "=Selection##CC_GUI_COLOR_PATTERN_SELECTION";
    strings["Selection (hovered)##CC_GUI_COLOR_PATTERN_SELECTION_HOVER"].plurals[0] = "=Selection (hovered)##CC_GUI_COLOR_PATTERN_SELECTION_HOVER";
    strings["Selection (clicked)##CC_GUI_COLOR_PATTERN_SELECTION_ACTIVE"].plurals[0] = "=Selection (clicked)##CC_GUI_COLOR_PATTERN_SELECTION_ACTIVE";
    strings["Highlight 1##CC_GUI_COLOR_PATTERN_HI_1"].plurals[0] = "=Highlight 1##CC_GUI_COLOR_PATTERN_HI_1";
    strings["Highlight 2##CC_GUI_COLOR_PATTERN_HI_2"].plurals[0] = "=Highlight 2##CC_GUI_COLOR_PATTERN_HI_2";
    strings["Row number##CC_GUI_COLOR_PATTERN_ROW_INDEX"].plurals[0] = "=Row number##CC_GUI_COLOR_PATTERN_ROW_INDEX";
    strings["Row number (highlight 1)##CC_GUI_COLOR_PATTERN_ROW_INDEX_HI1"].plurals[0] = "=Row number (highlight 1)##CC_GUI_COLOR_PATTERN_ROW_INDEX_HI1";
    strings["Row number (highlight 2)##CC_GUI_COLOR_PATTERN_ROW_INDEX_HI2"].plurals[0] = "=Row number (highlight 2)##CC_GUI_COLOR_PATTERN_ROW_INDEX_HI2";
    strings["Note##CC_GUI_COLOR_PATTERN_ACTIVE"].plurals[0] = "=Note##CC_GUI_COLOR_PATTERN_ACTIVE";
    strings["Note (highlight 1)##CC_GUI_COLOR_PATTERN_ACTIVE_HI1"].plurals[0] = "=Note (highlight 1)##CC_GUI_COLOR_PATTERN_ACTIVE_HI1";
    strings["Note (highlight 2)##CC_GUI_COLOR_PATTERN_ACTIVE_HI2"].plurals[0] = "=Note (highlight 2)##CC_GUI_COLOR_PATTERN_ACTIVE_HI2";
    strings["Blank##CC_GUI_COLOR_PATTERN_INACTIVE"].plurals[0] = "=Blank##CC_GUI_COLOR_PATTERN_INACTIVE";
    strings["Blank (highlight 1)##CC_GUI_COLOR_PATTERN_INACTIVE_HI1"].plurals[0] = "=Blank (highlight 1)##CC_GUI_COLOR_PATTERN_INACTIVE_HI1";
    strings["Blank (highlight 2)##CC_GUI_COLOR_PATTERN_INACTIVE_HI2"].plurals[0] = "=Blank (highlight 2)##CC_GUI_COLOR_PATTERN_INACTIVE_HI2";
    strings["Instrument##CC_GUI_COLOR_PATTERN_INS"].plurals[0] = "=Instrument##CC_GUI_COLOR_PATTERN_INS";
    strings["Instrument (invalid type)##CC_GUI_COLOR_PATTERN_INS_WARN"].plurals[0] = "=Instrument (invalid type)##CC_GUI_COLOR_PATTERN_INS_WARN";
    strings["Instrument (out of range)##CC_GUI_COLOR_PATTERN_INS_ERROR"].plurals[0] = "=Instrument (out of range)##CC_GUI_COLOR_PATTERN_INS_ERROR";
    strings["Volume (0%)##CC_GUI_COLOR_PATTERN_VOLUME_MIN"].plurals[0] = "=Volume (0%)##CC_GUI_COLOR_PATTERN_VOLUME_MIN";
    strings["Volume (50%)##CC_GUI_COLOR_PATTERN_VOLUME_HALF"].plurals[0] = "=Volume (50%)##CC_GUI_COLOR_PATTERN_VOLUME_HALF";
    strings["Volume (100%)##CC_GUI_COLOR_PATTERN_VOLUME_MAX"].plurals[0] = "=Volume (100%)##CC_GUI_COLOR_PATTERN_VOLUME_MAX";
    strings["Invalid effect##CC_GUI_COLOR_PATTERN_EFFECT_INVALID"].plurals[0] = "=Invalid effect##CC_GUI_COLOR_PATTERN_EFFECT_INVALID";
    strings["Pitch effect##CC_GUI_COLOR_PATTERN_EFFECT_PITCH"].plurals[0] = "=Pitch effect##CC_GUI_COLOR_PATTERN_EFFECT_PITCH";
    strings["Volume effect##CC_GUI_COLOR_PATTERN_EFFECT_VOLUME"].plurals[0] = "=Volume effect##CC_GUI_COLOR_PATTERN_EFFECT_VOLUME";
    strings["Panning effect##CC_GUI_COLOR_PATTERN_EFFECT_PANNING"].plurals[0] = "=Panning effect##CC_GUI_COLOR_PATTERN_EFFECT_PANNING";
    strings["Song effect##CC_GUI_COLOR_PATTERN_EFFECT_SONG"].plurals[0] = "=Song effect##CC_GUI_COLOR_PATTERN_EFFECT_SONG";
    strings["Time effect##CC_GUI_COLOR_PATTERN_EFFECT_TIME"].plurals[0] = "=Time effect##CC_GUI_COLOR_PATTERN_EFFECT_TIME";
    strings["Speed effect##CC_GUI_COLOR_PATTERN_EFFECT_SPEED"].plurals[0] = "=Speed effect##CC_GUI_COLOR_PATTERN_EFFECT_SPEED";
    strings["Primary specific effect##CC_GUI_COLOR_PATTERN_EFFECT_SYS_PRIMARY"].plurals[0] = "=Primary specific effect##CC_GUI_COLOR_PATTERN_EFFECT_SYS_PRIMARY";
    strings["Secondary specific effect##CC_GUI_COLOR_PATTERN_EFFECT_SYS_SECONDARY"].plurals[0] = "=Secondary specific effect##CC_GUI_COLOR_PATTERN_EFFECT_SYS_SECONDARY";
    strings["Miscellaneous##CC_GUI_COLOR_PATTERN_EFFECT_MISC"].plurals[0] = "=Miscellaneous##CC_GUI_COLOR_PATTERN_EFFECT_MISC";
    strings["External command output##CC_GUI_COLOR_EE_VALUE"].plurals[0] = "=External command output##CC_GUI_COLOR_EE_VALUE";
    strings["Status: off/disabled##CC_GUI_COLOR_PATTERN_STATUS_OFF"].plurals[0] = "=Status: off/disabled##CC_GUI_COLOR_PATTERN_STATUS_OFF";
    strings["Status: off + macro rel##CC_GUI_COLOR_PATTERN_STATUS_REL"].plurals[0] = "=Status: off + macro rel##CC_GUI_COLOR_PATTERN_STATUS_REL";
    strings["Status: on + macro rel##CC_GUI_COLOR_PATTERN_STATUS_REL_ON"].plurals[0] = "=Status: on + macro rel##CC_GUI_COLOR_PATTERN_STATUS_REL_ON";
    strings["Status: on##CC_GUI_COLOR_PATTERN_STATUS_ON"].plurals[0] = "=Status: on##CC_GUI_COLOR_PATTERN_STATUS_ON";
    strings["Status: volume##CC_GUI_COLOR_PATTERN_STATUS_VOLUME"].plurals[0] = "=Status: volume##CC_GUI_COLOR_PATTERN_STATUS_VOLUME";
    strings["Status: pitch##CC_GUI_COLOR_PATTERN_STATUS_PITCH"].plurals[0] = "=Status: pitch##CC_GUI_COLOR_PATTERN_STATUS_PITCH";
    strings["Status: panning##CC_GUI_COLOR_PATTERN_STATUS_PANNING"].plurals[0] = "=Status: panning##CC_GUI_COLOR_PATTERN_STATUS_PANNING";
    strings["Status: chip (primary)##CC_GUI_COLOR_PATTERN_STATUS_SYS1"].plurals[0] = "=Status: chip (primary)##CC_GUI_COLOR_PATTERN_STATUS_SYS1";
    strings["Status: chip (secondary)##CC_GUI_COLOR_PATTERN_STATUS_SYS2"].plurals[0] = "=Status: chip (secondary)##CC_GUI_COLOR_PATTERN_STATUS_SYS2";
    strings["Status: mixing##CC_GUI_COLOR_PATTERN_STATUS_MIXING"].plurals[0] = "=Status: mixing##CC_GUI_COLOR_PATTERN_STATUS_MIXING";
    strings["Status: DSP effect##CC_GUI_COLOR_PATTERN_STATUS_DSP"].plurals[0] = "=Status: DSP effect##CC_GUI_COLOR_PATTERN_STATUS_DSP";
    strings["Status: note altering##CC_GUI_COLOR_PATTERN_STATUS_NOTE"].plurals[0] = "=Status: note altering##CC_GUI_COLOR_PATTERN_STATUS_NOTE";
    strings["Status: misc color 1##CC_GUI_COLOR_PATTERN_STATUS_MISC1"].plurals[0] = "=Status: misc color 1##CC_GUI_COLOR_PATTERN_STATUS_MISC1";
    strings["Status: misc color 2##CC_GUI_COLOR_PATTERN_STATUS_MISC2"].plurals[0] = "=Status: misc color 2##CC_GUI_COLOR_PATTERN_STATUS_MISC2";
    strings["Status: misc color 3##CC_GUI_COLOR_PATTERN_STATUS_MISC3"].plurals[0] = "=Status: misc color 3##CC_GUI_COLOR_PATTERN_STATUS_MISC3";
    strings["Status: attack##CC_GUI_COLOR_PATTERN_STATUS_ATTACK"].plurals[0] = "=Status: attack##CC_GUI_COLOR_PATTERN_STATUS_ATTACK";
    strings["Status: decay##CC_GUI_COLOR_PATTERN_STATUS_DECAY"].plurals[0] = "=Status: decay##CC_GUI_COLOR_PATTERN_STATUS_DECAY";
    strings["Status: sustain##CC_GUI_COLOR_PATTERN_STATUS_SUSTAIN"].plurals[0] = "=Status: sustain##CC_GUI_COLOR_PATTERN_STATUS_SUSTAIN";
    strings["Status: release##CC_GUI_COLOR_PATTERN_STATUS_RELEASE"].plurals[0] = "=Status: release##CC_GUI_COLOR_PATTERN_STATUS_RELEASE";
    strings["Status: decrease linear##CC_GUI_COLOR_PATTERN_STATUS_DEC_LINEAR"].plurals[0] = "=Status: decrease linear##CC_GUI_COLOR_PATTERN_STATUS_DEC_LINEAR";
    strings["Status: decrease exp##CC_GUI_COLOR_PATTERN_STATUS_DEC_EXP"].plurals[0] = "=Status: decrease exp##CC_GUI_COLOR_PATTERN_STATUS_DEC_EXP";
    strings["Status: increase##CC_GUI_COLOR_PATTERN_STATUS_INC"].plurals[0] = "=Status: increase##CC_GUI_COLOR_PATTERN_STATUS_INC";
    strings["Status: bent##CC_GUI_COLOR_PATTERN_STATUS_BENT"].plurals[0] = "=Status: bent##CC_GUI_COLOR_PATTERN_STATUS_BENT";
    strings["Status: direct##CC_GUI_COLOR_PATTERN_STATUS_DIRECT"].plurals[0] = "=Status: direct##CC_GUI_COLOR_PATTERN_STATUS_DIRECT";

    strings["Background##CC_GUI_COLOR_SAMPLE_BG"].plurals[0] = "=Background##CC_GUI_COLOR_SAMPLE_BG";
    strings["Waveform##CC_GUI_COLOR_SAMPLE_FG"].plurals[0] = "=Waveform##CC_GUI_COLOR_SAMPLE_FG";
    strings["Time background##CC_GUI_COLOR_SAMPLE_TIME_BG"].plurals[0] = "=Time background##CC_GUI_COLOR_SAMPLE_TIME_BG";
    strings["Time text##CC_GUI_COLOR_SAMPLE_TIME_FG"].plurals[0] = "=Time text##CC_GUI_COLOR_SAMPLE_TIME_FG";
    strings["Loop region##CC_GUI_COLOR_SAMPLE_LOOP"].plurals[0] = "=Loop region##CC_GUI_COLOR_SAMPLE_LOOP";
    strings["Center guide##CC_GUI_COLOR_SAMPLE_CENTER"].plurals[0] = "=Center guide##CC_GUI_COLOR_SAMPLE_CENTER";
    strings["Grid##CC_GUI_COLOR_SAMPLE_GRID"].plurals[0] = "=Grid##CC_GUI_COLOR_SAMPLE_GRID";
    strings["Selection##CC_GUI_COLOR_SAMPLE_SEL"].plurals[0] = "=Selection##CC_GUI_COLOR_SAMPLE_SEL";
    strings["Selection points##CC_GUI_COLOR_SAMPLE_SEL_POINT"].plurals[0] = "=Selection points##CC_GUI_COLOR_SAMPLE_SEL_POINT";
    strings["Preview needle##CC_GUI_COLOR_SAMPLE_NEEDLE"].plurals[0] = "=Preview needle##CC_GUI_COLOR_SAMPLE_NEEDLE";
    strings["Playing needles##CC_GUI_COLOR_SAMPLE_NEEDLE_PLAYING"].plurals[0] = "=Playing needles##CC_GUI_COLOR_SAMPLE_NEEDLE_PLAYING";
    strings["Loop markers##CC_GUI_COLOR_SAMPLE_LOOP_POINT"].plurals[0] = "=Loop markers##CC_GUI_COLOR_SAMPLE_LOOP_POINT";
    strings["Chip select: disabled##CC_GUI_COLOR_SAMPLE_CHIP_DISABLED"].plurals[0] = "=Chip select: disabled##CC_GUI_COLOR_SAMPLE_CHIP_DISABLED";
    strings["Chip select: enabled##CC_GUI_COLOR_SAMPLE_CHIP_ENABLED"].plurals[0] = "=Chip select: enabled##CC_GUI_COLOR_SAMPLE_CHIP_ENABLED";
    strings["Chip select: enabled (failure)##CC_GUI_COLOR_SAMPLE_CHIP_WARNING"].plurals[0] = "=Chip select: enabled (failure)##CC_GUI_COLOR_SAMPLE_CHIP_WARNING";

    strings["Unallocated##CC_GUI_COLOR_PAT_MANAGER_NULL"].plurals[0] = "=Unallocated##CC_GUI_COLOR_PAT_MANAGER_NULL";
    strings["Unused##CC_GUI_COLOR_PAT_MANAGER_UNUSED"].plurals[0] = "=Unused##CC_GUI_COLOR_PAT_MANAGER_UNUSED";
    strings["Used##CC_GUI_COLOR_PAT_MANAGER_USED"].plurals[0] = "=Used##CC_GUI_COLOR_PAT_MANAGER_USED";
    strings["Overused##CC_GUI_COLOR_PAT_MANAGER_OVERUSED"].plurals[0] = "=Overused##CC_GUI_COLOR_PAT_MANAGER_OVERUSED";
    strings["Really overused##CC_GUI_COLOR_PAT_MANAGER_EXTREMELY_OVERUSED"].plurals[0] = "=Really overused##CC_GUI_COLOR_PAT_MANAGER_EXTREMELY_OVERUSED";
    strings["Combo Breaker##CC_GUI_COLOR_PAT_MANAGER_COMBO_BREAKER"].plurals[0] = "=Combo Breaker##CC_GUI_COLOR_PAT_MANAGER_COMBO_BREAKER";

    strings["Background##CC_GUI_COLOR_PIANO_BACKGROUND"].plurals[0] = "=Background##CC_GUI_COLOR_PIANO_BACKGROUND";
    strings["Upper key##CC_GUI_COLOR_PIANO_KEY_TOP"].plurals[0] = "=Upper key##CC_GUI_COLOR_PIANO_KEY_TOP";
    strings["Upper key (feedback)##CC_GUI_COLOR_PIANO_KEY_TOP_HIT"].plurals[0] = "=Upper key (feedback)##CC_GUI_COLOR_PIANO_KEY_TOP_HIT";
    strings["Upper key (pressed)##CC_GUI_COLOR_PIANO_KEY_TOP_ACTIVE"].plurals[0] = "=Upper key (pressed)##CC_GUI_COLOR_PIANO_KEY_TOP_ACTIVE";
    strings["Lower key##CC_GUI_COLOR_PIANO_KEY_BOTTOM"].plurals[0] = "=Lower key##CC_GUI_COLOR_PIANO_KEY_BOTTOM";
    strings["Lower key (feedback)##CC_GUI_COLOR_PIANO_KEY_BOTTOM_HIT"].plurals[0] = "=Lower key (feedback)##CC_GUI_COLOR_PIANO_KEY_BOTTOM_HIT";
    strings["Lower key (pressed)##CC_GUI_COLOR_PIANO_KEY_BOTTOM_ACTIVE"].plurals[0] = "=Lower key (pressed)##CC_GUI_COLOR_PIANO_KEY_BOTTOM_ACTIVE";

    strings["Clock text##CC_GUI_COLOR_CLOCK_TEXT"].plurals[0] = "=Clock text##CC_GUI_COLOR_CLOCK_TEXT";
    strings["Beat (off)##CC_GUI_COLOR_CLOCK_BEAT_LOW"].plurals[0] = "=Beat (off)##CC_GUI_COLOR_CLOCK_BEAT_LOW";
    strings["Beat (on)##CC_GUI_COLOR_CLOCK_BEAT_HIGH"].plurals[0] = "=Beat (on)##CC_GUI_COLOR_CLOCK_BEAT_HIGH";

    strings["PortSet##CC_GUI_COLOR_PATCHBAY_PORTSET"].plurals[0] = "=PortSet##CC_GUI_COLOR_PATCHBAY_PORTSET";
    strings["Port##CC_GUI_COLOR_PATCHBAY_PORT"].plurals[0] = "=Port##CC_GUI_COLOR_PATCHBAY_PORT";
    strings["Port (hidden/unavailable)##CC_GUI_COLOR_PATCHBAY_PORT_HIDDEN"].plurals[0] = "=Port (hidden/unavailable)##CC_GUI_COLOR_PATCHBAY_PORT_HIDDEN";
    strings["Connection (selected)##CC_GUI_COLOR_PATCHBAY_CONNECTION"].plurals[0] = "=Connection (selected)##CC_GUI_COLOR_PATCHBAY_CONNECTION";
    strings["Connection (other)##CC_GUI_COLOR_PATCHBAY_CONNECTION_BG"].plurals[0] = "=Connection (other)##CC_GUI_COLOR_PATCHBAY_CONNECTION_BG";

    strings["Log level: Error##CC_GUI_COLOR_LOGLEVEL_ERROR"].plurals[0] = "=Log level: Error##CC_GUI_COLOR_LOGLEVEL_ERROR";
    strings["Log level: Warning##CC_GUI_COLOR_LOGLEVEL_WARNING"].plurals[0] = "=Log level: Warning##CC_GUI_COLOR_LOGLEVEL_WARNING";
    strings["Log level: Info##CC_GUI_COLOR_LOGLEVEL_INFO"].plurals[0] = "=Log level: Info##CC_GUI_COLOR_LOGLEVEL_INFO";
    strings["Log level: Debug##CC_GUI_COLOR_LOGLEVEL_DEBUG"].plurals[0] = "=Log level: Debug##CC_GUI_COLOR_LOGLEVEL_DEBUG";
    strings["Log level: Trace/Verbose##CC_GUI_COLOR_LOGLEVEL_TRACE"].plurals[0] = "=Log level: Trace/Verbose##CC_GUI_COLOR_LOGLEVEL_TRACE";
    //strings[""].plurals[0] = "";
}