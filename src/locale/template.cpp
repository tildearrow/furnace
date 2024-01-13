#include <map>
#include <string>
#include "locale.h"

class DivLocale;

void DivLocale::addTranslationsTemplate()
{
    strings["test"].plurals[0] = "=test";

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

    //SETTINGS
    //TODO: all the "const char*" at the top of `settings.cpp`

    //keybind prompt
    strings["Press key..."].plurals[0] = "Press key...";

    //window name
    strings["Settings###Settings"].plurals[0] = "=Settings###Settings";
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

    strings["TEXT"].plurals[0] = "=TEXT";

    //appearance section

    strings["Appearance"].plurals[0] = "=Appearance";

    strings["Scaling"].plurals[0] = "=Scaling";

    //appearance section

    strings["Appearance"].plurals[0] = "=Appearance";

    strings["Scaling"].plurals[0] = "=Scaling";
    strings["Automatic UI scaling factor"].plurals[0] = "=Automatic UI scaling factor";
    strings["UI scaling factor"].plurals[0] = "=UI scaling factor";
    strings["Icon size"].plurals[0] = "=Icon size";

    strings["Text"].plurals[0] = "=Text";
    strings["Font renderer"].plurals[0] = "=Font renderer";
    strings["Main font"].plurals[0] = "=Main font";
    strings["Size##MainFontSize"].plurals[0] = "Size!##MainFontSize";
    strings["Header font"].plurals[0] = "=Header font";
    strings["Size##HeadFontSize"].plurals[0] = "Size!##HeadFontSize";
    strings["Pattern font"].plurals[0] = "=Pattern font";
    strings["Size##PatFontSize"].plurals[0] = "Size!##PatFontSize";
    strings["Anti-aliased fonts"].plurals[0] = "=Anti-aliased fonts";
    strings["Support bitmap fonts"].plurals[0] = "=Support bitmap fonts";
    strings["Hinting:"].plurals[0] = "=Hinting:";
    strings["Off (soft)##fh0"].plurals[0] = "Off (soft)!##fh0";
    strings["Slight##fh1"].plurals[0] = "Slight!##fh1";
    strings["Normal##fh2"].plurals[0] = "Normal!##fh2";
    strings["Full (hard)##fh3"].plurals[0] = "Full (hard)!##fh3";
    strings["Auto-hinter:"].plurals[0] = "=Auto-hinter:";
    strings["Disable##fah0"].plurals[0] = "Disable!##fah0";
    strings["Enable##fah1"].plurals[0] = "Enable!##fah1";
    strings["Force##fah2"].plurals[0] = "Force!##fah2";
    strings["Display Japanese characters"].plurals[0] = "=Display Japanese characters";
    strings["Display Chinese (Simplified) characters"].plurals[0] = "=Display Chinese (Simplified) characters";
    strings["Display Chinese (Traditional) characters"].plurals[0] = "=Display Chinese (Traditional) characters";
    strings["Display Korean characters"].plurals[0] = "=Display Korean characters";

    strings["Program"].plurals[0] = "=Program";
    strings["Title bar:"].plurals[0] = "=Title bar:";
    strings["Song Name - Furnace##tbar1"].plurals[0] = "Song Name - Furnace!##tbar1";
    strings["file_name.fur - Furnace##tbar2"].plurals[0] = "file_name.fur - Furnace!##tbar2";
    strings["/path/to/file.fur - Furnace##tbar3"].plurals[0] = "/path/to/file.fur - Furnace!##tbar3";
    strings["Display system name on title bar"].plurals[0] = "=Display system name on title bar";
    strings["Display chip names instead of \"multi-system\""].plurals[0] = "=Display chip names instead of \"multi-system\"";
    strings["Status bar:"].plurals[0] = "=Status bar:";
    strings["Cursor details##sbar0"].plurals[0] = "Cursor details!##sbar0";
    strings["File path##sbar1"].plurals[0] = "File path!##sbar1";
    strings["Cursor details or file path##sbar2"].plurals[0] = "Cursor details or file path!##sbar2";
    strings["Nothing##sbar3"].plurals[0] = "Nothing!##sbar3";
    strings["Export options layout:"].plurals[0] = "=Export options layout:";
    strings["Sub-menus in File menu##eol0"].plurals[0] = "Sub-menus in File menu!##eol0";
    strings["Modal window with tabs##eol1"].plurals[0] = "Modal window with tabs!##eol1";
    strings["Modal windows with options in File menu##eol2"].plurals[0] = "Modal windows with options in File menu!##eol2";
    strings["Capitalize menu bar"].plurals[0] = "=Capitalize menu bar";
    strings["Display add/configure/change/remove chip menus in File menu"].plurals[0] = "=Display add/configure/change/remove chip menus in File menu";

    strings["Orders"].plurals[0] = "=Orders";
    strings["Highlight channel at cursor in Orders"].plurals[0] = "=Highlight channel at cursor in Orders";
    strings["Orders row number format:"].plurals[0] = "=Orders row number format:";
    strings["Decimal##orbD"].plurals[0] = "Decimal!##orbD";
    strings["Hexadecimal##orbH"].plurals[0] = "Hexadecimal!##orbH";

    strings["Pattern"].plurals[0] = "=Pattern";
    strings["Center pattern view"].plurals[0] = "=Center pattern view";
    strings["Overflow pattern highlights"].plurals[0] = "=Overflow pattern highlights";
    strings["Display previous/next pattern"].plurals[0] = "=Display previous/next pattern";
    strings["Pattern row number format:"].plurals[0] = "=Pattern row number format:";
    strings["Decimal##prbD"].plurals[0] = "Decimal!##prbD";
    strings["Hexadecimal##prbH"].plurals[0] = "Hexadecimal!##prbH";
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
    strings["Classic##CHS0"].plurals[0] = "Classic!##CHS0";
    strings["Line##CHS1"].plurals[0] = "Line!##CHS1";
    strings["Round##CHS2"].plurals[0] = "Round!##CHS2";
    strings["Split button##CHS3"].plurals[0] = "Split button!##CHS3";
    strings["Square border##CHS4"].plurals[0] = "Square border!##CHS4";
    strings["Round border##CHS5"].plurals[0] = "Round border!##CHS5";
    strings["Channel volume bar:"].plurals[0] = "=Channel volume bar:";
    strings["Non##CHV0"].plurals[0] = "Non!##CHV0";
    strings["Simple##CHV1"].plurals[0] = "Simple!##CHV1";
    strings["Stereo##CHV2"].plurals[0] = "Stereo!##CHV2";
    strings["Real##CHV3"].plurals[0] = "Real!##CHV3";
    strings["Real (stereo)##CHV4"].plurals[0] = "Real (stereo)!##CHV4";
    strings["Channel feedback style:"].plurals[0] = "=Channel feedback style:";
    strings["Off##CHF0"].plurals[0] = "Off!##CHF0";
    strings["Note##CHF1"].plurals[0] = "Note!##CHF1";
    strings["Volume##CHF2"].plurals[0] = "Volume!##CHF2";
    strings["Active##CHF3"].plurals[0] = "Active!##CHF3";
    strings["Channel font:"].plurals[0] = "=Channel font:";
    strings["Regular##CHFont0"].plurals[0] = "Regular!##CHFont0";
    strings["Monospace##CHFont1"].plurals[0] = "Monospace!##CHFont1";
    strings["Center channel name"].plurals[0] = "=Center channel name";
    strings["Channel colors:"].plurals[0] = "=Channel colors:";
    strings["Single##CHC0"].plurals[0] = "Single!##CHC0";
    strings["Channel type##CHC1"].plurals[0] = "Channel type!##CHC1";
    strings["Instrument type##CHC2"].plurals[0] = "Instrument type!##CHC2";
    strings["Channel name colors:"].plurals[0] = "=Channel name colors:";
    strings["Single##CTC0"].plurals[0] = "Single!##CTC0";
    strings["Channel type##CTC1"].plurals[0] = "Channel type!##CTC1";
    strings["Instrument type##CTC2"].plurals[0] = "Instrument type!##CTC2";

    strings["Assets"].plurals[0] = "=Assets";
    strings["Unified instrument/wavetable/sample list"].plurals[0] = "=Unified instrument/wavetable/sample list";
    strings["Horizontal instrument list"].plurals[0] = "=Horizontal instrument list";
    strings["Instrument list icon style:"].plurals[0] = "=Instrument list icon style:";
    strings["None##iis0"].plurals[0] = "None!##iis0";
    strings["Graphical icons##iis1"].plurals[0] = "Graphical icons!##iis1";
    strings["Letter icons##iis2"].plurals[0] = "Letter icons!##iis2";
    strings["Colorize instrument editor using instrument type"].plurals[0] = "=Colorize instrument editor using instrument type";

    strings["Macro Editor"].plurals[0] = "=Macro Editor";
    strings["Macro editor layout:"].plurals[0] = "=Macro editor layout:";
    strings["Unified##mel0"].plurals[0] = "Unified!##mel0";
    strings["Grid##mel2"].plurals[0] = "Grid!##mel2";
    strings["Single (with list)##mel3"].plurals[0] = "Single (with list)!##mel3";
    strings["Use classic macro editor vertical slider"].plurals[0] = "=Use classic macro editor vertical slider";

    strings["Wave Editor"].plurals[0] = "=Wave Editor";
    strings["Use compact wave editor"].plurals[0] = "=Use compact wave editor";

    strings["FM Editor"].plurals[0] = "=FM Editor";
    strings["FM parameter names:"].plurals[0] = "=FM parameter names:";
    strings["Friendly##fmn0"].plurals[0] = "Friendly!##fmn0";
    strings["Technical##fmn1"].plurals[0] = "Technical!##fmn1";
    strings["Technical (alternate)##fmn2"].plurals[0] = "Technical (alternate)!##fmn2";
    strings["Use standard OPL waveform names"].plurals[0] = "=Use standard OPL waveform names";
    strings["FM parameter editor layout:"].plurals[0] = "=FM parameter editor layout:";
    strings["Modern##fml0"].plurals[0] = "Modern!##fml0";
    strings["Compact (2x2, classic)##fml1"].plurals[0] = "Compact (2x2, classic)!##fml1";
    strings["Compact (1x4)##fml2"].plurals[0] = "Compact (1x4)!##fml2";
    strings["Compact (4x1)##fml3"].plurals[0] = "Compact (4x1)!##fml3";
    strings["Alternate (2x2)##fml4"].plurals[0] = "Alternate (2x2)!##fml4";
    strings["Alternate (1x4)##fml5"].plurals[0] = "Alternate (1x4)!##fml5";
    strings["Alternate (4x1)##fml6"].plurals[0] = "Alternate (4x1)!##fml6";
    strings["Position of Sustain in FM editor:"].plurals[0] = "=Position of Sustain in FM editor:";
    strings["Between Decay and Sustain Rate##susp0"].plurals[0] = "Between Decay and Sustain Rate!##susp0";
    strings["After Release Rate##susp1"].plurals[0] = "After Release Rate!##susp1";
    strings["Use separate colors for carriers/modulators in FM editor"].plurals[0] = "=Use separate colors for carriers/modulators in FM editor";
    strings["Unsigned FM detune values"].plurals[0] = "=Unsigned FM detune values";

    strings["Statistics"].plurals[0] = "=Statistics";
    strings["Chip memory usage unit:"].plurals[0] = "=Chip memory usage unit:";
    strings["Bytes##MUU0"].plurals[0] = "Bytes!##MUU0";
    strings["Kilobytes##MUU1"].plurals[0] = "Kilobytes!##MUU1";

    strings["Oscilloscope"].plurals[0] = "=Oscilloscope";
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
}