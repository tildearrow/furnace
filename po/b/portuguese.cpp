#include <map>
#include <string>
#include "locale.h"

#include "template.h"

int getPluralIndexPortuguese(int n)
{
    return n > 1 ? 1 : 0;
    //here you can provide plural forms indices based on the integer.
    //you can find one-liners for common languages here:
    //https://www.gnu.org/software/gettext/manual/html_node/Plural-forms.html
    //these would need some adaptation to work in this code
    //see russian.cpp file for example of adapted statement
}

static const ImGuiLocEntry GLocalizationEntriesTmTM[] =
{
    { ImGuiLocKey_VersionStr,           "Dear ImGui " IMGUI_VERSION " (" IM_STRINGIFY(IMGUI_VERSION_NUM) ")" },
    { ImGuiLocKey_TableSizeOne,         "Ajustar largura da coluna###SizeOne"          },
    { ImGuiLocKey_TableSizeAllFit,      "Ajustar largura de todas as colunas###SizeAll"     },
    { ImGuiLocKey_TableSizeAllDefault,  "Redefinir largura de todas as colunas para o padrão###SizeAll" },
    { ImGuiLocKey_TableResetOrder,      "Redefinir ordem###ResetOrder"              },
    { ImGuiLocKey_WindowingMainMenuBar, "(Barra de menu principal)"                       },
    { ImGuiLocKey_WindowingPopup,       "(Popup)"                               },
    { ImGuiLocKey_WindowingUntitled,    "(Sem título)"                            },
    { ImGuiLocKey_DockingHideTabBar,    "Esconder barra de abas###HideTabBar"             },
};

class DivLocale;

void DivLocale::addTranslationsPortuguese()
{
    ImGui::LocalizeRegisterEntries(GLocalizationEntriesTmTM, IM_ARRAYSIZE(GLocalizationEntriesTmTM));

    strings["<nothing>##test"].plurals[0] = "<nada>##test";

    // everything in a string after the ## or ### must remain as is
    //   unless it's one of these purely-for-translation codes
    // example: "Sparkles!##sgab1" means the second instance of "Sparkles!"
    //   in `src/gui/about.cpp`.

    //progress tracker
    //   code  is the hashcode prefix (derived from path)

    //   sg**  src/gui/
    //   sgab  src/gui/about.cpp          --DONE--
    //   sgch  src/gui/channels.cpp       --DONE--
    //   sgco  src/gui/chanOsc.cpp        --    -- revise
    //   sgcl  src/gui/clock.cpp          --DONE--
    //   sgcp  src/gui/commandPalette.cpp --DONE--
    //   sgcf  src/gui/compatFlags.cpp    --    -- revise
    //   sgcs  src/gui/csPlayer.cpp
    //   sgdl  src/gui/dataList.cpp       --DONE--
    //   sgda  src/gui/doAction.cpp       --DONE--
    //   sgec  src/gui/editControls.cpp   --    -- revise
    //   sged  src/gui/editing.cpp        --DONE--
    //   sgef  src/gui/effectList.cpp     --DONE--
    //   sgeo  src/gui/exportOptions.cpp  --DONE--
    //   sgfd  src/gui/fileDialog.cpp
    //   sgfr  src/gui/findReplace.cpp    --    -- revise
    //   sgfm  src/gui/fmPreview.cpp
    //   sgfo  src/gui/fonts.cpp
    //   sggd  src/gui/gradient.cpp
    //   sggv  src/gui/grooves.cpp        --DONE--
    //   sggu  src/gui/gui.cpp            --DONE--
    //   sggc  src/gui/guiConst.cpp       --    -- revise
    //   sgim  src/gui/image.cpp
    //   sgie  src/gui/insEdit.cpp        --    -- revise
    //   sgme  src/gui/memory.cpp
    //   sgmm  src/gui/midiMap.cpp
    //   sgmx  src/gui/mixer.cpp          --DONE--
    //   sgns  src/gui/newSong.cpp        --DONE--
    //   sgor  src/gui/orders.cpp         --DONE--
    //   sgos  src/gui/osc.cpp            --DONE--
    //   sgpm  src/gui/patManager.cpp     --DONE--
    //   sgpa  src/gui/pattern.cpp        --DONE--
    //   sgpi  src/gui/piano.cpp          --    -- revise
    //   sgpr  src/gui/presets.cpp        --DONE--
    //   sgrv  src/gui/regView.cpp        --DONE--
    //   sgse  src/gui/sampleEdit.cpp     --    -- revise
    //   sgse  src/gui/settings.cpp       --DONE--
    //   sgsi  src/gui/songInfo.cpp       --DONE--
    //   sgsn  src/gui/songNotes.cpp      --DONE--
    //   sgsp  src/gui/speed.cpp          --    -- revise
    //   sgst  src/gui/stats.cpp          --DONE--
    //   sgss  src/gui/subSongs.cpp       --DONE--
    //   sgsc  src/gui/sysConf.cpp        --DONE--
    //   sgsm  src/gui/sysManager.cpp     --DONE--
    //   sgsa  src/gui/sysPartNumber.cpp  --DONE--
    //   sgsp  src/gui/sysPicker.cpp      --DONE--
    //   sgut  src/gui/util.cpp           --DONE--
    //   sgvm  src/gui/volMeter.cpp       --DONE--
    //   sgwe  src/gui/waveEdit.cpp       --DONE--
    //   sgxy  src/gui/xyOsc.cpp          --DONE--


    //   sgab  src/gui/about.cpp

    strings["About Furnace###About Furnace"].plurals[0] = "Sobre o Furnace###About Furnace";

    strings["and Furnace-B developers##sgab"].plurals[0] =                   "e desenvolvedores do Furnace-B";
    strings["are proud to present##sgab"].plurals[0] =                       "apresentam orgulhosamente";
    strings["the biggest multi-system chiptune tracker!##sgab"].plurals[0] = "o maior tracker chiptune multi-sistema!";
    strings["featuring DefleMask song compatibility.##sgab"].plurals[0] =    "com compatibilidade para músicas do DefleMask.";

    strings["> CREDITS <##sgab"].plurals[0] =                         "> CRÉDITOS <";
    strings["-- program --##sgab"].plurals[0] =                       "-- programa --";
    strings["A M 4 N (intro tune)##sgab"].plurals[0] =                "A M 4 N (tema de introdução)";
    strings["-- graphics/UI design --##sgab"].plurals[0] =            "-- gráficos/design de UI --";
    strings["-- documentation --##sgab"].plurals[0] =                 "-- documentação --";
    strings["-- demo songs --##sgab"].plurals[0] =                    "-- músicas de demonstração --";
    strings["-- additional feedback/fixes --##sgab"].plurals[0] =     "-- feedback/soluções adicionais --";
    strings["-- translations and related work --##sgab"].plurals[0] = "-- traduções e trabalho relacionado --";
    strings["LTVA1 (Russian translation)##sgab"].plurals[0] =         "LTVA1 (tradução em Russo)";
    strings["Kagamiin~ (Portuguese translation)##sgab"].plurals[0] =  "Kagamiin~ (tradução em Português)";
    strings["freq-mod (Polish translation)##sgab"].plurals[0] =       "freq-mod (tradução em Polonês)";

    strings["powered by:##sgab"].plurals[0] =                                                 "bibliotecas utilizadas:";
    strings["Dear ImGui by Omar Cornut##sgab"].plurals[0] =                                   "Dear ImGui por Omar Cornut";
    strings["SDL2 by Sam Lantinga##sgab"].plurals[0] =                                        "SDL2 por Sam Lantinga";
    strings["zlib by Jean-loup Gailly##sgab"].plurals[0] =                                    "zlib por Jean-loup Gailly";
    strings["and Mark Adler##sgab"].plurals[0] =                                              "e Mark Adler";
    strings["libsndfile by Erik de Castro Lopo##sgab"].plurals[0] =                           "libsndfile por Erik de Castro Lopo";
    strings["Portable File Dialogs by Sam Hocevar##sgab"].plurals[0] =                        "Portable File Dialogs por Sam Hocevar";
    strings["Native File Dialog by Frogtoss Games##sgab"].plurals[0] =                        "Native File Dialog por Frogtoss Games";
    strings["Weak-JACK by x42##sgab"].plurals[0] =                                            "Weak-JACK por x42";
    strings["RtMidi by Gary P. Scavone##sgab"].plurals[0] =                                   "RtMidi por Gary P. Scavone";
    strings["FFTW by Matteo Frigo and Steven G. Johnson##sgab"].plurals[0] =                  "FFTW por Matteo Frigo and Steven G. Johnson";
    strings["backward-cpp by Google##sgab"].plurals[0] =                                      "backward-cpp da Google";
    strings["adpcm by superctr##sgab"].plurals[0] =                                           "adpcm por superctr";
    strings["adpcm-xq by David Bryant##sgab"].plurals[0] =                                    "adpcm-xq por David Bryant";
    strings["Nuked-OPL3/OPLL/OPM/OPN2/PSG by nukeykt##sgab"].plurals[0] =                     "Nuked-OPL3/OPLL/OPM/OPN2/PSG por nukeykt";
    strings["YM3812-LLE, YMF262-LLE and YMF276-LLE by nukeykt##sgab"].plurals[0] =            "YM3812-LLE, YMF262-LLE e YMF276-LLE por nukeykt";
    strings["ymfm by Aaron Giles##sgab"].plurals[0] =                                         "ymfm por Aaron Giles";
    strings["MAME SN76496 by Nicola Salmoria##sgab"].plurals[0] =                             "MAME SN76496 por Nicola Salmoria";
    strings["MAME AY-3-8910 by Couriersud##sgab"].plurals[0] =                                "MAME AY-3-8910 por Couriersud";
    strings["with AY8930 fixes by Eulous, cam900 and Grauw##sgab"].plurals[0] =               "com melhorias de AY8930 por Eulous, cam900 e Grauw";
    strings["MAME SAA1099 by Juergen Buchmueller and Manuel Abadia##sgab"].plurals[0] =       "MAME SAA1099 por Juergen Buchmueller e Manuel Abadia";
    strings["MAME Namco WSG by Nicola Salmoria and Aaron Giles##sgab"].plurals[0] =           "MAME Namco WSG por Nicola Salmoria e Aaron Giles";
    strings["MAME RF5C68 core by Olivier Galibert and Aaron Giles##sgab"].plurals[0] =        "núcleo MAME RF5C68 por Olivier Galibert e Aaron Giles";
    strings["MAME MSM5232 core by Jarek Burczynski and Hiromitsu Shioya##sgab"].plurals[0] =  "núcleo MAME MSM5232 core por Jarek Burczynski e Hiromitsu Shioya";
    strings["MAME MSM6258 core by Barry Rodewald##sgab"].plurals[0] =                         "núcleo MAME MSM6258 por Barry Rodewald";
    strings["MAME YMZ280B core by Aaron Giles##sgab"].plurals[0] =                            "núcleo MAME YMZ280B por Aaron Giles";
    strings["MAME GA20 core by Acho A. Tang and R. Belmont##sgab"].plurals[0] =               "núcleo MAME GA20 por Acho A. Tang e R. Belmont";
    strings["MAME SegaPCM core by Hiromitsu Shioya and Olivier Galibert##sgab"].plurals[0] =  "núcleo MAME SegaPCM por Hiromitsu Shioya e Olivier Galibert";
    strings["SAASound by Dave Hooper and Simon Owen##sgab"].plurals[0] =                      "Sand AASound por Dave Hooper e Simon Owen";
    strings["SameBoy by Lior Halphon##sgab"].plurals[0] =                                     "SameBoy por Lior Halphon";
    strings["Mednafen PCE, WonderSwan, T6W28 and Virtual Boy audio cores##sgab"].plurals[0] = "núcleos de áudio Mednafen PCE, WonderSwan, T6W28 e Virtual Boy";
    strings["SNES DSP core by Blargg##sgab"].plurals[0] =                                     "núcleo SNES DSP por Blargg";
    strings["puNES (NES, MMC5 and FDS) by FHorse##sgab"].plurals[0] =                         "puNES (NES, MMC5 e FDS) por FHorse";
    strings["NSFPlay (NES and FDS) by Brad Smith and Brezza##sgab"].plurals[0] =              "NSFPlay (NES e FDS) por Brad Smith e Brezza";
    strings["reSID by Dag Lem##sgab"].plurals[0] =                                            "reSID por Dag Lem";
    strings["reSIDfp by Dag Lem, Antti Lankila##sgab"].plurals[0] =                           "reSIDfp por Dag Lem, Antti Lankila";
    strings["and Leandro Nini##sgab"].plurals[0] =                                            "e Leandro Nini";
    strings["dSID by DefleMask Team based on jsSID##sgab"].plurals[0] =                       "dSID pelo Time DefleMask baseado no jsSID";
    strings["Stella by Stella Team##sgab"].plurals[0] =                                       "Stella pelo Time Stella";
    strings["QSound emulator by superctr and Valley Bell##sgab"].plurals[0] =                 "emulador QSound por superctr e Valley Bell";
    strings["VICE VIC-20 sound core by Rami Rasanen and viznut##sgab"].plurals[0] =           "núcleo de som VICE VIC-20 por Rami Rasanen e viznut";
    strings["VICE TED sound core by Andreas Boose, Tibor Biczo##sgab"].plurals[0] =           "núcleo de som VICE TED por Andreas Boose, Tibor Biczo";
    strings["and Marco van den Heuvel##sgab"].plurals[0] =                                    "e Marco van den Heuvel";
    strings["VERA sound core by Frank van den Hoef##sgab"].plurals[0] =                       "núcleo de som VERA por Frank van den Hoef";
    strings["mzpokeysnd POKEY emulator by Michael Borisov##sgab"].plurals[0] =                "emulador POKEY mzpokeysnd por Michael Borisov";
    strings["ASAP POKEY emulator by Piotr Fusik##sgab"].plurals[0] =                          "emulador POKEY ASAP por Piotr Fusik";
    strings["ported by laoo to C++##sgab"].plurals[0] =                                       "traduzido para C++ por laoo";
    strings["vgsound_emu (second version, modified version) by cam900##sgab"].plurals[0] =    "vgsound_emu (segunda versão, modificada) por cam900";
    strings["SM8521 emulator (modified version) by cam900##sgab"].plurals[0] =                "emulador de SM8521 (versão modificada) by cam900";
    strings["D65010G031 emulator (modified version) by cam900##sgab"].plurals[0] =            "emulador de D65010G031 (versão modificada) by cam900";
    strings["Namco C140/C219 emulator (modified version) by cam900##sgab"].plurals[0] =       "emulador de Namco C140/C219 (versão modificada) by cam900";
    strings["ESFMu emulator by Kagamiin~##sgab"].plurals[0] =                                 "ESFMu (versão modificada) por Kagamiin~";
    strings["PowerNoise emulator by scratchminer##sgab"].plurals[0] =                         "emulador PowerNoise por scratchminer";
    strings["ep128emu by Istvan Varga##sgab"].plurals[0] =                                    "ep128emu por Istvan Varga";
    strings["SID2 emulator (modification of reSID) by LTVA##sgab"].plurals[0] =               "emulador SID2 (modificação do reSID) por LTVA";
    strings["5E01 emulator (modification of NSFPlay) by Euly##sgab"].plurals[0] =             "emulador 5E01 (modificação do NSFPlay) por Euly";
    strings["NDS sound emulator by cam900##sgab"].plurals[0] =                                "emulador de áudio de NDS por cam900";


    strings["greetings to:##sgab"].plurals[0] =                    "saudações a:";
    strings["NEOART Costa Rica##sgab"].plurals[0] =                "NEOART Costa Rica";
    strings["Xenium Demoparty##sgab"].plurals[0] =                 "Xenium Demoparty";
    strings["all members of Deflers of Noice!##sgab"].plurals[0] = "todos os membros de Deflers of Noice!";

    strings["copyright © 2021-2024 tildearrow##sgab"].plurals[0] = "copyright © 2021-2024 tildearrow";
    strings["(and contributors).##sgab"].plurals[0] =              "(e contribuidores).";
    strings["licensed under GPLv2+! see##sgab"].plurals[0] =       "licenciado sob a GPLv2+! veja o";
    strings["LICENSE for more information.##sgab"].plurals[0] =    "arquivo LICENSE para mais informações.";

    strings["help Furnace grow:##sgab"].plurals[0] = "ajude o Furnace a crescer:";
    strings["help Furnace-B:##sgab"].plurals[0] =    "ajude o Furnace-B:";

    strings["contact tildearrow at:##sgab"].plurals[0] = "entre em contato com tildearrow em:";

    strings["disclaimer:##sgab"].plurals[0] =                           "aviso:";
    strings["despite the fact this program works##sgab"].plurals[0] =   "apesar do fato de este programa funcionar";
    strings["with the .dmf file format, it is NOT##sgab"].plurals[0] =  "com o formato de arquivo .dmf, ele NÃO é";
    strings["affiliated with Delek or DefleMask in##sgab"].plurals[0] = "afiliado com Delek ou o DefleMask de forma";
    strings["any way, nor it is a replacement for##sgab"].plurals[0] =  "alguma, assim como também não é um substituto";
    strings["the original program.##sgab"].plurals[0] =                 "para o programa original.";

    strings["it also comes with ABSOLUTELY NO WARRANTY.##sgab"].plurals[0] = "ele também não vem com ABSOLUTAMENTE NENHUMA GARANTIA.";

    strings["thanks to all contributors/bug reporters!##sgab"].plurals[0] = "agradecemos a todos os contribuidores e bug reporters!";

    //   sgch  src/gui/channels.cpp

    strings["Channels###Channels"].plurals[0] = "Canais###Channels";
    strings["Pat##sgch"].plurals[0] = "Seq";
    strings["Osc##sgch"].plurals[0] = "Osc";
    strings["Swap##sgch"].plurals[0] = "Trocar";
    strings["Name##sgch"].plurals[0] = "Nome";
    strings["Show in pattern##sgch"].plurals[0] = "Mostrar na sequência";
    strings["Show in per-channel oscilloscope##sgch"].plurals[0] = "Mostrar no osciloscópio por canal";
    strings["%s #%d\n(drag to swap channels)##sgch"].plurals[0] = "%s #%d\n(arraste para trocar os canais)";

    //   sgco  src/gui/chanOsc.cpp

    strings["None (0%)##sgco"].plurals[0] = "Nenhum (0%)";
    strings["None (50%)##sgco"].plurals[0] = "Nenhum (50%)";
    strings["None (100%)##sgco"].plurals[0] = "Nenhum (100%)";
    strings["Frequency##sgco"].plurals[0] = "Frequência";
    strings["Volume##sgco"].plurals[0] = "Volume";
    strings["Channel##sgco"].plurals[0] = "Canal";
    strings["Brightness##sgco"].plurals[0] = "Brilho";
    strings["Note Trigger##sgco"].plurals[0] = "Toque da Nota"; // TODO melhorar mais
    strings["Off##sgco"].plurals[0] = "Desativado";
    strings["Mode 1##sgco"].plurals[0] = "Modo 1";
    strings["Mode 2##sgco"].plurals[0] = "Modo 2";
    strings["Mode 3##sgco"].plurals[0] = "Modo 3";

    strings["Oscilloscope (per-channel)###Oscilloscope (per-channel)"].plurals[0] = "Osciloscópio (por canal)###Oscilloscope (per-channel)";

    strings["Columns##sgco"].plurals[0] = "Colunas";
    strings["Size (ms)##sgco"].plurals[0] = "Tamanho (ms)";
    strings["Automatic columns##sgco"].plurals[0] = "Colunas automáticas";
    strings["Center waveform##sgco"].plurals[0] = "Centralizar onda";
    strings["Randomize phase on note##sgco"].plurals[0] = "Aleatorizar fase a cada nota";
    strings["Amplitude##sgco"].plurals[0] = "Amplitude";
    strings["Line size##sgco"].plurals[0] = "Espessura da linha";
    strings["Gradient##sgco"].plurals[0] = "Gradiente";
    strings["Color##sgco0"].plurals[0] = "Cor";
    strings["Distance##sgco"].plurals[0] = "Distância";
    strings["Spread##sgco"].plurals[0] = "Espalhamento";
    strings["Remove##sgco"].plurals[0] = "Remover";
    strings["Background##sgco"].plurals[0] = "Fundo";
    strings["X Axis##AxisX"].plurals[0] = "Eixo X##AxisX";
    strings["Y Axis##AxisY"].plurals[0] = "Eixo Y##AxisY";
    strings["Color##sgco1"].plurals[0] = "Cor";
    strings["Text format:##sgco"].plurals[0] = "Formato do texto:";

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
            "- %l: new line\n"
            "- %%: percent sign##sgco"].plurals[0] = 

            "guia de formatação:\n"
            "- %c: nome do canal\n"
            "- %C: nome curto do canal\n"
            "- %d: número do canal (começando em 0)\n"
            "- %D: número do canal (começando em 1)\n"
            "- %n: nota do canal\n"
            "- %i: nome do instrumento\n"
            "- %I: número do instrumento (decimal)\n"
            "- %x: número do instrumento (hex)\n"
            "- %s: nome do chip\n"
            "- %p: numeração do chip\n"
            "- %S: ID do chip\n"
            "- %v: volume (decimal)\n"
            "- %V: volume (porcentagem)\n"
            "- %b: volume (hex)\n"
            "- %l: nova linha\n"
            "- %%: símbolo de porcentagem";

    strings["Text color##sgco"].plurals[0] = "Cor do texto";
    strings["Error!##sgco"].plurals[0] = "Erro!";
    strings["\nquiet##sgco"].plurals[0] = "\nsilenciado";

    //   sgcl  src/gui/clock.cpp

    strings["Clock###Clock"].plurals[0] = "Clock###Clock";

    //   sgcp  src/gui/commandPalette.cpp

    strings["Search...##sgcp"].plurals[0] = "Procurar...";
    strings["Search recent files...##sgcp"].plurals[0] = "Buscar em arquivos recentes...";
    strings["Search instruments...##sgcp"].plurals[0] = "Procurar instrumentos...";
    strings["Search samples...##sgcp"].plurals[0] = "Procurar samples...";
    strings["Search instruments (to change to)...##sgcp"].plurals[0] = "Procurar insatrumentos (para selecionar)...";
    strings["Search chip (to add)...##sgcp"].plurals[0] = "Procurar chip (para adicionar)...";
    strings["Cancel##sgcp"].plurals[0] = "Cancelar";
    strings["cannot add chip! (##sgcp"].plurals[0] = "não pôde adicionar chip! (";
    strings["- None -"].plurals[0] = "- Nenhum -";

    //   sgcf  src/gui/compatFlags.cpp

    strings["Compatibility Flags###Compatibility Flags"].plurals[0] = "Opções de Compatibilidade###Compatibility Flags";
    strings["these flags are designed to provide better DefleMask/older Furnace compatibility.\nit is recommended to disable most of these unless you rely on specific quirks.##sgcf"].plurals[0] = "estas opções são desenvolvidas para prover melhor compatibilidade ao Deflemask ou versões antigas do Furnace.\né recomendado desabilitar a maior parte delas a não ser que você dependa de peculiaridades específicas.";
    strings["Game Boy instrument duty is wave volume##sgcf"].plurals[0] = "Ciclo de trabalho do instrumento do Game Boy é o volume do canal de onda";
    strings["if enabled, an instrument with duty macro in the wave channel will be mapped to wavetable volume.##sgcf"].plurals[0] = "se habilitado, um instrumento com macro de ciclo de trabalho no canal de onda será mapeado para o volume da wavetable.";
    strings["Restart macro on portamento##sgcf"].plurals[0] = "Reiniciar macro quando houver portamento";
    strings["when enabled, a portamento effect will reset the channel's macro if used in combination with a note.##sgcf"].plurals[0] = "quando habilitado, um efeito de portamento irá reiniciar a macro do canal se utilizado em combinação com uma nota.";
    strings["Ignore duplicate slide effects##sgcf"].plurals[0] = "Ignorar efeitos de slide duplicados";
    strings["if this is on, only the first slide of a row in a channel will be considered.##sgcf"].plurals[0] = "se habilitado, apenas o primeiro slide de uma linha em um canal será considerado.";
    strings["Ignore 0Dxx on the last order##sgcf"].plurals[0] = "Ignorar 0Dxx na última ordem";
    strings["if this is on, a jump to next row effect will not take place when it is on the last order of a song.##sgcf"].plurals[0] = "se isto estiver habilitado, o efeito para pular para a próxima ordem não irá funcionar quando estiver na última ordem de uma música.";
    strings["Don't apply Game Boy envelope on note-less instrument change##sgcf"].plurals[0] = "Não aplicar envelope do Game Boy em mudança de instrumento sem nota";
    strings["if this is on, an instrument change will not affect the envelope.##sgcf"].plurals[0] = "se isto estiver habilitado, uma mudança de instrumento não irá afetar o envelope.";
    strings["Ignore DAC mode change outside of intended channel in ExtCh mode##sgcf"].plurals[0] = "Ignorar mudança de modo DAC fora do canal correto no modo ExtCh";
    strings["if this is on, 17xx has no effect on the operator channels in YM2612.##sgcf"].plurals[0] = "se isso estiver habilitado, 17xx não tem nenhum efeito nos canais de operador no YM2612.";
    strings["SN76489 duty macro always resets phase##sgcf"].plurals[0] = "Macro de ciclo de trabalho do SN76489 sempre reinicia a fase";
    strings["when enabled, duty macro will always reset phase, even if its value hasn't changed.##sgcf"].plurals[0] = "quando habilitado, a macro de ciclo de trabalho sempre irá reiniciar a fase do canal, mesmo que o seu valor não tenha sido alterado.";
    strings["Don't persist volume macro after it finishes##sgcf"].plurals[0] = "Não persistir macro de volume depois que ela terminar";
    strings["when enabled, a value in the volume column that happens after the volume macro is done will disregard the macro.##sgcf"].plurals[0] = "quando habilitado, um valor na coluna de volume que aparece depois que a macro de volume termina irá desconsiderar o valor da macro.";
    strings[".mod import##sgcf"].plurals[0] = "importação de .mod";
    strings["Don't slide on the first tick of a row##sgcf"].plurals[0] = "Não fazer slide no primeiro tick de uma linha";
    strings["simulates ProTracker's behavior of not applying volume/pitch slides on the first tick of a row.##sgcf"].plurals[0] = "simula o comportamento do ProTracker de não aplicar slides de volume/tom no primeiro tick de uma linha.";
    strings["Reset arpeggio position on row change##sgcf"].plurals[0] = "Reiniciar posição do arpejo em mudança de linha";
    strings["simulates ProTracker's behavior of arpeggio being bound to the current tick of a row.##sgcf"].plurals[0] = "simula o comportamento do ProTracker em que o arpejo é ligado ao tick atual de uma linha.";
    strings["Pitch/Playback##sgcf"].plurals[0] = "Tom/Reprodução";
    strings["Pitch linearity:##sgcf"].plurals[0] = "Linearidade de frequências:";
    strings["None##sgcf"].plurals[0] = "Nenhuma";
    strings["like ProTracker/FamiTracker##sgcf"].plurals[0] = "como ProTracker/FamiTracker";
    strings["Full##sgcf"].plurals[0] = "Completa";
    strings["like Impulse Tracker##sgcf"].plurals[0] = "como Impulse Tracker";
    strings["Pitch slide speed multiplier##sgcf"].plurals[0] = "Multiplicador de velocidade de slide de frequência";
    strings["Loop modality:##sgcf"].plurals[0] = "Modalidade de loop:";
    strings["Reset channels##sgcf"].plurals[0] = "Reiniciar canais";
    strings["select to reset channels on loop. may trigger a voltage click on every loop!##sgcf"].plurals[0] = "escolha para reiniciar bruscamente os canais a cada loop. pode causar cliques de tensão em todos os loops!";
    strings["Soft reset channels##sgcf"].plurals[0] = "Desativar canais";
    strings["select to turn channels off on loop.##sgcf"].plurals[0] = "selecione para desligar os canais a cada loop.";
    strings["Do nothing##sgcf"].plurals[0] = "Não fazer nada";
    strings["select to not reset channels on loop.##sgcf"].plurals[0] = "selecione para não reiniciar os canais a cada loop.";
    strings["Cut/delay effect policy:##sgcf"].plurals[0] = "Política de efeito de corte/atraso:";
    strings["Strict##sgcf"].plurals[0] = "Estrita";
    strings["only when time is less than speed (like DefleMask/ProTracker)##sgcf"].plurals[0] = "somente quando o tempo é menor que a velocidade (como DefleMask/ProTracker)";
    strings["Lax##sgcf"].plurals[0] = "Relaxada";
    strings["no checks##sgcf"].plurals[0] = "sem checagens";
    strings["Simultaneous jump (0B+0D) treatment:##sgcf"].plurals[0] = "Tratamento de pulos simultâneos (0B+0D):";
    strings["Normal##sgcf"].plurals[0] = "Normal";
    strings["accept 0B+0D to jump to a specific row of an order##sgcf"].plurals[0] = "aceitar 0B+0D para pular para uma linha específica de uma ordem";
    strings["Other##sgcf"].plurals[0] = "Outro";
    strings["Auto-insert one tick gap between notes##sgcf"].plurals[0] = "Automaticamente inserir intervalo de um tick entre notas";
    strings["when enabled, a one-tick note cut will be inserted between non-legato/non-portamento notes.\nthis simulates the behavior of some Amiga/SNES music engines.\n\nineffective on C64.##sgcf"].plurals[0] = "quando habilitado, um corte de nota de um tick será inserido entre notas não-legato/não-portamento.\nisso simula o comportamento de alguns motores de música de Amiga e SNES.\nNão tem efeito no C64.";
    strings["Don't reset slides after note off##sgcf"].plurals[0] = "Não reiniciar slides após \"note-off\"";
    strings["when enabled, note off will not reset the channel's slide effect.##sgcf"].plurals[0] = "quando habilitado, a desativação de nota não irá reiniciar o efeito de slide do canal.";
    strings["Don't reset portamento after reaching target##sgcf"].plurals[0] = "Não reiniciar portamento ao atingir nota alvo";
    strings["when enabled, the slide effect will not be disabled after it reaches its target.##sgcf"].plurals[0] = "quando habilitado, o efeito portamento não será desligado após atingir sua nota alvo.";
    strings["Continuous vibrato##sgcf"].plurals[0] = "Vibrato contínuo";
    strings["when enabled, vibrato phase/position will not be reset on a new note.##sgcf"].plurals[0] = "quando habilitado, a fase/posição do vibrato não será reiniciada em novas notas.";
    strings["Pitch macro is not linear##sgcf"].plurals[0] = "Macro de tom não é linear";
    strings["when enabled, the pitch macro of an instrument is in frequency/period space.##sgcf"].plurals[0] = "quando habilitado, a macro de tom de um instrumento é aplicada em domínio de frequência/período.";
    strings["Reset arpeggio effect position on new note##sgcf"].plurals[0] = "Reiniciar posição do efeito de arpejo em novas notas";
    strings["when enabled, arpeggio effect (00xy) position is reset on a new note.##sgcf"].plurals[0] = "quando habilitado, a posição do efeito de arpejo (00xy) é reiniciada quando uma nova nota é tocada.";
    strings["Volume scaling rounds up##sgcf"].plurals[0] = "Escalonamento de volume arredonda para cima";
    strings["when enabled, volume macros round up when applied\nthis prevents volume scaling from causing vol=0, which is silent on some chips\n\nineffective on logarithmic channels##sgcf"].plurals[0] = "quando habilitado, macros de volume arredondam para cima ao serem aplicadas\nisso previne que o escalonamento de volume gere um volume igual a 0, que gera silêncio em alguns chips\n\nnão tem efeito em canais com volume logarítmico";
    strings["Stop NES pulse channels hardware sweep on new note##sgcf"].plurals[0] = "Interromper sweep de hardware dos canais de pulso do NES em nova nota";
    strings["Do not stop volume slide after reaching zero or full volume##sgcf"].plurals[0] = "Não interromper slide de volume após alcançar volume zero ou máximo";
    strings["Stop E1xy/E2xy effects on new note##sgcf"].plurals[0] = "Interromper efeitos E1xy/E2xy em nova nota";
    strings["Slower 0Axy volume slide##sgcf"].plurals[0] = "Slide de volume 0Axy é mais lento";

    //   sgcs  src/gui/csPlayer.cpp

    strings["Command Stream Player###Command Stream Player"].plurals[0] = "Reprodutor de fluxo de comandos###Command Stream Player";
    strings["Load##sgcs"].plurals[0] = "Carregar";
    strings["Kill##sgcs"].plurals[0] = "Matar";
    strings["Burn Current Song##sgcs"].plurals[0] = "Destruir Música Atual";
    strings["Status##sgcs"].plurals[0] = "Estado";
    strings["Hex##sgcs"].plurals[0] = "Hex";


    //   sgdl  src/gui/dataList.cpp

    strings["Bug!##sgdl"].plurals[0] = "Bug!";
    strings["Unknown##sgdl"].plurals[0] = "Desconhecido";
    strings["duplicate##sgdl0"].plurals[0] = "duplicado";
    strings["replace...##sgdl0"].plurals[0] = "trocar...";
    strings["save##sgdl0"].plurals[0] = "salvar";
    strings["export (.dmp)##sgdl"].plurals[0] = "exportar (.dmp)";
    strings["delete##sgdl0"].plurals[0] = "excluir";
    strings["%.2X: <INVALID>##sgdl"].plurals[0] = "%.2X: <INVÁLIDO>";
    strings["- None -##sgdl"].plurals[0] = "- Nenhum -";
    strings["out of memory for this sample!##sgdl"].plurals[0] = "sem memória para esta sample!";
    strings["make instrument##sgdl"].plurals[0] = "criar instrumento";
    strings["make me a drum kit##sgdl"].plurals[0] = "faça um kit de bateria para mim";
    strings["duplicate##sgdl1"].plurals[0] = "duplicar";
    strings["replace...##sgdl1"].plurals[0] = "trocar...";
    strings["save##sgdl1"].plurals[0] = "salvar";
    strings["delete##sgdl1"].plurals[0] = "excluir";
    strings["Instruments###Instruments"].plurals[0] = "Instrumentos###Instruments";
    strings["Add##sgdl0"].plurals[0] = "Adicionar";
    strings["Duplicate##sgdl2"].plurals[0] = "Duplicar";
    strings["Open##sgdl0"].plurals[0] = "Abrir";
    strings["replace instrument...##sgdl"].plurals[0] = "trocar instrumento...";
    strings["load instrument from TX81Z##sgdl"].plurals[0] = "carregar instrumento do TX81Z";
    strings["replace wavetable...##sgdl"].plurals[0] = "trocar wavetable...";
    strings["replace sample...##sgdl"].plurals[0] = "trocar sample...";
    strings["import raw sample...##sgdl"].plurals[0] = "importar sample em formato raw...";
    strings["import raw sample (replace)...##sgdl"].plurals[0] = "importar sample em formato raw (trocar)...";
    strings["replace...##sgdl2"].plurals[0] = "trocar...";
    strings["load from TX81Z##sgdl"].plurals[0] = "carregar do TX81Z";
    strings["Open (insert; right-click to replace)##sgdl"].plurals[0] = "Abrir (inserir; botão direito para trocar)";
    strings["Save##sgdl2"].plurals[0] = "Salvar";
    strings["save instrument as .dmp...##sgdl"].plurals[0] = "salvar instrumento como .dmp...";
    strings["save wavetable as .dmw...##sgdl"].plurals[0] = "salvar wavetable como .dmw...";
    strings["save raw wavetable...##sgdl"].plurals[0] = "salvar wavetable em formato raw...";
    strings["save raw sample...##sgdl"].plurals[0] = "salvar sample em formato raw...";
    strings["save as .dmp...##sgdl"].plurals[0] = "salvar como .dmp...";
    strings["Toggle folders/standard view##sgdl0"].plurals[0] = "Alternar entre pastas e visão padrão";
    strings["Move up##sgdl0"].plurals[0] = "Mover para cima";
    strings["Move down##sgdl0"].plurals[0] = "Mover para baixo";
    strings["Create##sgdl0"].plurals[0] = "Criar";
    strings["New folder##sgdl0"].plurals[0] = "Nova pasta";
    strings["Preview (right click to stop)##sgdl0"].plurals[0] = "Pré-visualização (clique com o botão direito para parar)";
    strings["Delete##sgdl2"].plurals[0] = "Excluir";
    strings["Instruments##sgdl"].plurals[0] = "Instrumentos";
    strings["<uncategorized>##sgdl0"].plurals[0] = "<não-categorizado>";
    strings["rename...##sgdl0"].plurals[0] = "renomear...";
    strings["delete##sgdl3"].plurals[0] = "excluir";
    strings["Wavetables##sgdl"].plurals[0] = "Wavetables";
    strings["Samples##sgdl"].plurals[0] = "Samples";
    strings["Wavetables###Wavetables"].plurals[0] = "Wavetables###Wavetables";
    strings["Add##sgdl2"].plurals[0] = "Adicionar";
    strings["Duplicate##sgdl3"].plurals[0] = "Duplicar";
    strings["Open##sgdl1"].plurals[0] = "Abrir";
    strings["replace...##sgdl3"].plurals[0] = "trocar...";
    strings["Save##sgdl3"].plurals[0] = "Salvar";
    strings["save as .dmw...##sgdl"].plurals[0] = "salvar como .dmw...";
    strings["save raw...##sgdl0"].plurals[0] = "salvar em formato raw...";
    strings["Toggle folders/standard view##sgdl1"].plurals[0] = "Alternar entre pastas e visão padrão";
    strings["Move up##sgdl1"].plurals[0] = "Mover para cima";
    strings["Move down##sgdl1"].plurals[0] = "Mover para baixo";
    strings["Create##sgdl1"].plurals[0] = "Criar";
    strings["New folder##sgdl1"].plurals[0] = "Nova pasta";
    strings["Delete##sgdl4"].plurals[0] = "Excluir";
    strings["Samples###Samples"].plurals[0] = "Samples###Samples";
    strings["Add##sgdl3"].plurals[0] = "Adicionar";
    strings["Duplicate##sgdl4"].plurals[0] = "Duplicar";
    strings["Open##sgdl2"].plurals[0] = "Abrir";
    strings["replace...##sgdl4"].plurals[0] = "trocar...";
    strings["import raw...##sgdl"].plurals[0] = "importar em formato raw...";
    strings["import raw (replace)...##sgdl"].plurals[0] = "importar em formato raw (trocar)...";
    strings["Save##sgdl4"].plurals[0] = "Salvar";
    strings["save raw...##sgdl1"].plurals[0] = "salvar em formato raw...";
    strings["Toggle folders/standard view##sgdl2"].plurals[0] = "Alternar entre pastas e visão padrão";
    strings["Move up##sgdl2"].plurals[0] = "Mover para cima";
    strings["Move down##sgdl2"].plurals[0] = "Mover para baixo";
    strings["Create##sgdl2"].plurals[0] = "Criar";
    strings["New folder##sgdl2"].plurals[0] = "Nova pasta";
    strings["Preview (right click to stop)##sgdl1"].plurals[0] = "Pré-visualização (clique com o botão direito para parar)";
    strings["Delete##sgdl5"].plurals[0] = "Excluir";
    strings["<uncategorized>##sgdl1"].plurals[0] = "<não-categorizado>";
    strings["rename...##sgdl1"].plurals[0] = "renomear...";
    strings["delete##sgdl6"].plurals[0] = "excluir";
    strings["rename...##sgdl2"].plurals[0] = "renomear...";
    strings["delete##sgdl7"].plurals[0] = "excluir";

    //   sggu  src/gui/gui.cpp

    strings["Instrument %d##sggu"].plurals[0] = "Instrumento %d";
    strings["Sample %d"].plurals[0] = "Sample %d";
    strings["the song is over!##sggu0"].plurals[0] = "a música terminou!";
    strings["the song is over!##sggu1"].plurals[0] = "a música terminou!";
    strings["Open File##sggu"].plurals[0] = "Abrir arquivo";
    strings["compatible files##sggu0"].plurals[0] = "arquivos compatíveis";
    strings["all files##sggu0"].plurals[0] = "todos os arquivos";
    strings["no backups made yet!##sggu"].plurals[0] = "nenhum backup realizado ainda!";
    strings["Restore Backup##sggu"].plurals[0] = "Restaurar Backup";
    strings["Furnace or Furnace-B song##sggu"].plurals[0] = "Música do Furnace ou Furnace-B";
    strings["Furnace song##sggu0"].plurals[0] = "Música do Furnace";
    strings["Save File##sggu0"].plurals[0] = "Salvar Arquivo";
    strings["DefleMask 1.1.3 module##sggu"].plurals[0] = "Módulo do DefleMask 1.1.3";
    strings["Save File##sggu1"].plurals[0] = "Salvar Arquivo";
    strings["DefleMask 1.0/legacy module##sggu"].plurals[0] = "Módulo do DefleMask 1.0/legado";
    strings["Save File##sggu2"].plurals[0] = "Salvar arquivo";
    strings["Furnace-B song##sggu"].plurals[0] = "Música do Furnace-B";
    strings["Load Instrument##sggu"].plurals[0] = "Carregar Instrumento";
    strings["all compatible files##sggu1"].plurals[0] = "todos os arquivos compatíveis";
    strings["Furnace instrument##sggu0"].plurals[0] = "Instrumento do Furnace";
    strings["DefleMask preset##sggu0"].plurals[0] = "Predefinição do DefleMask";
    strings["TFM Music Maker instrument##sggu"].plurals[0] = "Instrumento do TFM Music Maker";
    strings["VGM Music Maker instrument##sggu"].plurals[0] = "Instrumento do VGM Music Maker";
    strings["Scream Tracker 3 instrument##sggu"].plurals[0] = "Instrumento do Scream Tracker 3";
    strings["SoundBlaster instrument##sggu"].plurals[0] = "Instrumento SoundBlaster";
    strings["Wohlstand OPL instrument##sggu"].plurals[0] = "Instrumento Wohlstand OPL";
    strings["Wohlstand OPN instrument##sggu"].plurals[0] = "Instrumento Wohlstand OPN";
    strings["Gens KMod patch dump##sggu"].plurals[0] = "Extrato de patch Gens KMod";
    strings["BNK file (AdLib)##sggu"].plurals[0] = "Arquivo BNK (AdLib)";
    strings["FF preset bank##sggu"].plurals[0] = "Banco de predefinições do FF";
    strings["2612edit GYB preset bank##sggu"].plurals[0] = "Banco de predefinições GYB do 2612edit";
    strings["VOPM preset bank##sggu"].plurals[0] = "Banco de predefinições do VOPM";
    strings["Wohlstand WOPL bank##sggu"].plurals[0] = "Banco de predefinições Wohlstand WOPL";
    strings["Wohlstand WOPN bank##sggu"].plurals[0] = "Banco de predefinições Wohlstand WOPN";
    strings["all files##sggu1"].plurals[0] = "todos os arquivos";
    strings["Save Instrument##sggu0"].plurals[0] = "Salvar Instrumento";
    strings["Furnace instrument##sggu1"].plurals[0] = "Instrumento do Furnace";
    strings["Save Instrument##sggu1"].plurals[0] = "Salvar Instrumento";
    strings["DefleMask preset##sggu1"].plurals[0] = "Predefinição do DefleMask";
    strings["Load Wavetable##sggu"].plurals[0] = "Carregar Wavetable";
    strings["compatible files##sggu2"].plurals[0] = "arquivos compatíveis";
    strings["all files##sggu2"].plurals[0] = "todos os arquivos";
    strings["Save Wavetable##sggu0"].plurals[0] = "Salvar Wavetable";
    strings["Furnace wavetable##sggu"].plurals[0] = "Wavetable do Furnace";
    strings["Save Wavetable##sggu1"].plurals[0] = "Salvar Wavetable";
    strings["DefleMask wavetable##sggu"].plurals[0] = "Wavetable do DefleMask";
    strings["Save Wavetable##sggu2"].plurals[0] = "Salvar Wavetable";
    strings["raw data##sggu"].plurals[0] = "dados raw";
    strings["Load Sample##sggu"].plurals[0] = "Carregar Sample";
    strings["compatible files##sggu3"].plurals[0] = "arquivos compatíveis";
    strings["all files##sggu3"].plurals[0] = "todos os arquivos";
    strings["Load Raw Sample##sggu"].plurals[0] = "Carregar Sample em Formato Raw";
    strings["all files##sggu4"].plurals[0] = "todos os arquivos";
    strings["Save Sample##sggu"].plurals[0] = "Salvar Sample";
    strings["Wave file##sggu0"].plurals[0] = "Arquivo Wave";
    strings["Save Raw Sample##sggu"].plurals[0] = "Salvar Sample em Formato Raw";
    strings["all files##sggu5"].plurals[0] = "todos os arquivos";
    strings["Export Audio##sggu0"].plurals[0] = "Exportar Áudio";
    strings["Wave file##sggu1"].plurals[0] = "Arquivo Wave";
    strings["Export Audio##sggu1"].plurals[0] = "Exportar Áudio";
    strings["Wave file##sggu2"].plurals[0] = "Arquivo Wave";
    strings["Export Audio##sggu2"].plurals[0] = "Exportar Áudio";
    strings["Wave file##sggu3"].plurals[0] = "Arquivo Wave";
    strings["Export VGM##sggu"].plurals[0] = "Exportar VGM";
    strings["VGM file##sggu"].plurals[0] = "Arquivo VGM";
    strings["Export ZSM##sggu"].plurals[0] = "Exportar ZSM";
    strings["ZSM file##sggu"].plurals[0] = "Arquivo ZSM";
    strings["Export Command Stream##sggu0"].plurals[0] = "Exportar Fluxo de Comandos";
    strings["text file##sggu0"].plurals[0] = "arquivo de texto";
    strings["Export Command Stream##sggu1"].plurals[0] = "Exportar Fluxo de Comandos";
    strings["text file##sggu1"].plurals[0] = "arquivo de texto";
    strings["Export Command Stream##sggu2"].plurals[0] = "Exportar Fluxo de Comandos";
    strings["binary file##sggu"].plurals[0] = "arquivo binário";
    strings["Export Furnace song##sggu"].plurals[0] = "Exportar Música do Furnace";
    strings["Furnace song##sggu2"].plurals[0] = "Música do Furnace";
    strings["Coming soon!##sggu"].plurals[0] = "em breve!";
    strings["Select Font##sggu0"].plurals[0] = "Selecione a Fonte";
    strings["compatible files##sggu4"].plurals[0] = "arquivos compatíveis";
    strings["Select Font##sggu1"].plurals[0] = "Selecione a Fonte";
    strings["compatible files##sggu5"].plurals[0] = "arquivos compatíveis";
    strings["Select Font##sggu2"].plurals[0] = "Selecione a Fonte";
    strings["compatible files##sggu6"].plurals[0] = "arquivos compatíveis";
    strings["Select Color File##sggu"].plurals[0] = "Selecione o Arquivo de Cores";
    strings["configuration files##sggu0"].plurals[0] = "arquivos de configuração";
    strings["Select Keybind File##sggu"].plurals[0] = "Selecione o Arquivo de Teclas de Atalho";
    strings["configuration files##sggu1"].plurals[0] = "arquivos de configuração";
    strings["Select Layout File##sggu"].plurals[0] = "Selecione o Arquivo de Leiaute";
    strings[".ini files##sggu0"].plurals[0] = "arquivos .ini";
    strings["Export Colors##sggu"].plurals[0] = "Exportar Cores";
    strings["configuration files##sggu2"].plurals[0] = "arquivos de configuração";
    strings["Export Keybinds##sggu"].plurals[0] = "Exportar Teclas de Atalho";
    strings["configuration files##sggu3"].plurals[0] = "arquivos de configuração";
    strings["Export Layout##sggu"].plurals[0] = "Exportar Leiaute";
    strings[".ini files##sggu1"].plurals[0] = "arquivos .ini";
    strings["Load ROM##sggu"].plurals[0] = "Carregar ROM";
    strings["compatible files##sggu7"].plurals[0] = "arquivos compatíveis";
    strings["all files##sggu6"].plurals[0] = "todos os arquivos";
    strings["Play Command Stream##sggu"].plurals[0] = "Reproduzir Fluxo de Comandos";
    strings["command stream##sggu"].plurals[0] = "fluxo de comandos";
    strings["all files##sggu7"].plurals[0] = "todos os arquivos";
    strings["Open Test##sggu"].plurals[0] = "Abrir Teste";
    strings["compatible files##sggu8"].plurals[0] = "arquivos compatíveis";
    strings["another option##sggu0"].plurals[0] = "outra opção";
    strings["all files##sggu8"].plurals[0] = "todos os arquivos";
    strings["Open Test (Multi)##sggu"].plurals[0] = "Abrir Teste (Multi)";
    strings["compatible files##sggu9"].plurals[0] = "arquivos compatíveis";
    strings["another option##sggu1"].plurals[0] = "outra opção";
    strings["all files##sggu9"].plurals[0] = "todos os arquivos";
    strings["Save Test##sggu"].plurals[0] = "Salvar Teste";
    strings["Furnace song##sggu"].plurals[0] = "Música do Furnace";
    strings["DefleMask module##sggu"].plurals[0] = "Módulo do DefleMask";
    strings["you have loaded a backup!\nif you need to, please save it somewhere.\n\nDO NOT RELY ON THE BACKUP SYSTEM FOR AUTO-SAVE!\nFurnace will not save backups of backups.##sggu"].plurals[0] = "você carregou um backup!\nse você precisar, por favor salve-o em algum lugar.\n\nNÃO CONFIE NO SISTEMA DE BACKUP PARA SALVAMENTO AUTOMÁTICO!\nO Furnace não irá realizar backups de backups.";
    strings["cut##sggu"].plurals[0] = "recortar";
    strings["copy##sggu"].plurals[0] = "copiar";
    strings["paste##sggu0"].plurals[0] = "colar";
    strings["paste special...##sggu"].plurals[0] = "colar especial...";
    strings["paste mix##sggu"].plurals[0] = "colar e mesclar";
    strings["paste mix (background)##sggu"].plurals[0] = "colar e mesclar (por trás)";
    strings["paste with ins (foreground)##sggu"].plurals[0] = "colar+mesclar com instrumento (pela frente)";
    strings["no instruments available##sggu0"].plurals[0] = "nenhum instrumento disponível";
    strings["paste with ins (background)##sggu"].plurals[0] = "colar+mesclar com instrumento (por trás)";
    strings["no instruments available##sggu1"].plurals[0] = "nenhum instrumento disponível";
    strings["paste flood##sggu"].plurals[0] = "colar e repetir";
    strings["paste overflow##sggu"].plurals[0] = "colar através das bordas";
    strings["delete##sggu0"].plurals[0] = "excluir";
    strings["select all##sggu"].plurals[0] = "selecionar tudo";
    strings["operation mask...##sggu"].plurals[0] = "máscara de operação...";
    strings["delete##sggu1"].plurals[0] = "excluir";
    strings["pull delete##sggu"].plurals[0] = "excluir e puxar";
    strings["insert##sggu"].plurals[0] = "inserir";
    strings["paste##sggu1"].plurals[0] = "colar";
    strings["transpose (note)##sggu"].plurals[0] = "transpor (nota)";
    strings["transpose (value)##sggu"].plurals[0] = "transpor (valor)";
    strings["interpolate##sggu0"].plurals[0] = "interpolar";
    strings["fade##sggu"].plurals[0] = "degradê";
    strings["invert values##sggu0"].plurals[0] = "inverter valores";
    strings["scale##sggu"].plurals[0] = "escalonar";
    strings["randomize##sggu"].plurals[0] = "aleatorizar";
    strings["flip##sggu"].plurals[0] = "reverter";
    strings["collapse/expand##sggu"].plurals[0] = "contrair/expandir";
    strings["input latch##sggu"].plurals[0] = "máscara de entrada";
    strings["&&: selected instrument\n..: no instrument##sggu"].plurals[0] = "&&: instrumento selecionado\n..: nenhum instrumento";
    strings["Set##sggu"].plurals[0] = "Limpar"; // o comando "Set" limpa os valores do input latch
    strings["Reset##sggu"].plurals[0] = "Redefinir";
    strings["note up##sggu"].plurals[0] = "nota acima";
    strings["note down##sggu"].plurals[0] = "nota abaixo";
    strings["octave up##sggu"].plurals[0] = "oitava acima";
    strings["octave down##sggu"].plurals[0] = "oitava abaixo";
    strings["values up##sggu"].plurals[0] = "valores acima";
    strings["values down##sggu"].plurals[0] = "valores abaixo";
    strings["values up (+16)##sggu"].plurals[0] = "valores acima (+16)";
    strings["values down (-16)##sggu"].plurals[0] = "valores abaixo (-16)";
    strings["transpose##sggu"].plurals[0] = "transpor";
    strings["Notes##sggu"].plurals[0] = "Notas";
    strings["Values##sggu"].plurals[0] = "Valores";
    strings["interpolate##sggu1"].plurals[0] = "interpolar";
    strings["change instrument...##sggu"].plurals[0] = "mudar instrumento...";
    strings["no instruments available##sggu"].plurals[0] = "nenhum instrumento disponível";
    strings["gradient/fade...##sggu"].plurals[0] = "degradê...";
    strings["Start##sggu"].plurals[0] = "Início";
    strings["End##sggu"].plurals[0] = "Fim";
    strings["Nibble mode##sggu0"].plurals[0] = "Modo nibble";
    strings["Go ahead##sggu"].plurals[0] = "Vá em frente";
    strings["scale...##sggu"].plurals[0] = "escalonar...";
    strings["Scale##sggu"].plurals[0] = "Escalonar";
    strings["randomize...##sggu"].plurals[0] = "aleatorizar...";
    strings["Minimum##sggu"].plurals[0] = "Mínimo";
    strings["Maximum##sggu"].plurals[0] = "Máximo";
    strings["Nibble mode##sggu1"].plurals[0] = "Modo nibble";
    strings["Randomize##sggu"].plurals[0] = "Aleatorizar";
    strings["invert values##sggu1"].plurals[0] = "inverter valores";
    strings["flip selection##sggu"].plurals[0] = "reverter seleção";
    strings["collapse/expand amount##CollapseAmount"].plurals[0] = "contrair/expandir por quantidade##CollapseAmount";
    strings["collapse##sggu"].plurals[0] = "contrair";
    strings["expand##sggu"].plurals[0] = "expandir";
    strings["collapse pattern##sggu"].plurals[0] = "contrair sequência";
    strings["expand pattern##sggu"].plurals[0] = "expandir sequência";
    strings["collapse song##sggu"].plurals[0] = "contrair música";
    strings["expand song##sggu"].plurals[0] = "expandir música";
    strings["find/replace##sggu"].plurals[0] = "localizar/substituir";
    strings["Furnace has been started in Safe Mode.\nthis means that:\n\n- software rendering is being used\n- audio output may not work\n- font loading is disabled\n\ncheck any settings which may have made Furnace start up in this mode.\nfont loading is one of these.##sggu"].plurals[0] = "Furnace foi inicializado no Modo de Segurança.\nisso significa que:\n\n- renderização via software está sendo utilizada\n- a saída de áudio pode não funcionar\n- o carregamento de fontes está desabilitado\n\nconfira quaisquer opções que podem ter feito o Furnace iniciar nesse modo.\no carregamento de fontes é uma delas.";
    strings["Unsaved changes! Save changes before opening file?##sggu0"].plurals[0] = "Há mudanças não salvas! Salvar mudanças antes de abrir o arquivo?";
    strings["Unsaved changes! Save changes before opening file?##sggu2"].plurals[0] = "Há mudanças não salvas! Salvar mudanças antes de abrir o arquivo?";
    strings["Error while loading file! (%s)##sggu0"].plurals[0] = "Erro ao abrir o arquivo! (%s)";
    strings["Unsaved changes! Save changes before quitting?##sggu"].plurals[0] = "Há mudanças não salvas! Salvar mudanças antes de sair?";
    strings["error while loading fonts! please check your settings.##sggu0"].plurals[0] = "erro ao carregar fontes! por favor cheque suas configurações.";
    strings["File##menubar"].plurals[0] = "Arquivo##menubar";
    strings["file##menubar"].plurals[0] = "arquivo##menubar";
    strings["new...##sggu"].plurals[0] = "novo...";
    strings["Unsaved changes! Save changes before creating a new song?##sggu"].plurals[0] = "Há mudanças não salvas! Salvar mudanças antes de criar uma nova música?";
    strings["open...##sggu"].plurals[0] = "abrir...";
    strings["Unsaved changes! Save changes before opening another file?##sggu"].plurals[0] = "Há mudanças não salvas! Salvar mudanças antes de abrir outro arquivo?";
    strings["open recent##sggu"].plurals[0] = "abrir recente";
    strings["Unsaved changes! Save changes before opening file?##sggu1"].plurals[0] = "Há mudanças não salvas! Salvar mudanças antes de abrir o arquivo?";
    strings["Error while loading file! (%s)##sggu1"].plurals[0] = "Erro ao abrir o arquivo! (%s)";
    strings["nothing here yet##sggu"].plurals[0] = "nada aqui ainda";
    strings["clear history##sggu"].plurals[0] = "limpar histórico";
    strings["Are you sure you want to clear the recent file list?##sggu"].plurals[0] = "Tem certeza que quer limpar o histórico de arquivos recentes?";
    strings["save##sggu"].plurals[0] = "salvar";
    strings["Error while saving file! (%s)##sggu0"].plurals[0] = "Erro ao salvar o arquivo! (%s)";
    strings["save as...##sggu"].plurals[0] = "salvar como...";
    strings["export audio...##sggu0"].plurals[0] = "exportar áudio...";
    strings["export VGM...##sggu0"].plurals[0] = "exportar VGM...";
    strings["export .dmf (1.1.3+)...##sggu0"].plurals[0] = "exportar .dmf (1.1.3+)...";
    strings["export .dmf (1.0/legacy)...##sggu0"].plurals[0] = "exportar .dmf (1.0/legado)...";
    strings["export ZSM...##sggu0"].plurals[0] = "exportar ZSM...";
    strings["export Amiga validation data...##sggu0"].plurals[0] = "exportar dados de validação do Amiga...";
    strings["export text...##sggu0"].plurals[0] = "exportar texto...";
    strings["export command stream...##sggu0"].plurals[0] = "exportar fluxo de comandos...";
    strings["export Furnace module...##sggu"].plurals[0] = "exportar módulo do Furnace...";
    strings["export audio...##sggu1"].plurals[0] = "exportar audio...";
    strings["export VGM...##sggu1"].plurals[0] = "exportar VGM...";
    strings["export .dmf (1.1.3+)...##sggu1"].plurals[0] = "exportar .dmf (1.1.3+)...";
    strings["export .dmf (1.0/legacy)...##sggu1"].plurals[0] = "exportar .dmf (1.0/legado)...";
    strings["export ZSM...##sggu1"].plurals[0] = "exportar ZSM...";
    strings["export Amiga validation data...##sggu1"].plurals[0] = "exportar dados de validação do Amiga...";
    strings["export text...##sggu1"].plurals[0] = "exportar texto...";
    strings["export command stream...##sggu1"].plurals[0] = "exportar fluxo de comandos...";
    strings["export...##sggu"].plurals[0] = "exportar...";
    strings["manage chips##sggu"].plurals[0] = "gerenciar chips";
    strings["add chip...##sggu"].plurals[0] = "adicionar chip...";
    strings["cannot add chip! (##sggu"].plurals[0] = "não pôde adicionar chip! (";
    strings["configure chip...##sggu"].plurals[0] = "configurar chip...";
    strings["change chip...##sggu"].plurals[0] = "trocar chip...";
    strings["Preserve channel positions##sggu0"].plurals[0] = "Preservar posições dos canais";
    strings["remove chip...##sggu"].plurals[0] = "remover chip...";
    strings["Preserve channel positions##sggu1"].plurals[0] = "Preservar posições dos canais";
    strings["cannot remove chip! (##sggu"].plurals[0] = "não pôde remover chip! (";
    strings["cannot change chip! (##sggu"].plurals[0] = "não pôde trocar chip! (";
    strings["open built-in assets directory##sggu"].plurals[0] = "abrir diretório de recursos embutidos";
    strings["restore backup##sggu"].plurals[0] = "restaurar backup";
    strings["exit...##sggu"].plurals[0] = "sair...";
    strings["Unsaved changes! Save before quitting?##sggu"].plurals[0] = "Há mudanças não salvas! Salvar antes de sair?";
    strings["Edit##menubar"].plurals[0] = "Editar##menubar";
    strings["edit##menubar"].plurals[0] = "editar##menubar";
    strings["undo##sggu"].plurals[0] = "desfazer";
    strings["redo##sggu"].plurals[0] = "refazer";
    strings["clear...##sggu"].plurals[0] = "apagar...";
    strings["Settings##menubar"].plurals[0] = "Opções##menubar";
    strings["settings##menubar"].plurals[0] = "opções##menubar";
    strings["full screen##sggu"].plurals[0] = "tela cheia";
    strings["lock layout##sggu"].plurals[0] = "travar leiaute";
    strings["pattern visualizer##sggu"].plurals[0] = "visualizador de sequência";
    strings["reset layout##sggu"].plurals[0] = "redefinir leiaute";
    strings["Are you sure you want to reset the workspace layout?##sggu"].plurals[0] = "Tem certeza de que quer redefinir o leiaute do espaço de trabalho?";
    strings["switch to mobile view##sggu"].plurals[0] = "trocar para leiaute otimizado para dispositivos móveis";
    strings["settings...##sggu"].plurals[0] = "configurações...";
    strings["Window##menubar"].plurals[0] = "Janela##menubar";
    strings["window##menubar"].plurals[0] = "janela##menubar";
    strings["song information##sggu"].plurals[0] = "informações da música";
    strings["subsongs##sggu"].plurals[0] = "subfaixas";
    strings["speed##sggu"].plurals[0] = "velocidade";
    strings["assets##sggu"].plurals[0] = "recursos";
    strings["instruments##sggu"].plurals[0] = "instrumentos";
    strings["wavetables##sggu"].plurals[0] = "wavetables";
    strings["samples##sggu"].plurals[0] = "samples";
    strings["orders##sggu"].plurals[0] = "ordens";
    strings["pattern##sggu"].plurals[0] = "sequência";
    strings["mixer##sggu"].plurals[0] = "mixer";
    strings["grooves##sggu"].plurals[0] = "grooves";
    strings["channels##sggu"].plurals[0] = "canais";
    strings["pattern manager##sggu"].plurals[0] = "gerenciador de sequências";
    strings["chip manager##sggu"].plurals[0] = "gerenciador de chips";
    strings["compatibility flags##sggu"].plurals[0] = "opções de compatibilidade";
    strings["song##sggu"].plurals[0] = "música";
    strings["visualizers##sggu"].plurals[0] = "visualizadores";
    strings["tempo##sggu"].plurals[0] = "tempo";
    strings["debug##sggu"].plurals[0] = "depuração";
    strings["song comments##sggu"].plurals[0] = "comentários da música";
    strings["instrument editor##sggu"].plurals[0] = "editor de instrumentos";
    strings["wavetable editor##sggu"].plurals[0] = "editor de wavetables";
    strings["sample editor##sggu"].plurals[0] = "editor de samples";
    strings["play/edit controls##sggu"].plurals[0] = "controles de reprodução/edição";
    strings["piano/input pad##sggu"].plurals[0] = "piano/teclado virtual";
    strings["oscilloscope (master)##sggu"].plurals[0] = "osciloscópio (mestre)";
    strings["oscilloscope (per-channel)##sggu"].plurals[0] = "osciloscópio (por canal)";
    strings["oscilloscope (X-Y)##sggu"].plurals[0] = "osciloscópio (X-Y)";
    strings["volume meter##sggu"].plurals[0] = "medidor de volume";
    strings["clock##sggu"].plurals[0] = "clock";
    strings["register view##sggu"].plurals[0] = "visualização de registradores";
    strings["log viewer##sggu"].plurals[0] = "visualizador de logs";
    strings["statistics##sggu"].plurals[0] = "estatísticas";
    strings["memory composition##sggu"].plurals[0] = "composição da memória";
    strings["spoiler##sggu"].plurals[0] = "spoiler";
    strings["Help##menubar"].plurals[0] = "Ajuda##menubar";
    strings["help##menubar"].plurals[0] = "ajuda##menubar";
    strings["effect list##sggu"].plurals[0] = "lista de efeitos";
    strings["debug menu##sggu"].plurals[0] = "menu de depuração";
    strings["inspector##sggu"].plurals[0] = "inspetor";
    strings["panic##sggu"].plurals[0] = "pânico";
    strings["about...##sggu"].plurals[0] = "sobre...";
    strings["| Speed %d:%d##sggu"].plurals[0] = "| Velocidade %d:%d";
    strings["| Speed %d##sggu"].plurals[0] = "| Velocidade %d";
    strings["| Groove##sggu"].plurals[0] = "| Groove";
    strings[" @ %gHz (%g BPM) ##sggu"].plurals[0] = " em %gHz (%g BPM) ";
    strings["| Order %.2X/%.2X ##sggu"].plurals[0] = "| Ordem %.2X/%.2X ";
    strings["| Order %d/%d ##sggu"].plurals[0] = "| Ordem %d/%d ";
    strings["| Row %.2X/%.2X ##sggu"].plurals[0] = "| Linha %.2X/%.2X ";
    strings["| Row %d/%d ##sggu"].plurals[0] = "| Linha %d/%d ";
    strings["Don't you have anything better to do?##sggu"].plurals[0] = "Não tem mais o que fazer não?";
    strings["%d years ##sggu"].plurals[0] = "%d ano ";
    strings["%d years ##sggu"].plurals[1] = "%d anos ";
    strings["%d months ##sggu"].plurals[0] = "%d mês ";
    strings["%d months ##sggu"].plurals[1] = "%d meses ";
    strings["%d days ##sggu"].plurals[0] = "%d dia ";
    strings["%d days ##sggu"].plurals[1] = "%d dias ";
    strings["Note off (cut)##sggu"].plurals[0] = "Note off (corte)";
    strings["Note off (release)##sggu"].plurals[0] = "Note off (repouso)";
    strings["Macro release only##sggu"].plurals[0] = "Repouso de macro somente";
    strings["Note on: %s##sggu"].plurals[0] = "Note on: %s";
    strings["Ins %d: <invalid>##sggu"].plurals[0] = "Ins %d: <inválido>";
    strings["Ins %d: %s##sggu"].plurals[0] = "Ins %d: %s";
    strings["Set volume: %d (%.2X, INVALID!)##sggu"].plurals[0] = "Definir volume: %d (%.2X, INVÁLIDO!)";
    strings["Set volume: %d (%.2X, %d%%)##sggu"].plurals[0] = "Definir volume: %d (%.2X, %d%%)";
    strings["| modified##sggu"].plurals[0] = "| modificado";
    strings["there was an error in the file dialog! you may want to report this issue to:\nhttps://github.com/tildearrow/furnace/issues\ncheck the Log Viewer (window > log viewer) for more information.\n\nfor now please disable the system file picker in Settings > General.##sggu"].plurals[0] = "houve um erro no diálogo de arquivo! você talvez queira reportar esse problema em:\nhttps://github.com/tildearrow/furnace/issues\nconfira o Visualizador de Logs (janela > visualizador de logs) para mais informações.\n\npor enquanto, por favor desabilite o diálogo de arquivos do sistema em Configurações > Geral.";
    strings["can't do anything without Storage permissions!##sggu"].plurals[0] = "impossível fazer algo sem permissões de Armazenamento!";
    strings["Zenity/KDialog not available!\nplease install one of these, or disable the system file picker in Settings > General.##sggu"].plurals[0] = "Zenity/KDialog indisponíveis!\npor favor instale uma dessas bibliotecas, ou desabilite o diálogo de arquivos do sistema em Configurações > Geral.";
    strings["Error while loading file! (%s)##sggu2"].plurals[0] = "Erro ao carregar o arquivo! (%s)";
    strings["Error while saving file! (%s)##sggu1"].plurals[0] = "Erro ao salvar o arquivo! (%s)";
    strings["Error while loading file! (%s)##sggu3"].plurals[0] = "Erro ao carregar o arquivo! (%s)";
    strings["Error while saving file! (%s)##sggu2"].plurals[0] = "Erro ao salvar o arquivo! (%s)";
    strings["Error while saving file! (%s)##sggu3"].plurals[0] = "Erro ao salvar o arquivo! (%s)";
    strings["error while saving instrument! only the following instrument types are supported:\n- FM (OPN)\n- SN76489/Sega PSG\n- Game Boy\n- PC Engine\n- NES\n- C64\n- FM (OPLL)\n- FDS##sggu##sggu"].plurals[0] = "erro ao salvar o instrumento! somente os seguintes tipos de instrumento são suportados:\n- FM (OPN)\n- SN76489/Sega PSG\n- Game Boy\n- PC Engine\n- NES\n- C64\n- FM (OPLL)\n- FDS";
    strings["there were some errors while loading samples:\n#sggu"].plurals[0] = "ocorreram erros ao carregar as samples:\n";
    strings["...but you haven't selected a sample!##sggu0"].plurals[0] = "...mas você não selecionou uma sample!";
    strings["could not save sample! open Log Viewer for more information.##sggu0"].plurals[0] = "não foi possível salvar a sample! abra o Visualizador de Logs para mais informações.";
    strings["could not save sample! open Log Viewer for more information.##sggu1"].plurals[0] = "não foi possível salvar a sample! abra o Visualizador de Logs para mais informações.";
    strings["there were some warnings/errors while loading instruments:\n#sggu"].plurals[0] = "ocorreram alguns erros/avisos ao carregar os instrumentos:\n";
    strings["> %s: cannot load instrument! (%s)\n#sggu"].plurals[0] = "> %s: não pôde carregar o instrumento! (%s)\n";
    strings["...but you haven't selected an instrument!##sggu0"].plurals[0] = "...mas você não selecionou um instrumento!";
    strings["cannot load instrument! (##sggu"].plurals[0] = "não pôde carregar o instrumento! (";
    strings["congratulations! you managed to load nothing.\nyou are entitled to a bug report.##sggu"].plurals[0] = "parabéns! você foi capaz de carregar um monte de nada.\nvocê tem direito a um bug report.";
    strings["there were some errors while loading wavetables:\n##sggu"].plurals[0] = "ocorreram erros ao carregar as wavetables:\n";
    strings["cannot load wavetable! (##sggu"].plurals[0] = "não pôde carregar a wavetable! (";
    strings["...but you haven't selected a wavetable!##sggu"].plurals[0] = "...mas você não selecionou uma wavetable!";
    strings["could not open file!##sggu"].plurals[0] = "não pôde abrir o arquivo!";
    strings["Could not write ZSM! (%s)##sggu"].plurals[0] = "Não pôde escrever ZSM! (%s)";
    strings["could not write text! (%s)##sggu"].plurals[0] = "não pôde escrever texto! (%s)";
    strings["could not write command stream! (%s)##sggu"].plurals[0] = "não pôde escrever fluxo de comandos! (%s)";
    strings["could not write tildearrow version Furnace module! (%s)##sggu"].plurals[0] = "não pôde escrever módulo do Furnace versão tildearrow! (%s)";
    strings["Error while loading file! (%s)##sggu4"].plurals[0] = "Erro ao carregar o arquivo! (%s)";
    strings["You opened: %s##sggu"].plurals[0] = "Você abriu: %s";
    strings["You opened:##sggu"].plurals[0] = "Você abriu:##sggu";
    strings["You saved: %s##sggu"].plurals[0] = "Você salvou: %s";
    strings["Rendering...###Rendering..."].plurals[0] = "Renderizando...###Rendering...";
    strings["Please wait...##sggu"].plurals[0] = "Por favor aguarde...";
    strings["Abort##sggu"].plurals[0] = "Abortar";
    strings["New Song###New Song"].plurals[0] = "Nova Música###New Song";
    strings["Export###Export"].plurals[0] = "Exportar###Export";
    strings["Error###Error"].plurals[0] = "Erro###Error";
    strings["OK##sggu0"].plurals[0] = "OK";
    strings["Warning###Warning"].plurals[0] = "Aviso###Warning";
    strings["Yes##sggu0"].plurals[0] = "Sim";
    strings["No##sggu0"].plurals[0] = "Não";
    strings["Yes##sggu1"].plurals[0] = "Sim";
    strings["No##sggu1"].plurals[0] = "Não";
    strings["Yes##sggu2"].plurals[0] = "Sim";
    strings["No##sggu2"].plurals[0] = "Não";
    strings["Yes##sggu3"].plurals[0] = "Sim";
    strings["No##sggu3"].plurals[0] = "Não";
    strings["Yes##sggu7"].plurals[0] = "Sim";
    strings["Yes##sggu8"].plurals[0] = "Sim";
    strings["Yes##sggu9"].plurals[0] = "Sim";
    strings["Yes##sggu10"].plurals[0] = "Sim";
    strings["Yes##sggu11"].plurals[0] = "Sim";
    strings["No##sggu8"].plurals[0] = "Não";
    strings["No##sggu9"].plurals[0] = "Não";
    strings["No##sggu10"].plurals[0] = "Não";
    strings["No##sggu11"].plurals[0] = "Não";
    strings["No##sggu12"].plurals[0] = "Não";
    strings["Cancel##sggu4"].plurals[0] = "Cancelar";
    strings["Cancel##sggu5"].plurals[0] = "Cancelar";
    strings["Cancel##sggu6"].plurals[0] = "Cancelar";
    strings["Cancel##sggu7"].plurals[0] = "Cancelar";
    strings["Cancel##sggu8"].plurals[0] = "Cancelar";
    strings["Cancel##sggu0"].plurals[0] = "Cancelar";
    strings["Erasing##sggu"].plurals[0] = "Apagando";
    strings["All subsongs##sggu"].plurals[0] = "Todas as subfaixas";
    strings["Current subsong##sggu"].plurals[0] = "Subfaixa atual";
    strings["Orders##sggu"].plurals[0] = "Ordens";
    strings["Pattern##sggu"].plurals[0] = "Sequência";
    strings["Instruments##sggu"].plurals[0] = "Instrumentos";
    strings["Wavetables##sggu"].plurals[0] = "Wavetables";
    strings["Samples##sggu"].plurals[0] = "Samples";
    strings["Optimization##sggu"].plurals[0] = "Otimização";
    strings["De-duplicate patterns##sggu"].plurals[0] = "Deduplicar sequências";
    strings["Remove unused instruments##sggu"].plurals[0] = "Remover instrumentos não utilizados";
    strings["Remove unused samples##sggu"].plurals[0] = "Remover samples não utilizadas";
    strings["Never mind! Cancel##sggu1"].plurals[0] = "Deixa pra lá! Cancelar";
    strings["Yes##sggu4"].plurals[0] = "Sim";
    strings["No##sggu4"].plurals[0] = "Não";
    strings["Yes##sggu5"].plurals[0] = "Sim";
    strings["No##sggu5"].plurals[0] = "Não";
    strings["Yes##sggu6"].plurals[0] = "Sim";
    strings["No##sggu6"].plurals[0] = "Não";
    strings["OK##sggu1"].plurals[0] = "OK";
    strings["Drum kit mode:##sggu"].plurals[0] = "Modo de kit de bateria:";
    strings["Normal##sggu"].plurals[0] = "Normal";
    strings["12 samples per octave##sggu"].plurals[0] = "12 samples por oitava";
    strings["Starting octave##sggu"].plurals[0] = "Oitava inicial";
    strings["too many instruments!##sggu"].plurals[0] = "há muitos instrumentos!";
    strings["too many wavetables!##sggu"].plurals[0] = "há muitas wavetables!";
    strings["Select Instrument###Select Instrument"].plurals[0] = "Selecionar Instrumento###Select Instrument";
    strings["this is an instrument bank! select which one to use:##sggu"].plurals[0] = "isto é um banco de instrumentos! selecione quais instrumentos carregar:";
    strings["this is an instrument bank! select which ones to load:##sggu"].plurals[0] = "isto é um banco de instrumentos! selecione quais instrumentos carregar:";
    strings["All##sggu"].plurals[0] = "Todos";
    strings["None##sggu"].plurals[0] = "Nenhum";
    strings["OK##sggu2"].plurals[0] = "OK";
    strings["Cancel##sggu2"].plurals[0] = "Cancelar";
    strings["...but you haven't selected an instrument!##sggu1"].plurals[0] = "...mas você não selecionou um instrumento!";
    strings["Import Raw Sample###Import Raw Sample"].plurals[0] = "Importar Sample em Formato Raw###Import Raw Sample";
    strings["Data type:##sggu"].plurals[0] = "Tipo de dado:";
    strings["Sample rate##sggu"].plurals[0] = "Taxa de amostragem";
    strings["Channels##sggu"].plurals[0] = "Canais";
    strings["(will be mixed down to mono)##sggu"].plurals[0] = "(será misturado para mono)";
    strings["Unsigned##sggu"].plurals[0] = "Sem sinal";
    strings["Big endian##sggu"].plurals[0] = "Big endian";
    strings["Swap nibbles##sggu"].plurals[0] = "Trocar nibbles";
    strings["Swap words##sggu"].plurals[0] = "Trocar palavras";
    strings["Encoding:##sggu"].plurals[0] = "Codificação:";
    strings["Reverse bit order##sggu"].plurals[0] = "Trocar ordem dos bits";
    strings["OK##sggu3"].plurals[0] = "OK";
    strings["...but you haven't selected a sample!##sggu1"].plurals[0] = "...mas você não selecionou uma sample!";
    strings["Cancel##sggu3"].plurals[0] = "Cancelar";
    strings["Error! No string provided!##sggu"].plurals[0] = "Erro! Não foi passada uma string!";
    strings["OK##sggu4"].plurals[0] = "OK";
    strings["error while loading fonts! please check your settings.##sggu1"].plurals[0] = "erro ao carregar fontes! por favor cheque suas configurações.";
    strings["it appears I couldn't load these fonts. any setting you can check?##sggu"].plurals[0] = "parece que eu não pude carregar essas fontes. alguma configuração que você poderia checar?";
    strings["could not init renderer! %s##sggu"].plurals[0] = "não pôde inicializar o renderizador! %s";
    strings["\r\nthe render driver has been set to a safe value. please restart Furnace.##sggu"].plurals[0] = "\r\no driver de renderização foi definido para um valor seguro. por favor reinicie o Furnace.";
    strings["could not open window! %s##sggu"].plurals[0] = "não pôde abrir a janela! %s";
    strings["error while loading fonts! please check your settings.##sggu2"].plurals[0] = "erro ao carregar fontes! por favor cheque suas configurações.";
    strings["could NOT save layout! %s##sggu"].plurals[0] = "não pôde salvar o layout! %s";

    //   sggc  src/gui/guiConst.cpp

    strings["Generic Sample##sggc"].plurals[0] = "Sample Genérica";
    strings["Beeper##sggc"].plurals[0] = "Beeper";
    strings["VRC6 (saw)##sggc"].plurals[0] = "VRC6 (dente-de-serra)";
    strings["OPL (drums)##sggc"].plurals[0] = "OPL (bateria)";
    strings["PowerNoise (noise)##sggc"].plurals[0] = "PowerNoise (ruído)";
    strings["PowerNoise (slope)##sggc"].plurals[0] = "PowerNoise (rampa)";
    strings["Forward##sggc"].plurals[0] = "Para frente";
    strings["Backward##sggc"].plurals[0] = "Para trás";
    strings["Ping pong##sggc"].plurals[0] = "Pingue pongue";
    strings["1-bit PCM##sggc"].plurals[0] = "PCM 1-bit";
    strings["1-bit DPCM##sggc"].plurals[0] = "DPCM 1-bit";
    strings["8-bit PCM##sggc"].plurals[0] = "PCM 8-bit";
    strings["8-bit µ-law PCM##sggc"].plurals[0] = "PCM µ-law 8-bit";
    strings["16-bit PCM##sggc"].plurals[0] = "PCM 16-bit";
    strings["none##sggc"].plurals[0] = "nenhuma";
    strings["linear##sggc"].plurals[0] = "linear";
    strings["cubic spline##sggc"].plurals[0] = "spline cúbica";
    strings["blep synthesis##sggc"].plurals[0] = "síntese blep";
    strings["sinc##sggc"].plurals[0] = "sinc";
    strings["best possible##sggc"].plurals[0] = "melhor possível";
    strings["Pitch##sggc"].plurals[0] = "Tom";
    strings["Song##sggc"].plurals[0] = "Música";
    strings["Time##sggc"].plurals[0] = "Tempo";
    strings["Speed##sggc0"].plurals[0] = "Velocidade";
    strings["Panning##sggc"].plurals[0] = "Balanço";
    strings["Volume##sggc"].plurals[0] = "Volume";
    strings["System (Primary)##sggc"].plurals[0] = "Sistema (Primário)";
    strings["System (Secondary)##sggc"].plurals[0] = "Sistema (Secundário)";
    strings["Miscellaneous##sggc"].plurals[0] = "Miscelânea";
    strings["Invalid##sggc"].plurals[0] = "Inválido";

    strings["---Global##sggc"].plurals[0] = "---Global";
    strings["New##sggc"].plurals[0] = "Novo";
    strings["Open file##sggc"].plurals[0] = "Abrir arquivo";
    strings["Restore backup##sggc"].plurals[0] = "Restaurar backup";
    strings["Save file##sggc"].plurals[0] = "Salvar arquivo";
    strings["Save as##sggc"].plurals[0] = "Salvar como";
    strings["Export##sggc"].plurals[0] = "Exportar";
    strings["Undo##sggc"].plurals[0] = "Desfazer";
    strings["Redo##sggc"].plurals[0] = "Refazer";
    strings["Exit##sggc"].plurals[0] = "Sair";
    strings["Play/Stop (toggle)##sggc"].plurals[0] = "Reproduzir/Parar (alternar)";
    strings["Play##sggc"].plurals[0] = "Reproduzir";
    strings["Stop##sggc"].plurals[0] = "Parar";
    strings["Play (from beginning)##sggc"].plurals[0] = "Reproduzir (do início)";
    strings["Play (repeat pattern)##sggc"].plurals[0] = "Reproduzir (repetir sequência)";
    strings["Play from cursor##sggc"].plurals[0] = "Reproduzir do cursor";
    strings["Step row##sggc"].plurals[0] = "Pular linha";
    strings["Octave up##sggc"].plurals[0] = "Subir uma oitava";
    strings["Octave down##sggc"].plurals[0] = "Descer uma oitava";
    strings["Previous instrument##sggc"].plurals[0] = "Instrumento anterior";
    strings["Next instrument##sggc"].plurals[0] = "Próximo instrumento";
    strings["Increase edit step##sggc"].plurals[0] = "Aumentar passo de edição";
    strings["Decrease edit step##sggc"].plurals[0] = "Diminuir passo de edição";
    strings["Toggle edit mode##sggc"].plurals[0] = "Alternar modo de edição";
    strings["Metronome##sggc"].plurals[0] = "Metrônomo";
    strings["Toggle repeat pattern##sggc"].plurals[0] = "Alternar repetição de sequência";
    strings["Follow orders##sggc"].plurals[0] = "Seguir ordens";
    strings["Follow pattern##sggc"].plurals[0] = "Seguir sequência";
    strings["Toggle full-screen##sggc"].plurals[0] = "Alternar tela cheia";
    strings["Request voice from TX81Z##sggc"].plurals[0] = "Requisitar timbre do TX81Z";
    strings["Panic##sggc"].plurals[0] = "Pânico";
    strings["Clear song data##sggc"].plurals[0] = "Apagar dados da música";
    strings["Command Palette##sggc"].plurals[0] = "Paleta de Comandos";
    strings["Recent files (Palette)##sggc"].plurals[0] = "Arquivos recentes (Paleta)";
    strings["Instruments (Palette)##sggc"].plurals[0] = "Instrumentos (Paleta)";
    strings["Samples (Palette)##sggc"].plurals[0] = "Samples (Paleta)";
    strings["Change instrument (Palette)##sggc"].plurals[0] = "Trocar instrumento (Paleta)";
    strings["Add chip (Palette)##sggc"].plurals[0] = "Adicionar chip (Paleta)";
    strings["Edit Controls##sggc"].plurals[0] = "Controles de Edição";
    strings["Orders##sggc"].plurals[0] = "Ordens";
    strings["Instrument List##sggc"].plurals[0] = "Lista de Instrumentos";
    strings["Instrument Editor##sggc"].plurals[0] = "Editor de Instrumentos";
    strings["Song Information##sggc"].plurals[0] = "Informações da Música";
    strings["Speed##sggc1"].plurals[0] = "Velocidade";
    strings["Pattern##sggc"].plurals[0] = "Sequência";
    strings["Wavetable List##sggc"].plurals[0] = "Lista de Wavetables";
    strings["Wavetable Editor##sggc"].plurals[0] = "Editor de Wavetable";
    strings["Sample List##sggc"].plurals[0] = "Lista de Samples";
    strings["Sample Editor##sggc"].plurals[0] = "Editor de Sample";
    strings["About##sggc"].plurals[0] = "Sobre";
    strings["Settings##sggc"].plurals[0] = "Configurações";
    strings["Mixer##sggc"].plurals[0] = "Mixer";
    strings["Debug Menu##sggc"].plurals[0] = "Menu de Depuração";
    strings["Oscilloscope (master)##sggc"].plurals[0] = "Osciloscópio (mestre)";
    strings["Volume Meter##sggc"].plurals[0] = "Medidor de Volume";
    strings["Statistics##sggc"].plurals[0] = "Estatísticas";
    strings["Compatibility Flags##sggc"].plurals[0] = "Opções de Compatibilidade";
    strings["Piano##sggc"].plurals[0] = "Piano";
    strings["Song Comments##sggc"].plurals[0] = "Comentários da Música";
    strings["Channels##sggc"].plurals[0] = "Canais";
    strings["Pattern Manager##sggc"].plurals[0] = "Gerenciador de Sequências";
    strings["Chip Manager##sggc"].plurals[0] = "Gerenciador de Chips";
    strings["Register View##sggc"].plurals[0] = "Visualização de Registradores";
    strings["Log Viewer##sggc"].plurals[0] = "Visualizador de Logs";
    strings["Effect List##sggc"].plurals[0] = "Lista de Efeitos";
    strings["Oscilloscope (per-channel)##sggc"].plurals[0] = "Osciloscópio (por canal)";
    strings["Subsongs##sggc"].plurals[0] = "Subfaixas";
    strings["Find/Replace##sggc"].plurals[0] = "Localizar/Substituir";
    strings["Clock##sggc"].plurals[0] = "Clock";
    strings["Grooves##sggc"].plurals[0] = "Grooves";
    strings["Oscilloscope (X-Y)##sggc"].plurals[0] = "Osciloscópio (X-Y)";
    strings["Memory Composition##sggc"].plurals[0] = "Composição da Memória";
    strings["Command Stream Player##sggc"].plurals[0] = "Reprodutor de Fluxo de Comandos";
    strings["Collapse/expand current window##sggc"].plurals[0] = "Contrair/expandir janela atual";
    strings["Close current window##sggc"].plurals[0] = "Fechar janela atual";
    strings["---Pattern##sggc"].plurals[0] = "---Sequência";
    strings["Transpose (+1)##sggc"].plurals[0] = "Transpor (+1)";
    strings["Transpose (-1)##sggc"].plurals[0] = "Transpor (-1)";
    strings["Transpose (+1 octave)##sggc"].plurals[0] = "Transpor (+1 oitava)";
    strings["Transpose (-1 octave)##sggc"].plurals[0] = "Transpor (-1 oitava)";
    strings["Increase values (+1)##sggc"].plurals[0] = "Alterar valores (+1)";
    strings["Increase values (-1)##sggc"].plurals[0] = "Alterar valores (-1)";
    strings["Increase values (+16)##sggc"].plurals[0] = "Alterar valores (+16)";
    strings["Increase values (-16)##sggc"].plurals[0] = "Alterar valores (-16)";
    strings["Select all##sggc0"].plurals[0] = "Selecionar tudo";
    strings["Cut##sggc0"].plurals[0] = "Recortar";
    strings["Copy##sggc0"].plurals[0] = "Copiar";
    strings["Paste##sggc0"].plurals[0] = "Colar";
    strings["Paste Mix (foreground)##sggc"].plurals[0] = "Colar e Mesclar (pela frente)";
    strings["Paste Mix (background)##sggc"].plurals[0] = "Colar e Mesclar (por trás)";
    strings["Paste Flood##sggc"].plurals[0] = "Colar e Repetir";
    strings["Paste Overflow##sggc"].plurals[0] = "Colar Através das Bordas";
    strings["Move cursor up##sggc"].plurals[0] = "Mover cursor para cima";
    strings["Move cursor down##sggc"].plurals[0] = "Mover cursor para baixo";
    strings["Move cursor left##sggc"].plurals[0] = "Mover cursor para a esquerda";
    strings["Move cursor right##sggc"].plurals[0] = "Mover cursor para a direita";
    strings["Move cursor up by one (override Edit Step)##sggc"].plurals[0] = "Mover cursor uma célula para cima (ignora o Passo de Edição)";
    strings["Move cursor down by one (override Edit Step)##sggc"].plurals[0] = "Mover cursor uma célula para baixo (ignora o Passo de Edição)";
    strings["Move cursor to previous channel##sggc"].plurals[0] = "Mover cursor para o canal anterior";
    strings["Move cursor to next channel##sggc"].plurals[0] = "Mover cursor para o próximo canal";
    strings["Move cursor to next channel (overflow)##sggc"].plurals[0] = "Mover cursor para o próximo canal (através da borda)";
    strings["Move cursor to previous channel (overflow)##sggc"].plurals[0] = "Mover cursor para o canal anterior (através da borda)";
    strings["Move cursor to beginning of pattern##sggc"].plurals[0] = "Mover cursor para o começo da sequência";
    strings["Move cursor to end of pattern##sggc"].plurals[0] = "Mover cursor para o fim da sequência";
    strings["Move cursor up (coarse)##sggc"].plurals[0] = "Mover cursor para cima (grosso)";
    strings["Move cursor down (coarse)##sggc"].plurals[0] = "Mover cursor para baixo (grosso)";
    strings["Expand selection upwards##sggc"].plurals[0] = "Expandir seleção para cima";
    strings["Expand selection downwards##sggc"].plurals[0] = "Expandir seleção para baixo";
    strings["Expand selection to the left##sggc"].plurals[0] = "Expandir seleção para a esquerda";
    strings["Expand selection to the right##sggc"].plurals[0] = "Expandir seleção para a direita";
    strings["Expand selection upwards by one (override Edit Step)##sggc"].plurals[0] = "Expandir seleção uma célula para cima (ignora o Passo de Edição)";
    strings["Expand selection downwards by one (override Edit Step)##sggc"].plurals[0] = "Expandir seleção uma célula para baixo (ignora o Passo de Edição)";
    strings["Expand selection to beginning of pattern##sggc"].plurals[0] = "Expandir seleção para o começo da sequência";
    strings["Expand selection to end of pattern##sggc"].plurals[0] = "Expandir seleção para o fim da sequência";
    strings["Expand selection upwards (coarse)##sggc"].plurals[0] = "Expandir seleção para cima (grosso)";
    strings["Expand selection downwards (coarse)##sggc"].plurals[0] = "Expandir seleção para baixo (grosso)";
    strings["Delete##sggc"].plurals[0] = "Excluir";
    strings["Pull delete##sggc"].plurals[0] = "Excluir e puxar";
    strings["Insert##sggc"].plurals[0] = "Inserir";
    strings["Mute channel at cursor##sggc"].plurals[0] = "Silenciar canal sob o cursor";
    strings["Solo channel at cursor##sggc"].plurals[0] = "Isolar canal sob o cursor";
    strings["Unmute all channels##sggc"].plurals[0] = "Reativar todos os canais";
    strings["Go to next order##sggc"].plurals[0] = "Ir para a próxima ordem";
    strings["Go to previous order##sggc"].plurals[0] = "Ir para a ordem anterior";
    strings["Collapse channel at cursor##sggc"].plurals[0] = "Minimizar canal sob o cursor";
    strings["Increase effect columns##sggc"].plurals[0] = "Aumentar número de colunas de efeito";
    strings["Decrease effect columns##sggc"].plurals[0] = "Diminuir número de colunas de efeito";
    strings["Interpolate##sggc"].plurals[0] = "Interpolar";
    strings["Fade##sggc"].plurals[0] = "Degradê";
    strings["Invert values##sggc"].plurals[0] = "Inverter valores";
    strings["Flip selection##sggc"].plurals[0] = "Reverter seleção";
    strings["Collapse rows##sggc"].plurals[0] = "Contrair linhas";
    strings["Expand rows##sggc"].plurals[0] = "Expandir linhas";
    strings["Collapse pattern##sggc"].plurals[0] = "Contrair sequência";
    strings["Expand pattern##sggc"].plurals[0] = "Expandir sequência";
    strings["Collapse song##sggc"].plurals[0] = "Contrair música";
    strings["Expand song##sggc"].plurals[0] = "Expandir música";
    strings["Set note input latch##sggc"].plurals[0] = "Limpar máscara de entrada de nota"; // o comando "Set" limpa os valores do input latch
    strings["Change mobile scroll mode##sggc"].plurals[0] = "Alterar modo de rolagem para dispositivos móveis";
    strings["Clear note input latch##sggc"].plurals[0] = "Redefinir máscara de entrada de nota";
    strings["---Instrument list##sggc"].plurals[0] = "---Lista de instrumentos";
    strings["Add instrument##sggc0"].plurals[0] = "Adicionar instrumento";
    strings["Duplicate instrument##sggc0"].plurals[0] = "Duplicar instrumento";
    strings["Open instrument##sggc0"].plurals[0] = "Abrir instrumento";
    strings["Open instrument (replace current)##sggc0"].plurals[0] = "Abrir instrumento (substituir o atual)";
    strings["Save instrument##sggc0"].plurals[0] = "Salvar instrumento";
    strings["Export instrument (.dmp)##sggc"].plurals[0] = "Exportar instrumento (.dmp)";
    strings["Move instrument up in list##sggc0"].plurals[0] = "Mover instrumento para cima na lista";
    strings["Move instrument down in list##sggc0"].plurals[0] = "Mover instrumento para baixo na lista";
    strings["Delete instrument##sggc0"].plurals[0] = "Excluir instrumento";
    strings["Edit instrument##sggc0"].plurals[0] = "Editar instrumento";
    strings["Instrument cursor up##sggc0"].plurals[0] = "Mover cursor de instrumentos para cima";
    strings["Instrument cursor down##sggc0"].plurals[0] = "Mover cursor de instrumentos para baixo";
    strings["Instruments: toggle folders/standard view##sggc0"].plurals[0] = "Instrumentos: Alternar entre pastas e visão padrão";
    strings["---Wavetable list##sggc"].plurals[0] = "---Lista de wavetables";
    strings["Add wavetable##sggc1"].plurals[0] = "Adicionar wavetable";
    strings["Duplicate wavetable##sggc1"].plurals[0] = "Duplicar wavetable";
    strings["Open wavetable##sggc1"].plurals[0] = "Abrir wavetable";
    strings["Open wavetable (replace current)##sggc1"].plurals[0] = "Abrir wavetable (substituir a atual)";
    strings["Save wavetable##sggc1"].plurals[0] = "Salvar wavetable";
    strings["Save wavetable (.dmw)##sggc"].plurals[0] = "Salvar wavetable (.dmw)";
    strings["Save wavetable (raw)##sggc0"].plurals[0] = "Salvar wavetable (formato raw)";
    strings["Move wavetable up in list##sggc1"].plurals[0] = "Mover wavetable para cima na lista";
    strings["Move wavetable down in list##sggc1"].plurals[0] = "Mover wavetable para baixo na lista";
    strings["Delete wavetable##sggc1"].plurals[0] = "Excluir wavetable";
    strings["Edit wavetable##sggc1"].plurals[0] = "Editar wavetable";
    strings["Wavetable cursor up##sggc1"].plurals[0] = "Mover cursor de wavetables para cima";
    strings["Wavetable cursor down##sggc1"].plurals[0] = "Mover cursor de wavetables para baixo";
    strings["Wavetables: toggle folders/standard view##sggc1"].plurals[0] = "Wavetables: Alternar entre pastas e visão padrão";
    strings["Paste wavetables from clipboard##sggc"].plurals[0] = "Colar wavetables da área de transferência";
    strings["Paste local wavetables from clipboard##sggc1"].plurals[0] = "Colar wavetables locais da área de transferência";
    strings["---Sample list##sggc"].plurals[0] = "---Lista de samples";
    strings["Add sample##sggc2"].plurals[0] = "Adicionar sample";
    strings["Duplicate sample##sggc2"].plurals[0] = "Duplicar sample";
    strings["Open sample##sggc2"].plurals[0] = "Abrir sample";
    strings["Open sample (replace current)##sggc2"].plurals[0] = "Abrir sample (substituir a atual)";
    strings["Import raw sample data##sggc"].plurals[0] = "Importar dados de sample em formato raw";
    strings["Import raw sample data (replace current)##sggc"].plurals[0] = "Importar dados de sample em formato raw (substituir a atual)";
    strings["Save sample##sggc2"].plurals[0] = "Salvar sample";
    strings["Save sample (raw)##sggc1"].plurals[0] = "Salvar sample (formato raw)";
    strings["Move sample up##sggc2"].plurals[0] = "Mover sample para cima";
    strings["Move sample down##sggc2"].plurals[0] = "Mover sample para baixo";
    strings["Delete sample##sggc2"].plurals[0] = "Excluir sample";
    strings["Edit sample##sggc2"].plurals[0] = "Editar sample";
    strings["Sample cursor up##sggc2"].plurals[0] = "Mover cursor de samples para cima";
    strings["Sample cursor down##sggc2"].plurals[0] = "Mover cursor de samples para baixo";
    strings["Sample review##sggc"].plurals[0] = "Prever sample";
    strings["Stop sample preview##sggc"].plurals[0] = "Interromper previsão de sample";
    strings["Toggle folders/standard view##sggc2"].plurals[0] = "Samples: Alternar entre pastas e visão padrão";
    strings["Make me a drum kit##sggc"].plurals[0] = "Samples: Faça um kit de bateria para mim";
    strings["---Sample editor##sggc"].plurals[0] = "---Editor de sample";
    strings["Sample editor mode: Select##sggc"].plurals[0] = "Modo de edição: Selecionar";
    strings["Sample editor mode: Draw##sggc"].plurals[0] = "Modo de edição: Desenhar";
    strings["Sample editor: Cut##sggc1"].plurals[0] = "Editor de sample: Recortar";
    strings["Sample editor: Copy##sggc1"].plurals[0] = "Editor de sample: Copiar";
    strings["Sample editor: Paste##sggc1"].plurals[0] = "Editor de sample: Colar";
    strings["Sample editor: Paste replace##sggc"].plurals[0] = "Editor de sample: Colar e substituir";
    strings["Sample editor: Paste mix##sggc"].plurals[0] = "Editor de sample: Colar e mesclar";
    strings["Sample editor: Select all##sggc1"].plurals[0] = "Editor de sample: Selecionar tudo";
    strings["Sample editor: Resize##sggc"].plurals[0] = "Editor de sample: Redimensionar";
    strings["Sample editor: Resample##sggc"].plurals[0] = "Editor de sample: Reamostrar";
    strings["Sample editor: Amplify##sggc"].plurals[0] = "Editor de sample: Amplificar";
    strings["Sample editor: Normalize##sggc"].plurals[0] = "Editor de sample: Normalizar";
    strings["Sample editor: Fade in##sggc"].plurals[0] = "Editor de sample: Fade in";
    strings["Sample editor: Fade out##sggc"].plurals[0] = "Editor de sample: Fade out";
    strings["Sample editor: Apply silence##sggc"].plurals[0] = "Editor de sample: Aplicar silêncio";
    strings["Sample editor: Insert silence##sggc"].plurals[0] = "Editor de sample: Inserir silêncio";
    strings["Sample editor: Delete##sggc3"].plurals[0] = "Editor de sample: Excluir";
    strings["Sample editor: Trim##sggc"].plurals[0] = "Editor de sample: Cortar";
    strings["Sample editor: Reverse##sggc"].plurals[0] = "Editor de sample: Reverter";
    strings["Sample editor: Invert##sggc"].plurals[0] = "Editor de sample: Inverter";
    strings["Sample editor: Signed/unsigned exchange##sggc"].plurals[0] = "Editor de sample: Alternar sinalização (signed/unsigned)";
    strings["Sample editor: Apply filter##sggc"].plurals[0] = "Editor de sample: Aplicar filtro";
    strings["Sample editor: Crossfade loop points##sggc"].plurals[0] = "Editor de sample: Aplicar crossfade nos pontos de loop";
    strings["Sample editor: Preview sample##sggc"].plurals[0] = "Editor de sample: Prever sample";
    strings["Sample editor: Stop sample preview##sggc"].plurals[0] = "Editor de sample: Interromper previsão da sample";
    strings["Sample editor: Zoom in##sggc"].plurals[0] = "Editor de sample: Aumentar zoom";
    strings["Sample editor: Zoom out##sggc"].plurals[0] = "Editor de sample: Diminuir zoom";
    strings["Sample editor: Toggle auto-zoom##sggc"].plurals[0] = "Editor de sample: Alternar zoom automático";
    strings["Sample editor: Create instrument from sample##sggc"].plurals[0] = "Editor de sample: Criar instrumento a partir da sample";
    strings["Sample editor: Set loop to selection##sggc"].plurals[0] = "Editor de sample: Definir loop a partir da seleção";
    strings["Sample editor: Create wavetable from selection##sggc"].plurals[0] = "Editor de sample: Criar wavetable a partir da seleção";
    strings["---Orders##sggc"].plurals[0] = "---Ordens";
    strings["Previous order##sggc"].plurals[0] = "Ordem anterior";
    strings["Next order##sggc"].plurals[0] = "Próxima ordem";
    strings["Order cursor left##sggc"].plurals[0] = "Mover cursor de ordem para a esquerda";
    strings["Order cursor right##sggc"].plurals[0] = "Mover cursor de ordem para a direita";
    strings["Increase order value##sggc"].plurals[0] = "Aumentar valor da ordem";
    strings["Decrease order value##sggc"].plurals[0] = "Diminuir valor da ordem";
    strings["Switch order edit mode##sggc"].plurals[0] = "Alternar modo de edição da ordem";
    strings["Order: toggle alter entire row##sggc"].plurals[0] = "Ordem: alternar modo de alteração na linha inteira";
    strings["Add order##sggc3"].plurals[0] = "Adicionar ordem";
    strings["Duplicate order##sggc3"].plurals[0] = "Duplicar ordem";
    strings["Deep clone order##sggc"].plurals[0] = "Fazer clone profundo da ordem";
    strings["Copy current order to end of song##sggc"].plurals[0] = "Copiar ordem atual para o final da música";
    strings["Deep clone current order to end of song##sggc"].plurals[0] = "Fazer clone profundo da ordem ao final da música";
    strings["Remove order##sggc"].plurals[0] = "Excluir ordem";
    strings["Move order up##sggc3"].plurals[0] = "Mover ordem para cima";
    strings["Move order down##sggc3"].plurals[0] = "Mover ordem para baixo";
    strings["Replay order##sggc"].plurals[0] = "Reiniciar reprodução da ordem atual";

    strings["All chips##sggc"].plurals[0] = "Todos os chips";
    strings["Square##sggc"].plurals[0] = "Onda quadrada";
    strings["Wavetable##sggc"].plurals[0] = "Wavetable";
    strings["Special##sggc"].plurals[0] = "Especial";
    strings["Sample##sggc"].plurals[0] = "Sample";
    strings["Modern/fantasy##sggc"].plurals[0] = "Moderno/fantasia";

    //   sgda  src/gui/doAction.cpp

    strings["Unsaved changes! Save changes before creating a new song?##sgda"].plurals[0] = "Há mudanças não salvas! Salvar mudanças antes de criar uma nova música?";
    strings["Unsaved changes! Save changes before opening another file?##sgda"].plurals[0] = "Há mudanças não salvas! Salvar mudanças antes de abrir outro arquivo?";
    strings["Unsaved changes! Save changes before opening backup?##sgda"].plurals[0] = "Há mudanças não salvas! Salvar mudanças antes de abrir o backup?";
    strings["Error while saving file! (%s)##sgda"].plurals[0] = "Erro ao salvar o arquivo! (%s)";
    strings["Error while sending request (MIDI output not configured?)##sgda"].plurals[0] = "Erro ao enviar a requisição (a saída MIDI não está configurada?)";
    strings["Select an option: (cannot be undone!)##sgda"].plurals[0] = "Selecione uma opção: (não pode ser desfeita!)";
    strings["too many instruments!##sgda0"].plurals[0] = "há muitos instrumentos!";
    strings["too many instruments!##sgda1"].plurals[0] = "há muitos instrumentos!";
    strings["too many wavetables!##sgda0"].plurals[0] = "há muitas wavetables!";
    strings["too many wavetables!##sgda1"].plurals[0] = "há muitas wavetables!";
    strings["too many samples!##sgda0"].plurals[0] = "há muitas samples!";
    strings["too many samples!##sgda1"].plurals[0] = "há muitas samples!";
    strings["couldn't paste! make sure your sample is 8 or 16-bit.##sgda"].plurals[0] = "não pôde colar! tenha certeza de que sua sample é 8 ou 16-bit.";
    strings["too many instruments!##sgda2"].plurals[0] = "há muitos instrumentos!";
    strings["select at least one sample!##sgda"].plurals[0] = "selecione pelo menos uma sample!";
    strings["maximum size is 256 samples!##sgda"].plurals[0] = "o tamanho máximo é de 256 amostras!";
    strings["too many wavetables!##sgda2"].plurals[0] = "há muitas wavetables!";

    //   sgec  src/gui/editControls.cpp

    strings["Mobile Edit###MobileEdit"].plurals[0] = "Edição em dispositivos móveis###MobileEdit"; // essas 3 strings sequer são visíveis? lol
    strings["Mobile Controls###Mobile Controls"].plurals[0] = "Controles para dispositivos móveis###Mobile Controls";
    strings["Mobile Menu###Mobile Menu"].plurals[0] = "Menu para dispositivos móveis###Mobile Menu";
    strings["Pattern##sgec0"].plurals[0] = "Sequência";
    strings["Orders##sgec0"].plurals[0] = "Ordens";
    strings["Ins##sgec"].plurals[0] = "Ins";
    strings["Wave##sgec"].plurals[0] = "Wave";
    strings["Sample##sgec"].plurals[0] = "Sample";
    strings["Song##sgec"].plurals[0] = "Música";
    strings["Channels##sgec"].plurals[0] = "Canais";
    strings["Chips##sgec"].plurals[0] = "Chips";
    strings["Mixer##sgec"].plurals[0] = "Mixer";
    strings["Other##sgec"].plurals[0] = "Outro";
    strings["New##sgec"].plurals[0] = "Novo";
    strings["Unsaved changes! Save changes before creating a new song?##sgec"].plurals[0] = "Há mudanças não salvas! Salvar mudanças antes de criar uma nova música?";
    strings["Open##sgec"].plurals[0] = "Abrir";
    strings["Save##sgec"].plurals[0] = "Salvar";
    strings["Save as...##sgec"].plurals[0] = "Salvar como...";
    strings["Legacy .dmf##sgec"].plurals[0] = ".dmf legado";
    strings["Export##sgec"].plurals[0] = "Exportar";
    strings["Restore Backup##sgec"].plurals[0] = "Restaurar Backup";
    strings["Song Info##sgec"].plurals[0] = "Inform. Música";
    strings["Subsongs##sgec"].plurals[0] = "Subfaixas";
    strings["Speed##sgec"].plurals[0] = "Velocidade";
    strings["Channels here...##sgec"].plurals[0] = "Canais aqui...";
    strings["Chips here...##sgec"].plurals[0] = "Chips aqui...";
    strings["What the hell...##sgec"].plurals[0] = "Mas que porcaria?!";
    strings["Osc##sgec"].plurals[0] = "Osc";
    strings["ChanOsc##sgec"].plurals[0] = "ChanOsc";
    strings["RegView##sgec"].plurals[0] = "RegView";
    strings["Stats##sgec"].plurals[0] = "Stats";
    strings["Grooves##sgec"].plurals[0] = "Grooves";
    strings["Compat Flags##sgec"].plurals[0] = "Opçs. Compat.";
    strings["XYOsc##sgec"].plurals[0] = "XYOsc";
    strings["Panic##sgec"].plurals[0] = "Pânico";
    strings["Settings##sgec"].plurals[0] = "Configurações";
    strings["Log##sgec"].plurals[0] = "Log";
    strings["Debug##sgec"].plurals[0] = "Debug";
    strings["About##sgec"].plurals[0] = "Sobre";
    strings["Switch to Desktop Mode##sgec"].plurals[0] = "Trocar para o modo Desktop";
    strings["this is NOT ROM export! only use for making sure the\n"
            "Furnace Amiga emulator is working properly by\n"
            "comparing it with real Amiga output."].plurals[0] = 

            "esta opção não serve para exportar ROM! use apenas para garantir\n"
            "que o emulador de Amiga do Furnace está funcionando corretamente\n"
            "comparando-o com a saída de um Amiga de verdade.";
    strings["Directory##sgec"].plurals[0] = "Diretório";
    strings["Bake Data##sgec"].plurals[0] = "Precalcular dados";
    strings["Done! Baked %d files.##sgec"].plurals[0] = "Concluído! %d arquivo precalculado.";
    strings["Done! Baked %d files.##sgec"].plurals[1] = "Concluído! %d arquivos precalculados.";
    strings["Play/Edit Controls###Play/Edit Controls"].plurals[0] = "Controles de Reprodução/Edição###Play/Edit Controls";
    strings["Octave##sgec0"].plurals[0] = "Oitava";
    strings["Edit Step##sgec0"].plurals[0] = "Passo de Edição";
    strings["Play##sgec0"].plurals[0] = "Reproduzir";
    strings["Stop##sgec0"].plurals[0] = "Parar";
    strings["Edit##sgec0"].plurals[0] = "Editar";
    strings["Metronome##sgec0"].plurals[0] = "Metrônomo";
    strings["Follow##sgec0"].plurals[0] = "Seguir";
    strings["Orders##sgec1"].plurals[0] = "Ordens";
    strings["Pattern##sgec1"].plurals[0] = "Sequência";
    strings["Repeat pattern##sgec0"].plurals[0] = "Repetir sequência";
    strings["Step one row##sgec0"].plurals[0] = "Mover uma linha";
    strings["Poly##PolyInput"].plurals[0] = "Polif.##PolyInput";
    strings["Mono##PolyInput"].plurals[0] = "Monof.##PolyInput";
    strings["Polyphony##sgec0"].plurals[0] = "Polifonia";
    strings["Stop##sgec1"].plurals[0] = "Parar";
    strings["Play##sgec1"].plurals[0] = "Reproduzir";
    strings["Step one row##sgec1"].plurals[0] = "Mover uma linha";
    strings["Repeat pattern##sgec1"].plurals[0] = "Repetir padrão";
    strings["Edit##sgec1"].plurals[0] = "Editar";
    strings["Metronome##sgec1"].plurals[0] = "Metrônomo";
    strings["Octave##sgec1"].plurals[0] = "Oitava";
    strings["Edit Step##sgec1"].plurals[0] = "Passo de Edição";
    strings["Follow##sgec1"].plurals[0] = "Seguir";
    strings["Orders##sgec2"].plurals[0] = "Ordens";
    strings["Pattern##sgec2"].plurals[0] = "Sequência";
    strings["Poly##PolyInput"].plurals[0] = "Polif.##PolyInput";
    strings["Mono##PolyInput"].plurals[0] = "Monof.##PolyInput";
    strings["Polyphony##sgec1"].plurals[0] = "Polifonia";
    strings["Play##sgec2"].plurals[0] = "Reproduzir";
    strings["Stop##sgec2"].plurals[0] = "Parar";
    strings["Step one row##sgec2"].plurals[0] = "Mover uma linha";
    strings["Repeat pattern##sgec2"].plurals[0] = "Repetir padrão";
    strings["Edit##sgec2"].plurals[0] = "Editar";
    strings["Metronome##sgec2"].plurals[0] = "Metrônomo";
    strings["Oct.##sgec"].plurals[0] = "8va.";
    strings["Octave##sgec2"].plurals[0] = "Oitava";
    strings["Step##sgec0"].plurals[0] = "Passo";
    strings["Foll.##sgec"].plurals[0] = "Seguir";
    strings["Follow##sgec2"].plurals[0] = "Seguir";
    strings["Ord##FollowOrders"].plurals[0] = "Ord.##FollowOrders";
    strings["Orders##sgec3"].plurals[0] = "Ordens";
    strings["Pat##FollowPattern"].plurals[0] = "Seq.##FollowPattern";
    strings["Pattern##sgec3"].plurals[0] = "Sequência";
    strings["Poly##PolyInput"].plurals[0] = "Polif.##PolyInput";
    strings["Mono##PolyInput"].plurals[0] = "Monof.##PolyInput";
    strings["Polyphony##sgec2"].plurals[0] = "Polifonia";
    strings["Play Controls###Play Controls"].plurals[0] = "Controles de Reprodução###Play Controls";
    strings["Stop##sgec3"].plurals[0] = "Parar";
    strings["Play##sgec3"].plurals[0] = "Reproduzir";
    strings["Play from the beginning of this pattern##sgec"].plurals[0] = "Reproduzir do início desta sequência";
    strings["Repeat from the beginning of this pattern##sgec"].plurals[0] = "Repetir do início desta sequência";
    strings["Step one row##sgec3"].plurals[0] = "Mover uma linha";
    strings["Edit##sgec3"].plurals[0] = "Editar";
    strings["Metronome##sgec3"].plurals[0] = "Metrônomo";
    strings["Repeat pattern##sgec3"].plurals[0] = "Repetir sequência";
    strings["Poly##PolyInput"].plurals[0] = "Polif.##PolyInput";
    strings["Mono##PolyInput"].plurals[0] = "Monof.##PolyInput";
    strings["Polyphony##sgec3"].plurals[0] = "Polifonia";
    strings["Edit Controls###Edit Controls"].plurals[0] = "Controles de edição###Edit Controls";
    strings["Octave##sgec3"].plurals[0] = "Oitava";
    strings["Step##sgec1"].plurals[0] = "Passo";
    strings["Follow orders##sgec"].plurals[0] = "Seguir ordens";
    strings["Follow pattern##sgec"].plurals[0] = "Seguir sequência";

    //   sged  src/gui/editing.cpp

    strings["can't collapse any further!##sged"].plurals[0] = "impossível contrair mais!";
    strings["can't expand any further!##sged"].plurals[0] = "impossível expandir mais!";

    //   sgef  src/gui/effectList.cpp

    strings["Effect List###Effect List"].plurals[0] = "Lista de Efeitos###Effect List";
    strings["Chip at cursor: %s##sgef"].plurals[0] = "Chip sob o cursor: %s";
    strings["Search##sgef"].plurals[0] = "Procurar";
    strings["Effect types to show:##sgef"].plurals[0] = "Tipos de efeitos a serem exibidos:";
    strings["All##sgef"].plurals[0] = "Todos";
    strings["None##sgef"].plurals[0] = "Nenhum";
    strings["Name##sgef"].plurals[0] = "Nome";
    strings["Description##sgef"].plurals[0] = "Descrição";
    strings["ERROR##sgef"].plurals[0] = "ERRO";

    //   sgeo  src/gui/exportOptions.cpp

    strings["one file##sgeo"].plurals[0] = "um arquivo";
    strings["multiple files (one per chip)##sgeo"].plurals[0] = "múltiplos arquivos (um por chip)";
    strings["multiple files (one per channel)##sgeo"].plurals[0] = "múltiplos arquivos (um por canal)";
    strings["Loops##sgeo"].plurals[0] = "Loops";
    strings["Fade out (seconds)##sgeo"].plurals[0] = "Tempo de fade out (segundos)";
    strings["Cancel##sgeo0"].plurals[0] = "Cancelar";
    strings["Export##sgeo0"].plurals[0] = "Exportar";
    strings["settings:##sgeo"].plurals[0] = "opções:";
    strings["format version##sgeo"].plurals[0] = "versão do formato:";
    strings["loop##sgeo0"].plurals[0] = "loop";
    strings["loop trail:##sgeo"].plurals[0] = "loops ao final:";
    strings["auto-detect##sgeo"].plurals[0] = "detectar automaticamente";
    strings["add one loop##sgeo1"].plurals[0] = "adicionar um loop";
    strings["custom##sgeo"].plurals[0] = "personalizado";
    strings["add pattern change hints##sgeo"].plurals[0] = "adicionar dicas de mudança de sequência";
    strings["inserts data blocks on pattern changes.\n"
            "useful if you are writing a playback routine.\n\n"
            "the format of a pattern change data block is:\n"
            "67 66 FE ll ll ll ll 01 oo rr pp pp pp ...\n"
            "- ll: length, a 32-bit little-endian number\n"
            "- oo: order\n"
            "- rr: initial row (a 0Dxx effect is able to select a different row)\n"
            "- pp: pattern index (one per channel)\n\n"
            "pattern indexes are ordered as they appear in the song."].plurals[0] = 

            "insere blocos de dados a cada mudança de sequência.\n"
            "útil se você estiver escrevendo uma rotina de reprodução.\n\n"
            "o formato do bloco de dados de mudança de sequência é:\n"
            "67 66 FE ll ll ll ll 01 oo rr pp pp pp ...\n"
            "- ll: comprimento, um número 32-bit little-endian\n"
            "- oo: ordem\n"
            "- rr: linha inicial (um efeito 0Dxx é capaz de selecionar uma linha diferente de zero)\n"
            "- pp: índice da sequência (uma por canal)\n\n"
            "os índices da sequência são ordenados conforme elas aparecem na música.";
    strings["direct stream mode##sgeo"].plurals[0] = "modo de fluxo direto";
    strings["required for DualPCM and MSM6258 export.\n\n"
            "allows for volume/direction changes when playing samples,\n"
            "at the cost of a massive increase in file size."].plurals[0] = 

            "requerido para exportação de DualPCM e MSM6258..\n\n"
            "permite alterações de volume e direção ao reproduzir samples,\n"
            "ao custo de um enorme aumento de tamanho de arquivo.";
    strings["chips to export:##sgeo"].plurals[0] = "chips a exportar:";
    strings["this chip is only available in VGM %d.%.2x and higher!##sgeo"].plurals[0] = "este chip só está disponível no VGM versão %d.%.2x e superior!";
    strings["this chip is not supported by the VGM format!##sgeo"].plurals[0] = "este chip não é suportado pelo formato VGM!";
    strings["select the chip you wish to export, but only up to %d of each type.##sgeo"].plurals[0] = "selecione o chip que você quer exportar, mas no máximo %d de cada tipo.";
    strings["select the chip you wish to export, but only up to %d of each type.##sgeo"].plurals[1] = "selecione o chip que você quer exportar, mas no máximo %d de cada tipo.";
    strings["Cancel##sgeo1"].plurals[0] = "Cancelar";
    strings["Export##sgeo1"].plurals[0] = "Exportar";
    strings["nothing to export##sgeo2"].plurals[0] = "nada a ser exportado";
    strings["Cancel##sgeo2"].plurals[0] = "Cancelar";
    strings["Commander X16 Zsound Music File##sgeo"].plurals[0] = "Arquivo de Música ZSound do Commander X16";
    strings["Tick Rate (Hz)##sgeo"].plurals[0] = "Taxa de Ticks (Hz)";
    strings["loop##sgeo2"].plurals[0] = "loop";
    strings["optimize size##sgeo"].plurals[0] = "otimizar tamanho";
    strings["Cancel##sgeo3"].plurals[0] = "Cancelar";
    strings["Export##sgeo3"].plurals[0] = "Exportar";
    strings["DefleMask file (1.1.3+)##sgeo"].plurals[0] = "Arquivo do DefleMask (1.1.3+)";
    strings["Cancel##sgeo4"].plurals[0] = "Cancelar";
    strings["Export##sgeo4"].plurals[0] = "Exportar";
    strings["DefleMask file (1.0/legacy)##sgeo"].plurals[0] = "Arquivo do DefleMask (1.0/legado)";
    strings["Cancel##sgeo5"].plurals[0] = "Cancelar";
    strings["Export##sgeo5"].plurals[0] = "Exportar";
    strings["this is NOT ROM export! only use for making sure the\n"
            "Furnace Amiga emulator is working properly by\n"
            "comparing it with real Amiga output."].plurals[0] = 

            "esta opção não serve para exportar ROM! use apenas para garantir\n"
            "que o emulador de Amiga do Furnace está funcionando corretamente\n"
            "comparando-o com a saída de um Amiga de verdade.";
    strings["Directory##sgeo"].plurals[0] = "Diretório";
    strings["Cancel##sgeo6"].plurals[0] = "Cancelar";
    strings["Bake Data##sgeo"].plurals[0] = "Precalcular dados";
    strings["Done! Baked %d files.##sgeo"].plurals[0] = "Pronto! %d arquivo precalculado.";
    strings["Done! Baked %d files.##sgeo"].plurals[1] = "Pronto! %d arquivos precalculados.";
    strings["this option exports the song to a text file.\n##sgeo"].plurals[0] = "essa opção exporta a música para um arquivo texto.\n";
    strings["Cancel##sgeo7"].plurals[0] = "Cancelar";
    strings["Export##sgeo6"].plurals[0] = "Exportar";
    strings["this option exports a binary file which\n"
            "contains a dump of the internal command stream\n"
            "produced when playing the song.\n\n"
            "technical/development use only!"].plurals[0] = 

            "essa opção exporta um arquivo binário que\n"
            "contém um despejo do fluxo de comandos interno\n"
            "produzido ao reproduzir a música.\n\n"
            "somente para uso técnico ou para desenvolvimento!";
    strings["Cancel##sgeo8"].plurals[0] = "Cancelar";
    strings["Export##sgeo"].plurals[0] = "Exportar";
    strings["this option exports a module which is\n"
            "compatible with tildearrow Furnace app.\n\n"

            "not all chips and inst macros will be supported!"].plurals[0] = 

            "esta opção exporta um módulo que é\n"
            "compatível com o aplicatívo Furnace do tildearrow.\n\n"

            "nem todos os chips e macros de instrumento serão suportados!";
    strings["Cancel##sgeo9"].plurals[0] = "Cancelar";
    strings["Export##sgeo7"].plurals[0] = "Exportar";
    strings["Audio##sgeo"].plurals[0] = "Áudio";
    strings["DMF (1.0/legacy)##sgeo"].plurals[0] = "DMF (1.0/legado)";
    strings["Amiga Validation##sgeo"].plurals[0] = "Dados de Validação Amiga";
    strings["Text##sgeo"].plurals[0] = "Texto";
    strings["Command Stream##sgeo"].plurals[0] = "Fluxo de Comandos";
    strings["Furnace##sgeo"].plurals[0] = "Furnace";
    strings["congratulations! you've unlocked a secret panel.##sgeo"].plurals[0] = "parabéns! você desbloqueou um painel secreto.";
    strings["Toggle hidden systems##sgeo"].plurals[0] = "Habilitar/desabilitar sistemas escondidos";
    strings["Toggle all instrument types##sgeo"].plurals[0] = "Habilitar/desabilitar todos os tipos de instrumentos";
    strings["Set pitch linearity to Partial##sgeo"].plurals[0] = "Definir linearidade de tom para Parcial";
    strings["Enable multi-threading settings##sgeo"].plurals[0] = "Habilitar opções de multithreading";
    strings["Set fat to max##sgeo"].plurals[0] = "Definir gordura para máximo";
    strings["Set muscle and fat to zero##sgeo"].plurals[0] = "Definir músculo e gordura para zero";
    strings["Tell tildearrow this must be a mistake##sgeo"].plurals[0] = "Dizer para o tildearrow que isso só pode ser um engano";
    strings["yeah, it's a bug. write a bug report in the GitHub page and tell me how did you get here.##sgeo"].plurals[0] = "é, isso é um bug. abra um bug report na página do GitHub e diga-nos como você chegou aqui.";

    //   sgfr  src/gui/findReplace.cpp

    strings["ignore##sgfr"].plurals[0] = "ignorar";
    strings["equals##sgfr"].plurals[0] = "é igual a";
    strings["not equal##sgfr"].plurals[0] = "é diferente de";
    strings["between##sgfr"].plurals[0] = "está entre";
    strings["not between##sgfr"].plurals[0] = "não está entre";
    strings["any##sgfr"].plurals[0] = "qualquer";
    strings["none##sgfr"].plurals[0] = "nenhum";
    strings["set##sgfr"].plurals[0] = "definir";
    strings["add##sgfr"].plurals[0] = "somar";
    strings["add (overflow)##sgfr"].plurals[0] = "somar (com transbordo)";
    strings["scale %##sgfr"].plurals[0] = "escalonar %";
    strings["clear##sgfr"].plurals[0] = "limpar";

    strings["Find/Replace###Find/Replace"].plurals[0] = "Localizar/Substituir###Find/Replace";
    strings["Find##sgfr0"].plurals[0] = "Localizar";
    strings["order##sgfr0"].plurals[0] = "ordem";
    strings["row##sgfr0"].plurals[0] = "linha";
    strings["order##sgfr1"].plurals[0] = "ordem";
    strings["row##sgfr1"].plurals[0] = "linha";
    strings["channel##sgfr"].plurals[0] = "canal";
    strings["go##sgfr"].plurals[0] = "ir";
    strings["no matches found!##sgfr"].plurals[0] = "nenhum resultado encontrado!";
    strings["Back##sgfr"].plurals[0] = "Voltar";
    strings["Note##sgfr0"].plurals[0] = "Nota";
    strings["Ins##sgfr0"].plurals[0] = "Ins";
    strings["Volume##sgfr0"].plurals[0] = "Volume";
    strings["Effect##sgfr0"].plurals[0] = "Efeito";
    strings["Value##sgfr0"].plurals[0] = "Valor";
    strings["Delete query##sgfr"].plurals[0] = "Excluir consulta";
    strings["Add effect##sgfr0"].plurals[0] = "Adicionar efeito";
    strings["Remove effect##sgfr0"].plurals[0] = "Remover efeito";
    strings["Search range:##sgfr"].plurals[0] = "Intervalo de pesquisa:";
    strings["Song##sgfr"].plurals[0] = "Música";
    strings["Selection##sgfr"].plurals[0] = "Seleção";
    strings["Pattern##sgfr"].plurals[0] = "Sequência";
    strings["Confine to channels##sgfr"].plurals[0] = "Confinar aos canais";
    strings["From##sgfr"].plurals[0] = "De";
    strings["To##sgfr"].plurals[0] = "Até";
    strings["Match effect position:##sgfr"].plurals[0] = "Correspondência com posição de efeitos:"; // TODO: revisar?
    strings["No##sgfr"].plurals[0] = "Nenhuma";
    strings["match effects regardless of position.##sgfr"].plurals[0] = "corresponder com efeitos independente da posição.";
    strings["Lax##sgfr"].plurals[0] = "Relaxada";
    strings["match effects only if they appear in-order.##sgfr"].plurals[0] = "corresponder com efeitos somente se eles aparecerem em ordem.";
    strings["Strict##sgfr"].plurals[0] = "Estrita";
    strings["match effects only if they appear exactly as specified.##sgfr"].plurals[0] = "corresponder com efeitos somente se eles aparecerem exatamente como especificados.";
    strings["Find##sgfr1"].plurals[0] = "Localizar";
    strings["Replace##sgfr"].plurals[0] = "Substituir";
    strings["Note##sgfr1"].plurals[0] = "Nota";
    strings["INVALID##sgfr"].plurals[0] = "INVÁLIDO";
    strings["Ins##sgfr1"].plurals[0] = "Ins";
    strings["Volume##sgfr1"].plurals[0] = "Volume";
    strings["Effect##sgfr1"].plurals[0] = "Efeito";
    strings["Value##sgfr1"].plurals[0] = "Valor";
    strings["Add effect##sgfr1"].plurals[0] = "Adicionar efeito";
    strings["Remove effect##sgfr1"].plurals[0] = "Remover efeito";
    strings["Effect replace mode:##sgfr"].plurals[0] = "Modo de substituição de efeito:";
    strings["Replace matches only##sgfr"].plurals[0] = "Substituir correspondências somente";
    strings["Replace matches, then free spaces##sgfr"].plurals[0] = "Substituir correspondências, e então espaços livres";
    strings["Clear effects##sgfr"].plurals[0] = "Limpar efeitos";
    strings["Insert in free spaces##sgfr"].plurals[0] = "Inserir em espaços livres";
    strings["Replace##QueryReplace"].plurals[0] = "Substituir##QueryReplace";

    //   sggv  src/gui/grooves.cpp

    strings["Grooves###Grooves"].plurals[0] = "Grooves###Grooves";
    strings["use effect 09xx to select a groove pattern.##sggv"].plurals[0] = "use o efeito 09xx para selecionar um padrão de groove.##sggv";
    strings["pattern##sggv"].plurals[0] = "padrão##sggv";
    strings["remove##sggv"].plurals[0] = "excluir##sggv";

    //   sgie  src/gui/insEdit.cpp

    strings["Name##sgie"].plurals[0] = "Nome";
    strings["Open##sgie0"].plurals[0] = "Abrir";
    strings["Save##sgie"].plurals[0] = "Salvar";
    strings["export .dmp...##sgie"].plurals[0] = "exportar .dmp...";
    strings["Type##sgie"].plurals[0] = "Tipo";
    strings["Unknown##sgie"].plurals[0] = "Desconhecido";
    strings["none of the currently present chips are able to play this instrument type!##sgie"].plurals[0] = "nenhum dos chips atualmente presentes são capazes de reproduzir esse tipo de instrumento!";
    strings["Error##sgie"].plurals[0] = "Erro";
    strings["invalid instrument type! change it first.##sgie"].plurals[0] = "tipo de instrumento inválido! troque-o primeiro."; // TODO revisar; NOTA: a string original é uma bosta
    strings["Instrument Editor###Instrument Editor"].plurals[0] = "Editor de Instrumento###Instrument Editor";
    strings["waiting...##sgie0"].plurals[0] = "aguardando...";
    strings["waiting...##sgie1"].plurals[0] = "aguardando...";
    strings["no instrument selected##sgie0"].plurals[0] = "nenhum instrumento selecionado";
    strings["no instrument selected##sgie1"].plurals[0] = "nenhum instrumento selecionado";
    strings["select one...##sgie"].plurals[0] = "selecione um...";
    strings["or##sgie0"].plurals[0] = "ou";
    strings["Open##sgie1"].plurals[0] = "Abrir";
    strings["or##sgie1"].plurals[0] = "ou";
    strings["Create New##sgie"].plurals[0] = "Criar um novo";
    strings["copy##sgie"].plurals[0] = "copiar";
    strings["paste##sgie"].plurals[0] = "colar";
    strings["clear contents##sgie"].plurals[0] = "limpar conteúdo";
    strings["offset...##sgie"].plurals[0] = "deslocar...";
    strings["offset##sgie"].plurals[0] = "deslocar";
    strings["scale...##sgie"].plurals[0] = "escalonar";
    strings["scale##sgie"].plurals[0] = "escalonar";
    strings["randomize...##sgie"].plurals[0] = "aleatorizar";
    strings["Min##sgie"].plurals[0] = "Mín.";
    strings["Max##sgie"].plurals[0] = "Máx.";
    strings["randomize##sgie"].plurals[0] = "aleatorizar";

    //   sgmx  src/gui/mixer.cpp

    strings["input##sgmx"].plurals[0] = "entrada";
    strings["output##sgmx"].plurals[0] = "saída";
    strings["Mixer##sgmx"].plurals[0] = "Mixer";
    strings["Master Volume##sgmx"].plurals[0] = "Volume Mestre";
    strings["Invert##sgmx"].plurals[0] = "Inverter";
    strings["Volume##sgmx"].plurals[0] = "Volume";
    strings["Panning##sgmx"].plurals[0] = "Balanço";
    strings["Front/Rear##sgmx"].plurals[0] = "Frontal/Traseira";
    strings["Patchbay##sgmx"].plurals[0] = "Patchbay";
    strings["Automatic patchbay##sgmx"].plurals[0] = "Patchbay automático";
    strings["Display hidden ports##sgmx"].plurals[0] = "Mostrar portas ocultas";
    strings["Display internal##sgmx"].plurals[0] = "Mostrar portas internas";
    strings["System##sgmx0"].plurals[0] = "Sistema";
    strings["Sample Preview##sgmx"].plurals[0] = "Previsão de amostras";
    strings["Metronome##sgmx"].plurals[0] = "Metrônomo";
    strings["System##sgmx1"].plurals[0] = "Sistema";
    strings["disconnect all##sgmx"].plurals[0] = "desconectar todas";

    //   sgns  src/gui/newSong.cpp

    strings["Choose a System!##sgns"].plurals[0] = "Escolha um Sistema!";
    strings["Search...##sgns"].plurals[0] = "Procurar...";
    strings["Categories##sgns"].plurals[0] = "Categorias";
    strings["Systems##sgns"].plurals[0] = "Sistemas";
    strings["no systems here yet!##sgns"].plurals[0] = "nenhum sistema aqui ainda!";
    strings["no results##sgns"].plurals[0] = "nenhum resultado";
    strings["I'm feeling lucky##sgns"].plurals[0] = "Estou com sorte";
    strings["no categories available! what in the world.##sgns"].plurals[0] = "nenhuma categoria disponível! que país é esse...";
    strings["Cancel##sgns"].plurals[0] = "Cancelar";

    //   sgme  src/gui/memory.cpp

    strings["Memory Composition###Memory Composition"].plurals[0] = "Composição da Memória###Memory Composition";
    strings["bank %d##sgme"].plurals[0] = "banco %d";
    strings["%d-%d ($%x-$%x): %d bytes ($%x)##sgme"].plurals[0] = "%d-%d ($%x-$%x): %d bytes ($%x)";
    strings["click to open sample editor##sgme"].plurals[0] = "clique para abrir o editor de samples";
    strings["%d-%d ($%x-$%x): %d bytes ($%x)##sgme"].plurals[0] = "%d-%d ($%x-$%x): %d bytes ($%x)";
    strings["no chips with memory##sgme"].plurals[0] = "nenhum chip com memória";

    //   sgor  src/gui/orders.cpp

    strings["Orders###Orders"].plurals[0] = "Ordens###Orders";

    strings["Add new order##sgor"].plurals[0] = "Adicionar nova ordem";
    strings["Remove order##sgor"].plurals[0] = "Remover ordem";
    strings["Duplicate order (right-click to deep clone)##sgor"].plurals[0] = "Duplicar ordem (clique com o botão direito para um clone profundo)";
    strings["Move order up##sgor"].plurals[0] = "Mover ordem para cima";
    strings["Move order down##sgor"].plurals[0] = "Mover ordem para baixo";
    strings["Place copy of current order at end of song (right-click to deep clone)##sgor"].plurals[0] = "Adicionar cópia da ordem atual ao final da música (clique com o botão direito para um clone profundo)";
    strings["Order change mode: entire row##sgor"].plurals[0] = "Modo de mudança da ordem: linha inteira";
    strings["Order change mode: one##sgor"].plurals[0] = "Modo de mudança da ordem: uma célula";
    strings["Order edit mode: Select and type (scroll vertically)##sgor"].plurals[0] = "Modo de edição de ordem: selecionar e digitar (rolagem vertical)";
    strings["Order edit mode: Select and type (scroll horizontally)##sgor"].plurals[0] = "Modo de edição de ordem: selecionar e digitar (rolagem horizontal)";
    strings["Order edit mode: Select and type (don't scroll)##sgor"].plurals[0] = "Modo de edição de ordem: selecionar e digitar (não rolar)";
    strings["Order edit mode: Click to change##sgor"].plurals[0] = "Modo de edição de ordem: clicar para mudar";

    //   sgos  src/gui/osc.cpp

    strings["Oscilloscope###Oscilloscope"].plurals[0] = "Osciloscópio###Oscilloscope";
    strings["zoom: %.2fx (%.1fdB)##sgos"].plurals[0] = "zoom: %.2fx (%.1fdB)";
    strings["window size: %.1fms##sgos"].plurals[0] = "tamanho da janela: %.1fms";
    strings["(-Infinity)dB##sgos"].plurals[0] = "(-Infinito)dB";

    //   sgpm  src/gui/patManager.cpp

    strings["Pattern Manager###Pattern Manager"].plurals[0] = "Gerenciador de Sequências###Pattern Manager";
    strings["De-duplicate patterns##sgpm"].plurals[0] = "Deduplicar sequências";
    strings["Re-arrange patterns##sgpm"].plurals[0] = "Reordenar sequências";
    strings["Sort orders##sgpm"].plurals[0] = "Ordenar ordens";
    strings["Make patterns unique##sgpm"].plurals[0] = "Tornar sequências únicas";
    strings["Pattern %.2X\n- not allocated##sgpm"].plurals[0] = "Sequência %.2X\n- não alocada";
    strings["Pattern %.2X\n- use count: %d (%.0f%%)\n\nright-click to erase##sgpm"].plurals[0] = "Sequência %.2X\n- usada %d vezes (%.0f%%)\n\nclique com o botão direito para excluir";

    //   sgpa  src/gui/pattern.cpp

    strings["Pattern###Pattern"].plurals[0] = "Sequência###Pattern";
    strings["there aren't any channels to show.##sgpa"].plurals[0] = "não há nenhum canal a ser exibido.";
    strings["click for pattern options (effect columns/pattern names/visualizer)##sgpa"].plurals[0] = "clique para opções de sequência (colunas de efeito/nomes das faixas/visualizador)";
    strings["Options:##sgpa"].plurals[0] = "Opções:";
    strings["Effect columns/collapse##sgpa"].plurals[0] = "Botões de colunas de efeito/contrair faixa";
    strings["Pattern names##sgpa"].plurals[0] = "Nomes das faixas";
    strings["Channel group hints##sgpa"].plurals[0] = "Dicas de agrupamento de canais";
    strings["Visualizer##sgpa"].plurals[0] = "Visualizador";
    strings["Channel status:##sgpa"].plurals[0] = "Estado do canal:";
    strings["No##_PCS0"].plurals[0] = "Não##_PCS0";
    strings["Yes##_PCS1"].plurals[0] = "Sim##_PCS1";
    strings["WARNING!!##sgpa"].plurals[0] = "AVISO!!";
    strings["this instrument cannot be previewed because##sgpa"].plurals[0] = "este instrumento não pode ser previsto pois";
    strings["none of the chips can play it##sgpa"].plurals[0] = "nenhum dos chips pode tocá-lo";
    strings["your instrument is in peril!! be careful...##sgpa"].plurals[0] = "seu instrumento está em perigo! cuidado...";

    //   sgpi  src/gui/piano.cpp

    strings["Piano###Piano"].plurals[0] = "Piano###Piano";
    strings["Options##sgpi"].plurals[0] = "Opções";
    strings["Key layout:##sgpi"].plurals[0] = "Leiaute das teclas:";
    strings["Automatic##sgpi"].plurals[0] = "Automático";
    strings["Standard##sgpi"].plurals[0] = "Padrão";
    strings["Continuous##sgpi"].plurals[0] = "Contínuo";
    strings["Value input pad:##sgpi"].plurals[0] = "Teclado numérico:";
    strings["Disabled##sgpi"].plurals[0] = "Desabilitado";
    strings["Replace piano##sgpi"].plurals[0] = "Substituir piano";
    strings["Split (automatic)##sgpi"].plurals[0] = "Dividido (automático)";
    strings["Split (always visible)##sgpi"].plurals[0] = "Dividido (sempre visível)";
    strings["Share play/edit offset/range##sgpi"].plurals[0] = "Compartilhar intervalo/deslocamento entre modo de edição e reprodução";
    strings["Read-only (can't input notes)##sgpi"].plurals[0] = "Somente leitura (não permite tocar notas)";
    strings["Input Pad###Input Pad"].plurals[0] = "Teclado Virtual###Input Pad";

    //   sgpr  src/gui/presets.cpp

    strings["Game consoles##sgpr"].plurals[0] = "Consoles de videogame";
    strings["let's play some chiptune making games!##sgpr"].plurals[0] = "vamos jogar um pouco de chiptune fazendo jogos!";
    strings["Sega Genesis (extended channel 3)##sgpr"].plurals[0] = "Sega Mega Drive (canal 3 estendido)";
    strings["Sega Genesis (DualPCM, extended channel 3)##sgpr"].plurals[0] = "Sega Mega Drive (DualPCM, canal 3 estendido)";
    strings["Sega Genesis (with Sega CD)##sgpr"].plurals[0] = "Sega Mega Drive (com Mega CD)";
    strings["Sega Genesis (extended channel 3 with Sega CD)##sgpr"].plurals[0] = "Sega Mega Drive (canal 3 estendido com Mega CD)";
    strings["Sega Genesis (CSM with Sega CD)##sgpr"].plurals[0] = "Sega Mega Drive (CSM, com Mega CD)";
    strings["Sega Master System (with FM expansion)##sgpr"].plurals[0] = "Sega Master System (with expansão FM)";
    strings["Sega Master System (with FM expansion in drums mode)##sgpr"].plurals[0] = "Sega Master System (with expansão FM em modo bateria)";
    strings["Game Boy Advance (no software mixing)##sgpr"].plurals[0] = "Game Boy Advance (sem mixing em software)";
    strings["Game Boy Advance (with MinMod)##sgpr"].plurals[0] = "Game Boy Advance (com MinMod)";
    strings["Famicom with Konami VRC6##sgpr"].plurals[0] = "Famicom com Konami VRC6";
    strings["Famicom with Konami VRC7##sgpr0"].plurals[0] = "Famicom com Konami VRC7";
    strings["Famicom with MMC5##sgpr"].plurals[0] = "Famicom com MMC5";
    strings["Famicom with Sunsoft 5B##sgpr"].plurals[0] = "Famicom com Sunsoft 5B";
    strings["Famicom with Namco 163##sgpr"].plurals[0] = "Famicom com Namco 163";
    strings["Neo Geo AES (extended channel 2)##sgpr"].plurals[0] = "Neo Geo AES (canal 2 estendido)";
    strings["Neo Geo AES (extended channel 2 and CSM)##sgpr"].plurals[0] = "Neo Geo AES (canal 2 estendido e CSM)";
    strings["Computers##sgpr"].plurals[0] = "Computadores";
    strings["let's get to work on chiptune today.##sgpr"].plurals[0] = "porque hoje é dia de trabalhar em chiptune.";
    strings["Commodore 64 (C64, 6581 SID + Sound Expander in drums mode)##sgpr"].plurals[0] = "Commodore 64 (C64, SID 6581 + Sound Expander em modo bateria)";
    strings["Commodore 64 (C64, 8580 SID + Sound Expander in drums mode)##sgpr"].plurals[0] = "Commodore 64 (C64, SID 8580 + Sound Expander em modo bateria)";
    strings["Commodore 64 (C64, 6581 SID + FM-YAM in drums mode)##sgpr"].plurals[0] = "Commodore 64 (C64, SID 6581 + FM-YAM em modo bateria)";
    strings["Commodore 64 (C64, 8580 SID + FM-YAM in drums mode)##sgpr"].plurals[0] = "Commodore 64 (C64, SID 8580 + FM-YAM em modo bateria)";
    strings["MSX + MSX-AUDIO (drums mode)##sgpr"].plurals[0] = "MSX + MSX-AUDIO (modo bateria)";
    strings["MSX + MSX-MUSIC (drums mode)##sgpr"].plurals[0] = "MSX + MSX-MUSIC (modo bateria)";
    strings["MSX + Neotron (extended channel 2)##sgpr"].plurals[0] = "MSX + Neotron (canal 2 estendido)";
    strings["MSX + Neotron (extended channel 2 and CSM)##sgpr"].plurals[0] = "MSX + Neotron (canal 2 estendido e CSM)";
    strings["MSX + Neotron (with YM2610B)##sgpr"].plurals[0] = "MSX + Neotron (com YM2610B)";
    strings["MSX + Neotron (with YM2610B; extended channel 3)##sgpr"].plurals[0] = "MSX + Neotron (com YM2610B; canal 3 estendido)";
    strings["MSX + Neotron (with YM2610B; extended channel 3 and CSM)##sgpr"].plurals[0] = "MSX + Neotron (com YM2610B; canal 3 estendido e CSM)";
    strings["NEC PC-88 (with PC-8801-10)##sgpr"].plurals[0] ="NEC PC-88 (com PC-8801-10)";
    strings["NEC PC-88 (with PC-8801-11)##sgpr"].plurals[0] = "NEC PC-88 (com PC-8801-11)";
    strings["NEC PC-88 (with PC-8801-11; extended channel 3)##sgpr"].plurals[0] = "NEC PC-88 (com PC-8801-11; canal 3 estendido)";
    strings["NEC PC-88 (with PC-8801-11; extended channel 3 and CSM)##sgpr"].plurals[0] = "NEC PC-88 (com PC-8801-11; canal 3 estendido e CSM)";
    strings["NEC PC-88 (with PC-8801-23)##sgpr"].plurals[0] = "NEC PC-88 (com PC-8801-23)";
    strings["NEC PC-88 (with PC-8801-23; extended channel 3)##sgpr"].plurals[0] = "NEC PC-88 (com PC-8801-23; canal 3 estendido)";
    strings["NEC PC-88 (with PC-8801-23; extended channel 3 and CSM)##sgpr"].plurals[0] = "NEC PC-88 (com PC-8801-23; canal 3 estendido e CSM)";
    strings["NEC PC-88 (with HMB-20 HIBIKI-8800)##sgpr"].plurals[0] = "NEC PC-88 (com HMB-20 HIBIKI-8800)";
    strings["NEC PC-8801mk2SR (with PC-8801-10)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (com PC-8801-10)";
    strings["NEC PC-8801mk2SR (with PC-8801-10; extended channel 3)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (com PC-8801-10; canal 3 estendido)";
    strings["NEC PC-8801mk2SR (with PC-8801-10; extended channel 3 and CSM)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (with PC-8801-10; canal 3 estendido e CSM)";
    strings["NEC PC-8801mk2SR (with PC-8801-11)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (com PC-8801-11)";
    strings["NEC PC-8801mk2SR (with PC-8801-11; extended channel 3 on internal OPN)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (com PC-8801-11; canal 3 estendido no OPN interno)";
    strings["NEC PC-8801mk2SR (with PC-8801-11; extended channel 3 on external OPN)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (com PC-8801-11; canal 3 estendido no OPN externo)";
    strings["NEC PC-8801mk2SR (with PC-8801-11; extended channel 3 on both OPNs)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (com PC-8801-11; canal 3 estendido em ambos os OPNs)";
    strings["NEC PC-8801mk2SR (with PC-8801-11; extended channel 3 and CSM on both OPNs)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (com PC-8801-11; canal 3 estendido e CSM em ambos os OPNs)";
    strings["NEC PC-8801mk2SR (with PC-8801-23)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (com PC-8801-23)";
    strings["NEC PC-8801mk2SR (with PC-8801-23; extended channel 3 on internal OPN)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (com PC-8801-23; canal 3 estendido no OPN interno)";
    strings["NEC PC-8801mk2SR (with PC-8801-23; extended channel 3 and CSM on internal OPN)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (com PC-8801-23; canal 3 estendido e CSM no OPN interno)";
    strings["NEC PC-8801mk2SR (with PC-8801-23; extended channel 3 on external OPN)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (com PC-8801-23; canal 3 estendido no OPN externo)";
    strings["NEC PC-8801mk2SR (with PC-8801-23; extended channel 3 and CSM on external OPN)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (com PC-8801-23; canal 3 estendido e CSM no OPN externo)";
    strings["NEC PC-8801mk2SR (with PC-8801-23; extended channel 3 on both OPNs)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (com PC-8801-23; canal 3 estendido em ambos os OPNs)";
    strings["NEC PC-8801mk2SR (with PC-8801-23; extended channel 3 and CSM on both OPNs)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (com PC-8801-23; canal 3 estendido e CSM em ambos os OPNs)";
    strings["NEC PC-8801mk2SR (with HMB-20 HIBIKI-8800)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (com HMB-20 HIBIKI-8800)";
    strings["NEC PC-8801mk2SR (with HMB-20 HIBIKI-8800; extended channel 3)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (com HMB-20 HIBIKI-8800; canal 3 estendido)";
    strings["NEC PC-8801mk2SR (with HMB-20 HIBIKI-8800; extended channel 3 and CSM)##sgpr"].plurals[0] = "NEC PC-8801mk2SR (com HMB-20 HIBIKI-8800; canal 3 estendido e CSM)";
    strings["NEC PC-8801FA (with PC-8801-10)##sgpr"].plurals[0] = "NEC PC-8801FA (com PC-8801-10)";
    strings["NEC PC-8801FA (with PC-8801-10; extended channel 3)##sgpr"].plurals[0] = "NEC PC-8801FA (com PC-8801-10; canal 3 estendido)";
    strings["NEC PC-8801FA (with PC-8801-11)##sgpr"].plurals[0] = "NEC PC-8801FA (com PC-8801-11)";
    strings["NEC PC-8801FA (with PC-8801-11; extended channel 3 on internal OPN)##sgpr"].plurals[0] = "NEC PC-8801FA (com PC-8801-11; canal 3 estendido no OPN interno)";
    strings["NEC PC-8801FA (with PC-8801-11; extended channel 3 on external OPN)##sgpr"].plurals[0] = "NEC PC-8801FA (com PC-8801-11; canal 3 estendido no OPN externo)";
    strings["NEC PC-8801FA (with PC-8801-11; extended channel 3 and CSM on external OPN)##sgpr"].plurals[0] = "NEC PC-8801FA (com PC-8801-11; canal 3 estendido e CSM no OPN externo)";
    strings["NEC PC-8801FA (with PC-8801-11; extended channel 3 on both OPNs)##sgpr"].plurals[0] = "NEC PC-8801FA (com PC-8801-11; canal 3 estendido em ambos os OPNs)";
    strings["NEC PC-8801FA (with PC-8801-11; extended channel 3 and CSM on both OPNs)##sgpr"].plurals[0] = "NEC PC-8801FA (com PC-8801-11; canal 3 estendido e CSM em ambos os OPNs)";
    strings["NEC PC-8801FA (with PC-8801-23)##sgpr"].plurals[0] = "NEC PC-8801FA (com PC-8801-23)";
    strings["NEC PC-8801FA (with PC-8801-23; extended channel 3 on internal OPN)##sgpr"].plurals[0] = "NEC PC-8801FA (com PC-8801-23; canal 3 estendido no OPN interno)";
    strings["NEC PC-8801FA (with PC-8801-23; extended channel 3 and CSM on internal OPN)##sgpr"].plurals[0] = "NEC PC-8801FA (com PC-8801-23; canal 3 estendido e CSM no OPN interno)";
    strings["NEC PC-8801FA (with PC-8801-23; extended channel 3 on external OPN)##sgpr"].plurals[0] = "NEC PC-8801FA (com PC-8801-23; canal 3 estendido no OPN externo)";
    strings["NEC PC-8801FA (with PC-8801-23; extended channel 3 and CSM on external OPN)##sgpr"].plurals[0] = "NEC PC-8801FA (com PC-8801-23; canal 3 estendido e CSM no OPN externo)";
    strings["NEC PC-8801FA (with PC-8801-23; extended channel 3 on both OPNs)##sgpr"].plurals[0] = "NEC PC-8801FA (com PC-8801-23; canal 3 estendido em ambos os OPNs)";
    strings["NEC PC-8801FA (with PC-8801-23; extended channel 3 and CSM on both OPNs)##sgpr"].plurals[0] = "NEC PC-8801FA (com PC-8801-23; canal 3 estendido e CSM em ambos os OPNs)";
    strings["NEC PC-8801FA (with HMB-20 HIBIKI-8800)##sgpr"].plurals[0] = "NEC PC-8801FA (com HMB-20 HIBIKI-8800)";
    strings["NEC PC-8801FA (with HMB-20 HIBIKI-8800; extended channel 3)##sgpr"].plurals[0] = "NEC PC-8801FA (com HMB-20 HIBIKI-8800; canal 3 estendido)";
    strings["NEC PC-8801FA (with HMB-20 HIBIKI-8800; extended channel 3 and CSM)##sgpr"].plurals[0] = "NEC PC-8801FA (com HMB-20 HIBIKI-8800; canal 3 estendido e CSM)";
    strings["NEC PC-98 (with PC-9801-26/K)##sgpr"].plurals[0] = "NEC PC-98 (com PC-9801-26/K)";
    strings["NEC PC-98 (with PC-9801-26/K; extended channel 3)##sgpr"].plurals[0] = "NEC PC-98 (com PC-9801-26/K; canal 3 estendido)";
    strings["NEC PC-98 (with PC-9801-26/K; extended channel 3 and CSM)##sgpr"].plurals[0] = "NEC PC-98 (com PC-9801-26/K; canal 3 estendido e CSM)";
    strings["NEC PC-98 (with Sound Orchestra)##sgpr"].plurals[0] = "NEC PC-98 (com Sound Orchestra)";
    strings["NEC PC-98 (with Sound Orchestra; extended channel 3)##sgpr"].plurals[0] = "NEC PC-98 (com Sound Orchestra; canal 3 estendido)";
    strings["NEC PC-98 (with Sound Orchestra; extended channel 3 and CSM)##sgpr"].plurals[0] = "NEC PC-98 (com Sound Orchestra; canal 3 estendido e CSM)";
    strings["NEC PC-98 (with Sound Orchestra in drums mode)##sgpr"].plurals[0] = "NEC PC-98 (com Sound Orchestra em modo bateria)";
    strings["NEC PC-98 (with Sound Orchestra in drums mode; extended channel 3)##sgpr"].plurals[0] = "NEC PC-98 (com Sound Orchestra em modo bateria; canal 3 estendido)";
    strings["NEC PC-98 (with Sound Orchestra in drums mode; extended channel 3 and CSM)##sgpr"].plurals[0] = "NEC PC-98 (com Sound Orchestra em modo bateria; canal 3 estendido e CSM)";
    strings["NEC PC-98 (with Sound Orchestra V)##sgpr"].plurals[0] = "NEC PC-98 (com Sound Orchestra V)";
    strings["NEC PC-98 (with Sound Orchestra V; extended channel 3)##sgpr"].plurals[0] = "NEC PC-98 (com Sound Orchestra V; canal 3 estendido)";
    strings["NEC PC-98 (with Sound Orchestra V; extended channel 3 and CSM)##sgpr"].plurals[0] = "NEC PC-98 (com Sound Orchestra V; canal 3 estendido e CSM)";
    strings["NEC PC-98 (with Sound Orchestra V in drums mode)##sgpr"].plurals[0] = "NEC PC-98 (com Sound Orchestra V em modo bateria)";
    strings["NEC PC-98 (with Sound Orchestra V in drums mode; extended channel 3)##sgpr"].plurals[0] = "NEC PC-98 (com Sound Orchestra V em modo bateria; canal 3 estendido)";
    strings["NEC PC-98 (with Sound Orchestra V in drums mode; extended channel 3 and CSM)##sgpr"].plurals[0] = "NEC PC-98 (com Sound Orchestra V em modo bateria; canal 3 estendido e CSM)";
    strings["NEC PC-98 (with PC-9801-86)##sgpr"].plurals[0] = "NEC PC-98 (com PC-9801-86)";
    strings["NEC PC-98 (with PC-9801-86; extended channel 3)##sgpr"].plurals[0] = "NEC PC-98 (com PC-9801-86; canal 3 estendido)";
    strings["NEC PC-98 (with PC-9801-86; extended channel 3 and CSM)##sgpr"].plurals[0] = "NEC PC-98 (com PC-9801-86; canal 3 estendido e CSM)";
    strings["NEC PC-98 (with PC-9801-86) stereo##sgpr"].plurals[0] = "NEC PC-98 (com PC-9801-86) stereo";
    strings["NEC PC-98 (with PC-9801-86; extended channel 3) stereo##sgpr"].plurals[0] = "NEC PC-98 (com PC-9801-86; canal 3 estendido) stereo";
    strings["NEC PC-98 (with PC-9801-86; extended channel 3 and CSM) stereo##sgpr"].plurals[0] = "NEC PC-98 (com PC-9801-86; canal 3 estendido e CSM) stereo";
    strings["NEC PC-98 (with PC-9801-73)##sgpr"].plurals[0] = "NEC PC-98 (com PC-9801-73)";
    strings["NEC PC-98 (with PC-9801-73; extended channel 3)##sgpr"].plurals[0] = "NEC PC-98 (com PC-9801-73; canal 3 estendido)";
    strings["NEC PC-98 (with PC-9801-73; extended channel 3 and CSM)##sgpr"].plurals[0] = "NEC PC-98 (com PC-9801-73; canal 3 estendido e CSM)";
    strings["NEC PC-98 (with Sound Blaster 16 for PC-9800 w/PC-9801-26/K compatible)##sgpr"].plurals[0] = "NEC PC-98 (com Sound Blaster 16 para PC-9800 com PC-9801-26/K compatível)";
    strings["NEC PC-98 (with Sound Blaster 16 for PC-9800 w/PC-9801-26/K compatible; extended channel 3)##sgpr"].plurals[0] = "NEC PC-98 (com Sound Blaster 16 para PC-9800 com PC-9801-26/K compatível; canal 3 estendido)";
    strings["NEC PC-98 (with Sound Blaster 16 for PC-9800 w/PC-9801-26/K compatible; extended channel 3 and CSM)##sgpr"].plurals[0] = "NEC PC-98 (com Sound Blaster 16 para PC-9800 com PC-9801-26/K compatível; canal 3 estendido e CSM)";
    strings["NEC PC-98 (with Sound Blaster 16 for PC-9800 w/PC-9801-26/K compatible in drums mode)##sgpr"].plurals[0] = "NEC PC-98 (com Sound Blaster 16 para PC-9800 com PC-9801-26/K compatível em modo bateria)";
    strings["NEC PC-98 (with Sound Blaster 16 for PC-9800 w/PC-9801-26/K compatible in drums mode; extended channel 3)##sgpr"].plurals[0] = "NEC PC-98 (com Sound Blaster 16 para PC-9800 com PC-9801-26/K compatível em modo bateria; canal 3 estendido)";
    strings["NEC PC-98 (with Sound Blaster 16 for PC-9800 w/PC-9801-26/K compatible in drums mode; extended channel 3 and CSM)##sgpr"].plurals[0] = "NEC PC-98 (com Sound Blaster 16 para PC-9800 com PC-9801-26/K compatível em modo bateria; canal 3 estendido e CSM)";
    strings["ZX Spectrum (48K, SFX-like engine)##sgpr"].plurals[0] = "ZX Spectrum (48K, motor estilo SFX)";
    strings["ZX Spectrum (48K, QuadTone engine)##sgpr"].plurals[0] = "ZX Spectrum (48K, motor QuadTone)";
    strings["ZX Spectrum (128K) with TurboSound##sgpr"].plurals[0] = "ZX Spectrum (128K) com TurboSound";
    strings["ZX Spectrum (128K) with TurboSound FM##sgpr"].plurals[0] = "ZX Spectrum (128K) com TurboSound FM";
    strings["ZX Spectrum (128K) with TurboSound FM (extended channel 3 on first OPN)##sgpr"].plurals[0] = "ZX Spectrum (128K) com TurboSound FM (canal 3 estendido no primeiro OPN)";
    strings["ZX Spectrum (128K) with TurboSound FM (extended channel 3 and CSM on first OPN)##sgpr"].plurals[0] = "ZX Spectrum (128K) com TurboSound FM (canal 3 estendido e CSM no primeiro OPN)";
    strings["ZX Spectrum (128K) with TurboSound FM (extended channel 3 on second OPN)##sgpr"].plurals[0] = "ZX Spectrum (128K) com TurboSound FM (canal 3 estendido no segundo OPN)";
    strings["ZX Spectrum (128K) with TurboSound FM (extended channel 3 and CSM on second OPN)##sgpr"].plurals[0] = "ZX Spectrum (128K) com TurboSound FM (canal 3 estendido e CSM no segundo OPN)";
    strings["ZX Spectrum (128K) with TurboSound FM (extended channel 3 on both OPNs)##sgpr"].plurals[0] = "ZX Spectrum (128K) com TurboSound FM (canal 3 estendido em ambos os OPNs)";
    strings["ZX Spectrum (128K) with TurboSound FM (extended channel 3 and CSM on both OPNs)##sgpr"].plurals[0] = "ZX Spectrum (128K) com TurboSound FM (canal 3 estendido em ambos os OPNs)";
    strings["Atari 800 (stereo)##sgpr"].plurals[0] = "Atari 800 (estéreo)";
    strings["PC (beeper)##sgpr"].plurals[0] = "PC (beeper)";
    strings["PC + AdLib (drums mode)##sgpr"].plurals[0] = "PC + AdLib (modo bateria)";
    strings["PC + Sound Blaster (drums mode)##sgpr"].plurals[0] = "PC + Sound Blaster (modo bateria)";
    strings["PC + Sound Blaster w/Game Blaster Compatible##sgpr"].plurals[0] = "PC + Sound Blaster com compatibilidade Game Blaster";
    strings["PC + Sound Blaster w/Game Blaster Compatible (drums mode)##sgpr"].plurals[0] = "PC + Sound Blaster com compatibilidade Game Blaster (modo bateria)";
    strings["PC + Sound Blaster Pro (drums mode)##sgpr"].plurals[0] = "PC + Sound Blaster Pro (modo bateria)";
    strings["PC + Sound Blaster Pro 2 (drums mode)##sgpr"].plurals[0] = "PC + Sound Blaster Pro 2 (modo bateria)";
    strings["PC + ESS AudioDrive ES1488 (native ESFM mode)##sgpr"].plurals[0] = "PC + ESS AudioDrive ES1488 (modo ESFM nativo)";
    strings["Sharp X1 + FM addon##sgpr"].plurals[0] = "Sharp X1 + acessório FM";
    strings["FM Towns (extended channel 3)##sgpr"].plurals[0] = "FM Towns (canal 3 estendido)";
    strings["Commander X16 (VERA only)##sgpr0"].plurals[0] = "Commander X16 (VERA apenas)";
    strings["Commander X16 (with OPM)##sgpr"].plurals[0] = "Commander X16 (com OPM)";
    strings["Commander X16 (with Twin OPL3)##sgpr"].plurals[0] = "Commander X16 (com 2x OPL3)";
    strings["Arcade systems##sgpr"].plurals[0] = "Sistemas Arcade";
    strings["INSERT COIN##sgpr"].plurals[0] = "Ô tio, me vê uma ficha aí!";
    strings["Williams/Midway Y/T unit w/ADPCM sound board##sgpr"].plurals[0] = "Placa Williams/Midway Y/T com placa de som ADPCM";
    strings["Konami Battlantis (drums mode on first OPL2)##sgpr"].plurals[0] = "Konami Battlantis (modo bateria no primeiro OPL2)";
    strings["Konami Battlantis (drums mode on second OPL2)##sgpr"].plurals[0] = "Konami Battlantis (modo bateria no segundo OPL2)";
    strings["Konami Battlantis (drums mode on both OPL2s)##sgpr"].plurals[0] = "Konami Battlantis (modo bateria em ambos os OPL2s)";
    strings["Konami Haunted Castle (drums mode)##sgpr"].plurals[0] = "Konami Haunted Castle (modo bateria)";
    strings["Konami S.P.Y. (drums mode)##sgpr"].plurals[0] = "Konami S.P.Y. (modo bateria)";
    strings["Konami Rollergames (drums mode)##sgpr"].plurals[0] = "Konami Rollergames (modo bateria)";
    strings["Sega System E (with FM expansion)##sgpr"].plurals[0] = "Sega System E (com expansão FM)";
    strings["Sega System E (with FM expansion in drums mode)##sgpr"].plurals[0] = "Sega System E (com expansão FM em modo bateria)";
    strings["Sega Hang-On (extended channel 3)##sgpr"].plurals[0] = "Sega Hang-On (canal 3 estendido)";
    strings["Sega Hang-On (extended channel 3 and CSM)##sgpr"].plurals[0] = "Sega Hang-On (canal 3 estendido e CSM)";
    strings["Sega System 18 (extended channel 3 on first OPN2C)##sgpr"].plurals[0] = "Sega System 18 (canal 3 estendido no primeiro OPN2C)";
    strings["Sega System 18 (extended channel 3 and CSM on first OPN2C)##sgpr"].plurals[0] = "Sega System 18 (canal 3 estendido e CSM no primeiro OPN2C)";
    strings["Sega System 18 (extended channel 3 on second OPN2C)##sgpr"].plurals[0] = "Sega System 18 (canal 3 estendido no segundo OPN2C)";
    strings["Sega System 18 (extended channel 3 and CSM on second OPN2C)##sgpr"].plurals[0] = "Sega System 18 (canal 3 estendido e CSM no segundo OPN2C)";
    strings["Sega System 18 (extended channel 3 on both OPN2Cs)##sgpr"].plurals[0] = "Sega System 18 (canal 3 estendido em ambos os OPN2Cs)";
    strings["Sega System 18 (extended channel 3 and CSM on both OPN2Cs)##sgpr"].plurals[0] = "Sega System 18 (canal 3 estendido e CSM em ambos os OPN2Cs)";
    strings["Sega System 32 (extended channel 3 on first OPN2C)##sgpr"].plurals[0] = "Sega System 32 (canal 3 estendido no primeiro OPN2C)";
    strings["Sega System 32 (extended channel 3 and CSM on first OPN2C)##sgpr"].plurals[0] = "Sega System 32 (canal 3 estendido e CSM no primeiro OPN2C)";
    strings["Sega System 32 (extended channel 3 on second OPN2C)##sgpr"].plurals[0] = "Sega System 32 (canal 3 estendido no segundo OPN2C)";
    strings["Sega System 32 (extended channel 3 and CSM on second OPN2C)##sgpr"].plurals[0] = "Sega System 32 (canal 3 estendido e CSM no segundo OPN2C)";
    strings["Sega System 32 (extended channel 3 on both OPN2Cs)##sgpr"].plurals[0] = "Sega System 32 (canal 3 estendido em ambos os OPN2Cs)";
    strings["Sega System 32 (extended channel 3 and CSM on both OPN2Cs)##sgpr"].plurals[0] = "Sega System 32 (canal 3 estendido e CSM em ambos os OPN2Cs)";
    strings["Capcom Arcade##sgpr"].plurals[0] = "Capcom Arcade";
    strings["Capcom Arcade (extended channel 3 on first OPN)##sgpr"].plurals[0] = "Capcom Arcade (canal 3 estendido no primeiro OPN)";
    strings["Capcom Arcade (extended channel 3 and CSM on first OPN)##sgpr"].plurals[0] = "Capcom Arcade (canal 3 estendido e CSM no primeiro OPN)";
    strings["Capcom Arcade (extended channel 3 on second OPN)##sgpr"].plurals[0] = "Capcom Arcade (canal 3 estendido no segundo OPN)";
    strings["Capcom Arcade (extended channel 3 and CSM on second OPN)##sgpr"].plurals[0] = "Capcom Arcade (canal 3 estendido e CSM no segundo OPN)";
    strings["Capcom Arcade (extended channel 3 on both OPNs)##sgpr"].plurals[0] = "Capcom Arcade (canal 3 estendido em ambos os OPNs)";
    strings["Capcom Arcade (extended channel 3 and CSM on both OPNs)##sgpr"].plurals[0] = "Capcom Arcade (canal 3 estendido e CSM em ambos os OPNs)";
    strings["Jaleco Ginga NinkyouDen (drums mode)##sgpr"].plurals[0] = "Jaleco Ginga NinkyouDen (modo bateria)";
    strings["NMK 16-bit Arcade##sgpr"].plurals[0] = "NMK Arcade 16-bit";
    strings["NMK 16-bit Arcade (extended channel 3)##sgpr"].plurals[0] = "NMK Arcade 16-bit (canal 3 estendido)";
    strings["NMK 16-bit Arcade (extended channel 3 and CSM)##sgpr"].plurals[0] = "NMK Arcade 16-bit (canal 3 estendido e CSM)";
    strings["NMK 16-bit Arcade (w/NMK112 bankswitching)##sgpr"].plurals[0] = "NMK Arcade 16-bit (com bankswitching NMK112)";
    strings["NMK 16-bit Arcade (w/NMK112 bankswitching, extended channel 3)##sgpr"].plurals[0] = "NMK Arcade 16-bit (com bankswitching NMK112, canal 3 estendido)";
    strings["NMK 16-bit Arcade (w/NMK112 bankswitching, extended channel 3 and CSM)##sgpr"].plurals[0] = "NMK 16-bit Arcade (com bankswitching NMK112, canal 3 estendido e CSM)";
    strings["Atlus Power Instinct 2 (extended channel 3)##sgpr"].plurals[0] = "Atlus Power Instinct 2 (canal 3 estendido)";
    strings["Atlus Power Instinct 2 (extended channel 3 and CSM)##sgpr"].plurals[0] = "Atlus Power Instinct 2 (canal 3 estendido e CSM)";
    strings["Kaneko DJ Boy (extended channel 3)##sgpr"].plurals[0] = "Kaneko DJ Boy (canal 3 estendido)";
    strings["Kaneko DJ Boy (extended channel 3 and CSM)##sgpr"].plurals[0] = "Kaneko DJ Boy (canal 3 estendido e CSM)";
    strings["Kaneko Air Buster (extended channel 3)##sgpr"].plurals[0] = "Kaneko Air Buster (canal 3 estendido)";
    strings["Kaneko Air Buster (extended channel 3 and CSM)##sgpr"].plurals[0] = "Kaneko Air Buster (canal 3 estendido e CSM)";
    strings["Tecmo Ninja Gaiden (extended channel 3 on first OPN)##sgpr"].plurals[0] = "Tecmo Ninja Gaiden (canal 3 estendido no primeiro OPN)";
    strings["Tecmo Ninja Gaiden (extended channel 3 and CSM on first OPN)##sgpr"].plurals[0] = "Tecmo Ninja Gaiden (canal 3 estendido e CSM no primeiro OPN)";
    strings["Tecmo Ninja Gaiden (extended channel 3 on second OPN)##sgpr"].plurals[0] = "Tecmo Ninja Gaiden (canal 3 estendido no segundo OPN)";
    strings["Tecmo Ninja Gaiden (extended channel 3 and CSM on second OPN)##sgpr"].plurals[0] = "Tecmo Ninja Gaiden (canal 3 estendido e CSM no segundo OPN)";
    strings["Tecmo Ninja Gaiden (extended channel 3 on both OPNs)##sgpr"].plurals[0] = "Tecmo Ninja Gaiden (canal 3 estendido em ambos os OPNs)";
    strings["Tecmo Ninja Gaiden (extended channel 3 and CSM on both OPNs)##sgpr"].plurals[0] = "Tecmo Ninja Gaiden (canal 3 estendido e CSM em ambos os OPNs)";
    strings["Tecmo System (drums mode)##sgpr"].plurals[0] = "Tecmo System (modo bateria)";
    strings["Seibu Kaihatsu Raiden (drums mode)##sgpr"].plurals[0] = "Seibu Kaihatsu Raiden (modo bateria)";
    strings["Sunsoft Arcade##sgpr"].plurals[0] = "Sunsoft Arcade";
    strings["Sunsoft Arcade (extended channel 3)##sgpr"].plurals[0] = "Sunsoft Arcade (canal 3 estendido)";
    strings["Sunsoft Arcade (extended channel 3 and CSM)##sgpr"].plurals[0] = "Sunsoft Arcade (canal 3 estendido e CSM)";
    strings["Atari Rampart (drums mode)##sgpr"].plurals[0] = "Atari Rampart (modo bateria)";
    strings["Data East Karnov (extended channel 3)##sgpr"].plurals[0] = "Data East Karnov (canal 3 estendido)";
    strings["Data East Karnov (extended channel 3 and CSM)##sgpr"].plurals[0] = "Data East Karnov (canal 3 estendido e CSM)";
    strings["Data East Karnov (drums mode)##sgpr"].plurals[0] = "Data East Karnov (modo bateria)";
    strings["Data East Karnov (extended channel 3; drums mode)##sgpr"].plurals[0] = "Data East Karnov (canal 3 estendido; modo bateria)";
    strings["Data East Karnov (extended channel 3 and CSM; drums mode)##sgpr"].plurals[0] = "Data East Karnov (canal 3 estendido e CSM; modo bateria)";
    strings["Data East Arcade##sgpr"].plurals[0] = "Data East Arcade";
    strings["Data East Arcade (extended channel 3)##sgpr"].plurals[0] = "Data East Arcade (canal 3 estendido)";
    strings["Data East Arcade (extended channel 3 and CSM)##sgpr"].plurals[0] = "Data East Arcade (canal 3 estendido e CSM)";
    strings["Data East Arcade (drums mode)##sgpr"].plurals[0] = "Data East Arcade (modo bateria)";
    strings["Data East Arcade (extended channel 3; drums mode)##sgpr"].plurals[0] = "Data East Arcade (canal 3 estendido; modo bateria)";
    strings["Data East Arcade (extended channel 3 and CSM; drums mode)##sgpr"].plurals[0] = "Data East Arcade (canal 3 estendido e CSM; modo bateria)";
    strings["Data East PCX (extended channel 3)##sgpr"].plurals[0] = "Data East PCX (canal 3 estendido)";
    strings["Data East PCX (extended channel 3 and CSM)##sgpr"].plurals[0] = "Data East PCX (canal 3 estendido e CSM)";
    strings["Data East Dark Seal (extended channel 3)##sgpr"].plurals[0] = "Data East Dark Seal (canal 3 estendido)";
    strings["Data East Dark Seal (extended channel 3 and CSM)##sgpr"].plurals[0] = "Data East Dark Seal (canal 3 estendido e CSM)";
    strings["SNK Ikari Warriors (drums mode on first OPL)##sgpr"].plurals[0] = "SNK Ikari Warriors (modo bateria no primeiro OPL)";
    strings["SNK Ikari Warriors (drums mode on second OPL)##sgpr"].plurals[0] = "SNK Ikari Warriors (modo bateria no segundo OPL)";
    strings["SNK Ikari Warriors (drums mode on both OPLs)##sgpr"].plurals[0] = "SNK Ikari Warriors (modo bateria em ambos OPLs)";
    strings["SNK Triple Z80 (drums mode on Y8950)##sgpr"].plurals[0] = "SNK Triple Z80 (modo bateria no Y8950)";
    strings["SNK Triple Z80 (drums mode on OPL)##sgpr"].plurals[0] = "SNK Triple Z80 (modo bateria no OPL)";
    strings["SNK Triple Z80 (drums mode on Y8950 and OPL)##sgpr"].plurals[0] = "SNK Triple Z80 (modo bateria no Y8950 e no OPL)";
    strings["SNK Chopper I (drums mode on Y8950)##sgpr"].plurals[0] = "SNK Chopper I (modo bateria no Y8950)";
    strings["SNK Chopper I (drums mode on OPL2)##sgpr"].plurals[0] = "SNK Chopper I (modo bateria no OPL2)";
    strings["SNK Chopper I (drums mode on Y8950 and OPL2)##sgpr"].plurals[0] = "SNK Chopper I (modo bateria no Y8950 e no OPL2)";
    strings["SNK Touchdown Fever (drums mode on OPL)##sgpr"].plurals[0] = "SNK Touchdown Fever (modo bateria no OPL)";
    strings["SNK Touchdown Fever (drums mode on Y8950)##sgpr"].plurals[0] = "SNK Touchdown Fever (modo bateria no Y8950)";
    strings["SNK Touchdown Fever (drums mode on OPL and Y8950)##sgpr"].plurals[0] = "SNK Touchdown Fever (modo bateria no OPL e no Y8950)";
    strings["Alpha denshi Alpha-68K (extended channel 3)##sgpr"].plurals[0] = "Alpha denshi Alpha-68K (canal 3 estendido)";
    strings["Alpha denshi Alpha-68K (extended channel 3 and CSM)##sgpr"].plurals[0] = "Alpha denshi Alpha-68K (canal 3 estendido e CSM)";
    strings["Alpha denshi Alpha-68K (drums mode)##sgpr"].plurals[0] = "Alpha denshi Alpha-68K (modo bateria)";
    strings["Alpha denshi Alpha-68K (extended channel 3; drums mode)##sgpr"].plurals[0] = "Alpha denshi Alpha-68K (canal 3 estendido; modo bateria)";
    strings["Alpha denshi Alpha-68K (extended channel 3 and CSM; drums mode)##sgpr"].plurals[0] = "Alpha denshi Alpha-68K (canal 3 estendido e CSM; modo bateria)";
    strings["Neo Geo MVS (extended channel 2)##sgpr"].plurals[0] = "Neo Geo MVS (canal 2 estendido)";
    strings["Neo Geo MVS (extended channel 2 and CSM)##sgpr"].plurals[0] = "Neo Geo MVS (canal 2 estendido e CSM)";
    strings["Namco (3-channel WSG)##sgpr"].plurals[0] = "Namco (WSG de 3 canais)";
    strings["Taito Arcade##sgpr"].plurals[0] = "Taito Arcade";
    strings["Taito Arcade (extended channel 3)##sgpr"].plurals[0] = "Taito Arcade (canal 3 estendido)";
    strings["Taito Arcade (extended channel 3 and CSM)##sgpr"].plurals[0] = "Taito Arcade (canal 3 estendido e CSM)";
    strings["Seta 1 + FM addon##sgpr"].plurals[0] = "Seta 1 + acessório FM";
    strings["Seta 1 + FM addon (extended channel 3)##sgpr"].plurals[0] = "Seta 1 + acessório FM (canal 3 estendido)";
    strings["Seta 1 + FM addon (extended channel 3 and CSM)##sgpr"].plurals[0] = "Seta 1 + acessório FM (canal 3 estendido e CSM)";
    strings["Coreland Cyber Tank (drums mode)##sgpr"].plurals[0] = "Coreland Cyber Tank (modo bateria)";
    strings["Toaplan 1 (drums mode)##sgpr"].plurals[0] = "Toaplan 1 (modo bateria)";
    strings["Dynax/Nakanihon 3rd generation hardware##sgpr"].plurals[0] = "Hardware Dynax/Nakanihon de 3ª geração";
    strings["Dynax/Nakanihon 3rd generation hardware (drums mode)##sgpr"].plurals[0] = "Hardware Dynax/Nakanihon de 3ª geração (modo bateria)";
    strings["Dynax/Nakanihon Real Break (drums mode)##sgpr"].plurals[0] = "Dynax/Nakanihon Real Break (modo bateria)";
    strings["User##sgpr"].plurals[0] = "Usuário";
    strings["system presets that you have saved.##sgpr"].plurals[0] = "predefinições de sistema que você salvou.";
    strings["chips which use frequency modulation (FM) to generate sound.\nsome of these also pack more (like square and sample channels).\nActually \"FM\" here stands for phase modulation,\nbut these two are indistinguishable\nif you use sine waves.##sgpr"].plurals[0] = "chips que utilizam modulação de frequência (FM) para gerar som.\nalguns destes também possuem mais recursos (como canais de onda quadrada e samples).\nNa verdade, \"FM\" aqui significa modulação de fase,\nmas as duas técnicas são indistinguíveis\nse você utilizar ondas senoidais.";
    strings["Yamaha YM2203 (extended channel 3)##sgpr"].plurals[0] = "Yamaha YM2203 (canal 3 estendido)";
    strings["Yamaha YM2203 (extended channel 3 and CSM)##sgpr"].plurals[0] = "Yamaha YM2203 (canal 3 estendido e CSM)";
    strings["Yamaha YM2608 (extended channel 3)##sgpr"].plurals[0] = "Yamaha YM2608 (canal 3 estendido e CSM)";
    strings["Yamaha YM2608 (extended channel 3 and CSM)##sgpr"].plurals[0] = "Yamaha YM2608 (canal 3 estendido e CSM)";
    strings["Yamaha YM2610 (extended channel 2)##sgpr"].plurals[0] = "Yamaha YM2610 (canal 2 estendido)";
    strings["Yamaha YM2610 (extended channel 2 and CSM)##sgpr"].plurals[0] = "Yamaha YM2610 (canal 2 estendido e CSM)";
    strings["Yamaha YM2610B (extended channel 3)##sgpr"].plurals[0] = "Yamaha YM2610B (canal 3 estendido)";
    strings["Yamaha YM2610B (extended channel 3 and CSM)##sgpr"].plurals[0] = "Yamaha YM2610B (canal 3 estendido e CSM)";
    strings["Yamaha YM2612 (extended channel 3)##sgpr"].plurals[0] = "Yamaha YM2612 (canal 3 estendido)";
    strings["Yamaha YM2612 (OPN2) with DualPCM##sgpr"].plurals[0] = "Yamaha YM2612 (OPN2) com DualPCM";
    strings["Yamaha YM2612 (extended channel 3) with DualPCM and CSM##sgpr"].plurals[0] = "Yamaha YM2612 (canal 3 estendido) com DualPCM e CSM";
    strings["Yamaha YMF276 (extended channel 3)##sgpr"].plurals[0] = "Yamaha YMF276 (canal 3 estendido)";
    strings["Yamaha YMF276 with DualPCM##sgpr"].plurals[0] = "Yamaha YMF276 com DualPCM";
    strings["Yamaha YMF276 (extended channel 3) with DualPCM and CSM##sgpr"].plurals[0] = "Yamaha YMF276 (canal 3 estendido) com DualPCM e CSM";
    strings["Yamaha YM2413 (drums mode)##sgpr"].plurals[0] = "Yamaha YM2413 (modo bateria)";
    strings["Yamaha YM3438 (extended channel 3)##sgpr"].plurals[0] = "Yamaha YM3438 (canal 3 estendido)";
    strings["Yamaha YM3438 (OPN2C) with DualPCM##sgpr"].plurals[0] = "Yamaha YM3438 (OPN2C) com DualPCM";
    strings["Yamaha YM3438 (extended channel 3) with DualPCM and CSM##sgpr"].plurals[0] = "Yamaha YM3438 (canal 3 estendido) com DualPCM e CSM";
    strings["Yamaha YM3526 (drums mode)##sgpr"].plurals[0] = "Yamaha YM3526 (modo bateria)";
    strings["Yamaha Y8950 (drums mode)##sgpr"].plurals[0] = "Yamaha Y8950 (modo bateria)";
    strings["Yamaha YM3812 (drums mode)##sgpr"].plurals[0] = "Yamaha YM3812 (modo bateria)";
    strings["Yamaha YMF262 (drums mode)##sgpr"].plurals[0] = "Yamaha YMF262 (modo bateria)";
    strings["Yamaha YMF289B (drums mode)##sgpr"].plurals[0] = "Yamaha YMF289B (modo bateria)";
    strings["ESS ES1xxx series (ESFM)##sgpr"].plurals[0] = "ESS série ES1xxx (ESFM)";
    strings["Square##sgpr"].plurals[0] = "Onda quadrada";
    strings["these chips generate square/pulse tones only (but may include noise).##sgpr"].plurals[0] = "esses chips geram tons de onda quadrada/pulsada somente (mas podem incluir ruído).";
    strings["Tandy PSSJ 3-voice sound##sgpr"].plurals[0] = "Som integrado Tandy PSSJ de 3 canais";
    strings["Sega PSG (SN76489-like)##sgpr"].plurals[0] = "Sega PSG (similar ao SN76489)";
    strings["Sega PSG (SN76489-like, Stereo)##sgpr"].plurals[0] = "Sega PSG (similar ao SN76489, Estéreo)";
    strings["PC Speaker##sgpr"].plurals[0] = "PC Speaker";
    strings["Sample##sgpr"].plurals[0] = "Sample";
    strings["chips/systems which use PCM or ADPCM samples for sound synthesis.##sgpr"].plurals[0] = "chips/sistemas que usam samples PCM ou ADPCM para geração de som.";
    strings["Generic PCM DAC##sgpr"].plurals[0] = "DAC PCM Genérico";
    strings["Wavetable##sgpr"].plurals[0] = "Wavetable";
    strings["chips which use user-specified waveforms to generate sound.##sgpr"].plurals[0] = "chips que usam formas de onda especificadas pelo usuários para gerar som.";
    strings["Namco C15 (8-channel mono)##sgpr"].plurals[0] = "Namco C15 (mono, 8 canais)";
    strings["Namco C30 (8-channel stereo)##sgpr"].plurals[0] = "Namco C30 (estéreo, 8 canais)";
    strings["Famicom Disk System (chip)##sgpr"].plurals[0] = "Famicom Disk System (chip)";
    strings["Specialized##sgpr"].plurals[0] = "Especializado";
    strings["chips/systems with unique sound synthesis methods.##sgpr"].plurals[0] = "chips/sistemas com métodos únicos de geração/síntese de som.";
    strings["Commodore PET (pseudo-wavetable)##sgpr"].plurals[0] = "Commodore PET (pseudo-wavetable)";
    strings["ZX Spectrum (beeper only, SFX-like engine)##sgpr"].plurals[0] = "ZX Spectrum (beeper apenas, motor estilo SFX)";
    strings["ZX Spectrum (beeper only, QuadTone engine)##sgpr"].plurals[0] = "ZX Spectrum (beeper apenas, motor QuadTone)";
    strings["Modern/fantasy##sgpr"].plurals[0] = "Moderno/Fantasia";
    strings["chips/systems which do not exist in reality or were made just several years ago.##sgpr"].plurals[0] = "chips/sistemas que não existem fisicamente ou foram feitos apenas alguns anos atrás.";
    strings["Commander X16 (VERA only)##sgpr1"].plurals[0] = "Commander X16 (VERA apenas)";
    strings["DefleMask-compatible##sgpr"].plurals[0] = "Compatível com DefleMask";
    strings["these configurations are compatible with DefleMask.\nselect this if you need to save as .dmf or work with that program.##sgpr"].plurals[0] = "essas configurações são compatíveis com o DefleMask.\nselecione uma delas se você precisar salvar no formato .dmf ou trabalhar com este programa.";
    strings["Sega Genesis (extended channel 3)##sgpr1"].plurals[0] = "Sega Mega Drive (canal 3 estendido)";
    strings["Sega Master System (with FM expansion)##sgpr1"].plurals[0] = "Sega Master System (com expansão FM)";
    strings["Famicom with Konami VRC7##sgpr1"].plurals[0] = "Famicom com Konami VRC7";
    strings["Arcade (YM2151 and SegaPCM)##sgpr1"].plurals[0] = "Arcade (YM2151 e SegaPCM)";
    strings["Neo Geo CD (extended channel 2)##sgpr1"].plurals[0] = "Neo Geo CD (canal 2 estendido)";

    //   sgrv  src/gui/regView.cpp

    strings["Register View###Register View"].plurals[0] = "Visualização de Registradores###Register View";
    strings["- no register pool available##sgrv"].plurals[0] = "- nenhum mapa de registradores disponível";

    //  sgsed  src/gui/sampleEdit.cpp

    strings["%s: maximum sample rate is %d##sgsed"].plurals[0] = "%s: a taxa de amostragem máxima é %d";
    strings["%s: minimum sample rate is %d##sgsed"].plurals[0] = "%s: a taxa de amostragem mínima é %d";
    strings["%s: sample rate must be %d##sgsed"].plurals[0] = "%s: a taxa de amostragem deve ser %d";
    strings["Sample Editor###Sample Editor"].plurals[0] = "Editor de Sample###Sample Editor";
    strings["no sample selected##sgsed"].plurals[0] = "nenhuma sample selecionada";
    strings["select one...##sgsed"].plurals[0] = "selecione uma...";
    strings["or##sgsed0"].plurals[0] = "ou";
    strings["Open##sgsed0"].plurals[0] = "Abrir";
    strings["or##sgsed1"].plurals[0] = "ou";
    strings["Create New##sgsed"].plurals[0] = "Criar uma Nova";
    strings["Invalid##sgsed0"].plurals[0] = "Inválido";
    strings["Invalid##sgsed1"].plurals[0] = "Inválido";
    strings["%d: %s"].plurals[0] = "%d: %s";
    strings["Open##sgsed1"].plurals[0] = "Abrir";
    strings["import raw...##sgsed"].plurals[0] = "importar dados raw...";
    strings["Save##sgsed"].plurals[0] = "Salvar";
    strings["save raw...##sgsed"].plurals[0] = "salvar dados raw...";
    strings["Name##sgsed"].plurals[0] = "Nome";
    strings["SNES: loop start must be a multiple of 16 (try with %d)##sgsed"].plurals[0] = "SNES: início do loop precisa ser múltiplo de 16 (tente com %d)";
    strings["SNES: loop end must be a multiple of 16 (try with %d)##sgsed"].plurals[0] = "SNES: fim do loop precisa ser múltiplo de 16 (tente com %d)";
    strings["SNES: sample length will be padded to multiple of 16##sgsed"].plurals[0] = "SNES: tamanho da sample precisa ser preenchida até um múltiplo de 16";
    strings["QSound: loop cannot be longer than 32767 samples##sgsed"].plurals[0] = "QSound: loop não pode ser mais longo que 32767";
    strings["QSound: maximum sample length is 65535##sgsed"].plurals[0] = "QSound: o tamanho máximo de uma sample é 65535";
    strings["NES: loop point ignored on DPCM (may only loop entire sample)##sgsed"].plurals[0] = "NES: o ponto de repetição (loop) é ignorado no DPCM (só é possível repetir a sample inteira)";
    strings["NES: maximum DPCM sample length is 32648##sgsed"].plurals[0] = "NES: o tamanho máximo de uma sample DPCM é 32648";
    strings["X1-010: samples can't loop##sgsed"].plurals[0] = "X1-010: não é possível repetir (loop) samples";
    strings["X1-010: maximum sample length is 131072##sgsed"].plurals[0] = "X1-010: o tamanho máximo de uma sample é 131072";
    strings["GA20: samples can't loop##sgsed"].plurals[0] = "GA20: samples não podem repetir (loop)";
    strings["YM2608: loop point ignored on ADPCM (may only loop entire sample)##sgsed"].plurals[0] = "YM2608: o ponto de repetição (loop) é ignorado no ADPCM (só é possível repetir a sample inteira)";
    strings["YM2608: sample length will be padded to multiple of 512##sgsed"].plurals[0] = "YM2608: o comprimento da sample será preenchida até um múltiplo de 512";
    strings["YM2610: ADPCM-A samples can't loop##sgsed"].plurals[0] = "YM2610: samples ADPCM-A não podem repetir (loop)";
    strings["YM2610: loop point ignored on ADPCM-B (may only loop entire sample)##sgsed"].plurals[0] = "YM2610: o ponto de repetição (loop) é ignorado no ADPCM-B (só é possível repetir a sample inteira)";
    strings["YM2610: sample length will be padded to multiple of 512##sgsed"].plurals[0] = "YM2610: o comprimento da sample será preenchida até um múltiplo de 512";
    strings["YM2610: maximum ADPCM-A sample length is 2097152##sgsed"].plurals[0] = "YM2610: o comprimento máximo de uma sample ADPCM-A é 2097152";
    strings["Y8950: loop point ignored on ADPCM (may only loop entire sample)##sgsed"].plurals[0] = "Y8950: o ponto de repetição (loop) é ignorado no ADPCM (só é possível repetir a sample inteira)";
    strings["Y8950: sample length will be padded to multiple of 512##sgsed"].plurals[0] = "Y8950: o comprimento da sample será preenchida até um múltiplo de 512";
    strings["Amiga: loop start must be a multiple of 2##sgsed"].plurals[0] = "Amiga: o início do loop precisa ser um múltiplo de 2";
    strings["Amiga: loop end must be a multiple of 2##sgsed"].plurals[0] = "Amiga: o final do loop precisa ser um múltiplo de 2";
    strings["Amiga: maximum sample length is 131070##sgsed"].plurals[0] = "Amiga: o comprimento máximo da sample é 131070";
    strings["SegaPCM: maximum sample length is 65280##sgsed"].plurals[0] = "SegaPCM: o comprimento máximo da sample é 65280";
    strings["K053260: loop point ignored (may only loop entire sample)##sgsed"].plurals[0] = "K053260: o ponto de repetição (loop) é ignorado (só é possível repetir a sample inteira)";
    strings["K053260: maximum sample length is 65535##sgsed"].plurals[0] = "K053260: o comprimento máximo da sample é 65535";
    strings["C140: maximum sample length is 65535##sgsed"].plurals[0] = "C140: o comprimento máximo da sample é 65535";
    strings["C219: loop start must be a multiple of 2##sgsed"].plurals[0] = "C219: o início do loop precisa ser um múltiplo de 2";
    strings["C219: loop end must be a multiple of 2##sgsed"].plurals[0] = "C219: o final do loop precisa ser um múltiplo de 2";
    strings["C219: maximum sample length is 131072##sgsed"].plurals[0] = "C219: o comprimento máximo da sample é 131072";
    strings["MSM6295: samples can't loop##sgsed"].plurals[0] = "MSM6295: não é possível repetir (loop) samples";
    strings["MSM6295: maximum bankswitched sample length is 129024##sgsed"].plurals[0] = "MSM6295: o comprimento máximo da sample com bankswitching é 129024";
    strings["GBA DMA: loop start must be a multiple of 4##sgsed"].plurals[0] = "GBA DMA: o início do loop precisa ser um múltiplo de 4";
    strings["GBA DMA: loop length must be a multiple of 16##sgsed"].plurals[0] = "GBA DMA: o comprimento do loop precisa ser um múltiplo de 16";
    strings["GBA DMA: sample length will be padded to multiple of 16##sgsed"].plurals[0] = "GBA DMA: o comprimento da sample será preenchida até um múltiplo de 16";
    strings["ES5506: backward loop mode isn't supported##sgsed"].plurals[0] = "ES5506: modo de repetição (loop) reverso não é suportado";
    strings["backward/ping-pong only supported in Generic PCM DAC\nping-pong also on ES5506##sgsed"].plurals[0] = "reverso/pingue-pongue suportado somente no DAC PCM Genérico\npingue-pongue também suportado no ES5506";
    strings["Info##sgsed"].plurals[0] = "Informações";
    strings["Rate##sgsed0"].plurals[0] = "Taxa";
    strings["Compat Rate##sgsed"].plurals[0] = "Taxa de Compatibilidade";
    strings["used in DefleMask-compatible sample mode (17xx), in where samples are mapped to an octave.##sgsed"].plurals[0] = "usado no modo de sample compatível com Deflemask (17xx), em que samples são mapeadas a uma oitava.";
    strings["Loop (length: %d)##Loop"].plurals[0] = "Loop (comprimento: %d)";
    strings["Loop##sgsed"].plurals[0] = "Loop (repetição)";
    strings["changing the loop in a BRR sample may result in glitches!##sgsed0"].plurals[0] = "alterar o loop em uma sample BRR pode resultar em falhas!";
    strings["Chips##sgsed"].plurals[0] = "Chips";
    strings["Type##sgsed"].plurals[0] = "Tipo";
    strings["BRR emphasis##sgsed"].plurals[0] = "Ênfase BRR";
    strings["this is a BRR sample.\nenabling this option will muffle it (only affects non-SNES chips).##sgsed"].plurals[0] = "esta é uma sample BRR.\nhabilitar essa opção irá abafá-la (afeta somente chips além do SNES).";
    strings["enable this option to slightly boost high frequencies\nto compensate for the SNES' Gaussian filter's muffle.##sgsed"].plurals[0] = "habilite essa opção para enfatizar levemente as frequências altas\npara compensar pelo abafamento do filtro gaussiano do SNES.";
    strings["8-bit dither##sgsed"].plurals[0] = "Aplicar dither 8-bit";
    strings["dither the sample when used on a chip that only supports 8-bit samples.##sgsed"].plurals[0] = "Aplicar dithering 8-bit na sample quando utilizada em um chip que só suporta samples 8-bit.";
    strings["Note##sgsed"].plurals[0] = "Nota";
    strings["%s"].plurals[0] = "%s";
    strings["Fine##sgsed"].plurals[0] = "Ajuste fino";
    strings["Mode##sgsed"].plurals[0] = "Modo";
    strings["Start##sgsed"].plurals[0] = "Início";
    strings["changing the loop in a BRR sample may result in glitches!##sgsed1"].plurals[0] = "alterar o loop em uma sample BRR pode resultar em falhas!";
    strings["End##sgsed"].plurals[0] = "Final";
    strings["changing the loop in a BRR sample may result in glitches!##sgsed2"].plurals[0] = "alterar o loop em uma sample BRR pode resultar em falhas!";
    strings["%s\n%d bytes free##sgsed"].plurals[0] = "%s\n%d byte disponível";
    strings["%s\n%d bytes free##sgsed"].plurals[1] = "%s\n%d bytes disponíveis";
    strings["%s (%s)\n%d bytes free##sgsed"].plurals[0] = "%s (%s)\n%d byte disponível";
    strings["%s (%s)\n%d bytes free##sgsed"].plurals[1] = "%s (%s)\n%d bytes disponíveis";
    strings["\n\nnot enough memory for this sample!##sgsed"].plurals[0] = "\n\nnão há memória suficiente para esta sample!";
    strings["Edit mode: Select##sgsed"].plurals[0] = "Modo de edição: Selecionar";
    strings["Edit mode: Draw##sgsed"].plurals[0] = "Modo de edição: Desenhar";
    strings["Resize##sgsed0"].plurals[0] = "Redimensionar";
    strings["Samples##sgsed0"].plurals[0] = "Amostras";
    strings["Resize##sgsed1"].plurals[0] = "Redimensionar";
    strings["couldn't resize! make sure your sample is 8 or 16-bit.##sgsed"].plurals[0] = "não pôde redimensionar! verifique se sua sample é 8 or 16-bit.";
    strings["Resample##sgsed0"].plurals[0] = "Reamostrar";
    strings["Rate##sgsed1"].plurals[0] = "Taxa";
    strings["Factor##sgsed"].plurals[0] = "Razão"; // TODO: tem uma palavra melhor?
    strings["Filter##sgsed"].plurals[0] = "Filtro";
    strings["Resample##sgsed1"].plurals[0] = "Reamostrar";
    strings["couldn't resample! make sure your sample is 8 or 16-bit.##sgsed"].plurals[0] = "não pôde redimensionar! verifique se sua sample é 8 or 16-bit.";
    strings["Undo##sgsed"].plurals[0] = "Desfazer";
    strings["Redo##sgsed"].plurals[0] = "Refazer";
    strings["Amplify##sgsed"].plurals[0] = "Amplificar";
    strings["Volume##sgsed"].plurals[0] = "Volume";
    strings["Apply##sgsed0"].plurals[0] = "Aplicar";
    strings["Normalize##sgsed"].plurals[0] = "Normalizar";
    strings["Fade in##sgsed"].plurals[0] = "Fade in";
    strings["Fade out##sgsed"].plurals[0] = "Fade out";
    strings["Insert silence##sgsed"].plurals[0] = "Inserir silêncio";
    strings["Samples##sgsed1"].plurals[0] = "Amostras";
    strings["Go##sgsed"].plurals[0] = "Aplicar";
    strings["couldn't insert! make sure your sample is 8 or 16-bit.##sgsed"].plurals[0] = "não pôde inserir! verifique se sua sample é 8 or 16-bit.";
    strings["Apply silence##sgsed"].plurals[0] = "Aplicar silêncio";
    strings["Delete##sgsed"].plurals[0] = "Excluir";
    strings["Trim##sgsed"].plurals[0] = "Cortar";
    strings["Reverse##sgsed"].plurals[0] = "Reverter";
    strings["Invert##sgsed"].plurals[0] = "Inverter";
    strings["Signed/unsigned exchange##sgsed"].plurals[0] = "Alternar sinalização (signed/unsigned)";
    strings["Apply filter##sgsed"].plurals[0] = "Aplicar filtro";
    strings["Cutoff:##sgsed"].plurals[0] = "Ponto de corte:";
    strings["From##sgsed"].plurals[0] = "De";
    strings["To##sgsed"].plurals[0] = "Até";
    strings["Resonance##sgsed"].plurals[0] = "Ressonância";
    strings["Power##sgsed"].plurals[0] = "Potência";
    strings["Low-pass##sgsed"].plurals[0] = "Passa-baixa";
    strings["Band-pass##sgsed"].plurals[0] = "Passa-banda";
    strings["High-pass##sgsed"].plurals[0] = "Passa-alta";
    strings["Apply##sgsed1"].plurals[0] = "Aplicar";
    strings["Crossfade loop points##sgsed"].plurals[0] = "Aplicar crossfade nos pontos de loop";
    strings["Number of samples##sgsed"].plurals[0] = "Número de amostras";
    strings["Linear <-> Equal power##sgsed"].plurals[0] = "Linear <-> Equipotente";
    strings["Apply##sgsed2"].plurals[0] = "Aplicar";
    strings["Crossfade: length would go out of bounds. Aborted...##sgsed"].plurals[0] = "Crossfade: o comprimento estaria fora dos limites da sample. Abortado...";
    strings["Crossfade: length would overflow loopStart. Try a smaller random value.##sgsed"].plurals[0] = "Crossfade: o comprimento ultrapassaria o começo do loop. Tente um valor menor aleatório.";
    strings["Preview sample##sgsed"].plurals[0] = "Prever sample";
    strings["Stop sample preview##sgsed"].plurals[0] = "Interromper previsão da sample";
    strings["Create instrument from sample##sgsed"].plurals[0] = "Criar instrumento a partir da sample";
    strings["Zoom##sgsed0"].plurals[0] = "Zoom";
    strings["Zoom##sgsed1"].plurals[0] = "Zoom";
    strings["Auto##sgsed"].plurals[0] = "Automático";
    strings["cut##sgsed"].plurals[0] = "recortar";
    strings["copy##sgsed"].plurals[0] = "copiar";
    strings["paste##sgsed"].plurals[0] = "colar";
    strings["paste (replace)##sgsed"].plurals[0] = "colar (substituir)";
    strings["paste (mix)##sgsed"].plurals[0] = "colar (mesclar)";
    strings["select all##sgsed"].plurals[0] = "selecionar tudo";
    strings["set loop to selection##sgsed"].plurals[0] = "definir loop para seleção";
    strings["create wavetable from selection##sgsed"].plurals[0] = "criar wavetable a partir da seleção";
    strings["Draw##sgsed"].plurals[0] = "Desenhar";
    strings["Select##sgsed"].plurals[0] = "Selecionar";
    strings["%d samples##sgsed"].plurals[0] = "%d sample";
    strings["%d samples##sgsed"].plurals[1] = "%d samples";
    strings["%d bytes##sgsed"].plurals[0] = "%d byte";
    strings["%d bytes##sgsed"].plurals[1] = "%d bytes";
    strings[" (%d-%d: %d samples)##sgsed"].plurals[0] = " (%d-%d: %d sample)";
    strings[" (%d-%d: %d samples)##sgsed"].plurals[1] = " (%d-%d: %d samples)";
    strings["Non-8/16-bit samples cannot be edited without prior conversion.##sgsed"].plurals[0] = "Samples que não são 8/16-bit não podem ser editadas sem ser convertidas primeiro.";

    //   sgsi  src/gui/songInfo.cpp

    strings["Song Info###Song Information"].plurals[0] = "Informações da Música###Song Information";
    strings["Name##sgsi"].plurals[0] = "Nome";
    strings["Author##sgsi"].plurals[0] = "Autor";
    strings["Album##sgsi"].plurals[0] = "Álbum";
    strings["System##sgsi"].plurals[0] = "Sistema";
    strings["Auto##sgsi"].plurals[0] = "Auto";
    strings["Tuning (A-4)##sgsi"].plurals[0] = "Afinação (A-4)";

    // # sgsn  src/gui/songNotes.cpp

    strings["Song Comments###Song Comments"].plurals[0] = "Comentários da Música###Song Comments";

    //   sgsp  src/gui/speed.cpp

    strings["Speed###Speed"].plurals[0] = "Velocidade###Speed";
    strings["Base Tempo##TempoOrHz"].plurals[0] = "Tempo Base##TempoOrHz";
    strings["Tick Rate##TempoOrHz"].plurals[0] = "Taxa de Ticks##TempoOrHz";
    strings["click to display tick rate##sgsp"].plurals[0] = "clique para mostrar a taxa de ticks";
    strings["click to display base tempo##sgsp"].plurals[0] = "clique para mostrar o tempo base";
    strings["Groove##sgsp"].plurals[0] = "Groove";
    strings["click for one speed##sgsp"].plurals[0] = "clique para uma velocidade";
    strings["Speeds##sgsp"].plurals[0] = "Velocidades";
    strings["click for groove pattern##sgsp"].plurals[0] = "clique para padrão \"groove\"";
    strings["Speed##sgsp"].plurals[0] = "Velocidade";
    strings["click for two (alternating) speeds##sgsp"].plurals[0] = "clique para duas velocidades alternantes";
    strings["Virtual Tempo##sgsp"].plurals[0] = "Tempo Virtual";
    strings["Numerator##sgsp"].plurals[0] = "Numerador";
    strings["Denominator (set to base tempo)##sgsp"].plurals[0] = "Denominador (definido para o tempo base)"; // TODO: poderia melhorar essa string?
    strings["Divider##sgsp"].plurals[0] = "Divisor";
    strings["Highlight##sgsp"].plurals[0] = "Destaque";
    strings["Pattern Length##sgsp"].plurals[0] = "Tamanho da Sequência";
    strings["Song Length##sgsp"].plurals[0] = "Tamanho da Música";

    //   sgst  src/gui/stats.cpp

    strings["Statistics###Statistics"].plurals[0] = "Estatísticas###Statistics";
    strings["Audio load##sgst"].plurals[0] = "Carga de áudio";

    //   sgss  src/gui/subSongs.cpp

    strings["Subsongs###Subsongs"].plurals[0] = "Subfaixas###Subsongs";
    strings["%d. <no name>##sgss0"].plurals[0] = "%d. <sem nome>";
    strings["%d. <no name>##sgss1"].plurals[0] = "%d. <sem nome>";
    strings["Move up##sgss"].plurals[0] = "Mover para cima";
    strings["Move down##sgss"].plurals[0] = "Mover para baixo";
    strings["too many subsongs!##sgss0"].plurals[0] = "há muitas subfaixas!";
    strings["Add##sgss"].plurals[0] = "Adicionar";
    strings["too many subsongs!##sgss1"].plurals[0] = "há muitas subfaixas!";
    strings["Duplicate##sgss"].plurals[0] = "Duplicar";
    strings["this is the only subsong!##sgss"].plurals[0] = "essa é a única subfaixa!";
    strings["are you sure you want to remove this subsong?##sgss"].plurals[0] = "tem certeza de que quer remover esta subfaixa?";
    strings["Remove##sgss"].plurals[0] = "Remover";
    strings["Name##sgss"].plurals[0] = "Nome";

    //   sgsc  src/gui/sysConf.cpp

    strings["Clock rate:##sgsc0"].plurals[0] = "Taxa de clock:";
    strings["Chip type:##sgsc0"].plurals[0] = "Tipo de chip:";
    strings["YM3438 (9-bit DAC)##sgsc"].plurals[0] = "YM3438 (DAC de 9-bit)";
    strings["YM2612 (9-bit DAC with distortion)##sgsc"].plurals[0] = "YM2612 (DAC de 9-bit com distorção)";
    strings["YMF276 (external DAC)##sgsc"].plurals[0] = "YMF276 (DAC externo)";
    strings["Disable ExtCh FM macros (compatibility)##sgsc0"].plurals[0] = "Desabilitar macros FM ExtCh (compatibilidade)";
    strings["Ins change in ExtCh operator 2-4 affects FB (compatibility)##sgsc0"].plurals[0] = "Mudança de inst. no operador ExtCh 2-4 afeta FB (compatibilidade)";
    strings["Clock rate:##sgsc1"].plurals[0] = "Taxa de clock:";
    strings["1.79MHz (Half NTSC)##sgsc"].plurals[0] = "1.79MHz (Metade do NTSC)";
    strings["Chip type:##sgsc1"].plurals[0] = "Tipo de chip:";
    strings["TI SN76489 with Atari-like short noise##sgsc"].plurals[0] = "TI SN76489 com ruído curto estilo Atari";
    strings["Tandy PSSJ 3-voice sound##sgsc"].plurals[0] = "Som Tandy PSSJ de 3 canais";
    strings["Disable noise period change phase reset##sgsc"].plurals[0] = "Desabilitar redefinição de fase na mudança do período do ruído";
    strings["Disable easy period to note mapping on upper octaves##sgsc0"].plurals[0] = "Desabilitar mapeamento fácil de período para nota nas oitavas altas";
    strings["Pseudo-PAL##sgsc0"].plurals[0] = "Pseudo-PAL";
    strings["Disable anti-click##sgsc0"].plurals[0] = "Desabilitar anti-click";
    strings["Chip revision:##sgsc0"].plurals[0] = "Revisão do chip:";
    strings["HuC6280 (original)##sgsc"].plurals[0] = "HuC6280 (original)";
    strings["CPU rate:##sgsc"].plurals[0] = "Taxa da CPU:";
    strings["Sample memory:##sgsc"].plurals[0] = "Memória de sample:";
    strings["8K (rev A/B/E)##sgsc"].plurals[0] = "8K (rev A/B/E)";
    strings["64K (rev D/F)##sgsc"].plurals[0] = "64K (rev D/F)";
    strings["DAC resolution:##sgsc"].plurals[0] = "Resolução do DAC:";
    strings["16-bit (rev A/B/D/F)##sgsc"].plurals[0] = "16-bit (rev A/B/D/F)";
    strings["8-bit + TDM (rev C/E)##sgsc"].plurals[0] = "8-bit + TDM (rev C/E)";
    strings["Enable echo##sgsc0"].plurals[0] = "Habilitar eco";
    strings["Swap echo channels##sgsc"].plurals[0] = "Trocar canais de eco";
    strings["Echo delay:##sgsc0"].plurals[0] = "Atraso do eco:";
    strings["Echo resolution:##sgsc"].plurals[0] = "Resolução do eco:";
    strings["Echo feedback:##sgsc0"].plurals[0] = "Retorno (feedback) do eco:";
    strings["Echo volume:##sgsc0"].plurals[0] = "Volume do eco:";
    strings["Disable anti-click##sgsc1"].plurals[0] = "Desabilitar anti-click";
    strings["Chip revision:##sgsc1"].plurals[0] = "Revisão do chip:";
    strings["Original (DMG)##sgsc"].plurals[0] = "Original (DMG)";
    strings["Game Boy Color (rev C)##sgsc"].plurals[0] = "Game Boy Color (rev C)";
    strings["Game Boy Color (rev E)##sgsc"].plurals[0] = "Game Boy Color (rev E)";
    strings["Wave channel orientation:##sgsc"].plurals[0] = "Orientação do canal de onda:";
    strings["Normal##sgsc"].plurals[0] = "Normal";
    strings["Inverted##sgsc"].plurals[0] = "Invertido";
    strings["Exact data (inverted)##sgsc"].plurals[0] = "Dados exatos (inverted)";
    strings["Exact output (normal)##sgsc"].plurals[0] = "Saída exata (normal)";
    strings["Pretty please one more compat flag when I use arpeggio and my sound length##sgsc"].plurals[0] = "Porfavorzinho mais uma opção de compatibilidade quando eu usar arpejo e meu comprimento de som";
    strings["Clock rate:##sgsc2"].plurals[0] = "Taxa de clock:";
    strings["Pretty please one more compat flag when I use arpeggio and my sound length##sgsc"].plurals[0] = "Porfavorzinho mais uma opção de compatibilidade quando eu usar arpejo e meu comprimento de som";
    strings["DAC bit depth (reduces output rate):##sgsc"].plurals[0] = "Profundidade de bits do DAC (reduz a taxa de amostragem):";
    strings["Volume scale:##sgsc"].plurals[0] = "Escala de volume:";
    strings["Mix buffers (allows longer echo delay):##sgsc"].plurals[0] = "Quantidade de buffers de mixing (permite atraso de eco mais longo):";
    strings["Channel limit:##sgsc"].plurals[0] = "Limite de canais:";
    strings["Sample rate:##sgsc"].plurals[0] = "Taxa de amostragem:";
    strings["Actual sample rate: %d Hz##sgsc"].plurals[0] = "Taxa de amostragem real: %d Hz";
    strings["Max mixer CPU usage: %.0f%%##sgsc"].plurals[0] = "Uso de CPU máximo do mixer: %.0f%%";
    strings["Arcade (4MHz)##sgsc"].plurals[0] = "Arcade (4MHz)";
    strings["Half NTSC (1.79MHz)##sgsc"].plurals[0] = "Metade do NTSC (1.79MHz)";
    strings["Patch set:##sgsc"].plurals[0] = "Conjunto de timbres:";
    strings["Ignore top/hi-hat frequency changes##sgsc"].plurals[0] = "Ignorar mudanças de frequência no chimbal/prato";
    strings["Apply fixed frequency to all drums at once##sgsc"].plurals[0] = "Aplicar frequência fixa em todos os canais de bateria ao mesmo tempo";
    strings["Broken pitch macro/slides (compatibility)##sgsc0"].plurals[0] = "Macros/slide de tom quebrados (compatibilidade)";
    strings["Pseudo-PAL##sgsc1"].plurals[0] = "Pseudo-PAL";
    strings["Broken pitch macro/slides (compatibility)##sgsc1"].plurals[0] = "Macros/slide de tom quebrados (compatibilidade)";
    strings["Clock rate:##sgsc20"].plurals[0] = "Taxa de clock:";
    strings["DPCM channel mode:##sgsc"].plurals[0] = "Modo de canal DPCM:";
    strings["DPCM (muffled samples; low CPU usage)##sgsc"].plurals[0] = "DPCM (samples abafadas; baixo uso de CPU)";
    strings["PCM (crisp samples; high CPU usage)##sgsc"].plurals[0] = "PCM (samples nítidas; alto uso de CPU)";
    strings["Clock rate:##sgsc18"].plurals[0] = "Taxa de clock:";
    strings["Clock rate:##sgsc19"].plurals[0] = "Taxa de clock:";
    strings["Global parameter priority:##sgsc0"].plurals[0] = "Prioridade de parâmetros globais:";
    strings["Left to right##sgsc0"].plurals[0] = "Da esquerda para a direita";
    strings["Last used channel##sgsc0"].plurals[0] = "Último canal usado";
    strings["Hard reset envelope:##sgsc"].plurals[0] = "Reiniciar envelope bruscamente:";
    strings["Attack##sgsc"].plurals[0] = "Ataque";
    strings["Decay##sgsc"].plurals[0] = "Decaimento";
    strings["Sustain##sgsc"].plurals[0] = "Sustentação";
    strings["Release##sgsc"].plurals[0] = "Repouso";
    strings["Envelope reset time:##sgsc"].plurals[0] = "Tempo de reinício do envelope:";
    strings["- 0 disables envelope reset. not recommended!\n- 1 may trigger SID envelope bugs.\n- values that are too high may result in notes being skipped.##sgsc"].plurals[0] = "- 0 desabilita o reinício do envelope. não recomendado!\n- 1 pode causar bugs de envelope do SID.\n- valores muito altos podem resultar em notas sendo puladas.";
    strings["Disable 1Exy env update (compatibility)##sgsc"].plurals[0] = "Desabilitar atualização de envelope via 1Exy (compatibilidade)";
    strings["Relative duty and cutoff macros are coarse (compatibility)##sgsc"].plurals[0] = "Macros de ciclo de trabalho e ponto de corte relativos são grossas (compatibilidade)";
    strings["Cutoff macro race conditions (compatibility)##sgsc"].plurals[0] = "Condições de corrida nas macros de ponto de corte (compatibilidade)";
    strings["Disable ExtCh FM macros (compatibility)##sgsc1"].plurals[0] = "Desabilitar macros FM ExtCh (compatibilidade)";
    strings["Ins change in ExtCh operator 2-4 affects FB (compatibility)##sgsc1"].plurals[0] = "Mudança de inst. no operador ExtCh 2-4 afeta FB (compatibilidade)";
    strings["SSG Volume##sgsc0"].plurals[0] = "Volume do SSG";
    strings["FM/ADPCM Volume##sgsc0"].plurals[0] = "Volume do FM/ADPCM";
    strings["Clock rate:##sgsc3"].plurals[0] = "Taxa de clock:";
    strings["0.83MHz (Pre-divided Sunsoft 5B on PAL)##sgsc"].plurals[0] = "0.83MHz (Sunsoft 5B pré-dividido no PAL)";
    strings["0.89MHz (Pre-divided Sunsoft 5B)##sgsc"].plurals[0] = "0.89MHz (Sunsoft 5B pré-dividido)";
    strings["Chip type:##sgsc2"].plurals[0] = "Tipo de chip:";
    strings["note: AY-3-8914 is not supported by the VGM format!##sgsc"].plurals[0] = "nota: o AY-3-8914 não é suportado pelo formato VGM!";
    strings["Stereo##_AY_STEREO"].plurals[0] = "Estéreo##_AY_STEREO";
    strings["Separation##sgsc"].plurals[0] = "Separação";
    strings["Half Clock divider##_AY_CLKSEL"].plurals[0] = "Divisor de Clock pela Metade##_AY_CLKSEL";
    strings["Stereo separation:##sgsc"].plurals[0] = "Separação estéreo:";
    strings["Model:##sgsc"].plurals[0] = "Modelo:";
    strings["Chip memory:##sgsc"].plurals[0] = "Memória do chip:";
    strings["2MB (ECS/AGA max)##sgsc"].plurals[0] = "2MB (máx. ECS/AGA)";
    strings["512KB (OCS max)##sgsc"].plurals[0] = "512KB (máx. OCS)";
    strings["Bypass frequency limits##sgsc"].plurals[0] = "Ultrapassar limites de frequência";
    strings["Mixing mode:##sgsc"].plurals[0] = "Modo de mixing:";
    strings["Mono##sgsc"].plurals[0] = "Mono";
    strings["Mono (no distortion)##sgsc"].plurals[0] = "Mono (sem distorção)";
    strings["Stereo##sgsc0"].plurals[0] = "Estéreo";
    strings["Clock rate:##sgsc4"].plurals[0] = "Taxa de clock:";
    strings["Speaker type:##sgsc"].plurals[0] = "Tipo de speaker:";
    strings["Unfiltered##sgsc"].plurals[0] = "Não-filtrado";
    strings["Cone##sgsc"].plurals[0] = "Cone";
    strings["Piezo##sgsc"].plurals[0] = "Piezo";
    strings["Use system beeper (Linux only!)##sgsc"].plurals[0] = "Usar beeper do sistema (somente Linux!)";
    strings["Reset phase on frequency change##sgsc"].plurals[0] = "Reiniciar fase em mudança de frequência";
    strings["Echo delay:##sgsc1"].plurals[0] = "Atraso de eco:";
    strings["Echo feedback:##sgsc1"].plurals[0] = "Retorno (feedback) de eco:";
    strings["Clock rate:##sgsc5"].plurals[0] = "Taxa de clock:";
    strings["Stereo##sgsc1"].plurals[0] = "Estéreo";
    strings["Bankswitched (Seta 2)##sgsc"].plurals[0] = "Usar Bankswitching (Seta 2)";
    strings["Clock rate:##sgsc6"].plurals[0] = "Taxa de clock:";
    strings["Initial channel limit:##sgsc0"].plurals[0] = "Limite de canais inicial:";
    strings["Disable hissing##sgsc"].plurals[0] = "Desabilitar chiado";
    strings["Scale frequency to wave length##sgsc"].plurals[0] = "Escalonar frequência para tamanho da onda";
    strings["Initial channel limit:##sgsc1"].plurals[0] = "Limite de canais inicial:";
    strings["Volume scale:##sgsc0"].plurals[0] = "Escala de volume:";
    strings["Clock rate:##sgsc7"].plurals[0] = "Taxa de clock:";
    strings["Output rate:##sgsc0"].plurals[0] = "Taxa de saída:";
    strings["FM: clock / 72, SSG: clock / 16##sgsc0"].plurals[0] = "FM: clock / 72, SSG: clock / 16";
    strings["FM: clock / 36, SSG: clock / 8##sgsc"].plurals[0] = "FM: clock / 36, SSG: clock / 8";
    strings["FM: clock / 24, SSG: clock / 4##sgsc"].plurals[0] = "FM: clock / 24, SSG: clock / 4";
    strings["SSG Volume##sgsc1"].plurals[0] = "Volume do SSG";
    strings["FM Volume##sgsc"].plurals[0] = "Volume do FM";
    strings["Disable ExtCh FM macros (compatibility)##sgsc2"].plurals[0] = "Desabilitar macros FM ExtCh (compatibilidade)";
    strings["Ins change in ExtCh operator 2-4 affects FB (compatibility)##sgsc2"].plurals[0] = "Mudança de inst. no operador ExtCh 2-4 afeta FB (compatibilidade)";
    strings["Clock rate:##sgsc8"].plurals[0] = "Taxa de clock:";
    strings["8MHz (Arcade)##sgsc"].plurals[0] = "8MHz (Arcade)";
    strings["Output rate:##sgsc1"].plurals[0] = "Taxa de saída:";
    strings["FM: clock / 144, SSG: clock / 32##sgsc"].plurals[0] = "FM: clock / 144, SSG: clock / 32";
    strings["FM: clock / 72, SSG: clock / 16##sgsc1"].plurals[0] = "FM: clock / 72, SSG: clock / 16";
    strings["FM: clock / 48, SSG: clock / 8##sgsc"].plurals[0] = "FM: clock / 48, SSG: clock / 8";
    strings["SSG Volume##sgsc2"].plurals[0] = "Volume do SSG";
    strings["FM/ADPCM Volume##sgsc1"].plurals[0] = "Volume do FM/ADPCM";
    strings["Disable ExtCh FM macros (compatibility)##sgsc3"].plurals[0] = "Desabilitar macros FM ExtCh (compatibilidade)";
    strings["Ins change in ExtCh operator 2-4 affects FB (compatibility)##sgsc3"].plurals[0] = "Mudança de inst. no operador ExtCh 2-4 afeta FB (compatibilidade)";
    strings["Clock rate:##sgsc9"].plurals[0] = "Taxa de clock:";
    strings["Chip type:##sgsc3"].plurals[0] = "Tipo de chip:";
    strings["RF5C68 (10-bit output)##sgsc"].plurals[0] = "RF5C68 (saída 10-bit)";
    strings["RF5C164 (16-bit output)##sgsc"].plurals[0] = "RF5C164 (saída 16-bit)";
    strings["Clock rate:##sgsc10"].plurals[0] = "Taxa de clock:";
    strings["Sample rate table:##sgsc"].plurals[0] = "Tabela de taxa de amostragem:";
    strings["divider \\ clock##sgsc"].plurals[0] = "divisor \\ clock";
    strings["full##sgsc"].plurals[0] = "completo";
    strings["half##sgsc"].plurals[0] = "metade";
    strings["Clock rate:##sgsc11"].plurals[0] = "Taxa de clock:";
    strings["Output rate:##sgsc2"].plurals[0] = "Taxa de saída:";
    strings["clock / 132##sgsc"].plurals[0] = "clock / 132";
    strings["clock / 165##sgsc"].plurals[0] = "clock / 165";
    strings["Bankswitched (NMK112)##sgsc"].plurals[0] = "Usar Bankswitching (NMK112)";
    strings["Clock rate:##sgsc12"].plurals[0] = "Taxa de clock:";
    strings["1.5MHz (Arcade)##sgsc"].plurals[0] = "1.5MHz (Arcade)";
    strings["Consistent frequency across all duties##sgsc"].plurals[0] = "Frequência consistente ao longo de todos os ciclos de trabalho";
    strings["note: only works for an initial LFSR value of 0!##sgsc"].plurals[0] = "nota: só funciona para um valor inicial de LFSR igual a 0!";
    strings["Clock rate:##sgsc13"].plurals[0] = "Taxa de clock:";
    strings["Clock rate:##sgsc14"].plurals[0] = "Taxa de clock:";
    strings["Chip type:##sgsc4"].plurals[0] = "Tipo de chip:";
    strings["Compatible panning (0800)##sgsc"].plurals[0] = "Balanço compatível (0800)";
    strings["Clock rate:##sgsc15"].plurals[0] = "Taxa de clock:";
    strings["Output rate:##sgsc3"].plurals[0] = "Taxa de saída:";
    strings["Output bit depth:##sgsc"].plurals[0] = "Profundidade de bits de saída:";
    strings["Stereo##sgsc2"].plurals[0] = "Estéreo";
    strings["Interpolation:##sgsc"].plurals[0] = "Interpolação:";
    strings["None##sgsc"].plurals[0] = "Nenhuma";
    strings["Linear##sgsc"].plurals[0] = "Linear";
    strings["Cubic##sgsc"].plurals[0] = "Cúbica";
    strings["Sinc##sgsc"].plurals[0] = "Sinc";
    strings["Volume scale:##sgsc1"].plurals[0] = "Escala de volume:";
    strings["Left##VolScaleL"].plurals[0] = "Esquerdo##VolScaleL";
    strings["Right##VolScaleL"].plurals[0] = "Direito##VolScaleL";
    strings["Enable echo##sgsc1"].plurals[0] = "Habilitar eco";
    strings["Initial echo state:##sgsc"].plurals[0] = "Estado inicial do eco:";
    strings["Delay##EchoDelay"].plurals[0] = "Atraso##EchoDelay";
    strings["Feedback##EchoFeedback"].plurals[0] = "Retorno (feedback)##EchoFeedback";
    strings["Echo volume:##sgsc1"].plurals[0] = "Volume do eco:";
    strings["Left##EchoVolL"].plurals[0] = "Esquerdo##EchoVolL";
    strings["Right##EchoVolL"].plurals[0] = "Direito##EchoVolL";
    strings["Echo filter:##sgsc"].plurals[0] = "Filtro do eco:";
    strings["Hex##SNESFHex"].plurals[0] = "Hex##SNESFHex";
    strings["Dec##SNESFHex"].plurals[0] = "Dec##SNESFHex";
    strings["sum: %d##sgsc"].plurals[0] = "soma: %d";
    strings["Detune##sgsc"].plurals[0] = "Desafinação";
    strings["Capacitor values (nF):##sgsc"].plurals[0] = "Valores dos capacitores (nF):";
    strings["Initial part volume (channel 1-4):##sgsc"].plurals[0] = "Volume inicial das partes (canais 1-4):";
    strings["Initial part volume (channel 5-8):##sgsc"].plurals[0] = "Volume inicial das partes (canais 5-8):";
    strings["Envelope mode (channel 1-4):##sgsc"].plurals[0] = "Modo de envelope (canais 1-4):";
    strings["Capacitor (attack/decay)##EM00"].plurals[0] = "Capacitor (ataque/decaimento)##EM00";
    strings["External (volume macro)##EM01"].plurals[0] = "Externo (macro de volume)##EM01";
    strings["Envelope mode (channel 5-8):##sgsc"].plurals[0] = "Modo de envelope (canais 5-8):";
    strings["Capacitor (attack/decay)##EM10"].plurals[0] = "Capacitor (ataque/decaimento)##EM10";
    strings["External (volume macro)##EM11"].plurals[0] = "Externo (macro de volume)##EM11";
    strings["Global vibrato:##sgsc"].plurals[0] = "Vibrato global:";
    strings["Speed##sgsc"].plurals[0] = "Velocidade";
    strings["Depth##sgsc"].plurals[0] = "Profundidade";
    strings["Disable easy period to note mapping on upper octaves##sgsc1"].plurals[0] = "Desabilitar mapeamento fácil de período para nota nas oitavas superiores";
    strings["Stereo##sgsc3"].plurals[0] = "Estéreo";
    strings["Waveform storage mode:##sgsc0"].plurals[0] = "Modo de armazenamento das waveforms:";
    strings["RAM##sgsc"].plurals[0] = "RAM";
    strings["ROM (up to 8 waves)##sgsc"].plurals[0] = "ROM (até 8 ondas)";
    strings["Compatible noise frequencies##sgsc"].plurals[0] = "Frequências de ruído compatíveis";
    strings["Legacy slides and pitch (compatibility)##sgsc"].plurals[0] = "Slides e tom legados (compatibilidade)";
    strings["Clock rate:##sgsc16"].plurals[0] = "Taxa de clock:";
    strings["Clock rate:##sgsc17"].plurals[0] = "Taxa de clock:";
    strings["Global parameter priority:##sgsc1"].plurals[0] = "Prioridade de parâmetros globais:";
    strings["Left to right##sgsc1"].plurals[0] = "Da esquerda para a direita";
    strings["Last used channel##sgsc1"].plurals[0] = "Último canal utilizado";
    strings["Banking style:##sgsc"].plurals[0] = "Estilo de gerenciamento de bancos";
    strings["Raw (16MB; no VGM export!)##sgsc"].plurals[0] = "Cru (16MB; não é possível exportar VGM!)";
    strings["Waveform storage mode:##sgsc1"].plurals[0] = "Modo de armazenamento das waveforms:";
    strings["Dynamic (unconfirmed)##sgsc"].plurals[0] = "Dinâmico (não confirmado)";
    strings["Static (up to 5 waves)##sgsc"].plurals[0] = "Estático (até 5 ondas)";
    strings["DS (4MB RAM)##sgsc"].plurals[0] = "DS (4MB RAM)";
    strings["DSi (16MB RAM)##sgsc"].plurals[0] = "DSi (16MB RAM)";
    strings["nothing to configure##sgsc"].plurals[0] = "nada para configurar";
    strings["Downmix chip output to mono##sgsc"].plurals[0] = "Misturar saída do chip para mono";
    strings["Reserved blocks for wavetables:##sgsc"].plurals[0] = "Blocos reservados para wavetables:";
    strings["Reserve this many blocks 256 bytes each in sample memory.\nEach block holds one wavetable (is used for one wavetable channel),\nso reserve as many as you need.##sgsc"].plurals[0] = "Reserva tal quantidade de blocos, 256 bytes cada, na memória de sample.\nCada bloco irá conter uma wavetable (será usado para um canal wavetable),\nentão reserve tantos quanto você precisar.";
    strings["Custom clock rate##sgsc"].plurals[0] = "Taxa de clock personalizada";
    strings["Hz##sgscHz"].plurals[0] = "Hz##sgscHz";
    strings["1MB##sgsc"].plurals[0] = "1MB";
    strings["256KB##sgsc"].plurals[0] = "256KB";
    strings["Namco System 2 (2MB)##sgsc"].plurals[0] = "Namco System 2 (2MB)";
    strings["Namco System 21 (4MB)##sgsc"].plurals[0] = "Namco System 21 (4MB)";


    //   sgsm  src/gui/sysManager.cpp

    strings["Chip Manager###Chip Manager"].plurals[0] = "Gerenciador de Chips###Chip Manager";
    strings["Preserve channel order##sgsm"].plurals[0] = "Preservar ordem dos canais";
    strings["Clone channel data##sgsm"].plurals[0] = "Clonar dados dos canais";
    strings["Clone at end##sgsm"].plurals[0] = "Clonar ao final";
    strings["Name##sgsm"].plurals[0] = "Nome";
    strings["Actions##sgsm"].plurals[0] = "Ações";
    strings["(drag to swap chips)##sgsm"].plurals[0] = "(arraste para trocar chips)";
    strings["Clone##SysDup"].plurals[0] = "Clonar##SysDup";
    strings["cannot duplicate chip! (##sgsm"].plurals[0] = "não pôde duplicar chip! (";
    strings["max number of systems is %d##sgsm"].plurals[0] = "o número máximo de sistemas é %d";
    strings["max number of total channels is %d##sgsm"].plurals[0] = "o número máximo total de canais é %d";
    strings["Change##SysChange"].plurals[0] = "Trocar##SysChange";
    strings["Are you sure you want to remove this chip?##sgsm"].plurals[0] = "Tem certeza de que quer remover este chip?";
    strings["Remove##sgsm"].plurals[0] = "Remover";
    strings["cannot add chip! (##sgsm"].plurals[0] = "não pôde adicionar chip! (";

    //   sgsa  src/gui/sysPartNumber.cpp

    strings["ZXS Beeper##sgsa"].plurals[0] = "Beeper do ZX Spectrum";

    //   sgsp  src/gui/sysPicker.cpp

    strings["Search...##sgsp"].plurals[0] = "Procurar...";

    // # sgvm  src/gui/volMeter.cpp

    strings["Volume Meter###Volume Meter"].plurals[0] = "Medidor de Volume###Volume Meter";

    //   sgwe  src/gui/waveEdit.cpp

    strings["Sine##sgwe0"].plurals[0] = "Senoide";
    strings["Triangle##sgwe0"].plurals[0] = "Triângulo";
    strings["Saw##sgwe0"].plurals[0] = "Dente de serra";
    strings["Pulse##sgwe"].plurals[0] = "Pulsada";

    strings["None##sgwe"].plurals[0] = "Nenhuma";
    strings["Linear##sgwe"].plurals[0] = "Linear";
    strings["Cosine##sgwe"].plurals[0] = "Cosseno";
    strings["Cubic##sgwe"].plurals[0] = "Cúbica";

    strings["Sine##sgwe1"].plurals[0] = "Seno";
    strings["Rect. Sine##sgwe"].plurals[0] = "Seno Retif.";
    strings["Abs. Sine##sgwe"].plurals[0] = "Seno Abs.";
    strings["Quart. Sine##sgwe"].plurals[0] = "Seno 1/4";
    strings["Squish. Sine##sgwe"].plurals[0] = "Seno Espr.";
    strings["Abs. Squish. Sine##sgwe"].plurals[0] = "Seno Abs. Espr.";
    strings["Square##sgwe"].plurals[0] = "Quadrada";
    strings["rectSquare##sgwe"].plurals[0] = "Serra Logarítm.";
    strings["Saw##sgwe1"].plurals[0] = "Dente de serra";
    strings["Rect. Saw##sgwe"].plurals[0] = "Serra Retif.";
    strings["Abs. Saw##sgwe"].plurals[0] = "Serra Abs.";
    strings["Cubed Saw##sgwe"].plurals[0] = "Serra ao Cubo";
    strings["Rect. Cubed Saw##sgwe"].plurals[0] = "Serra ao Cubo Retif.";
    strings["Abs. Cubed Saw##sgwe"].plurals[0] = "Serra ao Cubo Abs.";
    strings["Cubed Sine##sgwe"].plurals[0] = "Seno ao Cubo";
    strings["Rect. Cubed Sine##sgwe"].plurals[0] = "Seno ao Cubo Retif.";
    strings["Abs. Cubed Sine##sgwe"].plurals[0] = "Seno ao Cubo Abs.";
    strings["Quart. Cubed Sine##sgwe"].plurals[0] = "Seno ao Cubo 1/4";
    strings["Squish. Cubed Sine##sgwe"].plurals[0] = "Seno ao Cubo Espr.";
    strings["Squish. Abs. Cub. Sine##sgwe"].plurals[0] = "Seno Cubo Espr. Abs.";
    strings["Triangle##sgwe1"].plurals[0] = "Triângulo";
    strings["Rect. Triangle##sgwe"].plurals[0] = "Triângulo Retif.";
    strings["Abs. Triangle##sgwe"].plurals[0] = "Triângulo Abs.";
    strings["Quart. Triangle##sgwe"].plurals[0] = "Triângulo 1/4";
    strings["Squish. Triangle##sgwe"].plurals[0] = "Triângulo Espr.";
    strings["Abs. Squish. Triangle##sgwe"].plurals[0] = "Triângulo Abs. Espr.";
    strings["Cubed Triangle##sgwe"].plurals[0] = "Triângulo ao Cubo";
    strings["Rect. Cubed Triangle##sgwe"].plurals[0] = "Triângulo ao Cubo Retif.";
    strings["Abs. Cubed Triangle##sgwe"].plurals[0] = "Triângulo ao Cubo Abs.";
    strings["Quart. Cubed Triangle##sgwe"].plurals[0] = "Triângulo ao Cubo 1/4";
    strings["Squish. Cubed Triangle##sgwe"].plurals[0] = "Triângulo ao Cubo Espr.";
    strings["Squish. Abs. Cub. Triangle##sgwe"].plurals[0] = "Triângulo Cubo Espr. Abs.";

    strings["Wavetable Editor###Wavetable Editor"].plurals[0] = "Editor de Wavetable###Wavetable Editor";
    strings["no wavetable selected##sgwe0"].plurals[0] = "nenhuma wavetable selecionada";
    strings["no wavetable selected##sgwe1"].plurals[0] = "nenhuma wavetable selecionada";
    strings["select one...##sgwe"].plurals[0] = "selecione um...";
    strings["or##sgwe0"].plurals[0] = "ou";
    strings["Open##sgwe0"].plurals[0] = "Abrir";
    strings["or##sgwe1"].plurals[0] = "ou";
    strings["Create New##sgwe"].plurals[0] = "Criar Nova";
    strings["Open##sgwe1"].plurals[0] = "Abrir";
    strings["Save##sgwe"].plurals[0] = "Salvar";
    strings["export .dmw...##sgwe"].plurals[0] = "exportar .dmw...";
    strings["export raw...##sgwe"].plurals[0] = "exportar em formato raw...";
    strings["Steps##sgwe"].plurals[0] = "Degraus";
    strings["Lines##sgwe"].plurals[0] = "Linhas";
    strings["Width##sgwe"].plurals[0] = "Largura";
    strings["use a width of:\n- any on Amiga/N163\n- 32 on Game Boy, PC Engine, SCC, Konami Bubble System, Namco WSG, Virtual Boy and WonderSwan\n- 64 on FDS\n- 128 on X1-010\n- 256 for ES5503\nany other widths will be scaled during playback.##sgwe"].plurals[0] = "use uma largura de:\n- qualquer uma no Amiga/N163\n- 32 no Game Boy, PC Engine, SCC, Konami Bubble System, Namco WSG, Virtual Boy e WonderSwan\n- 64 no FDS\n- 128 no X1-010\n- 256 para o ES5503\nquaisquer outras larguras serão escalonadas no momento da reprodução.";
    strings["Height##sgwe"].plurals[0] = "Altura";
    strings["use a height of:\n- 16 for Game Boy, WonderSwan, Namco WSG, Konami Bubble System, X1-010 Envelope shape and N163\n- 32 for PC Engine\n- 64 for FDS and Virtual Boy\n- 256 for X1-010, SCC and ES5503\nany other heights will be scaled during playback.##sgwe"].plurals[0] = "use uma altura de:\n- 16 para Game Boy, WonderSwan, Namco WSG, Konami Bubble System, forma do Envelope do X1-010 e N163\n- 32 para PC Engine\n- 64 para FDS e Virtual Boy\n- 256 para X1-010, SCC e ES5503\nquaisquer outras alturas serão escalonadas no momento da reprodução.";
    strings["Shapes##sgwe"].plurals[0] = "Formas";
    strings["Duty##sgwe"].plurals[0] = "Ciclo de trabalho";
    strings["Exponent##sgwe"].plurals[0] = "Exponente";
    strings["XOR Point##sgwe"].plurals[0] = "Ponto de XOR";
    strings["Amplitude/Phase##sgwe"].plurals[0] = "Amplitude/Fase";
    strings["Op##sgwe0"].plurals[0] = "Op";
    strings["Level##sgwe"].plurals[0] = "Nível";
    strings["Mult##sgwe"].plurals[0] = "Mult";
    strings["FB##sgwe"].plurals[0] = "FB";
    strings["Op##sgwe1"].plurals[0] = "Op";
    strings["Waveform##sgwe"].plurals[0] = "Forma de onda";
    strings["Connection Diagram##sgwe0"].plurals[0] = "Diagrama de Conexão";
    strings["Connection Diagram##sgwe1"].plurals[0] = "Diagrama de Conexão";
    strings["Out##sgwe"].plurals[0] = "Saída";
    strings["WaveTools##sgwe"].plurals[0] = "Ferramentas de Onda";
    strings["Scale X##sgwe"].plurals[0] = "Escala X";
    strings["wavetable longer than 256 samples!##sgwe"].plurals[0] = "wavetable mais longa que 256 amostras!";
    strings["Scale Y##sgwe"].plurals[0] = "Escala Y";
    strings["Offset X##sgwe"].plurals[0] = "Deslocamento X";
    strings["Offset Y##sgwe"].plurals[0] = "Deslocamento Y";
    strings["Smooth##sgwe"].plurals[0] = "Suavizar";
    strings["Amplify##sgwe"].plurals[0] = "Amplificar";
    strings["Normalize##sgwe"].plurals[0] = "Normalizar";
    strings["Invert##sgwe"].plurals[0] = "Inverter";
    strings["Reverse##sgwe"].plurals[0] = "Reverter";
    strings["Half##sgwe"].plurals[0] = "Cortar pela metade";
    strings["Double##sgwe"].plurals[0] = "Dobrar";
    strings["Convert Signed/Unsigned##sgwe"].plurals[0] = "Converter sinalização (signed/unsigned)";
    strings["Randomize##sgwe"].plurals[0] = "Aleatorizar";
    strings["Dec##sgwe"].plurals[0] = "Dec";
    strings["Hex##sgwe"].plurals[0] = "Hex";
    strings["Signed/Unsigned##sgwe"].plurals[0] = "Sinalização (signed/unsigned)";

    //   sgxy  src/gui/xyOsc.cpp

    strings["Oscilloscope (X-Y)###Oscilloscope (X-Y)"].plurals[0] = "Osciloscópio (X-Y)###Oscilloscope (X-Y)";
    strings["X Channel##sgxy"].plurals[0] = "Canal X";
    strings["Invert##X"].plurals[0] = "Inverter";
    strings["Y Channel##sgxy"].plurals[0] = "Canal Y";
    strings["Invert##Y"].plurals[0] = "Inverter";
    strings["Zoom##sgxy"].plurals[0] = "Zoom";
    strings["Samples##sgxy"].plurals[0] = "Amostras";
    strings["Decay Time (ms)##sgxy"].plurals[0] = "Tempo de decaimento (ms)";
    strings["Intensity##sgxy"].plurals[0] = "Intensidade";
    strings["Line Thickness##sgxy"].plurals[0] = "Espessura da linha";
    strings["OK##sgxy"].plurals[0] = "OK";
    strings["(-Infinity)dB,(-Infinity)dB##sgxy"].plurals[0] = "(-Infinito)dB,(-Infinito)dB";
    strings["(-Infinity)dB,%.1fdB##sgxy"].plurals[0] = "(-Infinito)dB,%.1fdB";
    strings["%.1fdB,(-Infinity)dB##sgxy"].plurals[0] = "%.1fdB,(-Infinito)dB";

    //MACRO EDITOR

    //macro hover notes

    strings["exponential##sgmu"].plurals[0] = "exponencial";
    strings["linear##sgmu"].plurals[0] = "linear";
    strings["direct##sgmu"].plurals[0] = "direto";

    strings["Release"].plurals[0] = "Repouso";
    strings["Loop"].plurals[0] = "Repetir";

    strings["HP/K2, HP/K2##sgmu"].plurals[0] = "HP/K2, HP/K2";
    strings["HP/K2, LP/K1##sgmu"].plurals[0] = "HP/K2, LP/K1";
    strings["LP/K2, LP/K2##sgmu"].plurals[0] = "LP/K2, LP/K2";
    strings["LP/K2, LP/K1##sgmu"].plurals[0] = "LP/K2, LP/K1";

    strings["Saw##sgmu"].plurals[0] = "Serra";
    strings["Square##sgmu"].plurals[0] = "Quadrada";
    strings["Triangle##sgmu"].plurals[0] = "Triângulo";
    strings["Random##sgmu"].plurals[0] = "Aleatório";

    //   sgse  src/gui/settings.cpp

    strings["<Use system font>##sgse0"].plurals[0] = "<Usar fonte do sistema>";
    strings["<Custom...>##sgse0"].plurals[0] = "<Personalizado...>";
    strings["<Use system font>##sgse1"].plurals[0] = "<Usar fonte do sistema>";
    strings["<Custom...>##sgse1"].plurals[0] = "<Personalizado...>";
    strings["<Use system font>##sgse2"].plurals[0] = "<Usar fonte do sistema>";
    strings["<Custom...>##sgse2"].plurals[0] = "<Personalizado...>";
    strings["High##sgse"].plurals[0] = "Alta";
    strings["Low##sgse"].plurals[0] = "Baixa";
    strings["ASAP (C++ port)##sgse"].plurals[0] = "ASAP (conversão para C++)";
    strings["ESFMu (fast)##sgse"].plurals[0] = "ESFMu (rápido)";
    strings["KIOCSOUND on /dev/tty1##sgse"].plurals[0] = "KIOCSOUND em /dev/tty1";
    strings["KIOCSOUND on standard output##sgse"].plurals[0] = "KIOCSOUND na saída padrão";
    strings["Disabled/custom##sgse0"].plurals[0] = "Desabilitado/personalizado";
    strings["Two octaves (0 is C-4, F is D#5)##sgse"].plurals[0] = "Duas oitavas (0 é C-4, F é D#5)";
    strings["Raw (note number is value)##sgse"].plurals[0] = "Cru (número da nota é o valor)";
    strings["Two octaves alternate (lower keys are 0-9, upper keys are A-F)##sgse"].plurals[0] = "Duas oitavas alternativo (teclas brancas são 0-9, teclas pretas são A-F)";
    strings["Use dual control change (one for each nibble)##sgse0"].plurals[0] = "Utilizar dois MIDI CC separados (um para cada nibble)";
    strings["Use 14-bit control change##sgse0"].plurals[0] = "Utilizar um MIDI CC de 14 bits";
    strings["Use single control change (imprecise)##sgse0"].plurals[0] = "Utilizar um único MIDI CC (impreciso)";
    strings["Disabled/custom##sgse1"].plurals[0] = "Desabilitado/personalizado";
    strings["Use dual control change (one for each nibble)##sgse1"].plurals[0] = "Utilizar dois MIDI CC separados (um para cada nibble)";
    strings["Use 14-bit control change##sgse1"].plurals[0] = "Utilizar um MIDI CC de 14 bits";
    strings["Use single control change (imprecise)##sgse1"].plurals[0] = "Utilizar um único MIDI CC (impreciso)";
    strings["--select--##sgse"].plurals[0] = "--selecione--";
    strings["Note Off##sgse"].plurals[0] = "Note Off";
    strings["Note On##sgse"].plurals[0] = "Note On";
    strings["Aftertouch##sgse"].plurals[0] = "Aftertouch";
    strings["Control##sgse"].plurals[0] = "Controle";
    strings["Program##sgse0"].plurals[0] = "Mudança de Programa";
    strings["ChanPressure##sgse"].plurals[0] = "Pressão do Canal";
    strings["Pitch Bend##sgse"].plurals[0] = "Pitch Bend";
    strings["SysEx##sgse"].plurals[0] = "SysEx";
    strings["Instrument##sgse0"].plurals[0] = "Instrumento";
    strings["Volume##sgse0"].plurals[0] = "Volume";
    strings["Effect 1 type##sgse"].plurals[0] = "Tipo do Efeito 1";
    strings["Effect 1 value##sgse"].plurals[0] = "Valor do Efeito 1";
    strings["Effect 2 type##sgse"].plurals[0] = "Tipo do Efeito 2";
    strings["Effect 2 value##sgse"].plurals[0] = "Valor do Efeito 2";
    strings["Effect 3 type##sgse"].plurals[0] = "Tipo do Efeito 3";
    strings["Effect 3 value##sgse"].plurals[0] = "Valor do Efeito 3";
    strings["Effect 4 type##sgse"].plurals[0] = "Tipo do Efeito 4";
    strings["Effect 4 value##sgse"].plurals[0] = "Valor do Efeito 4";
    strings["Effect 5 type##sgse"].plurals[0] = "Tipo do Efeito 5";
    strings["Effect 5 value##sgse"].plurals[0] = "Valor do Efeito 5";
    strings["Effect 6 type##sgse"].plurals[0] = "Tipo do Efeito 6";
    strings["Effect 6 value##sgse"].plurals[0] = "Valor do Efeito 6";
    strings["Effect 7 type##sgse"].plurals[0] = "Tipo do Efeito 7";
    strings["Effect 7 value##sgse"].plurals[0] = "Valor do Efeito 7";
    strings["Effect 8 type##sgse"].plurals[0] = "Tipo do Efeito 8";
    strings["Effect 8 value##sgse"].plurals[0] = "Valor do Efeito 8";

    strings["Press key...##sgse"].plurals[0] = "Pressione uma tecla...";
    strings["Settings###Settings"].plurals[0] = "Configurações###Settings";
    strings["Do you want to save your settings?##sgse"].plurals[0] = "Você quer salvar suas configurações?";

    strings["General##sgse"].plurals[0] = "Geral";
    strings["Program##sgse1"].plurals[0] = "Programa";
    strings["Render backend##sgse"].plurals[0] = "Back-end de renderização";
    strings["you may need to restart Furnace for this setting to take effect.##sgse0"].plurals[0] = "você pode ter que reiniciar o Furnace para essa configuração fazer efeito.";
    strings["Render driver##sgse"].plurals[0] = "Driver de renderização";
    strings["Automatic##sgse0"].plurals[0] = "Automático";
    strings["Automatic##sgse1"].plurals[0] = "Automático";
    strings["you may need to restart Furnace for this setting to take effect.##sgse1"].plurals[0] = "você pode ter que reiniciar o Furnace para essa configuração fazer efeito.";
    strings["Late render clear##sgse"].plurals[0] = "Limpeza de renderização atrasada";
    strings["calls rend->clear() after rend->present(). might reduce UI latency by one frame in some drivers.##sgse"].plurals[0] = "chama rend->clear() depois de rend->present(). pode reduzir o atraso da interface gráfica por um quadro em alguns drivers.";
    strings["Power-saving mode##sgse"].plurals[0] = "Modo de economia de energia";
    strings["saves power by lowering the frame rate to 2fps when idle.\nmay cause issues under Mesa drivers!##sgse"].plurals[0] = "economiza energia diminuindo a taxa de quadros para 2 FPS quando ocioso.\npode causar problemas com alguns drivers Mesa!";
    strings["Disable threaded input (restart after changing!)##sgse"].plurals[0] = "Desabilitar entrada em threads (reinicie após alterar!)";
    strings["threaded input processes key presses for note preview on a separate thread (on supported platforms), which reduces latency.\nhowever, crashes have been reported when threaded input is on. enable this option if that is the case.##sgse"].plurals[0] = "entrada em threads processa pressionamentos de tecla para previsão de notas em uma thread separada (em plataformas suportadas), o que reduz o atraso.\nporém, crashes já foram relatados quando a entrada em threads estava ativada. habilite esta opção se isso acontecer.";
    strings["Enable event delay##sgse"].plurals[0] = "Habilitar atraso de eventos";
    strings["may cause issues with high-polling-rate mice when previewing notes.##sgse"].plurals[0] = "pode causar problemas com mouses de alta taxa de amostragem ao prever notas.";
    strings["Per-channel oscilloscope threads##sgse"].plurals[0] = "Threads de osciloscópio por canal";
    strings["you're being silly, aren't you? that's enough.##sgse"].plurals[0] = "você tá de brincadeira, né? isso é mais do que suficiente.";
    strings["what are you doing? stop!##sgse"].plurals[0] = "o que você tá fazendo? para!";
    strings["it is a bad idea to set this number higher than your CPU core count (%d)!##sgse"].plurals[0] = "é uma má ideia definir esse número pra um valor mais alto do que o número de núcleos da sua CPU (%d)!";
    strings["Oscilloscope rendering engine:##sgse"].plurals[0] = "Motor de renderização de osciloscópio:";
    strings["ImGui line plot##sgse"].plurals[0] = "Plotagem de linhas do ImGui";
    strings["render using Dear ImGui's built-in line drawing functions.##sgse"].plurals[0] = "desenha o osciloscópio utilizando as funções embutidas de desenho de linha do Dear ImGui.";
    strings["GLSL (if available)##sgse"].plurals[0] = "GLSL (se disponível)";
    strings["render using shaders that run on the graphics card.\nonly available in OpenGL 3.0 render backend.##sgse"].plurals[0] = "renderiza utilizando sombreadores que executam na placa de vídeo.\nsomente disponível no backend de renderização OpenGL 3.0.";
    strings["File##sgse"].plurals[0] = "Arquivo";
    strings["Use system file picker##sgse"].plurals[0] = "Usar diálogo de arquivos do sistema";
    strings["Number of recent files##sgse"].plurals[0] = "Número de arquivos recentes";
    strings["Compress when saving##sgse"].plurals[0] = "Comprimir ao salvar";
    strings["use zlib to compress saved songs.##sgse"].plurals[0] = "usar zlib para comprimir músicas salvas.";
    strings["Save unused patterns##sgse"].plurals[0] = "Salvar sequências não utilizadas";
    strings["Use new pattern format when saving##sgse"].plurals[0] = "Utilizar novo formato de padrão ao salvar";
    strings["use a packed format which saves space when saving songs.\ndisable if you need compatibility with older Furnace and/or tools\nwhich do not support this format.##sgse"].plurals[0] = "utilizar um formato empacotado que economiza espaço ao salvar músicas.\ndesabilite se você precisa de compatibilidade com versões antigas do Furnace e/ou ferramentas\nque não suportam esse formato.";
    strings["Don't apply compatibility flags when loading .dmf##sgse"].plurals[0] = "Não aplicar opções de compatibilidade ao carregar .dmf";
    strings["do not report any issues arising from the use of this option!##sgse"].plurals[0] = "favor não relatar nenhum problema vindo do uso dessa opção!";
    strings["Play after opening song:##sgse"].plurals[0] = "Reproduzir após abrir música:";
    strings["No##pol0"].plurals[0] = "Não##pol0";
    strings["Only if already playing##pol1"].plurals[0] = "Somente se já estiver reproduzindo##pol1";
    strings["Yes##pol0"].plurals[0] = "Sim##pol0";
    strings["Audio export loop/fade out time:##sgse"].plurals[0] = "Tempo de repetição/fade out da exportação de áudio:";
    strings["Set to these values on start-up:##fot0"].plurals[0] = "Definir para estes valores na inicialização:##fot0";
    strings["Loops##sgse"].plurals[0] = "Repetições";
    strings["Fade out (seconds)##sgse"].plurals[0] = "Fade out (segundos)";
    strings["Remember last values##fot1"].plurals[0] = "Lembrar últimos valores##fot1";
    strings["Store instrument name in .fui##sgse"].plurals[0] = "Armazenar nome do instrumento no .fui";
    strings["when enabled, saving an instrument will store its name.\nthis may increase file size.##sgse"].plurals[0] = "quando habilitado, ao salvar um instrumento será armazenado o seu nome.\n isso pode aumentar o tamanho do arquivo.";
    strings["Load instrument name from .fui##sgse"].plurals[0] = "Carregar nome do instrumento do .fui";
    strings["when enabled, loading an instrument will use the stored name (if present).\notherwise, it will use the file name.##sgse"].plurals[0] = "quando habilitado, ao carregar um instrumento será utilizado o nome armazenado, se presente.\ncaso contrário, será utilizado o nome do arquivo.";
    strings["New Song##sgse"].plurals[0] = "Nova Música";
    strings["Initial system:##sgse"].plurals[0] = "Sistema inicial:";
    strings["Current system##sgse"].plurals[0] = "Sistema atual";
    strings["Randomize##sgse"].plurals[0] = "Aleatorizar";
    strings["Reset to defaults##sgse"].plurals[0] = "Reiniciar aos padrões";
    strings["Name##sgse"].plurals[0] = "Nome";
    strings["Invert##sgse0"].plurals[0] = "Inverter";
    strings["Invert##sgse1"].plurals[0] = "Inverter";
    strings["Volume##sgse1"].plurals[0] = "Volume";
    strings["Panning##sgse"].plurals[0] = "Balanço";
    strings["Front/Rear##sgse"].plurals[0] = "Frontal/Traseira";
    strings["Configure##sgse"].plurals[0] = "Configurar";
    strings["When creating new song:##sgse"].plurals[0] = "Ao criar uma nova música:";
    strings["Display system preset selector##NSB0"].plurals[0] = "Mostrar seletor de predefinições de sistema##NSB0";
    strings["Start with initial system##NSB1"].plurals[0] = "Começar com o sistema inicial##NSB1";
    strings["Default author name##sgse"].plurals[0] = "Nome do autor padrão";
    strings["Start-up##sgse"].plurals[0] = "Inicialização";
    strings["Disable fade-in during start-up##sgse"].plurals[0] = "Desabilitar transição fade-in durante inicialização";
    strings["About screen party time##sgse"].plurals[0] = "\"Hora da Festa\" na tela \"Sobre...\"";
    strings["Warning: may cause epileptic seizures.##sgse"].plurals[0] = "Aviso: pode causar ataques epiléticos.";
    strings["Behavior##sgse"].plurals[0] = "Comportamento";
    strings["New instruments are blank##sgse"].plurals[0] = "Novos instrumentos começam em branco";
    strings["Language##sgse"].plurals[0] = "Linguagem";
    strings["GUI language##sgse"].plurals[0] = "Linguagem da interface";
    strings["Translate channel names in pattern header##sgse"].plurals[0] = "Traduzir nomes dos canais no cabeçalho da sequência";
    strings["Translate channel names in channel oscilloscope label##sgse"].plurals[0] = "Traduzir nomes dos canais no rótulo do osciloscópio por canal";
    strings["Translate short channel names (in orders and other places)##sgse"].plurals[0] = "Traduzir nomes curtos dos canais (nas ordens e em outros lugares)";
    strings["test##sgse"].plurals[0] = "teste";
    strings["iulserghiueshgui##sgse"].plurals[0] = "kcjsghçzxjcmfgag"; // thanks to jvsTSX for this one!
    strings["Audio##sgse"].plurals[0] = "Áudio";
    strings["Output##sgse"].plurals[0] = "Saída";
    strings["Backend##sgse"].plurals[0] = "Back-end";
    strings["Driver##sgse"].plurals[0] = "Driver";
    strings["Automatic##sgse2"].plurals[0] = "Automático";
    strings["you may need to restart Furnace for this setting to take effect.##sgse2"].plurals[0] = "você pode ter que reiniciar o Furnace para essa configuração fazer efeito.";
    strings["Device##sgse"].plurals[0] = "Dispositivo";
    strings["<click on OK or Apply first>##sgse"].plurals[0] = "<clique em OK ou Aplicar primeiro>";
    strings["ALERT - TRESPASSER DETECTED##sgse"].plurals[0] = "ALERTA - INVASOR DETECTADO";
    strings["you have been arrested for trying to engage with a disabled combo box.##sgse"].plurals[0] = "você foi preso por tentar interagir com uma caixa de combinação desabilitada.";
    strings["<System default>##sgse0"].plurals[0] = "<Padrão do sistema>";
    strings["<System default>##sgse1"].plurals[0] = "<Padrão do sistema>";
    strings["Sample rate##sgse"].plurals[0] = "Taxa de amostragem";
    strings["Outputs##sgse"].plurals[0] = "Saídas";
    strings["Channels##sgse"].plurals[0] = "Canais";
    strings["What?##sgse3"].plurals[0] = "O quê?";
    strings["Buffer size##sgse"].plurals[0] = "Tamanho de buffer";
    strings["%d (latency: ~%.1fms)##sgse"].plurals[0] = "%d (atraso: ~%.1fms)";
    strings["Multi-threaded (EXPERIMENTAL)##sgse"].plurals[0] = "Multissegmentado (EXPERIMENTAL)";
    strings["runs chip emulation on separate threads.\nmay increase performance when using heavy emulation cores.\n\nwarnings:\n- experimental!\n- only useful on multi-chip songs.##sgse"].plurals[0] = "roda a emulação de chips em threads separadas.\npode aumentar a performance ao utilizar núcleos de emulação pesados.\n\navisos:\n- experimental!\n- só é útil em músicas com múltiplos chips.";
    strings["Number of threads##sgse"].plurals[0] = "Número de threads:";
    strings["that's the limit!##sgse"].plurals[0] = "esse é o limite!";
    strings["it is a VERY bad idea to set this number higher than your CPU core count (%d)!##sgse"].plurals[0] = "é uma PÉSSIMA ideia definir esse número pra um valor mais alto do que o número de núcleos da sua CPU (%d)!";
    strings["Low-latency mode##sgse"].plurals[0] = "Modo de baixo atraso";
    strings["reduces latency by running the engine faster than the tick rate.\nuseful for live playback/jam mode.\n\nwarning: only enable if your buffer size is small (10ms or less).##sgse"].plurals[0] = "reduz o atraso rodando o motor mais rápido que a taxa de ticks.\nútil para tocar ao vivo/\"modo jam\".\n\naviso: só habilite se o seu tamanho de buffer for pequeno (10ms ou menor).";
    strings["Force mono audio##sgse"].plurals[0] = "Forçar áudio mono";
    strings["Exclusive mode##sgse"].plurals[0] = "Modo exclusivo";
    strings["want: %d samples @ %.0fHz (%d %s)##sgse"].plurals[0] = "queria: %d amostras a %.0fHz (%d %s)";
    strings["channel##sgse"].plurals[0] = "canal";
    strings["channel##sgse"].plurals[1] = "canais";
    strings["got: %d samples @ %.0fHz (%d %s)##sgse"].plurals[0] = "conseguiu: %d amostras @ %.0fHz (%d %s)";
    strings["Mixing##sgse"].plurals[0] = "Mixing";
    strings["Quality##sgse"].plurals[0] = "Qualidade";
    strings["Software clipping##sgse"].plurals[0] = "Clipping em software";
    strings["DC offset correction##sgse"].plurals[0] = "Correção de deslocamento DC";
    strings["Metronome##sgse"].plurals[0] = "Metrônomo";
    strings["Volume##sgse2"].plurals[0] = "Volume";
    strings["Sample preview##sgse"].plurals[0] = "Previsão de sample";
    strings["Volume##sgse3"].plurals[0] = "Volume";
    strings["MIDI##sgse"].plurals[0] = "MIDI";
    strings["MIDI input##sgse0"].plurals[0] = "Entrada MIDI";
    strings["MIDI input##sgse1"].plurals[0] = "Entrada MIDI";
    strings["<disabled>##sgse0"].plurals[0] = "<desabilitada>";
    strings["<disabled>##sgse1"].plurals[0] = "<desabilitada>";
    strings["Re-scan MIDI devices##sgse"].plurals[0] = "Procurar novamente por dispositivos MIDI";
    strings["Note input##sgse0"].plurals[0] = "Entrada de notas";
    strings["Velocity input##sgse"].plurals[0] = "Entrada de velocidade";
    strings["Map MIDI channels to direct channels##sgse"].plurals[0] = "Mapear canais MIDI a canais diretos";
    strings["Program change pass-through##sgse"].plurals[0] = "Passagem de mudança de programa";
    strings["Map Yamaha FM voice data to instruments##sgse"].plurals[0] = "Mapear dados de timbre FM Yamaha aos instrumentos";
    strings["Program change is instrument selection##sgse"].plurals[0] = "Mudança de programa é seleção de instrumento";
    strings["Listen to MIDI clock##sgse"].plurals[0] = "Escutar clock MIDI";
    strings["Listen to MIDI time code##sgse"].plurals[0] = "Escutar timecode MIDI";
    strings["Value input style##sgse0"].plurals[0] = "Estilo de entrada de valor";
    strings["Value input style##sgse1"].plurals[0] = "Estilo de entrada de valor";
    strings["Control##valueCCS"].plurals[0] = "Controle##valueCCS";
    strings["CC of upper nibble##valueCC1"].plurals[0] = "CC do nibble superior##valueCC1";
    strings["MSB CC##valueCC1"].plurals[0] = "CC do byte mais significativo##valueCC1";
    strings["CC of lower nibble##valueCC2"].plurals[0] = "CC do nibble inferior##valueCC2";
    strings["LSB CC##valueCC2"].plurals[0] = "CC do byte menos significativo##valueCC2";
    strings["Per-column control change##sgse"].plurals[0] = "Mudança de controle por coluna";
    strings["Control##valueCCS"].plurals[0] = "Controle##valueCCS";
    strings["CC of upper nibble##valueCC1"].plurals[0] = "CC do nibble superior##valueCC1";
    strings["MSB CC##valueCC1"].plurals[0] = "CC do byte mais significativo##valueCC1";
    strings["CC of lower nibble##valueCC2"].plurals[0] = "CC do nibble inferior##valueCC2";
    strings["LSB CC##valueCC2"].plurals[0] = "CC do byte menos significativo##valueCC2";
    strings["Volume curve##sgse0"].plurals[0] = "Curva de volume";
    strings["Volume curve##sgse1"].plurals[0] = "Curva de volume";
    strings["Actions:##sgse"].plurals[0] = "Ações:";
    strings["(learning! press a button or move a slider/knob/something on your device.)##sgse"].plurals[0] = "(aprendendo! pressione, gire um botão, mova um controle deslizante ou outra coisa em seu dispositivo.)";
    strings["Type##sgse0"].plurals[0] = "Tipo";
    strings["Channel##sgse0"].plurals[0] = "Canal";
    strings["Note/Control##sgse"].plurals[0] = "Nota/Controle";
    strings["Velocity/Value##sgse"].plurals[0] = "Velocidade/Valor";
    strings["Action##sgse"].plurals[0] = "Ação";
    strings["Any##sgse0"].plurals[0] = "Qualquer";
    strings["Any##sgse1"].plurals[0] = "Qualquer";
    strings["Any##sgse2"].plurals[0] = "Qualquer";
    strings["Any##sgse3"].plurals[0] = "Qualquer";
    strings["--none--##sgse"].plurals[0] = "--nenhuma--";
    strings["waiting...##BLearn"].plurals[0] = "aguardando...##BLearn";
    strings["Learn##BLearn"].plurals[0] = "Aprender##BLearn";
    strings["MIDI output##sgse0"].plurals[0] = "Saída MIDI";
    strings["MIDI output##sgse1"].plurals[0] = "Saída MIDI";
    strings["<disabled>##sgse2"].plurals[0] = "<desabilitada>";
    strings["<disabled>##sgse3"].plurals[0] = "<desabilitada>";
    strings["Output mode:##sgse"].plurals[0] = "Modo de saída:";
    strings["Off (use for TX81Z)##sgse"].plurals[0] = "Desabilitada (use para TX81Z)";
    strings["Melodic##sgse"].plurals[0] = "Melódica";
    strings["Light Show (use for Launchpad)##sgse"].plurals[0] = "Show de luzes (use para o Launchpad)";
    strings["Send Program Change##sgse"].plurals[0] = "Enviar Mudança de Programa";
    strings["Send MIDI clock##sgse"].plurals[0] = "Enviar Clock MIDI";
    strings["Send MIDI timecode##sgse"].plurals[0] = "Enviar Timecode MIDI";
    strings["Timecode frame rate:##sgse"].plurals[0] = "Taxa de quadros do timecode:";
    strings["Closest to Tick Rate##sgse"].plurals[0] = "Mais próxima da taxa de ticks";
    strings["Film (24fps)##sgse"].plurals[0] = "Filme (24fps)";
    strings["PAL (25fps)##sgse"].plurals[0] = "PAL (25fps)";
    strings["NTSC drop (29.97fps)##sgse"].plurals[0] = "NTSC drop (29.97fps)";
    strings["NTSC non-drop (30fps)##sgse"].plurals[0] = "NTSC não-drop (30fps)";
    strings["Emulation##sgse"].plurals[0] = "Emulação";
    strings["Cores##sgse"].plurals[0] = "Núcleos";
    strings["System##sgse"].plurals[0] = "Sistema";
    strings["Playback Core(s)##sgse"].plurals[0] = "Núcleo(s) de reprodução";
    strings["used for playback##sgse"].plurals[0] = "utilizados para reprodução";
    strings["Render Core(s)##sgse"].plurals[0] = "Núcleo(s) de renderização";
    strings["used in audio export##sgse"].plurals[0] = "utilizados para exportação de áudio";
    strings["PC Speaker strategy##sgse"].plurals[0] = "Estratégia de PC Speaker";
    strings["Sample ROMs:##sgse"].plurals[0] = "ROMs de Sample:";
    strings["OPL4 YRW801 path##sgse"].plurals[0] = "Caminho da YRW801 do OPL4";
    strings["MultiPCM TG100 path##sgse"].plurals[0] = "Caminho da TG100 do MultiPCM";
    strings["MultiPCM MU5 path##sgse"].plurals[0] = "Caminho da MU5 do MultiPCM";
    strings["Keyboard##sgse0"].plurals[0] = "Teclado";
    strings["Keyboard##sgse1"].plurals[0] = "Teclado";
    strings["Import##sgse0"].plurals[0] = "Importar";
    strings["Export##sgse0"].plurals[0] = "Exportar";
    strings["Reset defaults##sgse0"].plurals[0] = "Redefinir padrões";
    strings["Are you sure you want to reset the keyboard settings?##sgse"].plurals[0] = "Tem certeza de que quer redefinir as configurações de teclado?";
    strings["Global hotkeys##sgse"].plurals[0] = "Atalhos globais";
    strings["Window activation##sgse"].plurals[0] = "Ativação de janela";
    strings["Note input##sgse1"].plurals[0] = "Entrada de nota";
    strings["Key##sgse"].plurals[0] = "Tecla";
    strings["Type##sgse1"].plurals[0] = "Tipo";
    strings["Value##sgse"].plurals[0] = "Valor";
    strings["Remove##sgse"].plurals[0] = "Remover";
    strings["Macro release##SNType_%d"].plurals[0] = "Repouso de macro##SNType_%d";
    strings["Note release##SNType_%d"].plurals[0] = "Repouso de nota##SNType_%d";
    strings["Note off##SNType_%d"].plurals[0] = "Note off##SNType_%d";
    strings["Note##SNType_%d"].plurals[0] = "Nota##SNType_%d";
    strings["Add...##sgse"].plurals[0] = "Adicionar...";
    strings["Pattern##sgse0"].plurals[0] = "Sequência";
    strings["Instrument list##sgse"].plurals[0] = "Lista de instrumentos";
    strings["Wavetable list##sgse"].plurals[0] = "Lista de wavetables";
    strings["Local wavetables list##sgse"].plurals[0] = "Lista de wavetables locais";
    strings["Sample list##sgse"].plurals[0] = "Lista de samples";
    strings["Orders##sgse0"].plurals[0] = "Ordens";
    strings["Sample editor##sgse"].plurals[0] = "Editor de samples";
    strings["Interface##sgse0"].plurals[0] = "Interface";
    strings["Layout##sgse"].plurals[0] = "Leiaute";
    strings["Workspace layout:##sgse"].plurals[0] = "Leiaute do espaço de trabalho:";
    strings["Import##sgse1"].plurals[0] = "Importar";
    strings["Export##sgse1"].plurals[0] = "Exportar";
    strings["Reset##sgse"].plurals[0] = "Redefinir";
    strings["Are you sure you want to reset the workspace layout?##sgse"].plurals[0] = "Tem certeza de que quer redefinir o leiaute do espaço de trabalho?";
    strings["Allow docking editors##sgse"].plurals[0] = "Permitir ancorar editores";
    strings["Remember window position##sgse"].plurals[0] = "Lembrar posição da janela";
    strings["remembers the window's last position on start-up.##sgse"].plurals[0] = "memoriza a última posição da janela no início do programa.";
    strings["Only allow window movement when clicking on title bar##sgse"].plurals[0] = "Permitir movimentação da janela somente ao clicar na barra de título";
    strings["Center pop-up windows##sgse"].plurals[0] = "Centralizar janelas flutuantes";
    strings["Play/edit controls layout:##sgse"].plurals[0] = "Leiaute de controles de reprodução/edição:";
    strings["Classic##ecl0"].plurals[0] = "Clássico##ecl0";
    strings["Compact##ecl1"].plurals[0] = "Compacto##ecl1";
    strings["Compact (vertical)##ecl2"].plurals[0] = "Compacto (vertical)##ecl2";
    strings["Split##ecl3"].plurals[0] = "Dividido##ecl3";
    strings["Position of buttons in Orders:##sgse"].plurals[0] = "Posição dos botões na janela Ordens:";
    strings["Top##obp0"].plurals[0] = "Topo##obp0";
    strings["Left##obp1"].plurals[0] = "Esquerda##obp1";
    strings["Right##obp2"].plurals[0] = "Direita##obp2";
    strings["Mouse##sgse"].plurals[0] = "Mouse";
    strings["Double-click time (seconds)##sgse"].plurals[0] = "Tempo de duplo clique (segundos)";
    strings["Don't raise pattern editor on click##sgse"].plurals[0] = "Não levantar o editor de sequência ao clicar";
    strings["Focus pattern editor when selecting instrument##sgse"].plurals[0] = "Focar editor de sequência ao selecionar instrumento";
    strings["Note preview behavior:##sgse"].plurals[0] = "Comportamento de previsão de nota:";
    strings["Never##npb0"].plurals[0] = "Nunca##npb0";
    strings["When cursor is in Note column##npb1"].plurals[0] = "Quando o cursor estiver na coluna de Nota##npb1";
    strings["When cursor is in Note column or not in edit mode##npb2"].plurals[0] = "Quando o cursor estiver na coluna de Nota ou não estiver no modo de edição##npb2";
    strings["Always##npb3"].plurals[0] = "Sempre##npb3";
    strings["Allow dragging selection:##sgse"].plurals[0] = "Permitir arrastar seleção:";
    strings["No##dms0"].plurals[0] = "Não##dms0";
    strings["Yes##dms1"].plurals[0] = "Sim##dms1";
    strings["Yes (while holding Ctrl only)##dms2"].plurals[0] = "Sim (somente quando segurando Ctrl)##dms2";
    strings["Toggle channel solo on:##sgse"].plurals[0] = "Alternar isolamento de canal ao:";
    strings["Right-click or double-click##soloA"].plurals[0] = "Clicar com o botão direito ou duplo clique##soloA";
    strings["Right-click##soloR"].plurals[0] = "Clicar com o botão direito##soloR";
    strings["Double-click##soloD"].plurals[0] = "Duplo clique##soloD";
    strings["Double click selects entire column##sgse"].plurals[0] = "Duplo clique seleciona a coluna inteira";
    strings["Cursor behavior##sgse"].plurals[0] = "Comportamento do cursor";
    strings["Insert pushes entire channel row##sgse"].plurals[0] = "Inserção empurra a linha do canal inteiro";
    strings["Pull delete affects entire channel row##sgse"].plurals[0] = "Excluir e puxar afeta a linha do canal inteiro";
    strings["Push value when overwriting instead of clearing it##sgse"].plurals[0] = "Empurrar valor ao sobrescrever ao invés de limpá-lo";
    strings["Effect input behavior:##sgse"].plurals[0] = "Comportamento de inserção de efeitos:";
    strings["Move down##eicb0"].plurals[0] = "Mover para baixo##eicb0";
    strings["Move to effect value (otherwise move down)##eicb1"].plurals[0] = "Mover para valor do efeito (caso contrário mover para baixo)##eicb1";
    strings["Move to effect value/next effect and wrap around##eicb2"].plurals[0] = "Mover para valor do efeito/próximo efeito e dar a volta##eicb2";
    strings["Delete effect value when deleting effect##sgse"].plurals[0] = "Excluir valor do efeito ao excluir efeito";
    strings["Change current instrument when changing instrument column (absorb)##sgse"].plurals[0] = "Alterar instrumento atual ao alterar coluna de instrumento (absorver)";
    strings["Remove instrument value when inserting note off/release##sgse"].plurals[0] = "Remover valor do instrumento ao inserir note off/repouso";
    strings["Remove volume value when inserting note off/release##sgse"].plurals[0] = "Remover valor do volume ao inserir note off/repouso";
    strings["Cursor movement##sgse"].plurals[0] = "Movimentação do cursor";
    strings["Wrap horizontally:##sgse"].plurals[0] = "Dar a volta horizontalmente:";
    strings["No##wrapH0"].plurals[0] = "Não##wrapH0";
    strings["Yes##wrapH1"].plurals[0] = "Sim##wrapH1";
    strings["Yes, and move to next/prev row##wrapH2"].plurals[0] = "Sim, e mover para a próxima linha/anterior##wrapH2";
    strings["Wrap vertically:##sgse"].plurals[0] = "Dar a volta verticalmente:";
    strings["No##wrapV0"].plurals[0] = "Não##wrapV0";
    strings["Yes##wrapV1"].plurals[0] = "Sim##wrapV1";
    strings["Yes, and move to next/prev pattern##wrapV2"].plurals[0] = "Sim, e mover para a próxima sequência/anterior##wrapV2";
    strings["Yes, and move to next/prev pattern (wrap around)##wrapV2"].plurals[0] = "Sim, e mover para a próxima sequência/anterior (dando a volta pela música)##wrapV2";
    strings["Cursor movement keys behavior:##sgse"].plurals[0] = "Comportamento das teclas de movimentação do cursor:";
    strings["Move by one##cmk0"].plurals[0] = "Mover uma célula##cmk0";
    strings["Move by Edit Step##cmk1"].plurals[0] = "Mover pelo Passo de Edição##cmk1";
    strings["Move cursor by edit step on delete##sgse"].plurals[0] = "Mover cursor pelo Passo de Edição ao excluir";
    strings["Move cursor by edit step on insert (push)##sgse"].plurals[0] = "Mover cursor pelo Passo de Edição ao inserir (empurrar)";
    strings["Move cursor up on backspace-delete##sgse"].plurals[0] = "Mover cursor para cima ao excluir-backspace";
    strings["Move cursor to end of clipboard content when pasting##sgse"].plurals[0] = "Mover cursor ao final do conteúdo da área de transferência ao colar";
    strings["Scrolling##sgse"].plurals[0] = "Rolagem";
    strings["Change order when scrolling outside of pattern bounds:##sgse"].plurals[0] = "Trocar ordem ao rolar fora dos limites da sequência:";
    strings["No##pscroll0"].plurals[0] = "Não##pscroll0";
    strings["Yes##pscroll1"].plurals[0] = "Sim##pscroll1";
    strings["Yes, and wrap around song##pscroll2"].plurals[0] = "Sim, e dar a volta pela música##pscroll2";
    strings["Cursor follows current order when moving it##sgse"].plurals[0] = "Cursor segue a ordem atual ao movê-lo";
    strings["applies when playback is stopped.##sgse"].plurals[0] = "aplica-se quando a reprodução está parada.";
    strings["Don't scroll when moving cursor##sgse"].plurals[0] = "Não rolar ao mover cursor";
    strings["Move cursor with scroll wheel:##sgse"].plurals[0] = "Mover cursor com o scroll do mouse:";
    strings["No##csw0"].plurals[0] = "Não##csw0";
    strings["Yes##csw1"].plurals[0] = "Sim##csw1";
    strings["Inverted##csw2"].plurals[0] = "Invertido##csw2";
    strings["How many steps to move with each scroll wheel step?##sgse"].plurals[0] = "Quantos passos mover com cada passo do scroll do mouse?";
    strings["One##cws0"].plurals[0] = "Um a um##cws0";
    strings["Edit Step##cws1"].plurals[0] = "Passo de edição##cws1";
    strings["Assets##sgse0"].plurals[0] = "Recursos";
    strings["Display instrument type menu when adding instrument##sgse"].plurals[0] = "Mostrar menu de tipo de instrumento ao adicionar instrumento";
    strings["Select asset after opening one##sgse"].plurals[0] = "Selecionar recurso após abrir um";
    strings["Appearance##sgse"].plurals[0] = "Aparência";
    strings["Scaling##sgse"].plurals[0] = "Escala";
    strings["Automatic UI scaling factor##sgse"].plurals[0] = "Fator de escala de interface automático";
    strings["UI scaling factor##sgse"].plurals[0] = "Fator de escala de interface";
    strings["Icon size##sgse"].plurals[0] = "Tamanho dos ícones";
    strings["Text##sgse"].plurals[0] = "Texto";
    strings["Font renderer##sgse"].plurals[0] = "Renderizador de fonte";
    strings["Main font##sgse"].plurals[0] = "Fonte principal";
    strings["Size##MainFontSize"].plurals[0] = "Tamanho##MainFontSize";
    strings["Header font##sgse"].plurals[0] = "Fonte do cabeçalho";
    strings["Size##HeadFontSize"].plurals[0] = "Tamanho##HeadFontSize";
    strings["Pattern font##sgse"].plurals[0] = "Fonte da sequência";
    strings["Size##PatFontSize"].plurals[0] = "Tamanho##PatFontSize";
    strings["Anti-aliased fonts##sgse"].plurals[0] = "Fontes suavizadas";
    strings["Support bitmap fonts##sgse"].plurals[0] = "Suporte a fontes bitmap";
    strings["Hinting:##sgse"].plurals[0] = "Hinting:";
    strings["Off (soft)##fh0"].plurals[0] = "Desligado (suave)##fh0";
    strings["Slight##fh1"].plurals[0] = "Leve##fh1";
    strings["Normal##fh2"].plurals[0] = "Normal##fh2";
    strings["Full (hard)##fh3"].plurals[0] = "Completo (duro)##fh3";
    strings["Auto-hinter:##sgse"].plurals[0] = "Auto-hinter:";
    strings["Disable##fah0"].plurals[0] = "Desabilitado##fah0";
    strings["Enable##fah1"].plurals[0] = "Habilitado##fah1";
    strings["Force##fah2"].plurals[0] = "Forçado##fah2";
    strings["Display Japanese characters##sgse"].plurals[0] = "Exibir caracteres Japoneses";
    strings["Only toggle this option if you have enough graphics memory.\n"
            "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
            "このオプションは、十分なグラフィックメモリがある場合にのみ切り替えてください。\n"
            "これは、Dear ImGuiにダイナミックフォントアトラスが実装されるまでの一時的な解決策です。##sgse"].plurals[0] = 
            
            "Somente habilite esta opção se você tiver memória gráfica suficiente.\n"
            "Isto é uma solução temporária até que atlas de fonte dinâmico seja implementado no Dear ImGui.\n\n"
            "このオプションは、十分なグラフィックメモリがある場合にのみ切り替えてください。\n"
            "これは、Dear ImGuiにダイナミックフォントアトラスが実装されるまでの一時的な解決策です。";
    strings["Display Chinese (Simplified) characters##sgse"].plurals[0] = "Exibir caracteres Chineses (Simplificado)";
    strings["Only toggle this option if you have enough graphics memory.\n"
            "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
            "请在确保你有足够的显存后再启动此设定\n"
            "这是一个在ImGui实现动态字体加载之前的临时解决方案##sgse"].plurals[0] = 
            
            "Somente habilite esta opção se você tiver memória gráfica suficiente.\n"
            "Isto é uma solução temporária até que atlas de fonte dinâmico seja implementado no Dear ImGui.\n\n"
            "请在确保你有足够的显存后再启动此设定\n"
            "这是一个在ImGui实现动态字体加载之前的临时解决方案";
    
    strings["Display Chinese (Traditional) characters##sgse"].plurals[0] = "Exibir caracteres Chineses (Simplificado)";
    strings["Only toggle this option if you have enough graphics memory.\n"
            "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
            "請在確保你有足夠的顯存后再啟動此設定\n"
            "這是一個在ImGui實現動態字體加載之前的臨時解決方案##sgse"].plurals[0] = 
            
            "Somente habilite esta opção se você tiver memória gráfica suficiente.\n"
            "Isto é uma solução temporária até que atlas de fonte dinâmico seja implementado no Dear ImGui.\n\n"
            "請在確保你有足夠的顯存后再啟動此設定\n"
            "這是一個在ImGui實現動態字體加載之前的臨時解決方案";
    strings["Display Korean characters##sgse"].plurals[0] = "Exibir caracteres Coreanos";
    strings["Only toggle this option if you have enough graphics memory.\n"
            "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
            "그래픽 메모리가 충분한 경우에만 이 옵션을 선택하십시오.\n"
            "이 옵션은 Dear ImGui에 동적 글꼴 아틀라스가 구현될 때까지 임시 솔루션입니다.##sgse"].plurals[0] = 
            
            "Somente habilite esta opção se você tiver memória gráfica suficiente.\n"
            "Isto é uma solução temporária até que atlas de fonte dinâmico seja implementado no Dear ImGui.\n\n"
            "그래픽 메모리가 충분한 경우에만 이 옵션을 선택하십시오.\n"
            "이 옵션은 Dear ImGui에 동적 글꼴 아틀라스가 구현될 때까지 임시 솔루션입니다.";
    strings["Program##sgse2"].plurals[0] = "Programa";
    strings["Title bar:##sgse"].plurals[0] = "Barra de título:";
    strings["Furnace-B##tbar0"].plurals[0] = "Furnace-B##tbar0";
    strings["Song Name - Furnace-B##tbar1"].plurals[0] = "Nome da Música - Furnace-B##tbar1";
    strings["file_name.fur - Furnace-B##tbar2"].plurals[0] = "nome_do_arquivo.fur - Furnace-B##tbar2";
    strings["/path/to/file.fur - Furnace-B##tbar3"].plurals[0] = "/caminho/para/o/arquivo.fur - Furnace-B##tbar3";
    strings["Display system name on title bar##sgse"].plurals[0] = "Mostrar nome do sistema na barra de título";
    strings["Display chip names instead of \"multi-system\" in title bar##sgse"].plurals[0] = "Mostrar nomes dos chips ao invés de \"multi-sistema\" na barra de título##sgse";
    strings["Status bar:##sgse"].plurals[0] = "Barra de status:";
    strings["Cursor details##sbar0"].plurals[0] = "Detalhes do cursor##sbar0";
    strings["File path##sbar1"].plurals[0] = "Caminho do arquivo##sbar1";
    strings["Cursor details or file path##sbar2"].plurals[0] = "Detalhes do cursor ou caminho do arquivo##sbar2";
    strings["Nothing##sbar3"].plurals[0] = "Nada##sbar3";
    strings["Display playback status when playing##sgse"].plurals[0] = "Exibir status da reprodução enquanto reproduz";
    strings["Export options layout:##sgse"].plurals[0] = "Leiaute das opções de exportação:";
    strings["Sub-menus in File menu##eol0"].plurals[0] = "Sub-menus no menu Arquivo##eol0";
    strings["Modal window with tabs##eol1"].plurals[0] = "Janela modal com abas##eol1";
    strings["Modal windows with options in File menu##eol2"].plurals[0] = "Janela modal com opções no menu Arquivo##eol2";
    strings["Capitalize menu bar##sgse"].plurals[0] = "Capitalizar barra de menus";
    strings["Display add/configure/change/remove chip menus in File menu##sgse"].plurals[0] = "Exibir menus para adicionar/configurar/alterar/remover chip no menu Arquivo";
    strings["Orders##sgse1"].plurals[0] = "Ordens";
    strings["Highlight channel at cursor in Orders##sgse"].plurals[0] = "Realçar canal sob o cursor na janela Ordens";
    strings["Orders row number format:##sgse"].plurals[0] = "Formato de número de linha da ordem:";
    strings["Decimal##orbD"].plurals[0] = "Decimal##orbD";
    strings["Hexadecimal##orbH"].plurals[0] = "Hexadecimal##orbH";
    strings["Pattern##sgse1"].plurals[0] = "Sequência";
    strings["Center pattern view##sgse"].plurals[0] = "Centralizar visualização da sequência";
    strings["Overflow pattern highlights##sgse"].plurals[0] = "Preencher destaques da sequência por todo o fundo";
    strings["Display previous/next pattern##sgse"].plurals[0] = "Exibir sequência anterior/próxima";
    strings["Pattern row number format:##sgse"].plurals[0] = "Formato de número de linha da sequência:";
    strings["Decimal##prbD"].plurals[0] = "Decimal##prbD";
    strings["Hexadecimal##prbH"].plurals[0] = "Hexadecimal##prbH";
    strings["Pattern view labels:##sgse"].plurals[0] = "Rótulos da visualização da sequência:";
    strings["Note off (3-char)##sgse"].plurals[0] = "Note off (3 caracteres)";
    strings["Note release (3-char)##sgse"].plurals[0] = "Repouso de nota (3 carac.)";
    strings["Macro release (3-char)##sgse"].plurals[0] = "Repouso de macro (3 carac.)";
    strings["Empty field (3-char)##sgse"].plurals[0] = "Campo vazio (3 carac.)";
    strings["Empty field (2-char)##sgse"].plurals[0] = "Campo vazio (2 carac.)";
    strings["Pattern view spacing after:##sgse"].plurals[0] = "Espaçamento da visualização de sequência após:";
    strings["Note##sgse"].plurals[0] = "Nota";
    strings["Instrument##sgse1"].plurals[0] = "Instrumento";
    strings["Volume##sgse4"].plurals[0] = "Volume";
    strings["Effect##sgse"].plurals[0] = "Efeito";
    strings["Effect value##sgse"].plurals[0] = "Valor do efeito";
    strings["Single-digit effects for 00-0F##sgse"].plurals[0] = "Efeitos de digito único para 00-0F";
    strings["Use flats instead of sharps##sgse"].plurals[0] = "Usar bemóis ao invés de sustenidos";
    strings["Use German notation##sgse"].plurals[0] = "Usar notação alemã";
    strings["Channel##sgse1"].plurals[0] = "Canal";
    strings["Channel style:##sgse"].plurals[0] = "Estilo de vanal:";
    strings["Classic##CHS0"].plurals[0] = "Clássico##CHS0";
    strings["Line##CHS1"].plurals[0] = "Linha##CHS1";
    strings["Round##CHS2"].plurals[0] = "Redondo##CHS2";
    strings["Split button##CHS3"].plurals[0] = "Botão dividido##CHS3";
    strings["Square border##CHS4"].plurals[0] = "Borda quadrada##CHS4";
    strings["Round border##CHS5"].plurals[0] = "Borda arredondada##CHS5";
    strings["Channel volume bar:##sgse"].plurals[0] = "Barra de volume do canal:";
    strings["Non##CHV0"].plurals[0] = "Nenhuma##CHV0";
    strings["Simple##CHV1"].plurals[0] = "Simples##CHV1";
    strings["Stereo##CHV2"].plurals[0] = "Estéreo##CHV2";
    strings["Real##CHV3"].plurals[0] = "Real##CHV3";
    strings["Real (stereo)##CHV4"].plurals[0] = "Real (estéreo)##CHV4";
    strings["Channel feedback style:##sgse"].plurals[0] = "Estilo de feedback do canal:";
    strings["Off##CHF0"].plurals[0] = "Desativado##CHF0";
    strings["Note##CHF1"].plurals[0] = "Nota##CHF1";
    strings["Volume##CHF2"].plurals[0] = "Volume##CHF2";
    strings["Active##CHF3"].plurals[0] = "Ativo##CHF3";
    strings["Channel font:##sgse"].plurals[0] = "Fonte do canal:";
    strings["Regular##CHFont0"].plurals[0] = "Regular##CHFont0";
    strings["Monospace##CHFont1"].plurals[0] = "Monoespaçada##CHFont1";
    strings["Center channel name##sgse"].plurals[0] = "Centralizar nome do canal";
    strings["Channel colors:##sgse"].plurals[0] = "Cores dos canais:";
    strings["Single##CHC0"].plurals[0] = "Única##CHC0";
    strings["Channel type##CHC1"].plurals[0] = "Tipo de canal##CHC1";
    strings["Instrument type##CHC2"].plurals[0] = "Tipo de instrumento##CHC2";
    strings["Channel name colors:##sgse"].plurals[0] = "Cores dos nomes dos canais:";
    strings["Single##CTC0"].plurals[0] = "Única##CTC0";
    strings["Channel type##CTC1"].plurals[0] = "Tipo de canal##CTC1";
    strings["Instrument type##CTC2"].plurals[0] = "Tipo de instrumento##CTC2";
    strings["Assets##sgse1"].plurals[0] = "Recursos";
    strings["Unified instrument/wavetable/sample list##sgse"].plurals[0] = "Lista unificada de instrumentos/wavetables/samples";
    strings["Horizontal instrument list##sgse"].plurals[0] = "Lista de instrumentos horizontal";
    strings["Instrument list icon style:##sgse"].plurals[0] = "Estilo de ícone da lista de instrumentos:";
    strings["None##iis0"].plurals[0] = "Nenhum##iis0";
    strings["Graphical icons##iis1"].plurals[0] = "Ícones gráficos##iis1";
    strings["Letter icons##iis2"].plurals[0] = "Ícones de letras##iis2";
    strings["Colorize instrument editor using instrument type##sgse"].plurals[0] = "Colorizar editor de instrumentos utilizando tipo de instrumento";
    strings["Macro Editor##sgse0"].plurals[0] = "Editor de Macro";
    strings["Macro editor layout:##sgse"].plurals[0] = "Leiaute do editor de macro:";
    strings["Unified##mel0"].plurals[0] = "Unificado##mel0";
    strings["Grid##mel2"].plurals[0] = "Grade##mel2";
    strings["Single (with list)##mel3"].plurals[0] = "Único (com lista)##mel3";
    strings["Use classic macro editor vertical slider##sgse"].plurals[0] = "Utilizar slider vertical clássico do editor de macro";
    strings["Wave Editor##sgse"].plurals[0] = "Editor de wavetable";
    strings["Use compact wave editor##sgse"].plurals[0] = "Utilizar editor de wavetable compacto";
    strings["FM Editor##sgse0"].plurals[0] = "Editor de FM";
    strings["FM parameter names:##sgse"].plurals[0] = "Nomes dos parâmetros FM:";
    strings["Friendly##fmn0"].plurals[0] = "Amigáveis##fmn0";
    strings["Technical##fmn1"].plurals[0] = "Técnicos##fmn1";
    strings["Technical (alternate)##fmn2"].plurals[0] = "Técnicos (alternativos)##fmn2";
    strings["Use standard OPL waveform names##sgse"].plurals[0] = "Utilizar nomes padrão de formas de onda OPL";
    strings["FM parameter editor layout:##sgse"].plurals[0] = "Leiaute do editor de parâmetros FM:";
    strings["Modern##fml0"].plurals[0] = "Moderno##fml0";
    strings["Compact (2x2, classic)##fml1"].plurals[0] = "Compacto (2x2, clássico)##fml1";
    strings["Compact (1x4)##fml2"].plurals[0] = "Compacto (1x4)##fml2";
    strings["Compact (4x1)##fml3"].plurals[0] = "Compacto (4x1)##fml3";
    strings["Alternate (2x2)##fml4"].plurals[0] = "Alternativo (2x2)##fml4";
    strings["Alternate (1x4)##fml5"].plurals[0] = "Alternativo (1x4)##fml5";
    strings["Alternate (4x1)##fml6"].plurals[0] = "Alternativo (4x1)##fml6";
    strings["Position of Sustain in FM editor:##sgse"].plurals[0] = "Posição do Nível de Sustentação no editor de FM:";
    strings["Between Decay and Sustain Rate##susp0"].plurals[0] = "Entre Decaimento e Taxa de Sustentação##susp0";
    strings["After Release Rate##susp1"].plurals[0] = "Após Taxa de Repouso##susp1";
    strings["Use separate colors for carriers/modulators in FM editor##sgse"].plurals[0] = "Utilizar cores separadas para portadoras/moduladoras no editor FM";
    strings["Unsigned FM detune values##sgse"].plurals[0] = "Valores de desafinação FM sem sinal (unsigned)";
    strings["Statistics##sgse"].plurals[0] = "Estatísticas";
    strings["Chip memory usage unit:##sgse"].plurals[0] = "Unidade de uso de memória dos chips:";
    strings["Bytes##MUU0"].plurals[0] = "Bytes##MUU0";
    strings["Kilobytes##MUU1"].plurals[0] = "Kilobytes##MUU1";
    strings["Oscilloscope##set"].plurals[0] = "Osciloscópio##set";
    strings["Rounded corners##sgse"].plurals[0] = "Cantos arredondados";
    strings["Border##sgse"].plurals[0] = "Borda";
    strings["Mono##sgse1"].plurals[0] = "Mono";
    strings["Anti-aliased##sgse"].plurals[0] = "Suavizado";
    strings["Fill entire window##sgse"].plurals[0] = "Preencher toda a janela";
    strings["Waveform goes out of bounds##sgse"].plurals[0] = "Forma de onda ultrapassa as bordas";
    strings["Line size##sgse"].plurals[0] = "Espessura da linha";
    strings["Windows##sgse"].plurals[0] = "Janelas";
    strings["Rounded window corners##sgse"].plurals[0] = "Cantos das janelas arredondados";
    strings["Rounded buttons##sgse"].plurals[0] = "Botões arredondados";
    strings["Rounded tabs##sgse"].plurals[0] = "Abas arredondadas";
    strings["Rounded scrollbars##sgse"].plurals[0] = "Barras de rolagem arredondadas";
    strings["Rounded menu corners##sgse"].plurals[0] = "Cantos de menu arredondados";
    strings["Borders around widgets##sgse"].plurals[0] = "Bordas em volta dos widgets";
    strings["Misc##sgse"].plurals[0] = "Miscelânea";
    strings["Wrap text##sgse"].plurals[0] = "Quebrar texto";
    strings["Wrap text in song/subsong comments window.##sgse"].plurals[0] = "Quebrar automaticamente texto na janela de comentários da música/subfaixa.";
    strings["Frame shading in text windows##sgse"].plurals[0] = "Sombreamento do quadro nas janelas de texto";
    strings["Apply frame shading to the multiline text fields\nsuch as song/subsong info/comments.##sgse"].plurals[0] = "Aplicar sombreamento de quadro para os campos de texto multilinha\ncomo na janela de comentários da música/subfaixa.";
    strings["Show chip info in chip manager##sgse"].plurals[0] = "Mostrar informações do chip no gerenciador de chips";
    strings["Display tooltip in chip manager when hovering over the chip. Tooltip shows chip name and description.##sgse"].plurals[0] = "Mostra uma dica de contexto no gerenciador de chips ao passar o mouse sobre o chip. A dica de contexto mostra o nome do chip e a descrição.";
    strings["Color##sgse"].plurals[0] = "Cor";
    strings["Color scheme##sgse"].plurals[0] = "Esquema de cores";
    strings["Import##sgse2"].plurals[0] = "Importar";
    strings["Export##sgse2"].plurals[0] = "Exportar";
    strings["Reset defaults##sgse1"].plurals[0] = "Redefinir padrões";
    strings["Are you sure you want to reset the color scheme?##sgse"].plurals[0] = "Tem certeza de que quer redefinir o esquema de cores?";
    strings["Interface##sgse1"].plurals[0] = "Interface";
    strings["Frame shading##sgse"].plurals[0] = "Sombreamento de quadro";
    strings["Interface (other)##sgse"].plurals[0] = "Interface (outros)";
    strings["Miscellaneous##sgse"].plurals[0] = "Miscelânea";
    strings["File Picker (built-in)##sgse"].plurals[0] = "Diálogo de arquivos (embutido)";
    strings["Oscilloscope##sgse"].plurals[0] = "Osciloscópio";
    strings["Wave (non-mono)##sgse"].plurals[0] = "Onda (não-mono)";
    strings["Volume Meter##sgse"].plurals[0] = "Medidor de volume";
    strings["Orders##sgse2"].plurals[0] = "Ordens";
    strings["Envelope View##sgse"].plurals[0] = "Visualização de envelope";
    strings["FM Editor##sgse1"].plurals[0] = "Editor de FM";
    strings["Macro Editor##sgse1"].plurals[0] = "Editor de Macro";
    strings["Instrument Types##sgse"].plurals[0] = "Tipos de Instrumento";
    strings["Channel##sgse2"].plurals[0] = "Canal";
    strings["Pattern##sgse2"].plurals[0] = "Sequência";
    strings["Sample Editor##sgse"].plurals[0] = "Editor de Sample";
    strings["Pattern Manager##sgse"].plurals[0] = "Gerenciador de Sequência";
    strings["Piano##sgse"].plurals[0] = "Piano";
    strings["Clock##sgse"].plurals[0] = "Clock";
    strings["Patchbay##sgse"].plurals[0] = "Patchbay";
    strings["Memory Composition##sgse"].plurals[0] = "Composição da Memória##sgse";
    strings["Log Viewer##sgse"].plurals[0] = "Visualizador de logs";

    // these are messy, but the ##CC_GUI... is required.
    strings["Button##CC_GUI_COLOR_BUTTON"].plurals[0] = "Botão##CC_GUI_COLOR_BUTTON";
    strings["Button (hovered)##CC_GUI_COLOR_BUTTON_HOVER"].plurals[0] = "Botão (flutuando sobre)##CC_GUI_COLOR_BUTTON_HOVER";
    strings["Button (active)##CC_GUI_COLOR_BUTTON_ACTIVE"].plurals[0] = "Botão (ativado)##CC_GUI_COLOR_BUTTON_ACTIVE";
    strings["Tab##CC_GUI_COLOR_TAB"].plurals[0] = "Aba##CC_GUI_COLOR_TAB";
    strings["Tab (hovered)##CC_GUI_COLOR_TAB_HOVER"].plurals[0] = "Aba (flutuando sobre)##CC_GUI_COLOR_TAB_HOVER";
    strings["Tab (active)##CC_GUI_COLOR_TAB_ACTIVE"].plurals[0] = "Aba (ativada)##CC_GUI_COLOR_TAB_ACTIVE";
    strings["Tab (unfocused)##CC_GUI_COLOR_TAB_UNFOCUSED"].plurals[0] = "Aba (desfocada)##CC_GUI_COLOR_TAB_UNFOCUSED";
    strings["Tab (unfocused and active)##CC_GUI_COLOR_TAB_UNFOCUSED_ACTIVE"].plurals[0] = "Aba (desfocada e ativada)##CC_GUI_COLOR_TAB_UNFOCUSED_ACTIVE";
    strings["ImGui header##CC_GUI_COLOR_IMGUI_HEADER"].plurals[0] = "Cabeçalho ImGui##CC_GUI_COLOR_IMGUI_HEADER";
    strings["ImGui header (hovered)##CC_GUI_COLOR_IMGUI_HEADER_HOVER"].plurals[0] = "Cabeçalho ImGui (flutuando sobre)##CC_GUI_COLOR_IMGUI_HEADER_HOVER";
    strings["ImGui header (active)##CC_GUI_COLOR_IMGUI_HEADER_ACTIVE"].plurals[0] = "Cabeçalho ImGui (ativado)##CC_GUI_COLOR_IMGUI_HEADER_ACTIVE";
    strings["Resize grip##CC_GUI_COLOR_RESIZE_GRIP"].plurals[0] = "Aba de redimensionamento##CC_GUI_COLOR_RESIZE_GRIP";
    strings["Resize grip (hovered)##CC_GUI_COLOR_RESIZE_GRIP_HOVER"].plurals[0] = "Aba de redimensionamento (flutuando sobre)##CC_GUI_COLOR_RESIZE_GRIP_HOVER";
    strings["Resize grip (active)##CC_GUI_COLOR_RESIZE_GRIP_ACTIVE"].plurals[0] = "Aba de redimensionamento (ativada)##CC_GUI_COLOR_RESIZE_GRIP_ACTIVE";
    strings["Widget background##CC_GUI_COLOR_WIDGET_BACKGROUND"].plurals[0] = "Fundo de elemento##CC_GUI_COLOR_WIDGET_BACKGROUND";
    strings["Widget background (hovered)##CC_GUI_COLOR_WIDGET_BACKGROUND_HOVER"].plurals[0] = "Fundo de elemento (flutuando sobre)##CC_GUI_COLOR_WIDGET_BACKGROUND_HOVER";
    strings["Widget background (active)##CC_GUI_COLOR_WIDGET_BACKGROUND_ACTIVE"].plurals[0] = "Fundo de elemento (ativado)##CC_GUI_COLOR_WIDGET_BACKGROUND_ACTIVE";
    strings["Slider grab##CC_GUI_COLOR_SLIDER_GRAB"].plurals[0] = "Botão deslizante##CC_GUI_COLOR_SLIDER_GRAB";
    strings["Slider grab (active)##CC_GUI_COLOR_SLIDER_GRAB_ACTIVE"].plurals[0] = "Botão deslizante (ativado)##CC_GUI_COLOR_SLIDER_GRAB_ACTIVE";
    strings["Title background (active)##CC_GUI_COLOR_TITLE_BACKGROUND_ACTIVE"].plurals[0] = "Fundo de título (ativado)##CC_GUI_COLOR_TITLE_BACKGROUND_ACTIVE";
    strings["Checkbox/radio button mark##CC_GUI_COLOR_CHECK_MARK"].plurals[0] = "Marca de caixa de checagem/botão de rádio##CC_GUI_COLOR_CHECK_MARK";
    strings["Text selection##CC_GUI_COLOR_TEXT_SELECTION"].plurals[0] = "Seleção de texto##CC_GUI_COLOR_TEXT_SELECTION";
    strings["Line plot##CC_GUI_COLOR_PLOT_LINES"].plurals[0] = "Traçado de linha##CC_GUI_COLOR_PLOT_LINES";
    strings["Line plot (hovered)##CC_GUI_COLOR_PLOT_LINES_HOVER"].plurals[0] = "Traçado de linha (flutuando sobre)##CC_GUI_COLOR_PLOT_LINES_HOVER";
    strings["Histogram plot##CC_GUI_COLOR_PLOT_HISTOGRAM"].plurals[0] = "Desenho de histograma##CC_GUI_COLOR_PLOT_HISTOGRAM";
    strings["Histogram plot (hovered)##CC_GUI_COLOR_PLOT_HISTOGRAM_HOVER"].plurals[0] = "Desenho de histograma (flutuando sobre)##CC_GUI_COLOR_PLOT_HISTOGRAM_HOVER";
    strings["Table row (even)##CC_GUI_COLOR_TABLE_ROW_EVEN"].plurals[0] = "Linha de tabela (par)##CC_GUI_COLOR_TABLE_ROW_EVEN";
    strings["Table row (odd)##CC_GUI_COLOR_TABLE_ROW_ODD"].plurals[0] = "Linha de tabela (ímpar)##CC_GUI_COLOR_TABLE_ROW_ODD";

    strings["Background##CC_GUI_COLOR_BACKGROUND"].plurals[0] = "Fundo##CC_GUI_COLOR_BACKGROUND";
    strings["Window background##CC_GUI_COLOR_FRAME_BACKGROUND"].plurals[0] = "Fundo de janela##CC_GUI_COLOR_FRAME_BACKGROUND";
    strings["Sub-window background##CC_GUI_COLOR_FRAME_BACKGROUND_CHILD"].plurals[0] = "Fundo de sub-janela##CC_GUI_COLOR_FRAME_BACKGROUND_CHILD";
    strings["Pop-up background##CC_GUI_COLOR_FRAME_BACKGROUND_POPUP"].plurals[0] = "Fundo de diálogo pop-up##CC_GUI_COLOR_FRAME_BACKGROUND_POPUP";
    strings["Modal backdrop##CC_GUI_COLOR_MODAL_BACKDROP"].plurals[0] = "Fundo de diálogo modal##CC_GUI_COLOR_MODAL_BACKDROP";
    strings["Header##CC_GUI_COLOR_HEADER"].plurals[0] = "Cabeçalho##CC_GUI_COLOR_HEADER";
    strings["Text##CC_GUI_COLOR_TEXT"].plurals[0] = "Texto##CC_GUI_COLOR_TEXT";
    strings["Text (disabled)##CC_GUI_COLOR_TEXT_DISABLED"].plurals[0] = "Texto (desabilitado)##CC_GUI_COLOR_TEXT_DISABLED";
    strings["Title bar (inactive)##CC_GUI_COLOR_TITLE_INACTIVE"].plurals[0] = "Barra de título (inativa)##CC_GUI_COLOR_TITLE_INACTIVE";
    strings["Title bar (collapsed)##CC_GUI_COLOR_TITLE_COLLAPSED"].plurals[0] = "Barra de título (contraída)##CC_GUI_COLOR_TITLE_COLLAPSED";
    strings["Menu bar##CC_GUI_COLOR_MENU_BAR"].plurals[0] = "Barra de menu##CC_GUI_COLOR_MENU_BAR";
    strings["Border##CC_GUI_COLOR_BORDER"].plurals[0] = "Borda##CC_GUI_COLOR_BORDER";
    strings["Border shadow##CC_GUI_COLOR_BORDER_SHADOW"].plurals[0] = "Sombra de borda##CC_GUI_COLOR_BORDER_SHADOW";
    strings["Scroll bar##CC_GUI_COLOR_SCROLL"].plurals[0] = "Barra de rolagem##CC_GUI_COLOR_SCROLL";
    strings["Scroll bar (hovered)##CC_GUI_COLOR_SCROLL_HOVER"].plurals[0] = "Barra de rolagem (flutuando sobre)##CC_GUI_COLOR_SCROLL_HOVER";
    strings["Scroll bar (clicked)##CC_GUI_COLOR_SCROLL_ACTIVE"].plurals[0] = "Barra de rolagem (clicada)##CC_GUI_COLOR_SCROLL_ACTIVE";
    strings["Scroll bar background##CC_GUI_COLOR_SCROLL_BACKGROUND"].plurals[0] = "Fundo da barra de rolagem##CC_GUI_COLOR_SCROLL_BACKGROUND";
    strings["Separator##CC_GUI_COLOR_SEPARATOR"].plurals[0] = "Separador##CC_GUI_COLOR_SEPARATOR";
    strings["Separator (hover)##CC_GUI_COLOR_SEPARATOR_HOVER"].plurals[0] = "Separador (flutuando sobre)##CC_GUI_COLOR_SEPARATOR_HOVER";
    strings["Separator (active)##CC_GUI_COLOR_SEPARATOR_ACTIVE"].plurals[0] = "Separador (ativado)##CC_GUI_COLOR_SEPARATOR_ACTIVE";
    strings["Docking preview##CC_GUI_COLOR_DOCKING_PREVIEW"].plurals[0] = "Previsão de ancoragem##CC_GUI_COLOR_DOCKING_PREVIEW";
    strings["Docking empty##CC_GUI_COLOR_DOCKING_EMPTY"].plurals[0] = "Ancoragem vazia##CC_GUI_COLOR_DOCKING_EMPTY";
    strings["Table header##CC_GUI_COLOR_TABLE_HEADER"].plurals[0] = "Cabeçalho de tabela##CC_GUI_COLOR_TABLE_HEADER";
    strings["Table border (hard)##CC_GUI_COLOR_TABLE_BORDER_HARD"].plurals[0] = "Borda de tabela (dura)##CC_GUI_COLOR_TABLE_BORDER_HARD";
    strings["Table border (soft)##CC_GUI_COLOR_TABLE_BORDER_SOFT"].plurals[0] = "Borda de tabela (suave)##CC_GUI_COLOR_TABLE_BORDER_SOFT";
    strings["Drag and drop target##CC_GUI_COLOR_DRAG_DROP_TARGET"].plurals[0] = "Alvo de arrastar e soltar##CC_GUI_COLOR_DRAG_DROP_TARGET";
    strings["Window switcher (highlight)##CC_GUI_COLOR_NAV_WIN_HIGHLIGHT"].plurals[0] = "Seletor de janelas (realce)##CC_GUI_COLOR_NAV_WIN_HIGHLIGHT";
    strings["Window switcher backdrop##CC_GUI_COLOR_NAV_WIN_BACKDROP"].plurals[0] = "Fundo do seletor de janelas##CC_GUI_COLOR_NAV_WIN_BACKDROP";

    strings["Toggle on##CC_GUI_COLOR_TOGGLE_ON"].plurals[0] = "Alternativa ligada##CC_GUI_COLOR_TOGGLE_ON";
    strings["Toggle off##CC_GUI_COLOR_TOGGLE_OFF"].plurals[0] = "Alternativa desligada##CC_GUI_COLOR_TOGGLE_OFF";
    strings["Playback status##CC_GUI_COLOR_PLAYBACK_STAT"].plurals[0] = "Estado de reprodução##CC_GUI_COLOR_PLAYBACK_STAT";
    strings["Destructive hint##CC_GUI_COLOR_DESTRUCTIVE"].plurals[0] = "Destaque destrutivo##CC_GUI_COLOR_DESTRUCTIVE";
    strings["Warning hint##CC_GUI_COLOR_WARNING"].plurals[0] = "Destaque de aviso##CC_GUI_COLOR_WARNING";
    strings["Error hint##CC_GUI_COLOR_ERROR"].plurals[0] = "Destaque de erro##CC_GUI_COLOR_ERROR";

    strings["Directory##CC_GUI_COLOR_FILE_DIR"].plurals[0] = "Diretório##CC_GUI_COLOR_FILE_DIR";
    strings["Song (native)##CC_GUI_COLOR_FILE_SONG_NATIVE"].plurals[0] = "Música (nativa)##CC_GUI_COLOR_FILE_SONG_NATIVE";
    strings["Song (import)##CC_GUI_COLOR_FILE_SONG_IMPORT"].plurals[0] = "Música (importada)##CC_GUI_COLOR_FILE_SONG_IMPORT";
    strings["Instrument##CC_GUI_COLOR_FILE_INSTR"].plurals[0] = "Instrumento##CC_GUI_COLOR_FILE_INSTR";
    strings["Audio##CC_GUI_COLOR_FILE_AUDIO"].plurals[0] = "Áudio##CC_GUI_COLOR_FILE_AUDIO";
    strings["Wavetable##CC_GUI_COLOR_FILE_WAVE"].plurals[0] = "Wavetable##CC_GUI_COLOR_FILE_WAVE";
    strings["VGM##CC_GUI_COLOR_FILE_VGM"].plurals[0] = "VGM##CC_GUI_COLOR_FILE_VGM";
    strings["ZSM##CC_GUI_COLOR_FILE_ZSM"].plurals[0] = "ZSM##CC_GUI_COLOR_FILE_ZSM";
    strings["Font##CC_GUI_COLOR_FILE_FONT"].plurals[0] = "Fonte##CC_GUI_COLOR_FILE_FONT";
    strings["Other##CC_GUI_COLOR_FILE_OTHER"].plurals[0] = "Outro##CC_GUI_COLOR_FILE_OTHER";

    strings["Border##CC_GUI_COLOR_OSC_BORDER"].plurals[0] = "Borda##CC_GUI_COLOR_OSC_BORDER";
    strings["Background (top-left)##CC_GUI_COLOR_OSC_BG1"].plurals[0] = "Fundo (superior esquerdo)##CC_GUI_COLOR_OSC_BG1";
    strings["Background (top-right)##CC_GUI_COLOR_OSC_BG2"].plurals[0] = "Fundo (superior direito)##CC_GUI_COLOR_OSC_BG2";
    strings["Background (bottom-left)##CC_GUI_COLOR_OSC_BG3"].plurals[0] = "Fundo (inferior esquerdo)##CC_GUI_COLOR_OSC_BG3";
    strings["Background (bottom-right)##CC_GUI_COLOR_OSC_BG4"].plurals[0] = "Fundo (inferior direito)##CC_GUI_COLOR_OSC_BG4";
    strings["Waveform##CC_GUI_COLOR_OSC_WAVE"].plurals[0] = "Forma de onda##CC_GUI_COLOR_OSC_WAVE";
    strings["Waveform (clip)##CC_GUI_COLOR_OSC_WAVE_PEAK"].plurals[0] = "Forma de onda (corte/clip)##CC_GUI_COLOR_OSC_WAVE_PEAK";
    strings["Reference##CC_GUI_COLOR_OSC_REF"].plurals[0] = "Referência##CC_GUI_COLOR_OSC_REF";
    strings["Guide##CC_GUI_COLOR_OSC_GUIDE"].plurals[0] = "Guia##CC_GUI_COLOR_OSC_GUIDE";

    strings["Waveform (1)##CC_GUI_COLOR_OSC_WAVE_CH0"].plurals[0] = "Forma de onda (1)##CC_GUI_COLOR_OSC_WAVE_CH0";
    strings["Waveform (2)##CC_GUI_COLOR_OSC_WAVE_CH1"].plurals[0] = "Forma de onda (2)##CC_GUI_COLOR_OSC_WAVE_CH1";
    strings["Waveform (3)##CC_GUI_COLOR_OSC_WAVE_CH2"].plurals[0] = "Forma de onda (3)##CC_GUI_COLOR_OSC_WAVE_CH2";
    strings["Waveform (4)##CC_GUI_COLOR_OSC_WAVE_CH3"].plurals[0] = "Forma de onda (4)##CC_GUI_COLOR_OSC_WAVE_CH3";
    strings["Waveform (5)##CC_GUI_COLOR_OSC_WAVE_CH4"].plurals[0] = "Forma de onda (5)##CC_GUI_COLOR_OSC_WAVE_CH4";
    strings["Waveform (6)##CC_GUI_COLOR_OSC_WAVE_CH5"].plurals[0] = "Forma de onda (6)##CC_GUI_COLOR_OSC_WAVE_CH5";
    strings["Waveform (7)##CC_GUI_COLOR_OSC_WAVE_CH6"].plurals[0] = "Forma de onda (7)##CC_GUI_COLOR_OSC_WAVE_CH6";
    strings["Waveform (8)##CC_GUI_COLOR_OSC_WAVE_CH7"].plurals[0] = "Forma de onda (8)##CC_GUI_COLOR_OSC_WAVE_CH7";
    strings["Waveform (9)##CC_GUI_COLOR_OSC_WAVE_CH8"].plurals[0] = "Forma de onda (9)##CC_GUI_COLOR_OSC_WAVE_CH8";
    strings["Waveform (10)##CC_GUI_COLOR_OSC_WAVE_CH9"].plurals[0] = "Forma de onda (10)##CC_GUI_COLOR_OSC_WAVE_CH9";
    strings["Waveform (11)##CC_GUI_COLOR_OSC_WAVE_CH10"].plurals[0] = "Forma de onda (11)##CC_GUI_COLOR_OSC_WAVE_CH10";
    strings["Waveform (12)##CC_GUI_COLOR_OSC_WAVE_CH11"].plurals[0] = "Forma de onda (12)##CC_GUI_COLOR_OSC_WAVE_CH11";
    strings["Waveform (13)##CC_GUI_COLOR_OSC_WAVE_CH12"].plurals[0] = "Forma de onda (13)##CC_GUI_COLOR_OSC_WAVE_CH12";
    strings["Waveform (14)##CC_GUI_COLOR_OSC_WAVE_CH13"].plurals[0] = "Forma de onda (14)##CC_GUI_COLOR_OSC_WAVE_CH13";
    strings["Waveform (15)##CC_GUI_COLOR_OSC_WAVE_CH14"].plurals[0] = "Forma de onda (15)##CC_GUI_COLOR_OSC_WAVE_CH14";
    strings["Waveform (16)##CC_GUI_COLOR_OSC_WAVE_CH15"].plurals[0] = "Forma de onda (16)##CC_GUI_COLOR_OSC_WAVE_CH15";

    strings["Low##CC_GUI_COLOR_VOLMETER_LOW"].plurals[0] = "Baixo##CC_GUI_COLOR_VOLMETER_LOW";
    strings["High##CC_GUI_COLOR_VOLMETER_HIGH"].plurals[0] = "Alto##CC_GUI_COLOR_VOLMETER_HIGH";
    strings["Clip##CC_GUI_COLOR_VOLMETER_PEAK"].plurals[0] = "Corte (clip)##CC_GUI_COLOR_VOLMETER_PEAK";

    strings["Order number##CC_GUI_COLOR_ORDER_ROW_INDEX"].plurals[0] = "Número da ordem##CC_GUI_COLOR_ORDER_ROW_INDEX";
    strings["Playing order background##CC_GUI_COLOR_ORDER_ACTIVE"].plurals[0] = "Fundo da ordem sendo reproduzida##CC_GUI_COLOR_ORDER_ACTIVE";
    strings["Song loop##CC_GUI_COLOR_SONG_LOOP"].plurals[0] = "Loop da música##CC_GUI_COLOR_SONG_LOOP";
    strings["Selected order##CC_GUI_COLOR_ORDER_SELECTED"].plurals[0] = "Ordem selecionada##CC_GUI_COLOR_ORDER_SELECTED";
    strings["Similar patterns##CC_GUI_COLOR_ORDER_SIMILAR"].plurals[0] = "Sequências similares##CC_GUI_COLOR_ORDER_SIMILAR";
    strings["Inactive patterns##CC_GUI_COLOR_ORDER_INACTIVE"].plurals[0] = "Sequências inativas##CC_GUI_COLOR_ORDER_INACTIVE";

    strings["Envelope##CC_GUI_COLOR_FM_ENVELOPE"].plurals[0] = "Envelope##CC_GUI_COLOR_FM_ENVELOPE";
    strings["Sustain guide##CC_GUI_COLOR_FM_ENVELOPE_SUS_GUIDE"].plurals[0] = "Guia de sustentação##CC_GUI_COLOR_FM_ENVELOPE_SUS_GUIDE";
    strings["Release##CC_GUI_COLOR_FM_ENVELOPE_RELEASE"].plurals[0] = "Repouso##CC_GUI_COLOR_FM_ENVELOPE_RELEASE";

    strings["Algorithm background##CC_GUI_COLOR_FM_ALG_BG"].plurals[0] = "Fundo do algoritmo##CC_GUI_COLOR_FM_ALG_BG";
    strings["Algorithm lines##CC_GUI_COLOR_FM_ALG_LINE"].plurals[0] = "Linhas do algoritmo##CC_GUI_COLOR_FM_ALG_LINE";
    strings["Modulator##CC_GUI_COLOR_FM_MOD"].plurals[0] = "Moduladora##CC_GUI_COLOR_FM_MOD";
    strings["Carrier##CC_GUI_COLOR_FM_CAR"].plurals[0] = "Portadora##CC_GUI_COLOR_FM_CAR";

    strings["SSG-EG##CC_GUI_COLOR_FM_SSG"].plurals[0] = "SSG-EG##CC_GUI_COLOR_FM_SSG";
    strings["Waveform##CC_GUI_COLOR_FM_WAVE"].plurals[0] = "Forma de onda##CC_GUI_COLOR_FM_WAVE";

    strings["Mod. accent (primary)##CC_GUI_COLOR_FM_PRIMARY_MOD"].plurals[0] = "Realce mod. (primário)##CC_GUI_COLOR_FM_PRIMARY_MOD";
    strings["Mod. accent (secondary)##CC_GUI_COLOR_FM_SECONDARY_MOD"].plurals[0] = "Realce mod. (secundário)##CC_GUI_COLOR_FM_SECONDARY_MOD";
    strings["Mod. border##CC_GUI_COLOR_FM_BORDER_MOD"].plurals[0] = "Borda mod.##CC_GUI_COLOR_FM_BORDER_MOD";
    strings["Mod. border shadow##CC_GUI_COLOR_FM_BORDER_SHADOW_MOD"].plurals[0] = "Sombra borda mod.##CC_GUI_COLOR_FM_BORDER_SHADOW_MOD";

    strings["Car. accent (primary)##CC_GUI_COLOR_FM_PRIMARY_CAR"].plurals[0] = "Realce port. (primário)##CC_GUI_COLOR_FM_PRIMARY_CAR";
    strings["Car. accent (secondary)##CC_GUI_COLOR_FM_SECONDARY_CAR"].plurals[0] = "Realce port. (secundário)##CC_GUI_COLOR_FM_SECONDARY_CAR";
    strings["Car. border##CC_GUI_COLOR_FM_BORDER_CAR"].plurals[0] = "Borda port.##CC_GUI_COLOR_FM_BORDER_CAR";
    strings["Car. border shadow##CC_GUI_COLOR_FM_BORDER_SHADOW_CAR"].plurals[0] = "Sombra borda port.##CC_GUI_COLOR_FM_BORDER_SHADOW_CAR";

    strings["Volume##CC_GUI_COLOR_MACRO_VOLUME"].plurals[0] = "Volume##CC_GUI_COLOR_MACRO_VOLUME";
    strings["Pitch##CC_GUI_COLOR_MACRO_PITCH"].plurals[0] = "Tom##CC_GUI_COLOR_MACRO_PITCH";
    strings["Wave##CC_GUI_COLOR_MACRO_WAVE"].plurals[0] = "Onda##CC_GUI_COLOR_MACRO_WAVE";
    strings["Other##CC_GUI_COLOR_MACRO_OTHER"].plurals[0] = "Outro##CC_GUI_COLOR_MACRO_OTHER";

    strings["FM (OPN)##CC_GUI_COLOR_INSTR_FM"].plurals[0] = "FM (OPN)##CC_GUI_COLOR_INSTR_FM";
    strings["SN76489/Sega PSG##CC_GUI_COLOR_INSTR_STD"].plurals[0] = "SN76489/Sega PSG##CC_GUI_COLOR_INSTR_STD";
    strings["T6W28##CC_GUI_COLOR_INSTR_T6W28"].plurals[0] = "T6W28##CC_GUI_COLOR_INSTR_T6W28";
    strings["Game Boy##CC_GUI_COLOR_INSTR_GB"].plurals[0] = "Game Boy##CC_GUI_COLOR_INSTR_GB";
    strings["C64##CC_GUI_COLOR_INSTR_C64"].plurals[0] = "C64##CC_GUI_COLOR_INSTR_C64";
    strings["Amiga/Generic Sample##CC_GUI_COLOR_INSTR_AMIGA"].plurals[0] = "Amiga/Sample Genérica##CC_GUI_COLOR_INSTR_AMIGA";
    strings["PC Engine##CC_GUI_COLOR_INSTR_PCE"].plurals[0] = "PC Engine##CC_GUI_COLOR_INSTR_PCE";
    strings["AY-3-8910/SSG##CC_GUI_COLOR_INSTR_AY"].plurals[0] = "AY-3-8910/SSG##CC_GUI_COLOR_INSTR_AY";
    strings["AY8930##CC_GUI_COLOR_INSTR_AY8930"].plurals[0] = "AY8930##CC_GUI_COLOR_INSTR_AY8930";
    strings["TIA##CC_GUI_COLOR_INSTR_TIA"].plurals[0] = "TIA##CC_GUI_COLOR_INSTR_TIA";
    strings["SAA1099##CC_GUI_COLOR_INSTR_SAA1099"].plurals[0] = "SAA1099##CC_GUI_COLOR_INSTR_SAA1099";
    strings["VIC##CC_GUI_COLOR_INSTR_VIC"].plurals[0] = "VIC##CC_GUI_COLOR_INSTR_VIC";
    strings["PET##CC_GUI_COLOR_INSTR_PET"].plurals[0] = "PET##CC_GUI_COLOR_INSTR_PET";
    strings["VRC6##CC_GUI_COLOR_INSTR_VRC6"].plurals[0] = "VRC6##CC_GUI_COLOR_INSTR_VRC6";
    strings["VRC6 (saw)##CC_GUI_COLOR_INSTR_VRC6_SAW"].plurals[0] = "VRC6 (serra)##CC_GUI_COLOR_INSTR_VRC6_SAW";
    strings["FM (OPLL)##CC_GUI_COLOR_INSTR_OPLL"].plurals[0] = "FM (OPLL)##CC_GUI_COLOR_INSTR_OPLL";
    strings["FM (OPL)##CC_GUI_COLOR_INSTR_OPL"].plurals[0] = "FM (OPL)##CC_GUI_COLOR_INSTR_OPL";
    strings["FDS##CC_GUI_COLOR_INSTR_FDS"].plurals[0] = "FDS##CC_GUI_COLOR_INSTR_FDS";
    strings["Virtual Boy##CC_GUI_COLOR_INSTR_VBOY"].plurals[0] = "Virtual Boy##CC_GUI_COLOR_INSTR_VBOY";
    strings["Namco 163##CC_GUI_COLOR_INSTR_N163"].plurals[0] = "Namco 163##CC_GUI_COLOR_INSTR_N163";
    strings["Konami SCC##CC_GUI_COLOR_INSTR_SCC"].plurals[0] = "Konami SCC##CC_GUI_COLOR_INSTR_SCC";
    strings["FM (OPZ)##CC_GUI_COLOR_INSTR_OPZ"].plurals[0] = "FM (OPZ)##CC_GUI_COLOR_INSTR_OPZ";
    strings["POKEY##CC_GUI_COLOR_INSTR_POKEY"].plurals[0] = "POKEY##CC_GUI_COLOR_INSTR_POKEY";
    strings["PC Beeper##CC_GUI_COLOR_INSTR_BEEPER"].plurals[0] = "Beeper PC##CC_GUI_COLOR_INSTR_BEEPER";
    strings["WonderSwan##CC_GUI_COLOR_INSTR_SWAN"].plurals[0] = "WonderSwan##CC_GUI_COLOR_INSTR_SWAN";
    strings["Lynx##CC_GUI_COLOR_INSTR_MIKEY"].plurals[0] = "Lynx##CC_GUI_COLOR_INSTR_MIKEY";
    strings["VERA##CC_GUI_COLOR_INSTR_VERA"].plurals[0] = "VERA##CC_GUI_COLOR_INSTR_VERA";
    strings["X1-010##CC_GUI_COLOR_INSTR_X1_010"].plurals[0] = "X1-010##CC_GUI_COLOR_INSTR_X1_010";
    strings["ES5506##CC_GUI_COLOR_INSTR_ES5506"].plurals[0] = "ES5506##CC_GUI_COLOR_INSTR_ES5506";
    strings["MultiPCM##CC_GUI_COLOR_INSTR_MULTIPCM"].plurals[0] = "MultiPCM##CC_GUI_COLOR_INSTR_MULTIPCM";
    strings["SNES##CC_GUI_COLOR_INSTR_SNES"].plurals[0] = "SNES##CC_GUI_COLOR_INSTR_SNES";
    strings["Sound Unit##CC_GUI_COLOR_INSTR_SU"].plurals[0] = "Sound Unit##CC_GUI_COLOR_INSTR_SU";
    strings["Namco WSG##CC_GUI_COLOR_INSTR_NAMCO"].plurals[0] = "Namco WSG##CC_GUI_COLOR_INSTR_NAMCO";
    strings["FM (OPL Drums)##CC_GUI_COLOR_INSTR_OPL_DRUMS"].plurals[0] = "FM (OPL Bateria)##CC_GUI_COLOR_INSTR_OPL_DRUMS";
    strings["FM (OPM)##CC_GUI_COLOR_INSTR_OPM"].plurals[0] = "FM (OPM)##CC_GUI_COLOR_INSTR_OPM";
    strings["NES##CC_GUI_COLOR_INSTR_NES"].plurals[0] = "NES##CC_GUI_COLOR_INSTR_NES";
    strings["MSM6258##CC_GUI_COLOR_INSTR_MSM6258"].plurals[0] = "MSM6258##CC_GUI_COLOR_INSTR_MSM6258";
    strings["MSM6295##CC_GUI_COLOR_INSTR_MSM6295"].plurals[0] = "MSM6295##CC_GUI_COLOR_INSTR_MSM6295";
    strings["ADPCM-A##CC_GUI_COLOR_INSTR_ADPCMA"].plurals[0] = "ADPCM-A##CC_GUI_COLOR_INSTR_ADPCMA";
    strings["ADPCM-B##CC_GUI_COLOR_INSTR_ADPCMB"].plurals[0] = "ADPCM-B##CC_GUI_COLOR_INSTR_ADPCMB";
    strings["Sega PCM##CC_GUI_COLOR_INSTR_SEGAPCM"].plurals[0] = "Sega PCM##CC_GUI_COLOR_INSTR_SEGAPCM";
    strings["QSound##CC_GUI_COLOR_INSTR_QSOUND"].plurals[0] = "QSound##CC_GUI_COLOR_INSTR_QSOUND";
    strings["YMZ280B##CC_GUI_COLOR_INSTR_YMZ280B"].plurals[0] = "YMZ280B##CC_GUI_COLOR_INSTR_YMZ280B";
    strings["RF5C68##CC_GUI_COLOR_INSTR_RF5C68"].plurals[0] = "RF5C68##CC_GUI_COLOR_INSTR_RF5C68";
    strings["MSM5232##CC_GUI_COLOR_INSTR_MSM5232"].plurals[0] = "MSM5232##CC_GUI_COLOR_INSTR_MSM5232";
    strings["K007232##CC_GUI_COLOR_INSTR_K007232"].plurals[0] = "K007232##CC_GUI_COLOR_INSTR_K007232";
    strings["GA20##CC_GUI_COLOR_INSTR_GA20"].plurals[0] = "GA20##CC_GUI_COLOR_INSTR_GA20";
    strings["Pokémon Mini##CC_GUI_COLOR_INSTR_POKEMINI"].plurals[0] = "Pokémon Mini##CC_GUI_COLOR_INSTR_POKEMINI";
    strings["SM8521##CC_GUI_COLOR_INSTR_SM8521"].plurals[0] = "SM8521##CC_GUI_COLOR_INSTR_SM8521";
    strings["PV-1000##CC_GUI_COLOR_INSTR_PV1000"].plurals[0] = "PV-1000##CC_GUI_COLOR_INSTR_PV1000";
    strings["K053260##CC_GUI_COLOR_INSTR_K053260"].plurals[0] = "K053260##CC_GUI_COLOR_INSTR_K053260";
    strings["C140##CC_GUI_COLOR_INSTR_C140"].plurals[0] = "C140##CC_GUI_COLOR_INSTR_C140";
    strings["C219##CC_GUI_COLOR_INSTR_C219"].plurals[0] = "C219##CC_GUI_COLOR_INSTR_C219";
    strings["ESFM##CC_GUI_COLOR_INSTR_ESFM"].plurals[0] = "ESFM##CC_GUI_COLOR_INSTR_ESFM";
    strings["ES5503##CC_GUI_COLOR_INSTR_ES5503"].plurals[0] = "ES5503##CC_GUI_COLOR_INSTR_ES5503";
    strings["PowerNoise (noise)##CC_GUI_COLOR_INSTR_POWERNOISE"].plurals[0] = "PowerNoise (ruído)##CC_GUI_COLOR_INSTR_POWERNOISE";
    strings["PowerNoise (slope)##CC_GUI_COLOR_INSTR_POWERNOISE_SLOPE"].plurals[0] = "PowerNoise (rampa)##CC_GUI_COLOR_INSTR_POWERNOISE_SLOPE";
    strings["Other/Unknown##CC_GUI_COLOR_INSTR_UNKNOWN"].plurals[0] = "Outro/Desconhecido##CC_GUI_COLOR_INSTR_UNKNOWN";

    strings["Single color (background)##CC_GUI_COLOR_CHANNEL_BG"].plurals[0] = "Cor única (fundo)##CC_GUI_COLOR_CHANNEL_BG";
    strings["Single color (text)##CC_GUI_COLOR_CHANNEL_FG"].plurals[0] = "Cor única (texto)##CC_GUI_COLOR_CHANNEL_FG";
    strings["FM##CC_GUI_COLOR_CHANNEL_FM"].plurals[0] = "FM##CC_GUI_COLOR_CHANNEL_FM";
    strings["Pulse##CC_GUI_COLOR_CHANNEL_PULSE"].plurals[0] = "Pulsada##CC_GUI_COLOR_CHANNEL_PULSE";
    strings["Noise##CC_GUI_COLOR_CHANNEL_NOISE"].plurals[0] = "Ruído##CC_GUI_COLOR_CHANNEL_NOISE";
    strings["PCM##CC_GUI_COLOR_CHANNEL_PCM"].plurals[0] = "PCM##CC_GUI_COLOR_CHANNEL_PCM";
    strings["Wave##CC_GUI_COLOR_CHANNEL_WAVE"].plurals[0] = "Onda##CC_GUI_COLOR_CHANNEL_WAVE";
    strings["FM operator##CC_GUI_COLOR_CHANNEL_OP"].plurals[0] = "Operador FM##CC_GUI_COLOR_CHANNEL_OP";
    strings["Muted##CC_GUI_COLOR_CHANNEL_MUTED"].plurals[0] = "Mudo##CC_GUI_COLOR_CHANNEL_MUTED";

    strings["Playhead##CC_GUI_COLOR_PATTERN_PLAY_HEAD"].plurals[0] = "Cabeçote de reprodução##CC_GUI_COLOR_PATTERN_PLAY_HEAD";
    strings["Editing##CC_GUI_COLOR_EDITING"].plurals[0] = "Edição##CC_GUI_COLOR_EDITING";
    strings["Editing (will clone)##CC_GUI_COLOR_EDITING_CLONE"].plurals[0] = "Edição (modo de clone)##CC_GUI_COLOR_EDITING_CLONE";
    strings["Cursor##CC_GUI_COLOR_PATTERN_CURSOR"].plurals[0] = "Cursor##CC_GUI_COLOR_PATTERN_CURSOR";
    strings["Cursor (hovered)##CC_GUI_COLOR_PATTERN_CURSOR_HOVER"].plurals[0] = "Cursor (flutuando sobre)##CC_GUI_COLOR_PATTERN_CURSOR_HOVER";
    strings["Cursor (clicked)##CC_GUI_COLOR_PATTERN_CURSOR_ACTIVE"].plurals[0] = "Cursor (clicado)##CC_GUI_COLOR_PATTERN_CURSOR_ACTIVE";
    strings["Selection##CC_GUI_COLOR_PATTERN_SELECTION"].plurals[0] = "Seleção##CC_GUI_COLOR_PATTERN_SELECTION"; // é tetra! ^^
    strings["Selection (hovered)##CC_GUI_COLOR_PATTERN_SELECTION_HOVER"].plurals[0] = "Seleção (flutuando sobre)##CC_GUI_COLOR_PATTERN_SELECTION_HOVER";
    strings["Selection (clicked)##CC_GUI_COLOR_PATTERN_SELECTION_ACTIVE"].plurals[0] = "Seleção (clicada)##CC_GUI_COLOR_PATTERN_SELECTION_ACTIVE";
    strings["Highlight 1##CC_GUI_COLOR_PATTERN_HI_1"].plurals[0] = "Destaque 1##CC_GUI_COLOR_PATTERN_HI_1";
    strings["Highlight 2##CC_GUI_COLOR_PATTERN_HI_2"].plurals[0] = "Destaque 2##CC_GUI_COLOR_PATTERN_HI_2";
    strings["Row number##CC_GUI_COLOR_PATTERN_ROW_INDEX"].plurals[0] = "Número da linha##CC_GUI_COLOR_PATTERN_ROW_INDEX";
    strings["Row number (highlight 1)##CC_GUI_COLOR_PATTERN_ROW_INDEX_HI1"].plurals[0] = "Número da linha (destaque 1)##CC_GUI_COLOR_PATTERN_ROW_INDEX_HI1";
    strings["Row number (highlight 2)##CC_GUI_COLOR_PATTERN_ROW_INDEX_HI2"].plurals[0] = "Número da linha (destaque 2)##CC_GUI_COLOR_PATTERN_ROW_INDEX_HI2";
    strings["Note##CC_GUI_COLOR_PATTERN_ACTIVE"].plurals[0] = "Nota##CC_GUI_COLOR_PATTERN_ACTIVE";
    strings["Note (highlight 1)##CC_GUI_COLOR_PATTERN_ACTIVE_HI1"].plurals[0] = "Nota (destaque 1)##CC_GUI_COLOR_PATTERN_ACTIVE_HI1";
    strings["Note (highlight 2)##CC_GUI_COLOR_PATTERN_ACTIVE_HI2"].plurals[0] = "Nota (destaque 2)##CC_GUI_COLOR_PATTERN_ACTIVE_HI2";
    strings["Blank##CC_GUI_COLOR_PATTERN_INACTIVE"].plurals[0] = "Em branco##CC_GUI_COLOR_PATTERN_INACTIVE";
    strings["Blank (highlight 1)##CC_GUI_COLOR_PATTERN_INACTIVE_HI1"].plurals[0] = "Em branco (destaque 1)##CC_GUI_COLOR_PATTERN_INACTIVE_HI1";
    strings["Blank (highlight 2)##CC_GUI_COLOR_PATTERN_INACTIVE_HI2"].plurals[0] = "Em branco (destaque 2)##CC_GUI_COLOR_PATTERN_INACTIVE_HI2";
    strings["Instrument##CC_GUI_COLOR_PATTERN_INS"].plurals[0] = "Instrumento##CC_GUI_COLOR_PATTERN_INS";
    strings["Instrument (invalid type)##CC_GUI_COLOR_PATTERN_INS_WARN"].plurals[0] = "Instrumento (tipo inválido)##CC_GUI_COLOR_PATTERN_INS_WARN";
    strings["Instrument (out of range)##CC_GUI_COLOR_PATTERN_INS_ERROR"].plurals[0] = "Instrumento (fora de faixa)##CC_GUI_COLOR_PATTERN_INS_ERROR";
    strings["Volume (0%)##CC_GUI_COLOR_PATTERN_VOLUME_MIN"].plurals[0] = "Volume (0%)##CC_GUI_COLOR_PATTERN_VOLUME_MIN";
    strings["Volume (50%)##CC_GUI_COLOR_PATTERN_VOLUME_HALF"].plurals[0] = "Volume (50%)##CC_GUI_COLOR_PATTERN_VOLUME_HALF";
    strings["Volume (100%)##CC_GUI_COLOR_PATTERN_VOLUME_MAX"].plurals[0] = "Volume (100%)##CC_GUI_COLOR_PATTERN_VOLUME_MAX";
    strings["Invalid effect##CC_GUI_COLOR_PATTERN_EFFECT_INVALID"].plurals[0] = "Efeito inválido##CC_GUI_COLOR_PATTERN_EFFECT_INVALID";
    strings["Pitch effect##CC_GUI_COLOR_PATTERN_EFFECT_PITCH"].plurals[0] = "Efeito de tom##CC_GUI_COLOR_PATTERN_EFFECT_PITCH";
    strings["Volume effect##CC_GUI_COLOR_PATTERN_EFFECT_VOLUME"].plurals[0] = "Efeito de volume##CC_GUI_COLOR_PATTERN_EFFECT_VOLUME";
    strings["Panning effect##CC_GUI_COLOR_PATTERN_EFFECT_PANNING"].plurals[0] = "Efeito de balanço##CC_GUI_COLOR_PATTERN_EFFECT_PANNING";
    strings["Song effect##CC_GUI_COLOR_PATTERN_EFFECT_SONG"].plurals[0] = "Efeito de música##CC_GUI_COLOR_PATTERN_EFFECT_SONG";
    strings["Time effect##CC_GUI_COLOR_PATTERN_EFFECT_TIME"].plurals[0] = "Efeito de tempo##CC_GUI_COLOR_PATTERN_EFFECT_TIME";
    strings["Speed effect##CC_GUI_COLOR_PATTERN_EFFECT_SPEED"].plurals[0] = "Efeito de velocidade##CC_GUI_COLOR_PATTERN_EFFECT_SPEED";
    strings["Primary specific effect##CC_GUI_COLOR_PATTERN_EFFECT_SYS_PRIMARY"].plurals[0] = "Efeito específico primário##CC_GUI_COLOR_PATTERN_EFFECT_SYS_PRIMARY";
    strings["Secondary specific effect##CC_GUI_COLOR_PATTERN_EFFECT_SYS_SECONDARY"].plurals[0] = "Efeito específico secundário##CC_GUI_COLOR_PATTERN_EFFECT_SYS_SECONDARY";
    strings["Miscellaneous##CC_GUI_COLOR_PATTERN_EFFECT_MISC"].plurals[0] = "Miscelânea##CC_GUI_COLOR_PATTERN_EFFECT_MISC";
    strings["External command output##CC_GUI_COLOR_EE_VALUE"].plurals[0] = "Saída de comandos externos##CC_GUI_COLOR_EE_VALUE";
    strings["Status: off/disabled##CC_GUI_COLOR_PATTERN_STATUS_OFF"].plurals[0] = "Estado do canal: desligado/desabilitado##CC_GUI_COLOR_PATTERN_STATUS_OFF";
    strings["Status: off + macro rel##CC_GUI_COLOR_PATTERN_STATUS_REL"].plurals[0] = "Estado do canal: desligado + repouso de macro##CC_GUI_COLOR_PATTERN_STATUS_REL";
    strings["Status: on + macro rel##CC_GUI_COLOR_PATTERN_STATUS_REL_ON"].plurals[0] = "Estado do canal: ligado + repouso de macro##CC_GUI_COLOR_PATTERN_STATUS_REL_ON";
    strings["Status: on##CC_GUI_COLOR_PATTERN_STATUS_ON"].plurals[0] = "Estado do canal: ligado##CC_GUI_COLOR_PATTERN_STATUS_ON";
    strings["Status: volume##CC_GUI_COLOR_PATTERN_STATUS_VOLUME"].plurals[0] = "Estado do canal: volume##CC_GUI_COLOR_PATTERN_STATUS_VOLUME";
    strings["Status: pitch##CC_GUI_COLOR_PATTERN_STATUS_PITCH"].plurals[0] = "Estado do canal: tom##CC_GUI_COLOR_PATTERN_STATUS_PITCH";
    strings["Status: panning##CC_GUI_COLOR_PATTERN_STATUS_PANNING"].plurals[0] = "Estado do canal: balanço##CC_GUI_COLOR_PATTERN_STATUS_PANNING";
    strings["Status: chip (primary)##CC_GUI_COLOR_PATTERN_STATUS_SYS1"].plurals[0] = "Estado do canal: chip (primário)##CC_GUI_COLOR_PATTERN_STATUS_SYS1";
    strings["Status: chip (secondary)##CC_GUI_COLOR_PATTERN_STATUS_SYS2"].plurals[0] = "Estado do canal: chip (secundário)##CC_GUI_COLOR_PATTERN_STATUS_SYS2";
    strings["Status: mixing##CC_GUI_COLOR_PATTERN_STATUS_MIXING"].plurals[0] = "Estado do canal: mixing##CC_GUI_COLOR_PATTERN_STATUS_MIXING";
    strings["Status: DSP effect##CC_GUI_COLOR_PATTERN_STATUS_DSP"].plurals[0] = "Estado do canal: efeito DSP##CC_GUI_COLOR_PATTERN_STATUS_DSP";
    strings["Status: note altering##CC_GUI_COLOR_PATTERN_STATUS_NOTE"].plurals[0] = "Estado do canal: alteração de nota##CC_GUI_COLOR_PATTERN_STATUS_NOTE";
    strings["Status: misc color 1##CC_GUI_COLOR_PATTERN_STATUS_MISC1"].plurals[0] = "Estado do canal: cor miscelânea 1##CC_GUI_COLOR_PATTERN_STATUS_MISC1";
    strings["Status: misc color 2##CC_GUI_COLOR_PATTERN_STATUS_MISC2"].plurals[0] = "Estado do canal: cor miscelânea 2##CC_GUI_COLOR_PATTERN_STATUS_MISC2";
    strings["Status: misc color 3##CC_GUI_COLOR_PATTERN_STATUS_MISC3"].plurals[0] = "Estado do canal: cor miscelânea 3##CC_GUI_COLOR_PATTERN_STATUS_MISC3";
    strings["Status: attack##CC_GUI_COLOR_PATTERN_STATUS_ATTACK"].plurals[0] = "Estado do canal: ataque##CC_GUI_COLOR_PATTERN_STATUS_ATTACK";
    strings["Status: decay##CC_GUI_COLOR_PATTERN_STATUS_DECAY"].plurals[0] = "Estado do canal: decaimento##CC_GUI_COLOR_PATTERN_STATUS_DECAY";
    strings["Status: sustain##CC_GUI_COLOR_PATTERN_STATUS_SUSTAIN"].plurals[0] = "Estado do canal: sustentação##CC_GUI_COLOR_PATTERN_STATUS_SUSTAIN";
    strings["Status: release##CC_GUI_COLOR_PATTERN_STATUS_RELEASE"].plurals[0] = "Estado do canal: repouso##CC_GUI_COLOR_PATTERN_STATUS_RELEASE";
    strings["Status: decrease linear##CC_GUI_COLOR_PATTERN_STATUS_DEC_LINEAR"].plurals[0] = "Estado do canal: redução linear##CC_GUI_COLOR_PATTERN_STATUS_DEC_LINEAR";
    strings["Status: decrease exp##CC_GUI_COLOR_PATTERN_STATUS_DEC_EXP"].plurals[0] = "Estado do canal: redução exponencial##CC_GUI_COLOR_PATTERN_STATUS_DEC_EXP";
    strings["Status: increase##CC_GUI_COLOR_PATTERN_STATUS_INC"].plurals[0] = "Estado do canal: aumento##CC_GUI_COLOR_PATTERN_STATUS_INC";
    strings["Status: bent##CC_GUI_COLOR_PATTERN_STATUS_BENT"].plurals[0] = "Estado do canal: pitchbend##CC_GUI_COLOR_PATTERN_STATUS_BENT";
    strings["Status: direct##CC_GUI_COLOR_PATTERN_STATUS_DIRECT"].plurals[0] = "Estado do canal: direto##CC_GUI_COLOR_PATTERN_STATUS_DIRECT";

    strings["Background##CC_GUI_COLOR_SAMPLE_BG"].plurals[0] = "Fundo##CC_GUI_COLOR_SAMPLE_BG";
    strings["Waveform##CC_GUI_COLOR_SAMPLE_FG"].plurals[0] = "Forma de onda##CC_GUI_COLOR_SAMPLE_FG";
    strings["Time background##CC_GUI_COLOR_SAMPLE_TIME_BG"].plurals[0] = "Fundo do tempo##CC_GUI_COLOR_SAMPLE_TIME_BG";
    strings["Time text##CC_GUI_COLOR_SAMPLE_TIME_FG"].plurals[0] = "Texto do tempo##CC_GUI_COLOR_SAMPLE_TIME_FG";
    strings["Loop region##CC_GUI_COLOR_SAMPLE_LOOP"].plurals[0] = "Região de repetição##CC_GUI_COLOR_SAMPLE_LOOP";
    strings["Center guide##CC_GUI_COLOR_SAMPLE_CENTER"].plurals[0] = "Guia do centro##CC_GUI_COLOR_SAMPLE_CENTER";
    strings["Grid##CC_GUI_COLOR_SAMPLE_GRID"].plurals[0] = "Grade##CC_GUI_COLOR_SAMPLE_GRID";
    strings["Selection##CC_GUI_COLOR_SAMPLE_SEL"].plurals[0] = "Seleção##CC_GUI_COLOR_SAMPLE_SEL";
    strings["Selection points##CC_GUI_COLOR_SAMPLE_SEL_POINT"].plurals[0] = "Pontos de seleção##CC_GUI_COLOR_SAMPLE_SEL_POINT";
    strings["Preview needle##CC_GUI_COLOR_SAMPLE_NEEDLE"].plurals[0] = "Agulha de previsão##CC_GUI_COLOR_SAMPLE_NEEDLE";
    strings["Playing needles##CC_GUI_COLOR_SAMPLE_NEEDLE_PLAYING"].plurals[0] = "Agulhas de reprodução##CC_GUI_COLOR_SAMPLE_NEEDLE_PLAYING";
    strings["Loop markers##CC_GUI_COLOR_SAMPLE_LOOP_POINT"].plurals[0] = "Marcadores de repetição##CC_GUI_COLOR_SAMPLE_LOOP_POINT";
    strings["Chip select: disabled##CC_GUI_COLOR_SAMPLE_CHIP_DISABLED"].plurals[0] = "Seleção de chip: desabilitada##CC_GUI_COLOR_SAMPLE_CHIP_DISABLED";
    strings["Chip select: enabled##CC_GUI_COLOR_SAMPLE_CHIP_ENABLED"].plurals[0] = "Seleção de chip: habilitada##CC_GUI_COLOR_SAMPLE_CHIP_ENABLED";
    strings["Chip select: enabled (failure)##CC_GUI_COLOR_SAMPLE_CHIP_WARNING"].plurals[0] = "Seleção de chip: habilitada (falha)##CC_GUI_COLOR_SAMPLE_CHIP_WARNING";

    strings["Unallocated##CC_GUI_COLOR_PAT_MANAGER_NULL"].plurals[0] = "Não alocado##CC_GUI_COLOR_PAT_MANAGER_NULL";
    strings["Unused##CC_GUI_COLOR_PAT_MANAGER_UNUSED"].plurals[0] = "Não utilizado##CC_GUI_COLOR_PAT_MANAGER_UNUSED";
    strings["Used##CC_GUI_COLOR_PAT_MANAGER_USED"].plurals[0] = "Utilizado##CC_GUI_COLOR_PAT_MANAGER_USED";
    strings["Overused##CC_GUI_COLOR_PAT_MANAGER_OVERUSED"].plurals[0] = "Muito utilizado##CC_GUI_COLOR_PAT_MANAGER_OVERUSED";
    strings["Really overused##CC_GUI_COLOR_PAT_MANAGER_EXTREMELY_OVERUSED"].plurals[0] = "Extremamente utilizado##CC_GUI_COLOR_PAT_MANAGER_EXTREMELY_OVERUSED";
    strings["Combo Breaker##CC_GUI_COLOR_PAT_MANAGER_COMBO_BREAKER"].plurals[0] = "Combo Breaker##CC_GUI_COLOR_PAT_MANAGER_COMBO_BREAKER";

    strings["Background##CC_GUI_COLOR_PIANO_BACKGROUND"].plurals[0] = "Fundo##CC_GUI_COLOR_PIANO_BACKGROUND";
    strings["Upper key##CC_GUI_COLOR_PIANO_KEY_TOP"].plurals[0] = "Tecla preta##CC_GUI_COLOR_PIANO_KEY_TOP";
    strings["Upper key (feedback)##CC_GUI_COLOR_PIANO_KEY_TOP_HIT"].plurals[0] = "Tecla preta (feedback)##CC_GUI_COLOR_PIANO_KEY_TOP_HIT";
    strings["Upper key (pressed)##CC_GUI_COLOR_PIANO_KEY_TOP_ACTIVE"].plurals[0] = "Tecla preta (pressionada)##CC_GUI_COLOR_PIANO_KEY_TOP_ACTIVE";
    strings["Lower key##CC_GUI_COLOR_PIANO_KEY_BOTTOM"].plurals[0] = "Tecla branca##CC_GUI_COLOR_PIANO_KEY_BOTTOM";
    strings["Lower key (feedback)##CC_GUI_COLOR_PIANO_KEY_BOTTOM_HIT"].plurals[0] = "Tecla branca (feedback)##CC_GUI_COLOR_PIANO_KEY_BOTTOM_HIT";
    strings["Lower key (pressed)##CC_GUI_COLOR_PIANO_KEY_BOTTOM_ACTIVE"].plurals[0] = "Tecla branca (pressionada)##CC_GUI_COLOR_PIANO_KEY_BOTTOM_ACTIVE";

    strings["Clock text##CC_GUI_COLOR_CLOCK_TEXT"].plurals[0] = "Texto do clock##CC_GUI_COLOR_CLOCK_TEXT";
    strings["Beat (off)##CC_GUI_COLOR_CLOCK_BEAT_LOW"].plurals[0] = "Batida (secundária)##CC_GUI_COLOR_CLOCK_BEAT_LOW";
    strings["Beat (on)##CC_GUI_COLOR_CLOCK_BEAT_HIGH"].plurals[0] = "Batida (primária)##CC_GUI_COLOR_CLOCK_BEAT_HIGH";

    strings["PortSet##CC_GUI_COLOR_PATCHBAY_PORTSET"].plurals[0] = "Conjunto de portas##CC_GUI_COLOR_PATCHBAY_PORTSET";
    strings["Port##CC_GUI_COLOR_PATCHBAY_PORT"].plurals[0] = "Porta##CC_GUI_COLOR_PATCHBAY_PORT";
    strings["Port (hidden/unavailable)##CC_GUI_COLOR_PATCHBAY_PORT_HIDDEN"].plurals[0] = "Porta (escondida/indisponível)##CC_GUI_COLOR_PATCHBAY_PORT_HIDDEN";
    strings["Connection (selected)##CC_GUI_COLOR_PATCHBAY_CONNECTION"].plurals[0] = "Conexão (selecionada)##CC_GUI_COLOR_PATCHBAY_CONNECTION";
    strings["Connection (other)##CC_GUI_COLOR_PATCHBAY_CONNECTION_BG"].plurals[0] = "Conexão (outra)##CC_GUI_COLOR_PATCHBAY_CONNECTION_BG";

    strings["Background##CC_GUI_COLOR_MEMORY_BG"].plurals[0] = "Fundo##CC_GUI_COLOR_MEMORY_BG";
    strings["Unknown##CC_GUI_COLOR_MEMORY_FREE"].plurals[0] = "Desconhecido##CC_GUI_COLOR_MEMORY_FREE";
    strings["Reserved##CC_GUI_COLOR_MEMORY_RESERVED"].plurals[0] = "Reservado##CC_GUI_COLOR_MEMORY_RESERVED";
    strings["Sample##CC_GUI_COLOR_MEMORY_SAMPLE"].plurals[0] = "Sample##CC_GUI_COLOR_MEMORY_SAMPLE";
    strings["Sample (alternate 1)##CC_GUI_COLOR_MEMORY_SAMPLE_ALT1"].plurals[0] = "Sample (alternativo 1)##CC_GUI_COLOR_MEMORY_SAMPLE_ALT1";
    strings["Sample (alternate 2)##CC_GUI_COLOR_MEMORY_SAMPLE_ALT2"].plurals[0] = "Sample (alternativo 2)##CC_GUI_COLOR_MEMORY_SAMPLE_ALT2";
    strings["Sample (alternate 3)##CC_GUI_COLOR_MEMORY_SAMPLE_ALT3"].plurals[0] = "Sample (alternativo 3)##CC_GUI_COLOR_MEMORY_SAMPLE_ALT3";
    strings["Wave RAM##CC_GUI_COLOR_MEMORY_WAVE_RAM"].plurals[0] = "Memória RAM de Onda##CC_GUI_COLOR_MEMORY_WAVE_RAM";
    strings["Wavetable (static)##CC_GUI_COLOR_MEMORY_WAVE_STATIC"].plurals[0] = "Wavetable (estática)##CC_GUI_COLOR_MEMORY_WAVE_STATIC";
    strings["Echo buffer##CC_GUI_COLOR_MEMORY_ECHO"].plurals[0] = "Buffer de Eco##CC_GUI_COLOR_MEMORY_ECHO";
    strings["Namco 163 load pos##CC_GUI_COLOR_MEMORY_N163_LOAD"].plurals[0] = "Posição de carga de onda do Namco 163##CC_GUI_COLOR_MEMORY_N163_LOAD";
    strings["Namco 163 play pos##CC_GUI_COLOR_MEMORY_N163_PLAY"].plurals[0] = "Posição de reprodução de onda do Namco 163##CC_GUI_COLOR_MEMORY_N163_PLAY";
    strings["Sample (bank 0)##CC_GUI_COLOR_MEMORY_BANK0"].plurals[0] = "Sample (banco 0)##CC_GUI_COLOR_MEMORY_BANK0";
    strings["Sample (bank 1)##CC_GUI_COLOR_MEMORY_BANK1"].plurals[0] = "Sample (banco 1)##CC_GUI_COLOR_MEMORY_BANK1";
    strings["Sample (bank 2)##CC_GUI_COLOR_MEMORY_BANK2"].plurals[0] = "Sample (banco 2)##CC_GUI_COLOR_MEMORY_BANK2";
    strings["Sample (bank 3)##CC_GUI_COLOR_MEMORY_BANK3"].plurals[0] = "Sample (banco 3)##CC_GUI_COLOR_MEMORY_BANK3";
    strings["Sample (bank 4)##CC_GUI_COLOR_MEMORY_BANK4"].plurals[0] = "Sample (banco 4)##CC_GUI_COLOR_MEMORY_BANK4";
    strings["Sample (bank 5)##CC_GUI_COLOR_MEMORY_BANK5"].plurals[0] = "Sample (banco 5)##CC_GUI_COLOR_MEMORY_BANK5";
    strings["Sample (bank 6)##CC_GUI_COLOR_MEMORY_BANK6"].plurals[0] = "Sample (banco 6)##CC_GUI_COLOR_MEMORY_BANK6";
    strings["Sample (bank 7)##CC_GUI_COLOR_MEMORY_BANK7"].plurals[0] = "Sample (banco 7)##CC_GUI_COLOR_MEMORY_BANK7";

    strings["Log level: Error##CC_GUI_COLOR_LOGLEVEL_ERROR"].plurals[0] = "Nível de log: Erro##CC_GUI_COLOR_LOGLEVEL_ERROR";
    strings["Log level: Warning##CC_GUI_COLOR_LOGLEVEL_WARNING"].plurals[0] = "Nível de log: Aviso##CC_GUI_COLOR_LOGLEVEL_WARNING";
    strings["Log level: Info##CC_GUI_COLOR_LOGLEVEL_INFO"].plurals[0] = "Nível de log: Informação##CC_GUI_COLOR_LOGLEVEL_INFO";
    strings["Log level: Debug##CC_GUI_COLOR_LOGLEVEL_DEBUG"].plurals[0] = "Nível de log: Depuração##CC_GUI_COLOR_LOGLEVEL_DEBUG";
    strings["Log level: Trace/Verbose##CC_GUI_COLOR_LOGLEVEL_TRACE"].plurals[0] = "Nível de log: Traço/Verboso##CC_GUI_COLOR_LOGLEVEL_TRACE";

    strings["OK##SettingsOK"].plurals[0] = "OK##SettingsOK";
    strings["Cancel##SettingsCancel"].plurals[0] = "Cancelar##SettingsCancel";
    strings["Apply##SettingsApply"].plurals[0] = "Aplicar##SettingsApply";

    //   sgut  src/gui/util.cpp
    strings["<nothing>##sgut"].plurals[0] = "<nada>";
    strings["Unknown##sgut0"].plurals[0] = "Desconhecido";
    strings["Unknown##sgut1"].plurals[0] = "Desconhecido";

    // instruments
    //      icon   is the letter icon for the instrument

    //   sgi**     src/gui/inst/
    //   sgiPCMA   src/gui/inst/adpcma.cpp           --DONE--
    //   sgiPCMB   src/gui/inst/adpcmb.cpp           --DONE--
    //   sgiSAMPLE src/gui/inst/amiga.cpp            --DONE--
    //   sgiAY     src/gui/inst/ay.cpp               --DONE--
    //   sgi8930   src/gui/inst/ay8930.cpp           --DONE--
    //   sgiB      src/gui/inst/beeper.cpp           --DONE--
    //   sgiC140   src/gui/inst/c140.cpp             --DONE--
    //   sgiC219   src/gui/inst/c219.cpp             --DONE--
    //   sgiC64    src/gui/inst/c64.cpp              --DONE--
    //   sgiDAVE   src/gui/inst/dave.cpp             --DONE--
    //   sgi5503   src/gui/inst/es5503.cpp           --DONE--
    //   sgiOTTO   src/gui/inst/es5506.cpp           --DONE--
    //   sgiESFM   src/gui/inst/esfm.cpp             --DONE--
    //   sgiFDS    src/gui/inst/fds.cpp              --DONE--
    //   sgifmeu   src/gui/inst/fmEnvUtil.cpp        --DONE--
    //   sgifmeh   src/gui/inst/fmEnvUtil.h          --DONE--
    //   sgiGA20   src/gui/inst/ga20.cpp             --DONE--
    //   sgiGB     src/gui/inst/gb.cpp               --DONE--
    //   sgiGBADMA src/gui/inst/gbadma.cpp
    //sgiGBAMINMOD src/gui/inst/gbaminmod.cpp
    //   sgiK00    src/gui/inst/k007232.cpp          --DONE--
    //   sgiK05    src/gui/inst/k053260.cpp          --DONE--
    //   sgimcd    src/gui/inst/macroDraw.cpp        --DONE--
    //   sgimcdu   src/gui/inst/macroDrawUtil.cpp    --
    //   sgiLYNX   src/gui/inst/mikey.cpp            --DONE--
    //   sgi5232   src/gui/inst/msm5232.cpp          --DONE--
    //   sgi6258   src/gui/inst/msm6258.cpp          --DONE--
    //   sgi6295   src/gui/inst/msm6295.cpp          --DONE--
    //   sgiMULPCM src/gui/inst/multipcm.cpp         --DONE--
    //   sgiN163   src/gui/inst/n163.cpp             --DONE--
    //   sgiNDS    src/gui/inst/nds.cpp
    //   sgiWSG    src/gui/inst/namco.cpp            --DONE--
    //   sgiNES    src/gui/inst/nes.cpp              --DONE--
    //   sgiOPL    src/gui/inst/opl.cpp              --DONE--
    //   sgiOPLL   src/gui/inst/opll.cpp             --DONE--
    //   sgiOPM    src/gui/inst/opm.cpp              --DONE--
    //   sgiOPN    src/gui/inst/opn.cpp              --DONE--
    //   sgiOPZ    src/gui/inst/opz.cpp              --DONE--
    //   sgiPCE    src/gui/inst/pce.cpp              --DONE--
    //   sgiPET    src/gui/inst/pet.cpp              --DONE--
    //   sgiPMQT   src/gui/inst/pokemini.cpp         --DONE--
    //   sgiPOKEY  src/gui/inst/pokey.cpp            --DONE--
    //   sgiPNN    src/gui/inst/powernoise.cpp       --DONE--
    //   sgiPNS    src/gui/inst/powernoise_slope.cpp --DONE--
    //   sgiPV     src/gui/inst/pv1000.cpp           --DONE--
    //   sgiQ      src/gui/inst/qsound.cpp           --DONE--
    //   sgiRF5    src/gui/inst/rf5c68.cpp           --DONE--
    //   sgiSAA    src/gui/inst/saa1099.cpp          --DONE--
    //   sgismpd   src/gui/inst/sampleDraw.cpp       --DONE--
    //   sgiSCC    src/gui/inst/scc.cpp              --DONE--
    //   sgiSEGA   src/gui/inst/segapcm.cpp          --DONE--
    //   sgiSID2   src/gui/inst/sid2.cpp             --DONE--
    //   sgiSM     src/gui/inst/sm8521.cpp           --DONE--
    //   sgiSNES   src/gui/inst/snes.cpp             --DONE--
    //   sgiPSG    src/gui/inst/std.cpp              --DONE--
    //   sgistru   src/gui/inst/stringsUtil.cpp      --    --
    //   sgiSU     src/gui/inst/su.cpp               --DONE--
    //   sgiSWAN   src/gui/inst/swan.cpp             --DONE--
    //   sgiT6W    src/gui/inst/t6w28.cpp            --DONE--
    //   sgiTED    src/gui/inst/ted.cpp              --DONE--
    //   sgiTIA    src/gui/inst/tia.cpp              --DONE--
    //   sgiVB     src/gui/inst/vboy.cpp             --DONE--
    //   sgiVERA   src/gui/inst/vera.cpp             --DONE--
    //   sgiVIC    src/gui/inst/vic.cpp              --DONE--
    //   sgiVRC6   src/gui/inst/vrc6.cpp             --DONE--
    //   sgiVRC6S  src/gui/inst/vrc6saw.cpp          --DONE--
    //   sgiwave   src/gui/inst/wavetable.cpp        --DONE--
    //   sgiX1     src/gui/inst/x1_010.cpp           --DONE--
    //   sgiYMZ    src/gui/inst/ymz280b.cpp          --DONE--

    //   sgiPCMA  src/gui/inst/adpcma.cpp

    strings["Macros##sgiPCMA"].plurals[0] = "Macros";
    strings["Volume##sgiPCMA"].plurals[0] = "Volume";
    strings["Global Volume##sgiPCMA"].plurals[0] = "Volume Global";
    strings["Panning##sgiPCMA"].plurals[0] = "Balanço";
    strings["Phase Reset##sgiPCMA"].plurals[0] = "Redef. de Fase";

    //   sgiPCMB   src/gui/inst/adpcmb.cpp

    strings["Macros##sgiPCMB"].plurals[0] = "Macros";
    strings["Volume##sgiPCMB"].plurals[0] = "Volume";
    strings["Arpeggio##sgiPCMB"].plurals[0] = "Arpejo";
    strings["Pitch##sgiPCMB"].plurals[0] = "Tom";
    strings["Panning##sgiPCMB"].plurals[0] = "Balanço";
    strings["Phase Reset##sgiPCMB"].plurals[0] = "Redef. de Fase";

    //   sgiSAMPLE src/gui/inst/amiga.cpp

    strings["Macros##sgiSAMPLE"].plurals[0] = "Macros";
    strings["Volume##sgiSAMPLE"].plurals[0] = "Volume";
    strings["Arpeggio##sgiSAMPLE"].plurals[0] = "Arpejo";
    strings["Pitch##sgiSAMPLE"].plurals[0] = "Tom";
    strings["Panning##sgiSAMPLE"].plurals[0] = "Balanço";
    strings["Panning (left)##sgiSAMPLE"].plurals[0] = "Balanço (esquerda)";
    strings["Surround##sgiSAMPLE"].plurals[0] = "Surround";
    strings["Panning (right)##sgiSAMPLE"].plurals[0] = "Balanço (direita)";
    strings["Waveform##sgiSAMPLE"].plurals[0] = "Forma de onda";
    strings["Phase Reset##sgiSAMPLE"].plurals[0] = "Redef. de Fase";

    //   sgiAY     src/gui/inst/ay.cpp

    strings["Macros##sgiAY"].plurals[0] = "Macros";
    strings["Volume##sgiAY"].plurals[0] = "Volume";
    strings["Arpeggio##sgiAY"].plurals[0] = "Arpejo";
    strings["Pitch##sgiAY"].plurals[0] = "Tom";
    strings["Noise Freq##sgiAY"].plurals[0] = "Freq. Ruído";
    strings["Waveform##sgiAY"].plurals[0] = "Forma de Onda";
    strings["Raw Period##sgiAY"].plurals[0] = "Período (val. registrador)";
    strings["Raw Envelope Period##sgiAY"].plurals[0] = "Período do Envelope (val. registrador)";
    strings["Phase Reset##sgiAY"].plurals[0] = "Redef. de Fase";
    strings["Envelope##sgiAY"].plurals[0] = "Envelope";
    strings["AutoEnv Num##sgiAY"].plurals[0] = "Num. EnvAuto";
    strings["AutoEnv Den##sgiAY"].plurals[0] = "Den. EnvAuto";

    //   sgi8930   src/gui/inst/ay8930.cpp

    strings["Macros##sgi8930"].plurals[0] = "Macros";
    strings["Volume##sgi8930"].plurals[0] = "Volume";
    strings["Arpeggio##sgi8930"].plurals[0] = "Arpejo";
    strings["Pitch##sgi8930"].plurals[0] = "Tom";
    strings["Noise Freq##sgi8930"].plurals[0] = "Freq. Ruído";
    strings["Waveform##sgi8930"].plurals[0] = "Forma de Onda";
    strings["Raw Period##sgi8930"].plurals[0] = "Período (val. registrador)";
    strings["Raw Envelope Period##sgi8930"].plurals[0] = "Período do Envelope (val. registrador)";
    strings["Phase Reset##sgi8930"].plurals[0] = "Redef. de Fase";
    strings["Duty##sgi8930"].plurals[0] = "Ciclo de Trab.";
    strings["Envelope##sgi8930"].plurals[0] = "Envelope";
    strings["AutoEnv Num##sgi8930"].plurals[0] = "Num. EnvAuto";
    strings["AutoEnv Den##sgi8930"].plurals[0] = "Den. EnvAuto";
    strings["Noise AND Mask##sgi8930"].plurals[0] = "Máscara AND Ruído";
    strings["Noise OR Mask##sgi8930"].plurals[0] = "Máscara OR Ruído";

    //   sgiB      src/gui/inst/beeper.cpp

    strings["Macros##sgiB"].plurals[0] = "Macros";
    strings["Volume##sgiB"].plurals[0] = "Volume";
    strings["Arpeggio##sgiB"].plurals[0] = "Arpejo";
    strings["Pitch##sgiB"].plurals[0] = "Tom";
    strings["Pulse Width##sgiB"].plurals[0] = "Largura de Pulso";

    //   sgiC140   src/gui/inst/c140.cpp

    strings["Macros##sgiC140"].plurals[0] = "Macros";
    strings["Volume##sgiC140"].plurals[0] = "Volume";
    strings["Arpeggio##sgiC140"].plurals[0] = "Arpejo";
    strings["Pitch##sgiC140"].plurals[0] = "Tom";
    strings["Panning (left)##sgiC140"].plurals[0] = "Balanço (esquerda)";
    strings["Panning (right)##sgiC140"].plurals[0] = "Balanço (direita)";
    strings["Phase Reset##sgiC140"].plurals[0] = "Redef. de Fase";

    //   sgiC219   src/gui/inst/c219.cpp

    strings["Macros##sgiC219"].plurals[0] = "Macros";
    strings["Volume##sgiC219"].plurals[0] = "Volume";
    strings["Arpeggio##sgiC219"].plurals[0] = "Arpejo";
    strings["Pitch##sgiC219"].plurals[0] = "Tom";
    strings["Control##sgiC219"].plurals[0] = "Controle";
    strings["Panning (left)##sgiC219"].plurals[0] = "Balanço (esquerda)";
    strings["Panning (right)##sgiC219"].plurals[0] = "Balanço (direita)";
    strings["Phase Reset##sgiC219"].plurals[0] = "Redef. de Fase";

    //   sgiC64    src/gui/inst/c64.cpp

    strings["Waveform##sgiC640"].plurals[0] = "Forma de Onda";
    strings["tri##sgiC64"].plurals[0] = "tri";
    strings["saw##sgiC64"].plurals[0] = "serra";
    strings["pulse##sgiC64"].plurals[0] = "pulso";
    strings["noise##sgiC64"].plurals[0] = "ruído";
    strings["A##sgiC640"].plurals[0] = "A";
    strings["A##sgiC641"].plurals[0] = "A";
    strings["D##sgiC640"].plurals[0] = "D";
    strings["D##sgiC641"].plurals[0] = "D";
    strings["S##sgiC640"].plurals[0] = "S";
    strings["S##sgiC641"].plurals[0] = "S";
    strings["R##sgiC640"].plurals[0] = "R";
    strings["R##sgiC641"].plurals[0] = "R";
    strings["Envelope##sgiC640"].plurals[0] = "Envelope";
    strings["Envelope##sgiC641"].plurals[0] = "Envelope";
    strings["Duty##sgiC640"].plurals[0] = "Ciclo de Trabalho";
    strings["Ring Modulation##sgiC64"].plurals[0] = "Modulação de Anel";
    strings["Oscillator Sync##sgiC64"].plurals[0] = "Sincronização de Osciladores";
    strings["Enable filter##sgiC64"].plurals[0] = "Habilitar filtro";
    strings["Initialize filter##sgiC64"].plurals[0] = "Inicializar filtro";
    strings["Cutoff##sgiC640"].plurals[0] = "Ponto de corte";
    strings["Resonance##sgiC640"].plurals[0] = "Ressonância";
    strings["Filter Mode##sgiC640"].plurals[0] = "Modo de filtro";
    strings["low##sgiC64"].plurals[0] = "baixa";
    strings["band##sgiC64"].plurals[0] = "banda";
    strings["high##sgiC64"].plurals[0] = "alta";
    strings["ch3off##sgiC64"].plurals[0] = "cn3desl";
    strings["Absolute Cutoff Macro##sgiC64"].plurals[0] = "Macro de Corte Absoluta";
    strings["Absolute Duty Macro##sgiC64"].plurals[0] = "Macro de Ciclo de Trab. Absoluta";
    strings["Don't test before new note##sgiC64"].plurals[0] = "Não testar antes de uma nova nota";
    strings["Macros##sgiC64"].plurals[0] = "Macros";
    strings["Volume##sgiC64"].plurals[0] = "Volume";
    strings["Arpeggio##sgiC64"].plurals[0] = "Arpejo";
    strings["Pitch##sgiC64"].plurals[0] = "Tom";
    strings["Duty##sgiC641"].plurals[0] = "Ciclo de Trabalho";
    strings["Waveform##sgiC641"].plurals[0] = "Forma de onda";
    strings["Cutoff##sgiC641"].plurals[0] = "Ponto de Corte";
    strings["Filter Mode##sgiC641"].plurals[0] = "Modo de Filtro";
    strings["Filter Toggle##sgiC64"].plurals[0] = "Alternar Filtro";
    strings["Resonance##sgiC641"].plurals[0] = "Ressonância";
    strings["Special##sgiC64"].plurals[0] = "Especial";
    strings["Attack##sgiC64"].plurals[0] = "Ataque";
    strings["Decay##sgiC64"].plurals[0] = "Decaimento";
    strings["Sustain##sgiC64"].plurals[0] = "Sustentação";
    strings["Release##sgiC64"].plurals[0] = "Repouso";

    //   sgiDAVE   src/gui/inst/dave.cpp

    strings["Macros##sgiDAVE"].plurals[0] = "Macros";
    strings["Volume##sgiDAVE"].plurals[0] = "Volume";
    strings["Arpeggio##sgiDAVE"].plurals[0] = "Arpejo";
    strings["Pitch##sgiDAVE"].plurals[0] = "Tom";
    strings["Noise Freq##sgiDAVE"].plurals[0] = "Freq. Ruído";
    strings["Waveform##sgiDAVE"].plurals[0] = "Forma de Onda";
    strings["Panning (left)##sgiDAVE"].plurals[0] = "Balanço (esquerda)";
    strings["Panning (right)##sgiDAVE"].plurals[0] = "Balanço (direita)";
    strings["Phase Reset##sgiDAVE"].plurals[0] = "Redef. de fase";
    strings["Control##sgiDAVE"].plurals[0] = "Controle";
    strings["Raw Frequency##sgiDAVE"].plurals[0] = "Frequência (val. registrador)";

    //   sgi5503   src/gui/inst/es5503.cpp

    strings["Oscillator mode:##sgi5503"].plurals[0] = "Modo de Oscilador:";
    strings["Freerun##sgi5503"].plurals[0] = "Livre/independente";
    strings["Oneshot##sgi5503"].plurals[0] = "Uma vez";
    strings["Sync/AM##sgi5503"].plurals[0] = "Sincronizado/AM";
    strings["Swap##sgi5503"].plurals[0] = "Trocar";
    strings["Virtual softpan channel##sgi5503"].plurals[0] = "Balanço suave virtual";
    strings["Combines odd and next even channel into one virtual channel with 256-step panning.\nInstrument, volume and effects need to be placed on the odd channel (e.g. 1st, 3rd, 5th etc.)##sgi5503"].plurals[0] = "Combina um canal ímpar com o próximo canal par em um canal virtual com balanço de 256 passos.\nInstrumento, volume e efeitos precisam ser colocados em um canal ímpar (ex: 1º, 3º, 5º etc.)";
    strings["Phase reset on key-on##sgi5503"].plurals[0] = "Realizar redefinição de fase em nova nota";
    strings["Macros##sgi5503"].plurals[0] = "Macros";
    strings["Volume##sgi5503"].plurals[0] = "Volume";
    strings["Arpeggio##sgi5503"].plurals[0] = "Arpejo";
    strings["Pitch##sgi5503"].plurals[0] = "Tom";
    strings["Osc. mode##sgi5503"].plurals[0] = "Modo de Oscilador";
    strings["Panning (left)##sgi5503"].plurals[0] = "Balanço (esquerda)";
    strings["Panning (right)##sgi5503"].plurals[0] = "Balanço (direita)";
    strings["Waveform##sgi5503"].plurals[0] = "Forma de Onda";
    strings["Phase Reset##sgi5503"].plurals[0] = "Redef. de Fase";
    strings["Wave/sample pos.##sgi5503"].plurals[0] = "Pos. onda/sample";
    strings["Osc. output##sgi5503"].plurals[0] = "Saída osc.";

    //   sgiOTTO   src/gui/inst/es5506.cpp

    strings["Filter Mode##sgiOTTO0"].plurals[0] = "Modo de filtro";
    strings["Filter K1##sgiOTTO0"].plurals[0] = "Coef. K1 do filtro";
    strings["Filter K2##sgiOTTO0"].plurals[0] = "Coef. K2 do filtro";
    strings["Envelope length##sgiOTTO"].plurals[0] = "Comprimento do envelope";
    strings["Left Volume Ramp##sgiOTTO"].plurals[0] = "Rampa do volume esquerdo";
    strings["Right Volume Ramp##sgiOTTO"].plurals[0] = "Rampa do volume direito";
    strings["Filter K1 Ramp##sgiOTTO"].plurals[0] = "Rampa do coef. K1 do filtro";
    strings["Filter K2 Ramp##sgiOTTO"].plurals[0] = "Rampa do coef. K2 do filtro";
    strings["K1 Ramp Slowdown##sgiOTTO"].plurals[0] = "Desaceleração da rampa do K1";
    strings["K2 Ramp Slowdown##sgiOTTO"].plurals[0] = "Desaceleração da rampa do K2";
    strings["Macros##sgiOTTO"].plurals[0] = "Macros";
    strings["Volume##sgiOTTO"].plurals[0] = "Volume";
    strings["Arpeggio##sgiOTTO"].plurals[0] = "Arpejo";
    strings["Pitch##sgiOTTO"].plurals[0] = "Tom";
    strings["Panning (left)##sgiOTTO"].plurals[0] = "Balanço (esquerda)";
    strings["Panning (right)##sgiOTTO"].plurals[0] = "Balanço (direita)";
    strings["Phase Reset##sgiOTTO"].plurals[0] = "Redef. de Fase";
    strings["Filter Mode##sgiOTTO1"].plurals[0] = "Modo de filtro";
    strings["Filter K1##sgiOTTO1"].plurals[0] = "Coef. K1 do filtro";
    strings["Filter K2##sgiOTTO1"].plurals[0] = "Coef. K2 do filtro";
    strings["Outputs##sgiOTTO"].plurals[0] = "Saídas";
    strings["Control##sgiOTTO"].plurals[0] = "Controle";

    //   sgiESFM   src/gui/inst/esfm.cpp

    strings["Other##sgiESFM0"].plurals[0] = "Outro";
    strings["Other##sgiESFM1"].plurals[0] = "Outro";
    strings["Envelope##sgiESFM0"].plurals[0] = "Envelope";
    strings["Envelope##sgiESFM1"].plurals[0] = "Envelope";
    strings["op%d##sgiESFM0"].plurals[0] = "op%d";
    strings["OP%d##sgiESFM1"].plurals[0] = "OP%d";
    strings["Detune in semitones##sgiESFM0"].plurals[0] = "Desafinação em semitons";
    strings["Detune in fractions of semitone.\n128 = +1 semitone, -128 = -1 semitone.##sgiESFM0"].plurals[0] = "Desafinação em frações de semitom.\n128 = +1 semitom, -128 = -1 semitom.";
    strings["If operator outputs sound, enable left channel output.##sgiESFM0"].plurals[0] = "Se o operador emite som, habilita a sua saída pelo canal esquerdo.";
    strings["If operator outputs sound, enable right channel output.##sgiESFM0"].plurals[0] = "Se o operador emite som, habilita a sua saída pelo canal direito.";
    strings["Block##sgiESFM0"].plurals[0] = "Bloco";
    strings["FreqNum##sgiESFM0"].plurals[0] = "NúmFreq.";
    strings["op%d##sgiESFM2"].plurals[0] = "op%d";
    strings["Operator %d##sgiESFM"].plurals[0] = "Operador %d";
    strings["Waveform##sgiESFM"].plurals[0] = "Forma de Onda";
    strings["Envelope##sgiESFM"].plurals[0] = "Envelope";
    strings["Blk##sgiESFM"].plurals[0] = "Blc";
    strings["Block##sgiESFM1"].plurals[0] = "Bloco";
    strings["F##sgiESFM"].plurals[0] = "F";
    strings["Frequency (F-Num)##sgiESFM"].plurals[0] = "Frequência (Núm-F)";
    strings["Detune in semitones##sgiESFM1"].plurals[0] = "Desafinação em semitons";
    strings["Detune in fractions of semitone.\n128 = +1 semitone, -128 = -1 semitone.##sgiESFM1"].plurals[0] = "Desafinação em frações de semitom.\n128 = +1 semitom, -128 = -1 semitom.";
    strings["If operator outputs sound, enable left channel output.##sgiESFM1"].plurals[0] = "Se o operador emite som, habilita a sua saída pelo canal esquerdo.";
    strings["If operator outputs sound, enable right channel output.##sgiESFM1"].plurals[0] = "Se o operador emite som, habilita a sua saída pelo canal direito.";
    strings["op%d##sgiESFM3"].plurals[0] = "op%d";
    strings["OP%d##sgiESFM4"].plurals[0] = "OP%d";
    strings["Block##sgiESFM2"].plurals[0] = "Bloco";
    strings["FreqNum##sgiESFM1"].plurals[0] = "NúmFreq.";
    strings["Detune in semitones##sgiESFM2"].plurals[0] = "Desafinação em semitons";
    strings["Detune in fractions of semitone.\n128 = +1 semitone, -128 = -1 semitone.##sgiESFM2"].plurals[0] = "Desafinação em frações de semitom.\n128 = +1 semitom, -128 = -1 semitom.";
    strings["If operator outputs sound, enable left channel output.##sgiESFM2"].plurals[0] = "Se o operador emite som, habilita a sua saída pelo canal esquerdo.";
    strings["If operator outputs sound, enable right channel output.##sgiESFM2"].plurals[0] = "Se o operador emite som, habilita a sua saída pelo canal direito.";
    strings["OP%d Macros##sgiESFM"].plurals[0] = "Macros OP%d";
    strings["Block##sgiESFM3"].plurals[0] = "Bloco";
    strings["FreqNum##sgiESFM2"].plurals[0] = "NúmFreq.";
    strings["Op. Arpeggio##sgiESFM"].plurals[0] = "Arpejo Op.";
    strings["Op. Pitch##sgiESFM"].plurals[0] = "Tom Op.";
    strings["Op. Panning##sgiESFM"].plurals[0] = "Balanço Op.";
    strings["Macros##sgiESFM"].plurals[0] = "Macros";
    strings["Volume##sgiESFM"].plurals[0] = "Volume";
    strings["Arpeggio##sgiESFM"].plurals[0] = "Arpejo";
    strings["Pitch##sgiESFM"].plurals[0] = "Tom";
    strings["OP4 Noise Mode##sgiESFM"].plurals[0] = "Modo de Ruído do OP4";
    strings["Panning##sgiESFM"].plurals[0] = "Balanço";
    strings["Phase Reset##sgiESFM"].plurals[0] = "Redef. de Fase";

    //   sgiFDS    src/gui/inst/fds.cpp

    strings["Compatibility mode##sgiFDS"].plurals[0] = "Modo de compatibilidade";
    strings["only use for compatibility with .dmf modules!\n- initializes modulation table with first wavetable\n- does not alter modulation parameters on instrument change##sgiFDS"].plurals[0] = "use apenas para compatibilidade com módulos .dmf!\n- inicializa a tabela de modulação com a primeira wavetable\n- não altera os parâmetros de modulação com mudança de instrumento";
    strings["Modulation depth##sgiFDS"].plurals[0] = "Profundidade de modulação";
    strings["Modulation speed##sgiFDS"].plurals[0] = "Velocidade de modulação";
    strings["Modulation table##sgiFDS"].plurals[0] = "Tabela de modulação";
    strings["Macros##sgiFDS"].plurals[0] = "Macros";
    strings["Volume##sgiFDS"].plurals[0] = "Volume";
    strings["Arpeggio##sgiFDS"].plurals[0] = "Arpejo";
    strings["Pitch##sgiFDS"].plurals[0] = "Tom";
    strings["Waveform##sgiFDS"].plurals[0] = "Forma de onda";
    strings["Mod Depth##sgiFDS"].plurals[0] = "Profundidade Mod.";
    strings["Mod Speed##sgiFDS"].plurals[0] = "Velocidade Mod.";
    strings["Mod Position##sgiFDS"].plurals[0] = "Posição Mod.";

    //   sgifmeu   src/gui/inst/fmEnvUtil.cpp

    strings["left click to restart\nmiddle click to pause\nright click to see algorithm##sgifmeu"].plurals[0] = "clique com o botão esquerdo para reiniciar\nclique com o botão do meio para pausar\nclique com o botão direito para ver o algoritmo";
    strings["left click to configure TL scaling\nright click to see FM preview##sgifmeu"].plurals[0] = "clique com o botão esquerdo para configurar o escalonamento de TL\nclique com o botão direito para ver a previsão da onda FM";
    strings["right click to see FM preview##sgifmeu"].plurals[0] = "clique com o botão direito para ver a previsão da onda FM";
    strings["operator level changes with volume?##sgifmeu"].plurals[0] = "nível do operador muda com o volume?";
    strings["AUTO##OPKVS"].plurals[0] = "AUTO##OPKVS";
    strings["NO##OPKVS"].plurals[0] = "NÃO##OPKVS";
    strings["YES##OPKVS"].plurals[0] = "SIM##OPKVS";

    //   sgifmeh   src/gui/inst/fmEnvUtil.h

    strings["(copying)##sgifmeh"].plurals[0] = "(copiando)";
    strings["(swapping)##sgifmeh"].plurals[0] = "(trocando)";
    strings["- drag to swap operator\n- shift-drag to copy operator##sgifmeh"].plurals[0] = "- arraste para trocar operadores\n- shift+arraste para copiar operador";

    //   sgiGA20   src/gui/inst/ga20.cpp

    strings["Macros##sgiGA20"].plurals[0] = "Macros";
    strings["Volume##sgiGA20"].plurals[0] = "Volume";
    strings["Arpeggio##sgiGA20"].plurals[0] = "Arpejo";
    strings["Pitch##sgiGA20"].plurals[0] = "Tom";
    strings["Phase Reset##sgiGA20"].plurals[0] = "Redef. de Fase";

    //   sgiGB     src/gui/inst/gb.cpp

    strings["Game Boy##sgiGB"].plurals[0] = "Game Boy";
    strings["Use software envelope##sgiGB"].plurals[0] = "Utilizar envelope em software";
    strings["Initialize envelope on every note##sgiGB"].plurals[0] = "Inicializar envelope em cada nota";
    strings["Volume##sgiGB0"].plurals[0] = "Volume";
    strings["Length##sgiGB"].plurals[0] = "Comprimento";
    strings["Sound Length##sgiGB0"].plurals[0] = "Comprimento do som";
    strings["Infinity##sgiGB"].plurals[0] = "Infinito";
    strings["Direction##sgiGB"].plurals[0] = "Direção";
    strings["Up##sgiGB0"].plurals[0] = "Cima";
    strings["Down##sgiGB0"].plurals[0] = "Baixo";
    strings["Hardware Sequence##sgiGB"].plurals[0] = "Sequência de Hardware";
    strings["Tick##sgiGB"].plurals[0] = "Tick";
    strings["Command##sgiGB"].plurals[0] = "Comando";
    strings["Move/Remove##sgiGB"].plurals[0] = "Mover/Remover";
    strings["Volume##sgiGB1"].plurals[0] = "Volume";
    strings["Env Length##sgiGB"].plurals[0] = "Comprimento do Envelope";
    strings["Sound Length##sgiGB1"].plurals[0] = "Comprimento do Som";
    strings["Up##sgiGB1"].plurals[0] = "Cima";
    strings["Down##sgiGB1"].plurals[0] = "Baixo";
    strings["Shift##sgiGB"].plurals[0] = "Deslocar";
    strings["Speed##sgiGB"].plurals[0] = "Velocidade";
    strings["Up##sgiGB2"].plurals[0] = "Cima";
    strings["Down##sgiGB2"].plurals[0] = "Baixo";
    strings["Ticks##sgiGB"].plurals[0] = "Ticks";
    strings["Position##sgiGB"].plurals[0] = "Posição";
    strings["Macros##sgiGB"].plurals[0] = "Macros";
    strings["Volume##sgiGB2"].plurals[0] = "Volume";
    strings["Arpeggio##sgiGB"].plurals[0] = "Arpejo";
    strings["Pitch##sgiGB"].plurals[0] = "Tom";
    strings["Duty/Noise##sgiGB"].plurals[0] = "Ciclo de Trab./Ruído";
    strings["Waveform##sgiGB"].plurals[0] = "Forma de Onda";
    strings["Panning##sgiGB"].plurals[0] = "Balanço";
    strings["Phase Reset##sgiGB"].plurals[0] = "Redef. de Fase";

    //   sgiGBADMA src/gui/inst/gbadma.cpp

    strings["Macros##sgiGBADMA"].plurals[0] = "Macros";
    strings["Volume##sgiGBADMA"].plurals[0] = "Volume";
    strings["Arpeggio##sgiGBADMA"].plurals[0] = "Arpejo";
    strings["Pitch##sgiGBADMA"].plurals[0] = "Tom";
    strings["Waveform##sgiGBADMA"].plurals[0] = "Forma de Onda";
    strings["Panning##sgiGBADMA"].plurals[0] = "Balanço";
    strings["Phase Reset##sgiGBADMA"].plurals[0] = "Redef. de Fase";

    //sgiGBAMINMOD src/gui/inst/gbaminmod.cpp

    strings["Macros##sgiGBAMINMOD"].plurals[0] = "Macros";
    strings["Volume##sgiGBAMINMOD"].plurals[0] = "Volume";
    strings["Arpeggio##sgiGBAMINMOD"].plurals[0] = "Arpejo";
    strings["Pitch##sgiGBAMINMOD"].plurals[0] = "Tom";
    strings["Waveform##sgiGBAMINMOD"].plurals[0] = "Forma de Onda";
    strings["Panning (left)##sgiGBAMINMOD"].plurals[0] = "Balanço (esquerda)";
    strings["Panning (right)##sgiGBAMINMOD"].plurals[0] = "Balanço (direita)";
    strings["Special##sgiGBAMINMOD"].plurals[0] = "Especial";
    strings["Phase Reset##sgiGBAMINMOD"].plurals[0] = "Redef. de Fase";

    //   sgiK00    src/gui/inst/k007232.cpp

    strings["Macros##sgiK00"].plurals[0] = "Macros";
    strings["Volume##sgiK00"].plurals[0] = "Volume";
    strings["Arpeggio##sgiK00"].plurals[0] = "Arpejo";
    strings["Pitch##sgiK00"].plurals[0] = "Tom";
    strings["Waveform##sgiK00"].plurals[0] = "Forma de Onda";
    strings["Panning (left)##sgiK00"].plurals[0] = "Balanço (esquerda)";
    strings["Panning (right)##sgiK00"].plurals[0] = "Balanço (direita)";
    strings["Phase Reset##sgiK00"].plurals[0] = "Redef. de Fase";

    //   sgiK05    src/gui/inst/k053260.cpp

    strings["Macros##sgiK05"].plurals[0] = "Macros";
    strings["Volume##sgiK05"].plurals[0] = "Volume";
    strings["Arpeggio##sgiK05"].plurals[0] = "Arpejo";
    strings["Pitch##sgiK05"].plurals[0] = "Tom";
    strings["Panning##sgiK05"].plurals[0] = "Balanço";
    strings["Phase Reset##sgiK05"].plurals[0] = "Redef. de Fase";

    //   sgimcd    src/gui/inst/macroDraw.cpp

    strings["Triangle##sgimcd"].plurals[0] = "Triângulo";
    strings["Saw##sgimcd"].plurals[0] = "Dente de serra";
    strings["Square##sgimcd"].plurals[0] = "Quadrada";
    strings["How did you even##sgimcd0"].plurals[0] = "Como diabos você...";
    strings["Bottom##sgimcd0"].plurals[0] = "Base";
    strings["Top##sgimcd0"].plurals[0] = "Topo";
    strings["Attack##sgimcd"].plurals[0] = "Ataque";
    strings["Sustain##sgimcd"].plurals[0] = "Sustentação";
    strings["Hold##sgimcd"].plurals[0] = "Retenção";
    strings["SusTime##sgimcd"].plurals[0] = "Tempo Sust.";
    strings["Decay##sgimcd"].plurals[0] = "Decaimento";
    strings["SusDecay##sgimcd"].plurals[0] = "Decaim. Sust.";
    strings["Release##sgimcd"].plurals[0] = "Repouso";
    strings["Bottom##sgimcd1"].plurals[0] = "Base";
    strings["Top##sgimcd1"].plurals[0] = "Topo";
    strings["Speed##sgimcd"].plurals[0] = "Velocidade";
    strings["Phase##sgimcd"].plurals[0] = "Fase";
    strings["Shape##sgimcd"].plurals[0] = "Formato";
    strings["Macro type: Sequence##sgimcd"].plurals[0] = "Tipo de macro: Sequência";
    strings["Macro type: ADSR##sgimcd"].plurals[0] = "Tipo de macro: ADSR";
    strings["Macro type: LFO##sgimcd"].plurals[0] = "Tipo de macro: LFO";
    strings["Macro type: What's going on here?##sgimcd"].plurals[0] = "Tipo de macro: Que porcaria tá acontecendo aqui?";
    strings["Delay/Step Length##sgimcd"].plurals[0] = "Atraso/Tamanho do passo";
    strings["Step Length (ticks)##IMacroSpeed"].plurals[0] = "Tamanho do passo (ticks)##IMacroSpeed";
    strings["Delay##IMacroDelay"].plurals[0] = "Atraso##IMacroDelay";
    strings["Release mode: Active (jump to release pos)##sgimcd"].plurals[0] = "Modo de repouso: Ativo (pula para a posição de repouso)";
    strings["Release mode: Passive (delayed release)##sgimcd"].plurals[0] = "Modo de repouso: Passivo (repouso atrasado)";
    strings["Tabs##sgimcd"].plurals[0] = "Abas";
    strings["Length##sgimcd"].plurals[0] = "Comprimento";
    strings["StepLen##sgimcd"].plurals[0] = "Compr. Passo";
    strings["Delay##sgimcd"].plurals[0] = "Atraso";
    strings["The heck? No, this isn't even working correctly...##sgimcd"].plurals[0] = "Cacildis? Não, não, isso aí tá com problema...";
    strings["The only problem with that selectedMacro is that it's a bug...##sgimcd"].plurals[0] = "O único problema com aquela \"selectedMacro\" é que ela é um bug...";
    strings["Single (combo box)##sgimcd"].plurals[0] = "Única (caixa de combinação)"; // é sério isso?

    //   sgiLYNX   src/gui/inst/mikey.cpp

    strings["Macros##sgiLYNX"].plurals[0] = "Macros";
    strings["Volume##sgiLYNX"].plurals[0] = "Volume";
    strings["Arpeggio##sgiLYNX"].plurals[0] = "Arpejo";
    strings["Pitch##sgiLYNX"].plurals[0] = "Tom";
    strings["Duty/Int##sgiLYNX"].plurals[0] = "Coeficientes/Integ.";
    strings["Panning (left)##sgiLYNX"].plurals[0] = "Balanço (esquerda)";
    strings["Panning (right)##sgiLYNX"].plurals[0] = "Balanço (direita)";
    strings["Phase Reset##sgiLYNX"].plurals[0] = "Redef. de Fase";
    strings["Load LFSR##sgiLYNX"].plurals[0] = "Carregar LFSR";

    //   sgi5232   src/gui/inst/msm5232.cpp

    strings["Macros##sgi5232"].plurals[0] = "Macros";
    strings["Volume##sgi5232"].plurals[0] = "Volume";
    strings["Arpeggio##sgi5232"].plurals[0] = "Arpejo";
    strings["Pitch##sgi5232"].plurals[0] = "Tom";
    strings["Group Ctrl##sgi5232"].plurals[0] = "Controle de Grupo";
    strings["Group Attack##sgi5232"].plurals[0] = "Ataque de Grupo";
    strings["Group Decay##sgi5232"].plurals[0] = "Decaimento de Grupo";
    strings["Noise##sgi5232"].plurals[0] = "Ruído";

    //   sgi6258   src/gui/inst/msm6258.cpp

    strings["Macros##sgi6258"].plurals[0] = "Macros";
    strings["Frequency Divider##sgi6258"].plurals[0] = "Divisor de Frequência";
    strings["Panning##sgi6258"].plurals[0] = "Balanço";
    strings["Phase Reset##sgi6258"].plurals[0] = "Redef. de Fase";
    strings["Clock Divider##sgi6258"].plurals[0] = "Divisor de Clock";

    //   sgi6295   src/gui/inst/msm6295.cpp

    strings["Macros##sgi6295"].plurals[0] = "Macros";
    strings["Volume##sgi6295"].plurals[0] = "Volume";
    strings["Frequency Divider##sgi6295"].plurals[0] = "Divisor de Frequência";
    strings["Phase Reset##sgi6295"].plurals[0] = "Redef. de Fase";

    //   sgiMULPCM src/gui/inst/multipcm.cpp

    strings["MultiPCM##sgiMULPCM"].plurals[0] = "MultiPCM";
    strings["AR##sgiMULPCM0"].plurals[0] = "AR";
    strings["AR##sgiMULPCM1"].plurals[0] = "AR";
    strings["Attack Rate##sgiMULPCM"].plurals[0] = "Taxa de Ataque";
    strings["D1R##sgiMULPCM0"].plurals[0] = "D1R";
    strings["D1R##sgiMULPCM1"].plurals[0] = "D1R";
    strings["Decay 1 Rate##sgiMULPCM"].plurals[0] = "Taxa de Decaimento 1";
    strings["DL##sgiMULPCM0"].plurals[0] = "DL";
    strings["DL##sgiMULPCM1"].plurals[0] = "DL";
    strings["Decay Level##sgiMULPCM"].plurals[0] = "Nível de Decaimento";
    strings["D2R##sgiMULPCM0"].plurals[0] = "D2R";
    strings["D2R##sgiMULPCM1"].plurals[0] = "D2R";
    strings["Decay 2 Rate##sgiMULPCM"].plurals[0] = "Taxa de Decaimento 2";
    strings["RR##sgiMULPCM0"].plurals[0] = "RR";
    strings["RR##sgiMULPCM1"].plurals[0] = "RR";
    strings["Release Rate##sgiMULPCM"].plurals[0] = "Taxa de Repouso";
    strings["RC##sgiMULPCM0"].plurals[0] = "RC";
    strings["RC##sgiMULPCM1"].plurals[0] = "RC";
    strings["Rate Correction##sgiMULPCM"].plurals[0] = "Correção de Taxa";
    strings["Envelope##sgiMULPCM0"].plurals[0] = "Envelope";
    strings["Envelope##sgiMULPCM1"].plurals[0] = "Envelope";
    strings["LFO Rate##sgiMULPCM"].plurals[0] = "Taxa de LFO";
    strings["PM Depth##sgiMULPCM"].plurals[0] = "Profundidade de PM";
    strings["AM Depth##sgiMULPCM"].plurals[0] = "Profundidade de AM";
    strings["Macros##sgiMULPCM"].plurals[0] = "Macros";
    strings["Volume##sgiMULPCM"].plurals[0] = "Volume";
    strings["Arpeggio##sgiMULPCM"].plurals[0] = "Arpejo";
    strings["Pitch##sgiMULPCM"].plurals[0] = "Tom";
    strings["Panning##sgiMULPCM"].plurals[0] = "Balanço";
    strings["Phase Reset##sgiMULPCM"].plurals[0] = "Redef. de Fase";

    //   sgiN163   src/gui/inst/n163.cpp

    strings["Namco 163##sgiN163"].plurals[0] = "Namco 163";
    strings["Load waveform##sgiN163"].plurals[0] = "Carregar wavetable";
    strings["when enabled, a waveform will be loaded into RAM.\nwhen disabled, only the offset and length change.##sgiN163"].plurals[0] = "quando habilitado, uma wavetable será carregada em RAM.\nquando desabilitado, somente o deslocamento e comprimento serão alterados.";
    strings["Waveform##WAVE"].plurals[0] = "Forma de onda##WAVE";
    strings["Per-channel wave position/length##sgiN163"].plurals[0] = "Posição/comprimento de onda por canal";
    strings["Ch##sgiN163"].plurals[0] = "Can";
    strings["Position##sgiN163"].plurals[0] = "Posição";
    strings["Length##sgiN163"].plurals[0] = "Comprimento";
    strings["Position##WAVEPOS"].plurals[0] = "Posição##WAVEPOS";
    strings["Length##WAVELEN"].plurals[0] = "Comprimento##WAVELEN";
    strings["Macros##sgiN163"].plurals[0] = "Macros";
    strings["Volume##sgiN163"].plurals[0] = "Volume";
    strings["Arpeggio##sgiN163"].plurals[0] = "Arpejo";
    strings["Pitch##sgiN163"].plurals[0] = "Tom";
    strings["Wave Pos##sgiN163"].plurals[0] = "Posição da onda";
    strings["Waveform##sgiN163"].plurals[0] = "Forma de onda";
    strings["Wave Length##sgiN163"].plurals[0] = "Comprimento da onda";

    //   sgiWSG    src/gui/inst/namco.cpp

    strings["Macros##sgiWSG"].plurals[0] = "Macros";
    strings["Volume##sgiWSG"].plurals[0] = "Volume";
    strings["Arpeggio##sgiWSG"].plurals[0] = "Arpejo";
    strings["Pitch##sgiWSG"].plurals[0] = "Tom";
    strings["Noise##sgiWSG"].plurals[0] = "Ruído";
    strings["Waveform##sgiWSG"].plurals[0] = "Forma de onda";
    strings["Panning (left)##sgiWSG"].plurals[0] = "Balanço (esquerda)";
    strings["Panning (right)##sgiWSG"].plurals[0] = "Balanço (direita)";

    //   sgiNDS    src/gui/inst/nds.cpp

    strings["Macros##sgiNDS"].plurals[0] = "Macros";
    strings["Volume##sgiNDS"].plurals[0] = "Volume";
    strings["Arpeggio##sgiNDS"].plurals[0] = "Arpejo";
    strings["Pitch##sgiNDS"].plurals[0] = "Tom";
    strings["Duty##sgiNDS"].plurals[0] = "Ciclo de Trab.";
    strings["Panning##sgiNDS"].plurals[0] = "Balanço";
    strings["Phase Reset##sgiNDS"].plurals[0] = "Redef. de Fase";

    //   sgiNES    src/gui/inst/nes.cpp

    strings["Macros##sgiNES"].plurals[0] = "Macros";
    strings["Volume##sgiNES"].plurals[0] = "Volume";
    strings["Arpeggio##sgiNES"].plurals[0] = "Arpejo";
    strings["Pitch##sgiNES"].plurals[0] = "Tom";
    strings["Duty/Noise##sgiNES"].plurals[0] = "Ciclo de Trab./Ruído";
    strings["Phase Reset##sgiNES"].plurals[0] = "Redef. de Fase";

    //   sgiOPL    src/gui/inst/opl.cpp

    strings["4-op##sgiOPL"].plurals[0] = "4-op";
    strings["Drums##sgiOPL"].plurals[0] = "Bateria";
    strings["Fixed frequency mode##sgiOPL"].plurals[0] = "Modo de frequência fixa";
    strings["when enabled, drums will be set to the specified frequencies, ignoring the note.##sgiOPL"].plurals[0] = "quando habilitado, os canais de bateria serão definidos para as frequências especificadas, ignorando a nota.";
    strings["Drum##sgiOPL"].plurals[0] = "Tambor";
    strings["Block##sgiOPL"].plurals[0] = "Bloco";
    strings["FreqNum##sgiOPL"].plurals[0] = "NúmFreq.";
    strings["Kick##sgiOPL0"].plurals[0] = "Bumbo";
    strings["Snare/Hi-hat##sgiOPL"].plurals[0] = "Caixa/Chimbal";
    strings["Tom/Top##sgiOPL"].plurals[0] = "Surdo/Prato";
    strings["Other##sgiOPL0"].plurals[0] = "Outro";
    strings["Other##sgiOPL1"].plurals[0] = "Outro";
    strings["Envelope##sgiOPL0"].plurals[0] = "Envelope";
    strings["Envelope##sgiOPL1"].plurals[0] = "Envelope";
    strings["op%d##sgiOPL0"].plurals[0] = "op%d";
    strings["Kick##sgiOPL1"].plurals[0] = "Bumbo";
    strings["Env##sgiOPL"].plurals[0] = "Env";
    strings["OP%d##sgiOPL1"].plurals[0] = "OP%d";
    strings["OPL2/3 only (last 4 waveforms are OPL3 only)##sgiOPL0"].plurals[0] = "OPL2/OPL3 somente (as últimas 4 ondas são somente para o OPL3)";
    strings["op%d##sgiOPL2"].plurals[0] = "op%d";
    strings["Envelope 2 (kick only)##sgiOPL0"].plurals[0] = "Envelope 2 (bumbo somente)";
    strings["Envelope##sgiOPL2"].plurals[0] = "Envelope";
    strings["Operator %d##sgiOPL"].plurals[0] = "Operador %d";
    strings["Waveform##sgiOPL"].plurals[0] = "Forma de onda";
    strings["Envelope##sgiOPL3"].plurals[0] = "Envelope";
    strings["OPL2/3 only (last 4 waveforms are OPL3 only)##sgiOPL1"].plurals[0] = "OPL2/OPL3 somente (as últimas 4 ondas são somente para o OPL3)";
    strings["op%d##sgiOPL3"].plurals[0] = "op%d";
    strings["Envelope 2 (kick only)##sgiOPL1"].plurals[0] = "Envelope 2 (kick only)";
    strings["Envelope##sgiOPL4"].plurals[0] = "Envelope";
    strings["OP%d##sgiOPL4"].plurals[0] = "OP%d";
    strings["OPL2/3 only (last 4 waveforms are OPL3 only)##sgiOPL2"].plurals[0] = "OPL2/OPL3 somente (as últimas 4 ondas são somente para o OPL3)";
    strings["FM Macros##sgiOPL"].plurals[0] = "Macros FM";
    strings["OP%d Macros##sgiOPL"].plurals[0] = "Macros OP%d";
    strings["Macros##sgiOPL"].plurals[0] = "Macros";
    strings["Volume##sgiOPL"].plurals[0] = "Volume";
    strings["Arpeggio##sgiOPL"].plurals[0] = "Arpejo";
    strings["Pitch##sgiOPL"].plurals[0] = "Tom";
    strings["Panning##sgiOPL"].plurals[0] = "Balanço";
    strings["Phase Reset##sgiOPL"].plurals[0] = "Redef. de Fase";

    //   sgiOPLL   src/gui/inst/opll.cpp

    strings["%s name##sgiOPLL"].plurals[0] = "Nomes do %s";
    strings["Fixed frequency mode##sgiOPLL"].plurals[0] = "Modo de frequência fixa";
    strings["when enabled, drums will be set to the specified frequencies, ignoring the note.##sgiOPLL"].plurals[0] = "quando habilitado, os canais de bateria serão definidos para as frequências especificadas, ignorando a nota.";
    strings["Drum##sgiOPLL"].plurals[0] = "Tambor";
    strings["Block##sgiOPLL"].plurals[0] = "Bloco";
    strings["FreqNum##sgiOPLL"].plurals[0] = "NúmFreq.";
    strings["Kick##sgiOPLL"].plurals[0] = "Bumbo";
    strings["Snare/Hi-hat##sgiOPLL"].plurals[0] = "Caixa/Chimbal";
    strings["Tom/Top##sgiOPLL"].plurals[0] = "Surdo/Prato";
    strings["Volume##TL"].plurals[0] = "Volume##TL";
    strings["this volume slider only works in compatibility (non-drums) system.##sgiOPLL"].plurals[0] = "este slider de volume só funciona no sistema de compatibilidade (não-bateria).";
    strings["Other##sgiOPLL0"].plurals[0] = "Outro";
    strings["Other##sgiOPLL1"].plurals[0] = "Outro";
    strings["Envelope##sgiOPLL0"].plurals[0] = "Envelope";
    strings["Envelope##sgiOPLL1"].plurals[0] = "Envelope";
    strings["op%d##sgiOPLL0"].plurals[0] = "op%d";
    strings["OP%d##sgiOPLL1"].plurals[0] = "OP%d";
    strings["op%d##sgiOPLL2"].plurals[0] = "op%d";
    strings["Operator %d##sgiOPLL"].plurals[0] = "Operador %d";
    strings["Waveform##sgiOPLL"].plurals[0] = "Forma de onda";
    strings["Envelope##sgiOPLL2"].plurals[0] = "Envelope";
    strings["op%d##sgiOPLL3"].plurals[0] = "op%d";
    strings["OP%d##sgiOPLL4"].plurals[0] = "OP%d";
    strings["SSG On##sgiOPLL"].plurals[0] = "Env. Sust."; // quem deu o nome de "SSG On" pra essa checkbox precisava de uma boa noite de sono...
    strings["FM Macros##sgiOPLL"].plurals[0] = "Macros FM";
    strings["OP%d Macros##sgiOPLL"].plurals[0] = "Macros OP%d";
    strings["Macros##sgiOPLL"].plurals[0] = "Macros";
    strings["Volume##sgiOPLL"].plurals[0] = "Volume";
    strings["Arpeggio##sgiOPLL"].plurals[0] = "Arpejo";
    strings["Pitch##sgiOPLL"].plurals[0] = "Tom";
    strings["Patch##sgiOPLL"].plurals[0] = "Timbre";
    strings["Phase Reset##sgiOPLL"].plurals[0] = "Redef. de Fase";

    //   sgiOPM    src/gui/inst/opm.cpp

    strings["Envelope##sgiOPM0"].plurals[0] = "Envelope";
    strings["op%d##sgiOPM0"].plurals[0] = "op%d";
    strings["OP%d##sgiOPM3"].plurals[0] = "OP%d";
    strings["op%d##sgiOPM1"].plurals[0] = "op%d";
    strings["Operator %d##sgiOPM"].plurals[0] = "Operador %d";
    strings["Waveform##sgiOPM"].plurals[0] = "Forma de Onda"; // por que tem waveform no opm? kkkkkk
    strings["Envelope##sgiOPM1"].plurals[0] = "Envelope";
    strings["op%d##sgiOPM2"].plurals[0] = "op%d";
    strings["OP%d##sgiOPM4"].plurals[0] = "OP%d";
    strings["FM Macros##sgiOPM"].plurals[0] = "Macros FM";
    strings["AM Depth##sgiOPM"].plurals[0] = "Profundidade AM";
    strings["PM Depth##sgiOPM"].plurals[0] = "Profundidade PM";
    strings["LFO Speed##sgiOPM"].plurals[0] = "Velocidade do LFO";
    strings["LFO Shape##sgiOPM"].plurals[0] = "Formato do LFO";
    strings["OpMask##sgiOPM"].plurals[0] = "Máscara de Op.";
    strings["OP%d Macros##sgiOPM"].plurals[0] = "Macros OP%d";
    strings["Macros##sgiOPM"].plurals[0] = "Macros";
    strings["Volume##sgiOPM"].plurals[0] = "Volume";
    strings["Arpeggio##sgiOPM"].plurals[0] = "Arpejo";
    strings["Pitch##sgiOPM"].plurals[0] = "Tom";
    strings["Noise Freq##sgiOPM"].plurals[0] = "Freq. Ruído";
    strings["Panning##sgiOPM"].plurals[0] = "Balanço";
    strings["Phase Reset##sgiOPM"].plurals[0] = "Redef. de Fase";

    //   sgiOPN    src/gui/inst/opn.cpp

    strings["Envelope##sgiOPN0"].plurals[0] = "Envelope";
    strings["Envelope##sgiOPN1"].plurals[0] = "Envelope";
    strings["op%d##sgiOPN0"].plurals[0] = "op%d";
    strings["OP%d##sgiOPN1"].plurals[0] = "OP%d";
    strings["op%d##sgiOPN2"].plurals[0] = "op%d";
    strings["Operator %d##sgiOPN"].plurals[0] = "Operador %d";
    strings["SSG-EG##sgiOPN"].plurals[0] = "SSG-EG";
    strings["Envelope##sgiOPN2"].plurals[0] = "Envelope";
    strings["op%d##sgiOPN3"].plurals[0] = "op%d";
    strings["OP%d##sgiOPN4"].plurals[0] = "OP%d";
    strings["SSG On##sgiOPN"].plurals[0] = "SSG On";
    strings["FM Macros##sgiOPN"].plurals[0] = "Macros FM";
    strings["LFO Speed##sgiOPN"].plurals[0] = "Velocidade LFO";
    strings["OpMask##sgiOPN"].plurals[0] = "Máscara de Op.";
    strings["OP%d Macros##sgiOPN"].plurals[0] = "Macros OP%d";
    strings["Macros##sgiOPN"].plurals[0] = "Macros";
    strings["Volume##sgiOPN"].plurals[0] = "Volume";
    strings["Arpeggio##sgiOPN"].plurals[0] = "Arpejo";
    strings["Pitch##sgiOPN"].plurals[0] = "Tom";
    strings["Panning##sgiOPN"].plurals[0] = "Balanço";
    strings["Phase Reset##sgiOPN"].plurals[0] = "Redef. de Fase";

    //   sgiOPZ    src/gui/inst/opz.cpp

    strings["Request from TX81Z##sgiOPZ"].plurals[0] = "Requisitar do TX81Z";
    strings["Other##sgiOPZ0"].plurals[0] = "Outro";
    strings["Other##sgiOPZ1"].plurals[0] = "Outro";
    strings["Envelope##sgiOPZ0"].plurals[0] = "Envelope";
    strings["Envelope##sgiOPZ1"].plurals[0] = "Envelope";
    strings["op%d##sgiOPZ0"].plurals[0] = "op%d";
    strings["OP%d##sgiOPZ1"].plurals[0] = "OP%d";
    strings["Fixed##sgiOPZ0"].plurals[0] = "F.Fixa";
    strings["Block##sgiOPZ0"].plurals[0] = "Bloco";
    strings["FreqNum##sgiOPZ0"].plurals[0] = "NúmFreq.";
    strings["op%d##sgiOPZ2"].plurals[0] = "op%d";
    strings["Operator %d##sgiOPZ"].plurals[0] = "Operador %d";
    strings["Waveform##sgiOPZ"].plurals[0] = "Forma de Onda";
    strings["Envelope##sgiOPZ2"].plurals[0] = "Envelope";
    strings["Block##sgiOPZ1"].plurals[0] = "Bloco";
    strings["Freq##sgiOPZ"].plurals[0] = "Freq.";
    strings["Fixed##sgiOPZ1"].plurals[0] = "F.Fixa";
    strings["op%d##sgiOPZ3"].plurals[0] = "op%d";
    strings["OP%d##sgiOPZ4"].plurals[0] = "OP%d";
    strings["Fixed##sgiOPZ2"].plurals[0] = "F.Fixa";
    strings["Block##sgiOPZ2"].plurals[0] = "Bloco";
    strings["FreqNum##sgiOPZ1"].plurals[0] = "NúmFreq.";
    strings["FM Macros##sgiOPZ"].plurals[0] = "Macros FM";
    strings["AM Depth##sgiOPZ"].plurals[0] = "Profundidade AM";
    strings["PM Depth##sgiOPZ"].plurals[0] = "Profundidade PM";
    strings["LFO Speed##sgiOPZ"].plurals[0] = "Velocidade do LFO";
    strings["LFO Shape##sgiOPZ"].plurals[0] = "Formato do LFO";
    strings["AM Depth 2##sgiOPZ"].plurals[0] = "Profundidade do AM 2";
    strings["PM Depth 2##sgiOPZ"].plurals[0] = "Profundidade do PM 2";
    strings["LFO2 Speed##sgiOPZ"].plurals[0] = "Velocidade do LFO2";
    strings["LFO2 Shape##sgiOPZ"].plurals[0] = "Formato do LFO2";
    strings["OP%d Macros##sgiOPZ"].plurals[0] = "Macros OP%d";
    strings["Macros##sgiOPZ"].plurals[0] = "Macros";
    strings["Volume##sgiOPZ"].plurals[0] = "Volume";
    strings["Arpeggio##sgiOPZ"].plurals[0] = "Arpejo";
    strings["Pitch##sgiOPZ"].plurals[0] = "Tom";
    strings["Noise Freq##sgiOPZ"].plurals[0] = "Freq. Ruído";
    strings["Panning##sgiOPZ"].plurals[0] = "Balanço";
    strings["Phase Reset##sgiOPZ"].plurals[0] = "Redef. de Fase";

    //   sgiPCE    src/gui/inst/pce.cpp

    strings["Macros##sgiPCE"].plurals[0] = "Macros";
    strings["Volume##sgiPCE"].plurals[0] = "Volume";
    strings["Arpeggio##sgiPCE"].plurals[0] = "Arpejo";
    strings["Pitch##sgiPCE"].plurals[0] = "Tom";
    strings["Noise##sgiPCE"].plurals[0] = "Ruído";
    strings["Waveform##sgiPCE"].plurals[0] = "Forma de Onda";
    strings["Panning (left)##sgiPCE"].plurals[0] = "Balanço (esquerda)";
    strings["Panning (right)##sgiPCE"].plurals[0] = "Balanço (direita)";
    strings["Phase Reset##sgiPCE"].plurals[0] = "Redef. de Fase";

    //   sgiPET    src/gui/inst/pet.cpp

    strings["Macros##sgiPET"].plurals[0] = "Macros";
    strings["Volume##sgiPET"].plurals[0] = "Volume";
    strings["Arpeggio##sgiPET"].plurals[0] = "Arpejo";
    strings["Pitch##sgiPET"].plurals[0] = "Tom";
    strings["Waveform##sgiPET"].plurals[0] = "Forma de Onda";

    //   sgiPMQT   src/gui/inst/pokemini.cpp

    strings["Macros##sgiPMQT"].plurals[0] = "Macros";
    strings["Volume##sgiPMQT"].plurals[0] = "Volume";
    strings["Arpeggio##sgiPMQT"].plurals[0] = "Arpejo";
    strings["Pitch##sgiPMQT"].plurals[0] = "Tom";
    strings["Pulse Width##sgiPMQT"].plurals[0] = "Largura de Pulso";

    //   sgiPOKEY  src/gui/inst/pokey.cpp

    strings["16-bit raw period macro##sgiPOKEY"].plurals[0] = "Macro de período 16-bit (val. registrador)";
    strings["Macros##sgiPOKEY"].plurals[0] = "Macros";
    strings["Volume##sgiPOKEY"].plurals[0] = "Volume";
    strings["Arpeggio##sgiPOKEY"].plurals[0] = "Arpejo";
    strings["Pitch##sgiPOKEY"].plurals[0] = "Tom";
    strings["Waveform##sgiPOKEY"].plurals[0] = "Forma de Onda";
    strings["Raw Period##sgiPOKEY"].plurals[0] = "Período (val. registrador)";

    //   sgiPNN    src/gui/inst/powernoise.cpp

    strings["Octave offset##sgiPNN"].plurals[0] = "Compensação de oitava";
    strings["go to Macros for other parameters.##sgiPNN"].plurals[0] = "vá para Macros para outros parâmetros.";
    strings["Macros##sgiPNN"].plurals[0] = "Macros";
    strings["Volume##sgiPNN"].plurals[0] = "Volume";
    strings["Arpeggio##sgiPNN"].plurals[0] = "Arpejo";
    strings["Pitch##sgiPNN"].plurals[0] = "Tom";
    strings["Panning (left)##sgiPNN"].plurals[0] = "Balanço (esquerda)";
    strings["Panning (right)##sgiPNN"].plurals[0] = "Balanço (direita)";
    strings["Phase Reset##sgiPNN"].plurals[0] = "Redef. de Fase";
    strings["Control##sgiPNN"].plurals[0] = "Controle";
    strings["Tap A Location##sgiPNN"].plurals[0] = "Local do Tap A";
    strings["Tap B Location##sgiPNN"].plurals[0] = "Local do Tap B";
    strings["Load LFSR##sgiPNN"].plurals[0] = "Carregar LFSR";

    //   sgiPNS    src/gui/inst/powernoise_slope.cpp

    strings["Octave offset##sgiPNS"].plurals[0] = "Compensação de oitava";
    strings["go to Macros for other parameters.##sgiPNS"].plurals[0] = "vá para Macros para outros parâmetros.";
    strings["Macros##sgiPNS"].plurals[0] = "Macros";
    strings["Volume##sgiPNS"].plurals[0] = "Volume";
    strings["Arpeggio##sgiPNS"].plurals[0] = "Arpejo";
    strings["Pitch##sgiPNS"].plurals[0] = "Tom";
    strings["Panning (left)##sgiPNS"].plurals[0] = "Balanço (esquerda)";
    strings["Panning (right)##sgiPNS"].plurals[0] = "Balanço (direita)";
    strings["Phase Reset##sgiPNS"].plurals[0] = "Redef. de Fase";
    strings["Control##sgiPNS"].plurals[0] = "Controle";
    strings["Portion A Length##sgiPNS"].plurals[0] = "Comprimento da Porção A";
    strings["Portion B Length##sgiPNS"].plurals[0] = "Comprimento da Porção B";
    strings["Portion A Offset##sgiPNS"].plurals[0] = "Deslocamento da Porção A";
    strings["Portion B Offset##sgiPNS"].plurals[0] = "Deslocamento da Porção B";

    //   sgiPV     src/gui/inst/pv1000.cpp

    strings["Macros##sgiPV"].plurals[0] = "Macros";
    strings["Volume##sgiPV"].plurals[0] = "Volume";
    strings["Arpeggio##sgiPV"].plurals[0] = "Arpejo";
    strings["Pitch##sgiPV"].plurals[0] = "Tom";
    strings["Raw Frequency##sgiPV"].plurals[0] = "Frequência (val. registrador)";

    //   sgiQ      src/gui/inst/qsound.cpp

    strings["Macros##sgiQ"].plurals[0] = "Macros";
    strings["Volume##sgiQ"].plurals[0] = "Volume";
    strings["Arpeggio##sgiQ"].plurals[0] = "Arpejo";
    strings["Pitch##sgiQ"].plurals[0] = "Tom";
    strings["Panning##sgiQ"].plurals[0] = "Balanço";
    strings["Surround##sgiQ"].plurals[0] = "Surround";
    strings["Phase Reset##sgiQ"].plurals[0] = "Redef. de Fase";
    strings["Echo Level##sgiQ"].plurals[0] = "Nível de Eco";
    strings["Echo Feedback##sgiQ"].plurals[0] = "Retorno do Eco";
    strings["Echo Length##sgiQ"].plurals[0] = "Atraso do Eco";

    //   sgiRF5    src/gui/inst/rf5c68.cpp

    strings["Macros##sgiRF5"].plurals[0] = "Macros";
    strings["Volume##sgiRF5"].plurals[0] = "Volume";
    strings["Arpeggio##sgiRF5"].plurals[0] = "Arpejo";
    strings["Pitch##sgiRF5"].plurals[0] = "Tom";
    strings["Panning (left)##sgiRF5"].plurals[0] = "Balanço (esquerda)";
    strings["Panning (right)##sgiRF5"].plurals[0] = "Balanço (direita)";
    strings["Phase Reset##sgiRF5"].plurals[0] = "Redef. de Fase";

    //   sgiSAA    src/gui/inst/saa1099.cpp

    strings["Macros##sgiSAA"].plurals[0] = "Macros";
    strings["Volume##sgiSAA"].plurals[0] = "Volume";
    strings["Arpeggio##sgiSAA"].plurals[0] = "Arpejo";
    strings["Pitch##sgiSAA"].plurals[0] = "Tom";
    strings["Duty/Noise##sgiSAA"].plurals[0] = "Ciclo de Trab./Ruído";
    strings["Waveform##sgiSAA"].plurals[0] = "Forma de Onda";
    strings["Panning (left)##sgiSAA"].plurals[0] = "Balanço (esquerda)";
    strings["Panning (right)##sgiSAA"].plurals[0] = "Balanço (direita)";
    strings["Envelope##sgiSAA"].plurals[0] = "Envelope";

    //   sgismpd   src/gui/inst/sampleDraw.cpp

    strings["Sample##sgismpd0"].plurals[0] = "Sample";
    strings["DPCM##sgismpd"].plurals[0] = "DPCM";
    strings["new DPCM features disabled (compatibility)!##sgismpd"].plurals[0] = "novos recursos de DPCM desabilitados (compatibilidade)!";
    strings["click here to enable them.##sgismpd"].plurals[0] = "clique aqui para habilitá-los.";
    strings["none selected##sgismpd"].plurals[0] = "nenhuma selecionada";
    strings["Use sample##sgismpd"].plurals[0] = "Usar amostra";
    strings["Sample bank slot##BANKSLOT"].plurals[0] = "Vaga do banco de samples##BANKSLOT";
    strings["Sample##sgismpd1"].plurals[0] = "Sample";
    strings["Use wavetable (Amiga/Generic DAC only)##sgismpd"].plurals[0] = "Utilizar wavetable (Amiga/DAC Genérico somente)";
    strings["Use wavetable##sgismpd"].plurals[0] = "Utilizar wavetable";
    strings["Width##sgismpd"].plurals[0] = "Largura";
    strings["Use sample map##sgismpd"].plurals[0] = "Utilizar mapa de samples";
    strings["pitch##sgismpd"].plurals[0] = "tom";
    strings["loop##sgismpd"].plurals[0] = "repetir";
    strings["delta##sgismpd"].plurals[0] = "delta";
    strings["note##sgismpd"].plurals[0] = "nota";
    strings["sample name##sgismpd"].plurals[0] = "nome da sample";
    strings["L##L%d"].plurals[0] = "R##L%d";
    strings["set entire map to this pitch##sgismpd"].plurals[0] = "definir mapa inteiro para esse tom";
    strings["set entire map to this delta counter value##sgismpd"].plurals[0] = "definir mapa inteiro para esse valor de contador delta";
    strings["set entire map to this note##sgismpd"].plurals[0] = "definir mapa inteiro para essa nota";
    strings["set entire map to this sample##sgismpd"].plurals[0] = "definir mapa inteiro para essa sample";
    strings["reset pitches##sgismpd"].plurals[0] = "redefinir tons";
    strings["clear delta counter values##sgismpd"].plurals[0] = "limpar valores de contador delta";
    strings["reset notes##sgismpd"].plurals[0] = "redefinir notas";
    strings["clear map samples##sgismpd"].plurals[0] = "limpar samples do mapa";

    //   sgiSCC    src/gui/inst/scc.cpp

    strings["Macros##sgiSCC"].plurals[0] = "Macros";
    strings["Volume##sgiSCC"].plurals[0] = "Volume";
    strings["Arpeggio##sgiSCC"].plurals[0] = "Arpejo";
    strings["Pitch##sgiSCC"].plurals[0] = "Tom";
    strings["Waveform##sgiSCC"].plurals[0] = "Forma de Onda";

    //   sgiSEGA   src/gui/inst/segapcm.cpp

    strings["Macros##sgiSEGA"].plurals[0] = "Macros";
    strings["Volume##sgiSEGA"].plurals[0] = "Volume";
    strings["Arpeggio##sgiSEGA"].plurals[0] = "Arpejo";
    strings["Pitch##sgiSEGA"].plurals[0] = "Tom";
    strings["Panning (left)##sgiSEGA"].plurals[0] = "Balanço (esquerda)";
    strings["Panning (right)##sgiSEGA"].plurals[0] = "Balanço (direita)";
    strings["Phase Reset##sgiSEGA"].plurals[0] = "Redef. de Fase";
    
    //   sgiSID2   src/gui/inst/sid2.cpp

    strings["Waveform##sgiSID2"].plurals[0] = "Forma de Onda";
    strings["tri##sgiSID2"].plurals[0] = "tri";
    strings["saw##sgiSID2"].plurals[0] = "serra";
    strings["pulse##sgiSID2"].plurals[0] = "pulso";
    strings["noise##sgiSID2"].plurals[0] = "ruído";
    strings["A##sgiSID2"].plurals[0] = "A";
    strings["D##sgiSID2"].plurals[0] = "D";
    strings["S##sgiSID2"].plurals[0] = "S";
    strings["R##sgiSID2"].plurals[0] = "R";
    strings["VOL##sgiSID2"].plurals[0] = "VOL";
    strings["Envelope##sgiSID2"].plurals[0] = "Envelope";
    strings["Duty##sgiSID2"].plurals[0] = "Ciclo de Trabalho";
    strings["Ring Modulation##sgiSID2"].plurals[0] = "Modulação de Anel";
    strings["Oscillator Sync##sgiSID2"].plurals[0] = "Sincronização de Osciladores";
    strings["Enable filter##sgiSID2"].plurals[0] = "Habilitar filtro";
    strings["Initialize filter##sgiSID2"].plurals[0] = "Inicializar filtro";
    strings["Cutoff##sgiSID2"].plurals[0] = "Ponto de corte";
    strings["Resonance##sgiSID2"].plurals[0] = "Ressonância";
    strings["Filter Mode##sgiSID2"].plurals[0] = "Modo de filtro";
    strings["low##sgiSID2"].plurals[0] = "baixa";
    strings["band##sgiSID2"].plurals[0] = "banda";
    strings["high##sgiSID2"].plurals[0] = "alta";
    strings["Noise Mode##sgiSID2"].plurals[0] = "Modo de Ruído";
    strings["Wave Mix Mode##sgiSID2"].plurals[0] = "Modo de Mistura de Ondas";
    strings["Absolute Cutoff Macro##sgiSID2"].plurals[0] = "Macro de Ponto de Corte Absoluta";
    strings["Absolute Duty Macro##sgiSID2"].plurals[0] = "Macro de Ciclo de Trabalho Absoluta";
    strings["Macros##sgiSID2"].plurals[0] = "Macros";
    strings["Volume##sgiSID2"].plurals[0] = "Volume";
    strings["Arpeggio##sgiSID2"].plurals[0] = "Arpejo";
    strings["Pitch##sgiSID2"].plurals[0] = "Tom";
    strings["Duty##sgiSID21"].plurals[0] = "Ciclo de Trabalho";
    strings["Waveform##sgiSID21"].plurals[0] = "Forma de Onda";
    strings["Noise Mode##sgiSID21"].plurals[0] = "Modo de Ruído";
    strings["Wave Mix Mode##sgiSID21"].plurals[0] = "Modo de Mistura de Ondas";
    strings["Cutoff##sgiSID21"].plurals[0] = "Ponto de Corte";
    strings["Filter Mode##sgiSID21"].plurals[0] = "Modo de Filtro";
    strings["Filter Toggle##sgiSID2"].plurals[0] = "Alternar Filtro";
    strings["Resonance##sgiSID21"].plurals[0] = "Ressonância";
    strings["Phase Reset##sgiSID2"].plurals[0] = "Redef. de Fase";
    strings["Envelope Reset/Key Control##sgiSID2"].plurals[0] = "Reinício de Envelope/Controle de Chaveamento";
    strings["Special##sgiSID2"].plurals[0] = "Especial";
    strings["Attack##sgiSID2"].plurals[0] = "Ataque";
    strings["Decay##sgiSID2"].plurals[0] = "Decaimento";
    strings["Sustain##sgiSID2"].plurals[0] = "Sustentação";
    strings["Release##sgiSID2"].plurals[0] = "Repouso";

    //   sgiSM     src/gui/inst/sm8521.cpp

    strings["Macros##sgiSM"].plurals[0] = "Macros";
    strings["Volume##sgiSM"].plurals[0] = "Volume";
    strings["Arpeggio##sgiSM"].plurals[0] = "Arpejo";
    strings["Pitch##sgiSM"].plurals[0] = "Tom";
    strings["Waveform##sgiSM"].plurals[0] = "Forma de Onda";

    //   sgiSNES   src/gui/inst/snes.cpp

    strings["Use envelope##sgiSNES"].plurals[0] = "Usar envelope";
    strings["Envelope##sgiSNES0"].plurals[0] = "Envelope";
    strings["Envelope##sgiSNES1"].plurals[0] = "Envelope";
    strings["Sustain/release mode:##sgiSNES"].plurals[0] = "Modo de sustentação/repouso:";
    strings["Direct (cut on release)##sgiSNES"].plurals[0] = "Direto (cortar no repouso)";
    strings["Effective (linear decrease)##sgiSNES"].plurals[0] = "Efetivo (redução linear)";
    strings["Effective (exponential decrease)##sgiSNES"].plurals[0] = "Efetivo (redução exponencial)";
    strings["Delayed (write R on release)##sgiSNES"].plurals[0] = "Atrasado (escrever R no momento do repouso)";
    strings["Gain Mode##sgiSNES0"].plurals[0] = "Modo de Ganho";
    strings["Gain Mode##sgiSNES1"].plurals[0] = "Modo de Ganho";
    strings["Gain##sgiSNES0"].plurals[0] = "Ganho";
    strings["Gain##sgiSNES1"].plurals[0] = "Ganho";
    strings["Direct##sgiSNES"].plurals[0] = "Direto";
    strings["Decrease (linear)##sgiSNES"].plurals[0] = "Diminuir (linear)";
    strings["Decrease (logarithmic)##sgiSNES"].plurals[0] = "Diminuir (logarítmico)";
    strings["Increase (linear)##sgiSNES"].plurals[0] = "Aumentar (linear)";
    strings["Increase (bent line)##sgiSNES"].plurals[0] = "Aumentar (linha torta)";
    strings["using decrease modes will not produce any sound at all, unless you know what you are doing.\nit is recommended to use the Gain macro for decrease instead.##sgiSNES"].plurals[0] = "utilizar modos de diminuição não irá produzir som nenhum, a não ser que você saiba o que está fazendo.\né recomendado utilizar a macro de Ganho para diminuição ao invés disso.";
    strings["Macros##sgiSNES"].plurals[0] = "Macros";
    strings["Volume##sgiSNES"].plurals[0] = "Volume";
    strings["Arpeggio##sgiSNES"].plurals[0] = "Arpejo";
    strings["Pitch##sgiSNES"].plurals[0] = "Tom";
    strings["Noise Freq##sgiSNES"].plurals[0] = "Freq. Ruído";
    strings["Waveform##sgiSNES"].plurals[0] = "Forma de Onda";
    strings["Panning (left)##sgiSNES"].plurals[0] = "Balanço (esquerda)";
    strings["Panning (right)##sgiSNES"].plurals[0] = "Balanço (direita)";
    strings["Special##sgiSNES"].plurals[0] = "Especial";
    strings["Gain##sgiSNES2"].plurals[0] = "Ganho";

    //   sgiPSG    src/gui/inst/std.cpp

    strings["Macros##sgiPSG"].plurals[0] = "Macros";
    strings["Volume##sgiPSG"].plurals[0] = "Volume";
    strings["Arpeggio##sgiPSG"].plurals[0] = "Arpejo";
    strings["Pitch##sgiPSG"].plurals[0] = "Tom";
    strings["Duty##sgiPSG"].plurals[0] = "Ciclo de Trabalho"; // na verdade nesses chips é qualquer coisa menos ciclo de trabalho... acho isso tão besta, viu?
    strings["Panning##sgiPSG"].plurals[0] = "Balanço";
    strings["Phase Reset##sgiPSG"].plurals[0] = "Redef. de Fase";

    //   sgistru   src/gui/inst/stringsUtil.cpp

    strings["Down Down Down##sgistru"].plurals[0] = "Baixo Baixo Baixo";
    strings["Down.##sgistru"].plurals[0] = "Baixo.";
    strings["Down Up Down Up##sgistru"].plurals[0] = "Baixo Cima Baixo Cima";
    strings["Down UP##sgistru"].plurals[0] = "Baixo CIMA";
    strings["Up Up Up##sgistru"].plurals[0] = "Cima Cima Cima";
    strings["Up.##sgistru"].plurals[0] = "Cima.";
    strings["Up Down Up Down##sgistru"].plurals[0] = "Cima Baixo Cima Baixo";
    strings["Up DOWN##sgistru"].plurals[0] = "Cima BAIXO";
    strings["Algorithm##sgistru"].plurals[0] = "Algoritmo";
    strings["Feedback##sgistru"].plurals[0] = "Feedback";
    strings["LFO > Freq##sgistru"].plurals[0] = "LFO > Freq";
    strings["LFO > Amp##sgistru"].plurals[0] = "LFO > Amp";
    strings["Attack##sgistru"].plurals[0] = "Ataque";
    strings["Decay##sgistru"].plurals[0] = "Decaimento";
    strings["Decay 2##sgistru"].plurals[0] = "Decaimento 2";
    strings["Release##sgistru"].plurals[0] = "Repouso";
    strings["Sustain##sgistru0"].plurals[0] = "Sustentação";
    strings["Level##sgistru"].plurals[0] = "Nível";
    strings["EnvScale##sgistru"].plurals[0] = "EscalaEnv"; // TODO melhorar; se alguém tiver uma ideia melhor do que colocar que couber no espaço, me fale ou abra um PR
    strings["Multiplier##sgistru"].plurals[0] = "Multiplicador";
    strings["Detune##sgistru0"].plurals[0] = "Desafinação";
    strings["Detune 2##sgistru"].plurals[0] = "Desafinação 2";
    strings["SSG-EG##sgistru"].plurals[0] = "SSG-EG";
    strings["AM Depth##sgistru"].plurals[0] = "Profundidade AM";
    strings["Vibrato Depth##sgistru"].plurals[0] = "Profundidade Vibrato";
    strings["Sustained##sgistru0"].plurals[0] = "Env. Sustentado";
    strings["Sustained##sgistru1"].plurals[0] = "Env. Sustentado";
    strings["Level Scaling##sgistru"].plurals[0] = "Escalonamento de Nível";
    strings["Sustain##sgistru1"].plurals[0] = "Sustentação";
    strings["Vibrato##sgistru"].plurals[0] = "Vibrato";
    strings["Waveform##sgistru"].plurals[0] = "Forma de Onda";
    strings["Scale Rate##sgistru"].plurals[0] = "Escala de Taxas";
    strings["OP2 Half Sine##sgistru"].plurals[0] = "OP2 Meio Seno";
    strings["OP1 Half Sine##sgistru"].plurals[0] = "OP1 Meio Seno";
    strings["EnvShift##sgistru"].plurals[0] = "DeslocEnv"; // TODO melhorar; se alguém tiver uma ideia melhor do que colocar que couber no espaço, me fale ou abra um PR
    strings["Reverb##sgistru"].plurals[0] = "Reverb";
    strings["Fine##sgistru0"].plurals[0] = "Fino";
    strings["LFO2 > Freq##sgistru"].plurals[0] = "LFO2 > Freq";
    strings["LFO2 > Amp##sgistru"].plurals[0] = "LFO2 > Amp";
    strings["Fine##sgistru1"].plurals[0] = "Fino";
    strings["Fine##sgistru2"].plurals[0] = "Fino";
    strings["OP4 Noise Mode##sgistru0"].plurals[0] = "Modo de Ruído do OP4";
    strings["Envelope Delay##sgistru"].plurals[0] = "Atraso de Envelope";
    strings["Output Level##sgistru0"].plurals[0] = "Nível de Saída";
    strings["Modulation Input Level##sgistru"].plurals[0] = "Nível de Entrada de Modulação";
    strings["Left Output##sgistru"].plurals[0] = "Saída Esquerda";
    strings["Right Output##sgistru"].plurals[0] = "Saída Direita";
    strings["Coarse Tune (semitones)##sgistru"].plurals[0] = "Afinação Grossa (semitons)";
    strings["Detune##sgistru1"].plurals[0] = "Desafinação";
    strings["Fixed Frequency Mode##sgistru"].plurals[0] = "Modo de Frequência Fixa";
    strings["OP4 Noise Mode##sgistru1"].plurals[0] = "Modo de Ruído do OP4";
    strings["Env. Delay##sgistru"].plurals[0] = "Atraso Env.";
    strings["Output Level##sgistru1"].plurals[0] = "Nív. Saída";
    strings["ModInput##sgistru"].plurals[0] = "EntraMod.";
    strings["Left##sgistru"].plurals[0] = "Esq.";
    strings["Right##sgistru"].plurals[0] = "Dir.";
    strings["Tune##sgistru"].plurals[0] = "Afin.";
    strings["Detune##sgistru2"].plurals[0] = "Desaf.";
    strings["Fixed##sgistru0"].plurals[0] = "F.Fixa";
    strings["Fine##sgistru3"].plurals[0] = "Fina";
    strings["Fine##sgistru4"].plurals[0] = "Fina";
    strings["Fine##sgistru5"].plurals[0] = "Fina";
    strings["User##sgistru0"].plurals[0] = "Usuário";
    strings["1. Violin##sgistru"].plurals[0] = "1. Violino";
    strings["2. Guitar##sgistru0"].plurals[0] = "2. Violão";
    strings["3. Piano##sgistru0"].plurals[0] = "3. Piano";
    strings["4. Flute##sgistru0"].plurals[0] = "4. Flauta";
    strings["5. Clarinet##sgistru0"].plurals[0] = "5. Clarinete";
    strings["6. Oboe##sgistru"].plurals[0] = "6. Oboé";
    strings["7. Trumpet##sgistru0"].plurals[0] = "7. Trompete";
    strings["8. Organ##sgistru"].plurals[0] = "8. Órgão";
    strings["9. Horn##sgistru"].plurals[0] = "9. Trompa";
    strings["10. Synth##sgistru"].plurals[0] = "10. Sintetizador";
    strings["11. Harpsichord##sgistru"].plurals[0] = "11. Cravo";
    strings["12. Vibraphone##sgistru0"].plurals[0] = "12. Vibrafone";
    strings["13. Synth Bass##sgistru"].plurals[0] = "13. Baixo Sintético";
    strings["14. Acoustic Bass##sgistru"].plurals[0] = "14. Baixo Acústico";
    strings["15. Electric Guitar##sgistru"].plurals[0] = "15. Guitarra Elétrica";
    strings["Drums##sgistru0"].plurals[0] = "Bateria";
    strings["User##sgistru1"].plurals[0] = "Usuário";
    strings["1. Electric String##sgistru"].plurals[0] = "1. Cordas Elétricas";
    strings["2. Bow wow##sgistru"].plurals[0] = "2. Bow wow";
    strings["3. Electric Guitar##sgistru0"].plurals[0] = "3. Guitarra Elétrica"; // nada a ver com a guitarra elétrica do YM2413 lol
    strings["4. Organ##sgistru"].plurals[0] = "4. Órgão";
    strings["5. Clarinet##sgistru1"].plurals[0] = "5. Clarinete";
    strings["6. Saxophone##sgistru"].plurals[0] = "6. Saxofone";
    strings["7. Trumpet##sgistru1"].plurals[0] = "7. Trompete";
    strings["8. Street Organ##sgistru"].plurals[0] = "8. Órgão de Rua";
    strings["9. Synth Brass##sgistru"].plurals[0] = "9. Synth Brass";
    strings["10. Electric Piano##sgistru"].plurals[0] = "10. Piano Elétrico"; // parece mais um piano de brinquedo kkk
    strings["11. Bass##sgistru"].plurals[0] = "11. Baixo";
    strings["12. Vibraphone##sgistru1"].plurals[0] = "12. Vibrafone";
    strings["13. Chime##sgistru"].plurals[0] = "13. Campainha";
    strings["14. Tom Tom II##sgistru"].plurals[0] = "14. Surdo 2";
    strings["15. Noise##sgistru"].plurals[0] = "15. Ruído";
    strings["Drums##sgistru1"].plurals[0] = "Bateria";
    strings["User##sgistru2"].plurals[0] = "Usuário";
    strings["1. Strings##sgistru"].plurals[0] = "1. Cordas";
    strings["2. Guitar##sgistru1"].plurals[0] = "2. Violão"; // parece mais uma guitarra clean com pedal de efeito de tremolo...
    strings["3. Electric Guitar##sgistru1"].plurals[0] = "3. Guitarra Elétrica";
    strings["4. Electric Piano##sgistru"].plurals[0] = "4. Piano Elétrico"; // eu não sei que diabos de som é esse
    strings["5. Flute##sgistru"].plurals[0] = "5. Flauta";
    strings["6. Marimba##sgistru"].plurals[0] = "6. Marimba";
    strings["7. Trumpet##sgistru2"].plurals[0] = "7. Trompete";
    strings["8. Harmonica##sgistru"].plurals[0] = "8. Gaita";
    strings["9. Tuba##sgistru"].plurals[0] = "9. Tuba";
    strings["10. Synth Brass##sgistru"].plurals[0] = "10. Synth Brass";
    strings["11. Short Saw##sgistru"].plurals[0] = "11. Serra Curta";
    strings["12. Vibraphone##sgistru2"].plurals[0] = "12. Vibrafone";
    strings["13. Electric Guitar 2##sgistru"].plurals[0] = "13. Guitarra Elétrica 2";
    strings["14. Synth Bass##sgistru"].plurals[0] = "14. Baixo Sintético";
    strings["15. Sitar##sgistru"].plurals[0] = "15. Cítara";
    strings["Drums##sgistru2"].plurals[0] = "Bateria";
    strings["User##sgistru3"].plurals[0] = "Usuário";
    strings["1. Bell##sgistru"].plurals[0] = "1. Sino";
    strings["2. Guitar##sgistru2"].plurals[0] = "2. Violão";
    strings["3. Piano##sgistru1"].plurals[0] = "3. Piano";
    strings["4. Flute##sgistru1"].plurals[0] = "4. Flauta";
    strings["5. Clarinet##sgistru2"].plurals[0] = "5. Clarinete";
    strings["6. Rattling Bell##sgistru"].plurals[0] = "6. Sino Chacoalhante";
    strings["7. Trumpet##sgistru3"].plurals[0] = "7. Trompete";
    strings["8. Reed Organ##sgistru"].plurals[0] = "8. Órgão de Palheta";
    strings["9. Soft Bell##sgistru"].plurals[0] = "9. Sino Suave";
    strings["10. Xylophone##sgistru"].plurals[0] = "10. Xilofone";
    strings["11. Vibraphone##sgistru"].plurals[0] = "11. Vibrafone";
    strings["12. Brass##sgistru"].plurals[0] = "12. Metais";
    strings["13. Bass Guitar##sgistru"].plurals[0] = "13. Baixo";
    strings["14. Synth##sgistru"].plurals[0] = "14. Sintetizador";
    strings["15. Chorus##sgistru"].plurals[0] = "15. Coro";
    strings["Drums##sgistru3"].plurals[0] = "Bateria";
    strings["Sine##sgistru0"].plurals[0] = "Seno";
    strings["Half Sine##sgistru0"].plurals[0] = "Meio Seno";
    strings["Absolute Sine##sgistru0"].plurals[0] = "Seno Absoluto";
    strings["Quarter Sine##sgistru"].plurals[0] = "1/4 Seno";
    strings["Squished Sine##sgistru0"].plurals[0] = "Seno Espremido";
    strings["Squished AbsSine##sgistru0"].plurals[0] = "Seno Absoluto Espremido";
    strings["Square##sgistru0"].plurals[0] = "Onda Quadrada";
    strings["Derived Square##sgistru0"].plurals[0] = "Dente de Serra Logarítmica"; // quadrado derivado é o cúmulo, aqui só vai nome tecnicamente correto
    strings["Sine##sgistru1"].plurals[0] = "Seno";
    strings["Half Sine##sgistru1"].plurals[0] = "Meio Seno";
    strings["Absolute Sine##sgistru1"].plurals[0] = "Seno Absoluto";
    strings["Pulse Sine##sgistru"].plurals[0] = "Seno Pulsado";
    strings["Sine (Even Periods)##sgistru"].plurals[0] = "Seno (Períodos Pares)";
    strings["AbsSine (Even Periods)##sgistru"].plurals[0] = "Seno Absoluto (Períodos Pares)";
    strings["Square##sgistru1"].plurals[0] = "Onda Quadrada";
    strings["Derived Square##sgistru1"].plurals[0] = "Dente de Serra Logarítmica";
    strings["Sine##sgistru2"].plurals[0] = "Seno";
    strings["Triangle##sgistru"].plurals[0] = "Triângulo";
    strings["Cut Sine##sgistru"].plurals[0] = "Seno Cortado";
    strings["Cut Triangle##sgistru"].plurals[0] = "Triângulo Cortado";
    strings["Squished Sine##sgistru1"].plurals[0] = "Seno Espremido";
    strings["Squished Triangle##sgistru"].plurals[0] = "Triângulo Espremido";
    strings["Squished AbsSine##sgistru1"].plurals[0] = "Seno Absoluto Espremido";
    strings["Squished AbsTriangle##sgistru"].plurals[0] = "Triângulo Absoluto Espremido";
    strings["Snare##sgistru0"].plurals[0] = "Caixa";
    strings["Tom##sgistru"].plurals[0] = "Surdo";
    strings["Top##sgistru0"].plurals[0] = "Prato";
    strings["HiHat##sgistru0"].plurals[0] = "Chimbal";
    strings["Normal##sgistru"].plurals[0] = "Normal";
    strings["Snare##sgistru1"].plurals[0] = "Caixa";
    strings["HiHat##sgistru1"].plurals[0] = "Chimbal";
    strings["Top##sgistru1"].plurals[0] = "Prato";
    strings["Noise disabled##sgistru"].plurals[0] = "Ruído desabilitado";
    strings["Square + noise##sgistru"].plurals[0] = "Onda quadrada + ruído";
    strings["Ringmod from OP3 + noise##sgistru"].plurals[0] = "Modulação anel do OP3 + ruído";
    strings["Ringmod from OP3 + double pitch ModInput\nWARNING - has emulation issues, subject to change##sgistru"].plurals[0] = "Modulação anel do OP3 + EntraMod com tom dobrado\nAVISO - há problemas na emulação desse modo, está sujeito a mudanças";
    strings["op1##sgistru"].plurals[0] = "op1";
    strings["op2##sgistru"].plurals[0] = "op2";
    strings["op3##sgistru"].plurals[0] = "op3";
    strings["op4##sgistru"].plurals[0] = "op4";
    strings["triangle##sgistru"].plurals[0] = "triângulo";
    strings["saw##sgistru"].plurals[0] = "serra";
    strings["pulse##sgistru"].plurals[0] = "pulso";
    strings["noise##sgistru0"].plurals[0] = "ruído";
    strings["tone##sgistru"].plurals[0] = "tom";
    strings["noise##sgistru1"].plurals[0] = "ruído";
    strings["envelope##sgistru"].plurals[0] = "envelope";
    strings["hold##sgistru"].plurals[0] = "segurar";
    strings["alternate##sgistru"].plurals[0] = "alternar";
    strings["direction##sgistru0"].plurals[0] = "direção";
    strings["enable##sgistru0"].plurals[0] = "habilitar";
    strings["enabled##sgistru0"].plurals[0] = "habilitado";
    strings["mirror##sgistru"].plurals[0] = "espelhar";
    strings["loop##sgistru0"].plurals[0] = "repetir";
    strings["cut##sgistru"].plurals[0] = "cortar";
    strings["direction##sgistru1"].plurals[0] = "direção";
    strings["resolution##sgistru"].plurals[0] = "resolução";
    strings["fixed##sgistru1"].plurals[0] = "fixo";
    strings["N/A##sgistru"].plurals[0] = "N/A";
    strings["enabled##sgistru1"].plurals[0] = "habilitado";
    strings["noise##sgistru2"].plurals[0] = "ruído";
    strings["echo##sgistru"].plurals[0] = "eco";
    strings["pitch mod##sgistru"].plurals[0] = "mod. de tom";
    strings["invert right##sgistru"].plurals[0] = "inverter direita";
    strings["invert left##sgistru"].plurals[0] = "inverter esquerda";
    strings["low##sgistru"].plurals[0] = "baixa";
    strings["band##sgistru"].plurals[0] = "banda";
    strings["high##sgistru"].plurals[0] = "alta";
    strings["ch3off##sgistru"].plurals[0] = "can3desl";
    strings["gate##sgistru"].plurals[0] = "gate";
    strings["sync##sgistru"].plurals[0] = "sincron.";
    strings["ring##sgistru"].plurals[0] = "anel";
    strings["test##sgistru"].plurals[0] = "teste";
    strings["15kHz##sgistru"].plurals[0] = "15kHz";
    strings["filter 2+4##sgistru"].plurals[0] = "filtrar 2+4";
    strings["filter 1+3##sgistru"].plurals[0] = "filtrar 1+3";
    strings["16-bit 3+4##sgistru"].plurals[0] = "16-bit 3+4";
    strings["16-bit 1+2##sgistru"].plurals[0] = "16-bit 1+2";
    strings["high3##sgistru"].plurals[0] = "altafreq3";
    strings["high1##sgistru"].plurals[0] = "altafreq1";
    strings["poly9##sgistru"].plurals[0] = "poly9";
    strings["int##sgistru"].plurals[0] = "int";
    strings["sustain##sgistru2"].plurals[0] = "sustentação";
    strings["square##sgistru2"].plurals[0] = "quadrada";
    strings["noise##sgistru3"].plurals[0] = "ruído";
    strings["noise##sgistru4"].plurals[0] = "ruído";
    strings["invert##sgistru"].plurals[0] = "inverter";
    strings["surround##sgistru"].plurals[0] = "surround";
    strings["enable##sgistru1"].plurals[0] = "habilitar";
    strings["oneshot##sgistru"].plurals[0] = "oneshot";
    strings["split L/R##sgistru"].plurals[0] = "separar esq/dir";
    strings["HinvR##sgistru"].plurals[0] = "HinvR";
    strings["VinvR##sgistru"].plurals[0] = "VinvR";
    strings["HinvL##sgistru"].plurals[0] = "HinvL";
    strings["VinvL##sgistru"].plurals[0] = "VinvL";
    strings["ring mod##sgistru"].plurals[0] = "mod anel";
    strings["low pass##sgistru"].plurals[0] = "passa baixa";
    strings["high pass##sgistru"].plurals[0] = "passa alta";
    strings["band pass##sgistru"].plurals[0] = "passa banda";
    strings["HP/K2, HP/K2##sgistru"].plurals[0] = "alta/K2, alta/K2";
    strings["HP/K2, LP/K1##sgistru"].plurals[0] = "alta/K2, baixa/K1";
    strings["LP/K2, LP/K2##sgistru"].plurals[0] = "baixa/K2, baixa/K2";
    strings["LP/K2, LP/K1##sgistru"].plurals[0] = "baixa/K2, baixa/K1";
    strings["right##sgistru"].plurals[0] = "direita";
    strings["left##sgistru"].plurals[0] = "esquerda";
    strings["rear right##sgistru"].plurals[0] = "traseira direita";
    strings["rear left##sgistru"].plurals[0] = "traseira esquerda";
    strings["enable tap B##sgistru"].plurals[0] = "habilitar tap B";
    strings["AM with slope##sgistru"].plurals[0] = "AM com rampa";
    strings["invert B##sgistru"].plurals[0] = "inverter B";
    strings["invert A##sgistru"].plurals[0] = "inverter A";
    strings["reset B##sgistru"].plurals[0] = "reiniciar B";
    strings["reset A##sgistru"].plurals[0] = "reiniciar A";
    strings["clip B##sgistru"].plurals[0] = "cortar B";
    strings["clip A##sgistru"].plurals[0] = "cortar A";
    strings["on##sgistru"].plurals[0] = "ligado";
    strings["k1 slowdown##sgistru"].plurals[0] = "desacelerar k1";
    strings["k2 slowdown##sgistru"].plurals[0] = "desacelerar k2";
    strings["pause##sgistru"].plurals[0] = "pausar";
    strings["reverse##sgistru"].plurals[0] = "reverter";
    strings["high pass##sgistru1"].plurals[0] = "passa alta";
    strings["ring mod##sgistru1"].plurals[0] = "modulação anel";
    strings["swap counters (noise)##sgistru"].plurals[0] = "trocar contadores (ruído)";
    strings["low pass (noise)##sgistru"].plurals[0] = "passa baixa (ruído)";
    strings["sync##sgistru1"].plurals[0] = "sincronizar";
    strings["ring##sgistru1"].plurals[0] = "anel";
    strings["low##sgistru1"].plurals[0] = "baixa";
    strings["band##sgistru1"].plurals[0] = "banda";
    strings["high##sgistru1"].plurals[0] = "alta";
    strings["8580 SID##sgistru"].plurals[0] = "SID 8580";
    strings["bitwise AND##sgistru"].plurals[0] = "AND bit a bit";
    strings["bitwise OR##sgistru"].plurals[0] = "OR bit a bit";
    strings["bitwise XOR##sgistru"].plurals[0] = "XOR bit a bit";
    strings["None##sgistru"].plurals[0] = "Nenhum";
    strings["Invert##sgistru"].plurals[0] = "Inverter";
    strings["Add##sgistru"].plurals[0] = "Somar";
    strings["Subtract##sgistru"].plurals[0] = "Subtrair";
    strings["Average##sgistru"].plurals[0] = "Média";
    strings["Phase##sgistru"].plurals[0] = "Fase";
    strings["Chorus##sgistru"].plurals[0] = "Coro";
    strings["None (dual)##sgistru"].plurals[0] = "Nenhum (dupla)";
    strings["Wipe##sgistru"].plurals[0] = "Wipe";
    strings["Fade##sgistru"].plurals[0] = "Misturar";
    strings["Fade (ping-pong)##sgistru"].plurals[0] = "Misturar (ping-pong)";
    strings["Overlay##sgistru"].plurals[0] = "Sobrepor";
    strings["Negative Overlay##sgistru"].plurals[0] = "Sobreposição Negativa";
    strings["Slide##sgistru"].plurals[0] = "Deslizar";
    strings["Mix Chorus##sgistru"].plurals[0] = "Coro Misto";
    strings["Phase Modulation##sgistru"].plurals[0] = "Modulação de Fase";
    strings["Envelope##sgistru"].plurals[0] = "Envelope";
    strings["Sweep##sgistru"].plurals[0] = "Sweep";
    strings["Wait##sgistru0"].plurals[0] = "Aguardar";
    strings["Wait for Release##sgistru0"].plurals[0] = "Aguardar pelo Repouso";
    strings["Loop##sgistru1"].plurals[0] = "Repetir";
    strings["Loop until Release##sgistru0"].plurals[0] = "Repetir até o Repouso";
    strings["Volume Sweep##sgistru"].plurals[0] = "Sweep de Volume";
    strings["Frequency Sweep##sgistru"].plurals[0] = "Sweep de Frequência";
    strings["Cutoff Sweep##sgistru"].plurals[0] = "Sweep de Ponto de Corte";
    strings["Wait##sgistru1"].plurals[0] = "Esperar";
    strings["Wait for Release##sgistru1"].plurals[0] = "Esperar pelo Repouso";
    strings["Loop##sgistru2"].plurals[0] = "Repetir";
    strings["Loop until Release##sgistru1"].plurals[0] = "Repetir até o Repouso";
    strings["Direct##sgistru"].plurals[0] = "Direto";
    strings["Decrease (linear)##sgistru"].plurals[0] = "Diminuir (linear)";
    strings["Decrease (logarithmic)##sgistru"].plurals[0] = "Diminuir (logarítmico)";
    strings["Increase (linear)##sgistru"].plurals[0] = "Aumentar (linear)";
    strings["Increase (bent line)##sgistru"].plurals[0] = "Aumentar (linha torta)";
    strings["Fixed##sgistru2"].plurals[0] = "Fixo";
    strings["Relative##sgistru"].plurals[0] = "Relativo";
    strings["QSound##sgistru"].plurals[0] = "QSound";
    strings["Bug##sgistru"].plurals[0] = "Bug";

    strings["Fixed"].plurals[0] = "Fixo";
    strings["Relative"].plurals[0] = "Relativo";
    strings["Local"].plurals[0] = "Local";
    strings["Global"].plurals[0] = "Global";

    //   sgiSU     src/gui/inst/su.cpp

    strings["Sound Unit##sgiSU"].plurals[0] = "Unidade de Som";
    strings["Switch roles of frequency and phase reset timer##sgiSU"].plurals[0] = "Trocar papéis dos temporizadores de frequência e redefinição de fase";
    strings["Hardware Sequence##sgiSU"].plurals[0] = "Sequência de Hardware";
    strings["Tick##sgiSU0"].plurals[0] = "Tick";
    strings["Command##sgiSU0"].plurals[0] = "Comando";
    strings["Move/Remove##sgiSU0"].plurals[0] = "Mover/Remover";
    strings["Period##sgiSU0"].plurals[0] = "Período";
    strings["Amount##sgiSU0"].plurals[0] = "Quantidade";
    strings["Bound##sgiSU0"].plurals[0] = "Limite";
    strings["Up##sgiSU0"].plurals[0] = "Cima";
    strings["Down##sgiSU0"].plurals[0] = "Baixo";
    strings["Loop##sgiSU"].plurals[0] = "Repetir";
    strings["Flip##sgiSU"].plurals[0] = "Virar";
    strings["Period##sgiSU1"].plurals[0] = "Período";
    strings["Amount##sgiSU1"].plurals[0] = "Quantidade";
    strings["Bound##sgiSU1"].plurals[0] = "Limite";
    strings["Up##sgiSU1"].plurals[0] = "Cima";
    strings["Down##sgiSU1"].plurals[0] = "Baixo";
    strings["Ticks##sgiSU"].plurals[0] = "Ticks";
    strings["Position##sgiSU"].plurals[0] = "Posição";
    strings["Macros##sgiSU"].plurals[0] = "Macros";
    strings["Volume##sgiSU"].plurals[0] = "Volume";
    strings["Arpeggio##sgiSU"].plurals[0] = "Arpejo";
    strings["Pitch##sgiSU"].plurals[0] = "Tom";
    strings["Duty/Noise##sgiSU"].plurals[0] = "Ciclo de Trab./Ruído";
    strings["Waveform##sgiSU"].plurals[0] = "Forma de Onda";
    strings["Panning##sgiSU"].plurals[0] = "Balanço";
    strings["Phase Reset##sgiSU"].plurals[0] = "Redef. de Fase";
    strings["Cutoff##sgiSU"].plurals[0] = "Ponto de Corte";
    strings["Resonance##sgiSU"].plurals[0] = "Ressonância";
    strings["Control##sgiSU"].plurals[0] = "Controle";
    strings["Phase Reset Timer##sgiSU"].plurals[0] = "Temporizador de Redef. de Fase";

    //   sgiSWAN   src/gui/inst/swan.cpp

    strings["Macros##sgiSWAN"].plurals[0] = "Macros";
    strings["Volume##sgiSWAN"].plurals[0] = "Volume";
    strings["Arpeggio##sgiSWAN"].plurals[0] = "Arpejo";
    strings["Pitch##sgiSWAN"].plurals[0] = "Tom";
    strings["Noise##sgiSWAN"].plurals[0] = "Ruído";
    strings["Waveform##sgiSWAN"].plurals[0] = "Forma de Onda";
    strings["Phase Reset##sgiSWAN"].plurals[0] = "Redef. de Fase";

    //   sgiT6W    src/gui/inst/t6w28.cpp

    strings["Macros##sgiT6W"].plurals[0] = "Macros";
    strings["Volume##sgiT6W"].plurals[0] = "Volume";
    strings["Arpeggio##sgiT6W"].plurals[0] = "Arpejo";
    strings["Pitch##sgiT6W"].plurals[0] = "Tom";
    strings["Panning (left)##sgiT6W"].plurals[0] = "Balanço (esquerda)";
    strings["Panning (right)##sgiT6W"].plurals[0] = "Balanço (direita)";
    strings["Noise Type##sgiT6W"].plurals[0] = "Tipo de Ruído";
    strings["Phase Reset##sgiT6W"].plurals[0] = "Redef. de Fase";

    //   sgiTED    src/gui/inst/ted.cpp

    strings["Macros##sgiTED"].plurals[0] = "Macros";
    strings["Volume##sgiTED"].plurals[0] = "Volume";
    strings["Arpeggio##sgiTED"].plurals[0] = "Arpejo";
    strings["Pitch##sgiTED"].plurals[0] = "Tom";
    strings["Square/Noise##sgiTED"].plurals[0] = "Quadrado/Ruído";
    strings["Phase Reset##sgiTED"].plurals[0] = "Redef. de Fase";

    //   sgiTIA    src/gui/inst/tia.cpp

    strings["Macros##sgiTIA"].plurals[0] = "Macros";
    strings["Volume##sgiTIA"].plurals[0] = "Volume";
    strings["Arpeggio##sgiTIA"].plurals[0] = "Arpejo";
    strings["Pitch##sgiTIA"].plurals[0] = "Tom";
    strings["Raw Period##sgiTIA"].plurals[0] = "Período (val. registrador)";
    strings["Waveform##sgiTIA"].plurals[0] = "Forma de Onda";

    //   sgiVB     src/gui/inst/vboy.cpp

    strings["Set modulation table (channel 5 only)##sgiVB"].plurals[0] = "Definir tabela de modulação (somente canal 5)";
    strings["Hex##MTHex"].plurals[0] = "Hex##MTHex";
    strings["Dec##MTHex"].plurals[0] = "Dec##MTHex";
    strings["Macros##sgiVB"].plurals[0] = "Macros";
    strings["Volume##sgiVB"].plurals[0] = "Volume";
    strings["Arpeggio##sgiVB"].plurals[0] = "Arpejo";
    strings["Pitch##sgiVB"].plurals[0] = "Tom";
    strings["Noise Length##sgiVB"].plurals[0] = "Comprimento do Ruído";
    strings["Waveform##sgiVB"].plurals[0] = "Forma de Onda";
    strings["Panning (left)##sgiVB"].plurals[0] = "Balanço (esquerda)";
    strings["Panning (right)##sgiVB"].plurals[0] = "Balanço (direita)";
    strings["Phase Reset##sgiVB"].plurals[0] = "Redef. de Fase";

    //   sgiVERA   src/gui/inst/vera.cpp

    strings["Macros##sgiVERA"].plurals[0] = "Macros";
    strings["Volume##sgiVERA"].plurals[0] = "Volume";
    strings["Arpeggio##sgiVERA"].plurals[0] = "Arpejo";
    strings["Pitch##sgiVERA"].plurals[0] = "Tom";
    strings["Duty##sgiVERA"].plurals[0] = "Ciclo de Trab.";
    strings["Waveform##sgiVERA"].plurals[0] = "Forma de Onda";
    strings["Panning##sgiVERA"].plurals[0] = "Balanço";

    //   sgiVIC    src/gui/inst/vic.cpp

    strings["Macros##sgiVIC"].plurals[0] = "Macros";
    strings["Volume##sgiVIC"].plurals[0] = "Volume";
    strings["Arpeggio##sgiVIC"].plurals[0] = "Arpejo";
    strings["Pitch##sgiVIC"].plurals[0] = "Tom";
    strings["On/Off##sgiVIC"].plurals[0] = "Liga/Desl.";
    strings["Waveform##sgiVIC"].plurals[0] = "Forma de Onda";

    //   sgiVRC6   src/gui/inst/vrc6.cpp

    strings["Macros##sgiVRC6"].plurals[0] = "Macros";
    strings["Volume##sgiVRC6"].plurals[0] = "Volume";
    strings["Arpeggio##sgiVRC6"].plurals[0] = "Arpejo";
    strings["Pitch##sgiVRC6"].plurals[0] = "Tom";
    strings["Duty##sgiVRC6"].plurals[0] = "Ciclo de Trab.";
    strings["Waveform##sgiVRC6"].plurals[0] = "Forma de Onda";
    strings["Phase Reset##sgiVRC6"].plurals[0] = "Redef. de Fase";

    //   sgiVRC6S  src/gui/inst/vrc6saw.cpp

    strings["Macros##sgiVRC6S"].plurals[0] = "Macros";
    strings["Volume##sgiVRC6S"].plurals[0] = "Volume";
    strings["Arpeggio##sgiVRC6S"].plurals[0] = "Arpejo";
    strings["Pitch##sgiVRC6S"].plurals[0] = "Tom";

    //   sgiwave   src/gui/inst/wavetable.cpp

    strings["Wavetable##sgiwave"].plurals[0] = "Wavetable";
    strings["Enable synthesizer##sgiwave"].plurals[0] = "Habilitar sintetizador";
    strings["Single-waveform##sgiwave"].plurals[0] = "Forma de onda única";
    strings["Dual-waveform##sgiwave"].plurals[0] = "Forma de onda dupla";
    strings["Wave 1##sgiwave0"].plurals[0] = "Onda 1";
    strings["Wave 2##sgiwave0"].plurals[0] = "Onda 2";
    strings["Result##sgiwave"].plurals[0] = "Resultado";
    strings["Wave 1 ##sgiwave"].plurals[0] = "Onda 1 ";
    strings["waveform macro is controlling wave 1!\nthis value will be ineffective.##sgiwave"].plurals[0] = "a macro de forma de onda está controlando a onda 1! este valor não será efetivo.";
    strings["Wave 1##sgiwave1"].plurals[0] = "Onda 1";
    strings["Wave 2##sgiwave1"].plurals[0] = "Onda 2";
    strings["Resume preview##sgiwave"].plurals[0] = "Continuar previsão";
    strings["Pause preview##sgiwave"].plurals[0] = "Pausar previsão";
    strings["Restart preview##sgiwave"].plurals[0] = "Reiniciar previsão";
    strings["too many wavetables!##sgiwave"].plurals[0] = "há muitas wavetables!";
    strings["Copy to new wavetable##sgiwave"].plurals[0] = "Copiar para nova wavetable";
    strings["Update Rate##sgiwave"].plurals[0] = "Taxa de Atualização";
    strings["Speed##sgiwave"].plurals[0] = "Velocidade";
    strings["Amount##sgiwave"].plurals[0] = "Quantidade";
    strings["Power##sgiwave"].plurals[0] = "Potência";
    strings["Global##sgiwave"].plurals[0] = "Global";
    strings["Global##sgiwave1"].plurals[0] = "Global";
    strings["Global##sgiwave2"].plurals[0] = "Global##jesus";
    strings["wavetable synthesizer disabled.\nuse the Waveform macro to set the wave for this instrument.##sgiwave"].plurals[0] = "sintetizador de wavetable desabilitado.\nuse a macro de Forma de Onda para definir a onda para este instrumento.";
    strings["Local Waves##sgiwave"].plurals[0] = "Ondas Locais";

    //   sgiX1     src/gui/inst/x1_010.cpp

    strings["Macros##sgiX1"].plurals[0] = "Macros";
    strings["Volume##sgiX1"].plurals[0] = "Volume";
    strings["Arpeggio##sgiX1"].plurals[0] = "Arpejo";
    strings["Pitch##sgiX1"].plurals[0] = "Tom";
    strings["Waveform##sgiX1"].plurals[0] = "Forma de Onda";
    strings["Panning (left)##sgiX1"].plurals[0] = "Balanço (esquerda)";
    strings["Panning (right)##sgiX1"].plurals[0] = "Balanço (direita)";
    strings["Phase Reset##sgiX1"].plurals[0] = "Redef. de Fase";
    strings["Envelope##sgiX1"].plurals[0] = "Envelope";
    strings["Envelope Mode##sgiX1"].plurals[0] = "Modo de Envelope";
    strings["AutoEnv Num##sgiX1"].plurals[0] = "Num. EnvAuto";
    strings["AutoEnv Den##sgiX1"].plurals[0] = "Den. EnvAuto";

    //   sgiYMZ    src/gui/inst/ymz280b.cpp

    strings["Macros##sgiYMZ"].plurals[0] = "Macros";
    strings["Volume##sgiYMZ"].plurals[0] = "Volume";
    strings["Arpeggio##sgiYMZ"].plurals[0] = "Arpejo";
    strings["Pitch##sgiYMZ"].plurals[0] = "Tom";
    strings["Panning##sgiYMZ"].plurals[0] = "Balanço";
    strings["Phase Reset##sgiYMZ"].plurals[0] = "Redef. de Fase";

    //   se**    src/engine/
    //   seen    src/engine/engine.cpp  --DONE--
    //   sesd    src/engine/sysDef.cpp  --

    //           src/engine/cmdStream.cpp

    strings["not a command stream"].plurals[0] = "não é um fluxo de comandos";

    //   seen    src/engine/engine.cpp

    strings["00xy: Arpeggio##seen"].plurals[0] = "00xy: Arpejo";
    strings["01xx: Pitch slide up##seen"].plurals[0] = "01xx: Slide de tom para cima";
    strings["02xx: Pitch slide down##seen"].plurals[0] = "02xx: Slide de tom para baixo";
    strings["03xx: Portamento##seen"].plurals[0] = "03xx: Portamento";
    strings["04xy: Vibrato (x: speed; y: depth)##seen"].plurals[0] = "04xy: Vibrato (x: velocidade; y: profundidade)";
    strings["05xy: Volume slide + vibrato (compatibility only!)##seen"].plurals[0] = "05xy: Slide de volume + vibrato (apenas para compatibilidade!)";
    strings["06xy: Volume slide + portamento (compatibility only!)##seen"].plurals[0] = "06xy: Slide de volume + portamento (apenas para compatibilidade!)";
    strings["07xy: Tremolo (x: speed; y: depth)##seen"].plurals[0] = "07xy: Tremolo (x: velocidade; y: profundidade)";
    strings["08xy: Set panning (x: left; y: right)##seen"].plurals[0] = "08xy: Definir balanço (x: esquerda; y: direita)";
    strings["09xx: Set groove pattern (speed 1 if no grooves exist)##seen"].plurals[0] = "09xx: Escolher padrão de groove (velocidade 1 se não houverem grooves)";
    strings["0Axy: Volume slide (0y: down; x0: up)##seen"].plurals[0] = "0Axy: Slide de volume (0y: para baixo; x0: para cima)";
    strings["0Bxx: Jump to pattern##seen"].plurals[0] = "0Bxx: Pular para sequência"; // não deveria ser "pular para ordem"?
    strings["0Cxx: Retrigger##seen"].plurals[0] = "0Cxx: Reacionar nota";
    strings["0Dxx: Jump to next pattern##seen"].plurals[0] = "0Dxx: Pular para a próxima sequência";
    strings["0Fxx: Set speed (speed 2 if no grooves exist)##seen"].plurals[0] = "0Fxx: Definir velocidade (define a velocidade 2 se não houverem grooves)";
    strings["80xx: Set panning (00: left; 80: center; FF: right)##seen"].plurals[0] = "80xx: Definir balanço (00: esquerda; 80: centro; FF: direita)";
    strings["81xx: Set panning (left channel)##seen"].plurals[0] = "81xx: Definir balanço (canal esquerdo)";
    strings["82xx: Set panning (right channel)##seen"].plurals[0] = "82xx: Definir balanço (canal direito)";
    strings["88xy: Set panning (rear channels; x: left; y: right)##seen"].plurals[0] = "88xy: Definir balanço (canais traseiros; x: esquerda; y: direita)";
    strings["89xx: Set panning (rear left channel)##seen"].plurals[0] = "89xx: Definir balanço (canal traseiro esquerdo)";
    strings["8Axx: Set panning (rear right channel)##seen"].plurals[0] = "8Axx: Definir balanço (canal traseiro direito)";
    strings["Cxxx: Set tick rate (hz)##seen"].plurals[0] = "Cxxx: Definir taxa de ticks (Hz)";
    strings["E0xx: Set arp speed##seen"].plurals[0] = "E0xx: Definir velocidade do arpejo";
    strings["E1xy: Note slide up (x: speed; y: semitones)##seen"].plurals[0] = "E1xy: Slide de nota para cima (x: velocidade; y: semitons)";
    strings["E2xy: Note slide down (x: speed; y: semitones)##seen"].plurals[0] = "E2xy: Slide de nota para baixo (x: velocidae; y: semitons)";
    strings["E3xx: Set vibrato shape (0: up/down; 1: up only; 2: down only)##seen"].plurals[0] = "E3xx: Definir formato do vibrato (0: cima/baixo; 1: cima somente; 2: baixo somente)";
    strings["E4xx: Set vibrato range##seen"].plurals[0] = "E4xx: Definir alcance do vibrato";
    strings["E5xx: Set pitch (80: center)##seen"].plurals[0] = "E5xx: Definir tom (desafinação; 80: centro)";
    strings["E6xy: Delayed note transpose (x: 0-7 = up, 8-F = down (after (x % 7) ticks); y: semitones)##seen"].plurals[0] = "E6xy: Transposição atrasada de nota (x: 0-7 = para cima, 8-F = para baixo (após (x % 7) ticks); y: semitons)";
    strings["E7xx: Macro release##seen"].plurals[0] = "E7xx: Repouso de macro atrasado (x: ticks)";
    strings["E8xy: Delayed note transpose up (x: ticks; y: semitones)##seen"].plurals[0] = "E8xy: Transposição de nota atrasada para cima (x: ticks; y: semitons)";
    strings["E9xy: Delayed note transpose down (x: ticks; y: semitones)##seen"].plurals[0] = "E9xy: Transposição de nota atrasada para baixo (x: ticks; y: semitons)";
    strings["EAxx: Legato##seen"].plurals[0] = "EAxx: Legato";
    strings["EBxx: Set LEGACY sample mode bank##seen"].plurals[0] = "EBxx: Definir banco de samples modo LEGADO";
    strings["ECxx: Note cut##seen"].plurals[0] = "ECxx: Corte de nota";
    strings["EDxx: Note delay##seen"].plurals[0] = "EDxx: Atraso de nota";
    strings["EExx: Send external command##seen"].plurals[0] = "EExx: Enviar comando externo";
    strings["F0xx: Set tick rate (bpm)##seen"].plurals[0] = "F0xx: Definir taxa de ticks (bpm)";
    strings["F1xx: Single tick note slide up##seen"].plurals[0] = "F1xx: Slide de um tick de nota para cima";
    strings["F2xx: Single tick note slide down##seen"].plurals[0] = "F2xx: Slide de um tick de nota para baixo";
    strings["F3xx: Fine volume slide up##seen"].plurals[0] = "F3xx: Slide fino de volume para cima";
    strings["F4xx: Fine volume slide down##seen"].plurals[0] = "F4xx: Slide fino de volume para baixo";
    strings["F5xx: Disable macro (see manual)##seen"].plurals[0] = "F5xx: Desabilitar macro (veja o manual)"; // aí a pessoa vai ler o manual e:
    strings["F6xx: Enable macro (see manual)##seen"].plurals[0] = "F6xx: Habilitar macro (veja o manual)";    // "aff ta em ingreis num sei ler nada *** vai toma nu **"
    strings["F7xx: Restart macro (see manual)##seen"].plurals[0] = "F7xx: Reiniciar macro (veja o manual)";   // -- hey @LTVA1 let's translate the docs too!
    strings["F8xx: Single tick volume slide up##seen"].plurals[0] = "F8xx: Slide de um tick de volume para cima";
    strings["F9xx: Single tick volume slide down##seen"].plurals[0] = "F9xx: Slide de um tick de volume para baixo";
    strings["FAxx: Fast volume slide (0y: down; x0: up)##seen"].plurals[0] = "FAxx: Slide rápido de volume (0y: para baixo; x0: para cima)";
    strings["FCxx: Note release##seen"].plurals[0] = "FCxx: Repouso de nota atrasado (x: ticks)";
    strings["FDxx: Set virtual tempo numerator##seen"].plurals[0] = "FDxx: Definir numerador do tempo virtual";
    strings["FExx: Set virtual tempo denominator##seen"].plurals[0] = "FExx: Definir denominador do tempo virtual";
    strings["FFxx: Stop song##seen"].plurals[0] = "FFxx: Parar música";
    strings["9xxx: Set sample offset*256##seen"].plurals[0] = "9xxx: Definir deslocamento de sample*256";

    strings["on seek: %s"].plurals[0] = "ao avançar: %s";
    strings["on pre tell: %s"].plurals[0] = "ao predeterminar posição: %s";
    strings["file is empty"].plurals[0] = "o arquivo está vazio";
    strings["on tell: %s"].plurals[0] = "ao determinar posição: %s";
    strings["ROM size mismatch, expected: %d bytes, was: %d"].plurals[0] = "divergência de tamanho da ROM, esperado: %d bytes, encontrado: %d";
    strings["on get size: %s"].plurals[0] = "ao obter tamanho: %s";
    strings["on read: %s"].plurals[0] = "ao ler: %s";
    strings["invalid index"].plurals[0] = "índice inválido";
    strings["max number of total channels is %d"].plurals[0] = "número máximo total de canais é %d";
    strings["max number of systems is %d"].plurals[0] = "número máximo de sistemas é %d";
    strings["cannot remove the last one"].plurals[0] = "não pode remover o último";
    strings["source and destination are equal"].plurals[0] = "a origem e o destino são os mesmos";
    strings["invalid source index"].plurals[0] = "índice de origem inválido";
    strings["invalid destination index"].plurals[0] = "índice de destino inválido";
    strings["too many wavetables!"].plurals[0] = "há muitas wavetables!";
    strings["could not seek to end: %s"].plurals[0] = "não pôde avançar até o final: %s";
    strings["could not determine file size: %s"].plurals[0] = "não pode determinar o tamanho do arquivo: %s";
    strings["file size is invalid!"].plurals[0] = "o tamanho do arquivo é inválido!";
    strings["could not seek to beginning: %s"].plurals[0] = "não pode rebobinar até o começo: %s";
    strings["could not read entire file: %s"].plurals[0] = "não pôde ler o arquivo inteiro: %s";
    strings["invalid wavetable header/data!"].plurals[0] = "cabeçalho/dados da wavetable inválidos!";
    strings["premature end of file"].plurals[0] = "fim prematuro do arquivo";
    strings["too many samples!"].plurals[0] = "há muitas samples!";
    strings["no free patterns in channel %d!"].plurals[0] = "não há sequências livres no canal %d!";

    //           src/engine/fileOps.cpp

    strings["this module was created with a more recent version of Furnace!"].plurals[0] = "este módulo foi criado com uma versão mais recente do Furnace!";
    strings["couldn't seek to info header!"].plurals[0] = "não pôde avançar até o header \"info\"!";
    strings["invalid info header!"].plurals[0] = "header \"info\" inválido!";
    strings["pattern length is negative!"].plurals[0] = "o comprimento da sequência é negativo!";
    strings["pattern length is too large!"].plurals[0] = "o comprimento da sequência é muito longo!";
    strings["song length is negative!"].plurals[0] = "o comprimento da música é negativo!";
    strings["song is too long!"].plurals[0] = "a música é muito longa!";
    strings["invalid instrument count!"].plurals[0] = "quantidade de instrumentos inválida!";
    strings["invalid wavetable count!"].plurals[0] = "quantidade de wavetables inválida!";
    strings["invalid sample count!"].plurals[0] = "quantidade de samples inválida!";
    strings["invalid pattern count!"].plurals[0] = "quantidade de sequências inválida!";
    strings["unrecognized system ID %.2x!"].plurals[0] = "identificador de sistema %.2x não reconhecido!";
    strings["zero chips!"].plurals[0] = "zero chips!";
    strings["channel %d has too many effect columns! (%d)"].plurals[0] = "o canal %d contém muitas colunas de efeito! (%d)";
    strings["couldn't seek to chip %d flags!"].plurals[0] = "não pôde avançar até as opções do chip %d!";
    strings["invalid flag header!"].plurals[0] = "cabeçalho \"flag\" inválido!";
    strings["couldn't read instrument directory"].plurals[0] = "não pôde ler o diretório de instrumentos";
    strings["invalid instrument directory data!"].plurals[0] = "dados do diretório de instrumentos inválidos!";
    strings["couldn't read wavetable directory"].plurals[0] = "não pôde ler o diretório de wavetables";
    strings["invalid wavetable directory data!"].plurals[0] = "dados do diretório de wavetables inválidos!";
    strings["couldn't read sample directory"].plurals[0] = "não pôde ler o diretório de samples";
    strings["invalid sample directory data!"].plurals[0] = "dados do diretório de samples inválidos!";
    strings["couldn't seek to subsong %d!"].plurals[0] = "não pôde avançar até a subfaixa %d!";
    strings["invalid subsong header!"].plurals[0] = "cabeçalho da subfaixa inválido!";
    strings["couldn't seek to instrument %d!"].plurals[0] = "não pôde avançar até o instrumento %d!";
    strings["invalid instrument header/data!"].plurals[0] = "cabeçalho/dados do instrumento inválidos!";
    strings["couldn't seek to wavetable %d!"].plurals[0] = "não pôde avançar até a wavetable %d!";
    strings["couldn't seek to sample %d!"].plurals[0] = "não pôde avançar até a sample %d!";
    strings["invalid sample header/data!"].plurals[0] = "cabeçalho/dados da sample inválidos!";
    strings["couldn't seek to pattern in %x!"].plurals[0] = "não pôde avançar até a sequência em %x!";
    strings["invalid pattern header!"].plurals[0] = "cabeçalho da sequência inválido!";
    strings["pattern channel out of range!"].plurals[0] = "canal da sequência fora do intervalo!";
    strings["pattern index out of range!"].plurals[0] = "índice da sequência fora do intervalo!";
    strings["pattern subsong out of range!"].plurals[0] = "subfaixa da sequência fora do intervalo!";
    strings["incomplete file"].plurals[0] = "arquivo incompleto";
    strings["file is too small"].plurals[0] = "o arquivo é muito pequeno";
    strings["not a .dmf/.fur/.fub song"].plurals[0] = "esta não é uma música .dmf/.fur/.fub";
    strings["unknown decompression error"].plurals[0] = "erro desconhecido de descompressõa";
    strings["decompression error: %s"].plurals[0] = "erro de descompressão: %s";
    strings["unknown decompression finish error"].plurals[0] = "erro desconhecido ao final da descompressão";
    strings["decompression finish error: %s"].plurals[0] = "erro ao final da descompressão: %s";
    strings["not a compatible song/instrument"].plurals[0] = "esta não é uma música/instrumento compatível";
    strings["maximum number of instruments is 256"].plurals[0] = "o número máximo de instrumentos é 256";
    strings["maximum number of wavetables is 256"].plurals[0] = "o número máximo de wavetables é 256";
    strings["maximum number of samples is 256"].plurals[0] = "o número máximo de samples é 256";

    //           src/engine/fileOpsIns.cpp

    strings["did not read entire instrument file!"].plurals[0] = "o arquivo de instrumento não foi lido por completo!";
    strings["this instrument is made with a more recent version of Furnace!"].plurals[0] = "este instrumento foi feito com uma versão mais recente do Furnace!";
    strings["unknown instrument format"].plurals[0] = "formato de instrumento desconhecido";
    strings["there is more data at the end of the file! what happened here!"].plurals[0] = "há mais dados ao final do arquivo! que diacho é esse!?";
    strings["exactly %d bytes, if you are curious"].plurals[0] = "exatamente %d bytes, caso você tenha a curiosidade";

    //           src/engine/fileOpsSample.cpp

    strings["could not open file! (%s)"].plurals[0] = "não pôde abrir o arquivo! (%s)";
    strings["could not get file length! (%s)"].plurals[0] = "não pôde obter o tamanho do arquivo! (%s)";
    strings["file is empty!"].plurals[0] = "o arquivo está vazio!";
    strings["file is invalid!"].plurals[0] = "o arquivo é inválido!";
    strings["could not seek to beginning of file! (%s)"].plurals[0] = "não pôde localizar o início do arquivo! (%s)";
    strings["wait... is that right? no I don't think so..."].plurals[0] = "peraí... isso está certo? não, eu não acho que esteja...";
    strings["BRR sample is empty!"].plurals[0] = "a sample BRR está vazia!";
    strings["possibly corrupt BRR sample!"].plurals[0] = "a sample BRR pode estar corrompida!";
    strings["could not read file! (%s)"].plurals[0] = "não pôde ler o arquivo! (%s)";
    strings["Furnace was not compiled with libsndfile!"].plurals[0] = "Furnace não foi compilado com a biblioteca libsndfile!";
    strings["could not open file! (%s %s)"].plurals[0] = "não pôde abrir o arquivo! (%s %s)";
    strings["could not open file! (%s)\nif this is raw sample data, you may import it by right-clicking the Load Sample icon and selecting \"import raw\"."].plurals[0] = "não pôde abrir o arquivo! (%s)\nse isso for uma sample em formato \"raw\", você pode importá-la clicando com o botão direito no ícone Abrir e selecionando \"importar em formato raw\".";
    strings["this sample is too big! max sample size is 16777215."].plurals[0] = "esta sample é muito grande! o tamanho máximo de uma sample é 16777215.";

    //           src/engine/importExport/bnk.cpp

    strings["GEMS BNK currently not supported."].plurals[0] = "O formato BNK do GEMS atualmente não é suportado.";

    //           src/engine/importExport/dmf.cpp

    strings["this version is not supported by Furnace yet"].plurals[0] = "esta versão ainda não é suportada pelo Furnace";
    strings["system not supported. running old version?"].plurals[0] = "sistema não suportado. usando uma versão antiga?";
    strings["Yamaha YMU759 emulation is incomplete! please migrate your song to the OPL3 system."].plurals[0] = "a emulação do Yamaha YMU759 é incompleta! por favor migre sua música para o sistema OPL3.";
    strings["order at %d, %d out of range! (%d)"].plurals[0] = "ordem em %d, %d fora do intervalo! (%d)";
    strings["file is corrupt or unreadable at operators"].plurals[0] = "o arquivo está corrompido ou ilegível nos operadores";
    strings["file is corrupt or unreadable at wavetables"].plurals[0] = "o arquivo está corrompido ou ilegível nas wavetables";
    strings["file is corrupt or unreadable at effect columns"].plurals[0] = "o arquivo está corrompido ou ilegível nas colunas de efeito";
    strings["file is corrupt or unreadable at samples"].plurals[0] = "o arquivo está corrompido ou ilegível nas samples";
    strings["invalid version to save in! this is a bug!"].plurals[0] = "versão inválida para salvar! isso é um bug!";
    strings["multiple systems not possible on .dmf"].plurals[0] = "não é possível ter múltiplos sistemas em um arquivo .dmf";
    strings["YMU759 song saving is not supported"].plurals[0] = "salvar músicas do YMU759 não é suportado";
    strings["Master System FM expansion not supported in 1.0/legacy .dmf!"].plurals[0] = "a expansão FM do Master System não é suportada na versão 1.0/legada do formato .dmf!";
    strings["NES + VRC7 not supported in 1.0/legacy .dmf!"].plurals[0] = "NES + VRC7 não é suportado na versão 1.0/legada do formato .dmf!";
    strings["FDS not supported in 1.0/legacy .dmf!"].plurals[0] = "FDS não é suportado na versão 1.0/legada do formato .dmf!";
    strings["this system is not possible on .dmf"].plurals[0] = "esse sistema não é possível no formato .dmf";
    strings["maximum .dmf song length is 127"].plurals[0] = "o comprimento máximo da música no formato .dmf é 127";
    strings["maximum number of instruments in .dmf is 128"].plurals[0] = "o número máximo de instrumentos no formato .dmf é 128";
    strings["maximum number of wavetables in .dmf is 64"].plurals[0] = "o número máximo de wavetables no formato .dmf é 64";
    strings["order %d, %d is out of range (0-127)"].plurals[0] = "a ordem %d, %d está fora do intervalo (0-127)";
    strings["only the currently selected subsong will be saved"].plurals[0] = "somente a subfaixa atualmente selecionada será salva";
    strings["grooves will not be saved"].plurals[0] = "grooves não serão salvos";
    strings["only the first two speeds will be effective"].plurals[0] = "somente as primeiras duas velocidades serão efetivas";
    strings[".dmf format does not support virtual tempo"].plurals[0] = "o formato .dmf não suporta tempo virtual";
    strings[".dmf format does not support tuning"].plurals[0] = "o formato .dmf não suporta afinação";
    strings["absolute duty/cutoff macro not available in .dmf!"].plurals[0] = "a macro de ponto de corte/ciclo de trabalho absoluto não está disponível no formato .dmf";
    strings["duty precision will be lost"].plurals[0] = "haverá perda de precisão de ciclo de trabalho";
    strings[".dmf format does not support arbitrary-pitch sample mode"].plurals[0] = "o formato .dmf não suporta modo de sample de tom arbitrário";
    strings["no FM macros in .dmf format"].plurals[0] = "não há macros FM no formato .dmf";
    strings[".dmf only supports volume or cutoff macro in C64, but not both. volume macro will be lost."].plurals[0] = ".dmf só suporta macros de volume ou ponto de corte no C64, não ambas ao mesmo tempo. a macro de volume será perdida.";
    strings["note/macro release will be converted to note off!"].plurals[0] = "repouso de nota/macro serão convertidas pra note-off!";
    strings["samples' rates will be rounded to nearest compatible value"].plurals[0] = "as taxas das samples serão arredondadas para os valores compatíveis mais próximos";

    //           src/engine/importExport/dmp.cpp

    strings["unknown instrument type %d!"].plurals[0] = "tipo de instrumento %d desconhecido!";

    //           src/engine/importExport/fc.cpp

    strings["invalid header!"].plurals[0] = "cabeçalho inválido!";

    //           src/engine/importExport/ftm.cpp

    strings["incompatible version"].plurals[0] = "versão incompatível";
    strings["channel counts do not match"].plurals[0] = "a quantidade de canais não corresponde ao esperado";
    strings["too many instruments/out of range"].plurals[0] = "há muitos instrumentos/fora do intervalo";
    strings["invalid instrument type"].plurals[0] = "tipo de instrumento inválido";
    strings["too many sequences"].plurals[0] = "há muitas macros";
    strings["sequences block version is too old"].plurals[0] = "a versão do bloco de macros é muito antiga";
    strings["unknown block "].plurals[0] = "bloco desconhecido: ";
    strings["incomplete block "].plurals[0] = "bloco incompleto: ";
    strings[" [VRC6 copy]"].plurals[0] = " [cópia do VRC6]";
    strings[" [VRC6 saw copy]"].plurals[0] = " [cópia do VRC6 serra]";
    strings[" [NES copy]"].plurals[0] = " [cópia do NES]";

    //           src/engine/importExport/gyb.cpp

    strings["GYBv3 file appears to have invalid data offsets."].plurals[0] = "o arquivo GYBv3 parece ter ponteiros de dados inválidos.";
    strings["Invalid value found in patch file. %s"].plurals[0] = "Dados inválidos encontrados no arquivo de timbre. %s";

    //           src/engine/importExport/s3i.cpp

    strings["S3I PCM samples currently not supported."].plurals[0] = "samples PCM S3I atualmente não são suportadas.";

    //           src/engine/vgmOps.cpp

    strings["VGM version is too low"].plurals[0] = "a versão do VGM é muito antiga";

    //names of memory composition memories

    strings["DPCM"].plurals[0] = "DPCM";
    strings["Chip Memory"].plurals[0] = "Memória do Chip";
    strings["Sample ROM"].plurals[0] = "ROM de Sample";
    strings["Sample Memory"].plurals[0] = "Memória de Sample";
    strings["SPC/DSP Memory"].plurals[0] = "Memória do S-SMP/DSP"; // terminologia correta, a gente se vê por aqui!
    strings["Sample RAM"].plurals[0] = "RAM de Sample";
    strings["ADPCM"].plurals[0] = "ADPCM";

    //names of memory entries

    strings["Sample"].plurals[0] = "Sample";
    strings["Wave RAM"].plurals[0] = "RAM de Onda";
    strings["End of Sample"].plurals[0] = "Final da Sample";
    strings["Wavetable RAM"].plurals[0] = "RAM de Wavetable";
    strings["Reserved wavetable RAM"].plurals[0] = "RAM de wavetable reservada";
    strings["Phrase Book"].plurals[0] = "Livro de Frases";
    strings["Channel %d (load)"].plurals[0] = "Canal %d (carregar)";
    strings["Channel %d (play)"].plurals[0] = "Canal %d (reproduzir)";
    strings["Channel %d"].plurals[0] = "Canal %d";
    strings["Buffer %d Left"].plurals[0] = "Buffer %d Esquerdo";
    strings["Buffer %d Right"].plurals[0] = "Buffer %d Direito";
    strings["Registers"].plurals[0] = "Registradores";
    strings["PCM"].plurals[0] = "PCM";
    strings["ADPCM"].plurals[0] = "ADPCM";
    strings["State"].plurals[0] = "Estado";
    strings["Sample Directory"].plurals[0] = "Diretório de Sample";
    strings["Echo Buffer"].plurals[0] = "Buffer de Eco";
    strings["Mix/Echo Buffer"].plurals[0] = "Buffer de Mix/Eco";
    strings["Main Memory"].plurals[0] = "Memória Principal";
    
    //   sesd    src/engine/sysDef.cpp

    strings["help! what's going on!"].plurals[0] = "socorro! o que está acontecendo?!";
    strings["Sunsoft 5B standalone##sesd"].plurals[0] = "Sunsoft 5B independente";
    strings["Sunsoft 5B standalone (PAL)##sesd"].plurals[0] = "Sunsoft 5B independente (PAL)";
    strings["Overclocked Sunsoft 5B##sesd"].plurals[0] = "Sunsoft 5B (com overclock)";
    strings["Sega Genesis Extended Channel 3##sesd0"].plurals[0] = "Sega Mega Drive com Canal 3 Estendido";
    strings["NTSC-J Sega Master System + drums##sesd"].plurals[0] = "Sega Master System NTSC-J + bateria";
    strings["MSX-MUSIC + drums##sesd"].plurals[0] = "MSX-MUSIC + bateria";
    strings["Commodore 64 with dual 6581##sesd"].plurals[0] = "Commodore 64 com dois 6581";
    strings["Commodore 64 with dual 8580##sesd"].plurals[0] = "Commodore 64 com dois 8580";
    strings["YM2151 + SegaPCM Arcade (compatibility)##sesd"].plurals[0] = "Arcade com YM2151 + SegaPCM (compatibilidade)";
    strings["YM2151 + SegaPCM Arcade##sesd"].plurals[0] = "Arcade com YM2151 + SegaPCM";
    strings["Game Boy with AY expansion##sesd"].plurals[0] = "Game Boy with expansão AY";
    strings["multi-system##sesd"].plurals[0] = "multi-sistema";
    strings["Unknown##sesd"].plurals[0] = "Desconhecido";
    strings["15xx: Set raw period##sesd"].plurals[0] = "15xx: Definir período (val. registrador)";
    strings["16xx: Set raw period higher nybble (0-F)##sesd"].plurals[0] = "16xx: Definir nibble superior do período (val. registrador) (0-F)";
    strings["20xx: Set channel mode (bit 0: square; bit 1: noise; bit 2: envelope)##sesd"].plurals[0] = "20xx: Definir modo do canal (bit 0: onda quadrada; bit 1: ruído; bit 2: envelope)";
    strings["21xx: Set noise frequency (0 to 1F)##sesd"].plurals[0] = "21xx: Definir frequência do ruído (0 a 1F)";
    strings["22xy: Set envelope mode (x: shape, y: enable for this channel)##sesd"].plurals[0] = "22xy: Definir modo do envelope (x: formato, y: habilitar para este canal)";
    strings["23xx: Set envelope period low byte##sesd"].plurals[0] = "23xx: Definir byte inferior do período do envelope";
    strings["24xx: Set envelope period high byte##sesd"].plurals[0] = "24xx: Definir byte superior do período do envelope";
    strings["25xx: Envelope slide up##sesd0"].plurals[0] = "25xx: Slide de envelope para cima";
    strings["26xx: Envelope slide down##sesd0"].plurals[0] = "26xx: Slide de envelope para baixo";
    strings["29xy: Set auto-envelope (x: numerator; y: denominator)##sesd0"].plurals[0] = "29xy: Definir envelope automático (x: numerador; y: denominador)";
    strings["2Exx: Write to I/O port A##sesd"].plurals[0] = "2Exx: Escrever na porta E/S A";
    strings["2Fxx: Write to I/O port B##sesd"].plurals[0] = "2Fxx: Escrever na porta E/S B";
    strings["12xx: Set duty cycle (0 to 8)##sesd"].plurals[0] = "12xx: Definir ciclo de trabalho (0 a 8)";
    strings["16xx: Set raw period higher byte##sesd"].plurals[0] = "16xx: Definir byte superior do período (val. registrador)";
    strings["27xx: Set noise AND mask##sesd"].plurals[0] = "27xx: Definir máscara AND do ruído";
    strings["28xx: Set noise OR mask##sesd"].plurals[0] = "28xx: Definir máscara OR do ruído";
    strings["2Dxx: NOT TO BE EMPLOYED BY THE COMPOSER##sesd"].plurals[0] = "2Dxx: NÃO É PARA SER USADO PELO COMPOSITOR";
    strings["30xx: Toggle hard envelope reset on new notes##sesd"].plurals[0] = "30xx: Alternar reinício brusco do envelope em novas notas";
    strings["18xx: Toggle extended channel 3 mode##sesd"].plurals[0] = "18xx: Alternar modo de canal 3 estendido";
    strings["17xx: Toggle PCM mode (LEGACY)##sesd0"].plurals[0] = "17xx: Alternar modo PCM (LEGADO)";
    strings["DFxx: Set sample playback direction (0: normal; 1: reverse)##sesd0"].plurals[0] = "DFxx: Definir direção da reprodução de samples (0: normal; 1: reversa)";
    strings["18xx: Toggle drums mode (1: enabled; 0: disabled)##sesd"].plurals[0] = "18xx: Alternar modo bateria (1: habilitado; 0: desabilitado)";
    strings["11xx: Set feedback (0 to 7)##sesd0"].plurals[0] = "11xx: Definir feedback (0 a 7)";
    strings["12xx: Set level of operator 1 (0 highest, 7F lowest)##sesd"].plurals[0] = "12xx: Definir nível do operador 1 (0 é o mais alto, 7F o mais baixo)";
    strings["13xx: Set level of operator 2 (0 highest, 7F lowest)##sesd"].plurals[0] = "13xx: Definir nível do operador 2 (0 é o mais alto, 7F o mais baixo)";
    strings["14xx: Set level of operator 3 (0 highest, 7F lowest)##sesd"].plurals[0] = "14xx: Definir nível do operador 3 (0 é o mais alto, 7F o mais baixo)";
    strings["15xx: Set level of operator 4 (0 highest, 7F lowest)##sesd"].plurals[0] = "15xx: Definir nível do operador 4 (0 é o mais alto, 7F o mais baixo)";
    strings["16xy: Set operator multiplier (x: operator from 1 to 4; y: multiplier)##sesd0"].plurals[0] = "16xy: Definir multiplicador do operador (x: operador de 1 a 4; y: multiplicador)";
    strings["19xx: Set attack of all operators (0 to 1F)##sesd"].plurals[0] = "19xx: Definir ataque de todos os operadores (0 a 1F)";
    strings["1Axx: Set attack of operator 1 (0 to 1F)##sesd"].plurals[0] = "1Axx: Definir ataque do operador 1 (0 a 1F)";
    strings["1Bxx: Set attack of operator 2 (0 to 1F)##sesd"].plurals[0] = "1Bxx: Definir ataque do operador 2 (0 a 1F)";
    strings["1Cxx: Set attack of operator 3 (0 to 1F)##sesd"].plurals[0] = "1Cxx: Definir ataque do operador 3 (0 a 1F)";
    strings["1Dxx: Set attack of operator 4 (0 to 1F)##sesd"].plurals[0] = "1Dxx: Definir ataque do operador 4 (0 a 1F)";
    strings["50xy: Set AM (x: operator from 1 to 4 (0 for all ops); y: AM)##sesd0"].plurals[0] = "50xy: Definir AM (x: operador de 1 a 4 (0 para todos os ops.); y: AM)";
    strings["51xy: Set sustain level (x: operator from 1 to 4 (0 for all ops); y: sustain)##sesd0"].plurals[0] = "51xy: Definir nível de sustentação (x: operador de 1 a 4 (0 para todos os ops.); y: sustentação)";
    strings["52xy: Set release (x: operator from 1 to 4 (0 for all ops); y: release)##sesd0"].plurals[0] = "52xy: Definir repouso (x: operador de 1 a 4 (0 para todos os ops.); y: repouso)";
    strings["53xy: Set detune (x: operator from 1 to 4 (0 for all ops); y: detune where 3 is center)##sesd"].plurals[0] = "53xy: Definir desafinação (x: operador de 1 a 4 (0 para todos os ops.); y: desafinação, onde 3 é o centro)";
    strings["54xy: Set envelope scale (x: operator from 1 to 4 (0 for all ops); y: scale from 0 to 3)##sesd0"].plurals[0] = "54xy: Definir escalonamento do envelope (x: operador de 1 a 4 (0 para todos os ops.); y: escala de 0 a 3)";
    strings["56xx: Set decay of all operators (0 to 1F)##sesd"].plurals[0] = "56xx: Definir decaimento de todos os operadores (0 a 1F)";
    strings["57xx: Set decay of operator 1 (0 to 1F)##sesd"].plurals[0] = "57xx: Definir decaimento do operador 1 (0 a 1F)";
    strings["58xx: Set decay of operator 2 (0 to 1F)##sesd"].plurals[0] = "58xx: Definir decaimento do operador 2 (0 a 1F)";
    strings["59xx: Set decay of operator 3 (0 to 1F)##sesd"].plurals[0] = "59xx: Definir decaimento do operador 3 (0 a 1F)";
    strings["5Axx: Set decay of operator 4 (0 to 1F)##sesd"].plurals[0] = "5Axx: Definir decaimento do operador 4 (0 a 1F)";
    strings["5Bxx: Set decay 2 of all operators (0 to 1F)##sesd"].plurals[0] = "5Bxx: Definir decaimento 2 de todos os operadores (0 a 1F)";
    strings["5Cxx: Set decay 2 of operator 1 (0 to 1F)##sesd"].plurals[0] = "5Cxx: Definir decaimento 2 do operador 1 (0 a 1F)";
    strings["5Dxx: Set decay 2 of operator 2 (0 to 1F)##sesd"].plurals[0] = "5Dxx: Definir decaimento 2 do operador 2 (0 a 1F)";
    strings["5Exx: Set decay 2 of operator 3 (0 to 1F)##sesd"].plurals[0] = "5Exx: Definir decaimento 2 do operador 3 (0 a 1F)";
    strings["5Fxx: Set decay 2 of operator 4 (0 to 1F)##sesd"].plurals[0] = "5Fxx: Definir decaimento 2 do operador 4 (0 a 1F)";
    strings["10xx: Set noise frequency (xx: value; 0 disables noise)##sesd"].plurals[0] = "10xx: Definir frequência do ruído (xx: valor; 0 desabilita o ruído)";
    strings["17xx: Set LFO speed##sesd"].plurals[0] = "17xx: Definir velocidade do LFO";
    strings["18xx: Set LFO waveform (0 saw, 1 square, 2 triangle, 3 noise)##sesd"].plurals[0] = "18xx: Definir forma de onda do LFO (0 serra, 1 quadrada, 2 triângulo, 3 ruído)";
    strings["1Exx: Set AM depth (0 to 7F)##sesd"].plurals[0] = "1Exx: Definir profundidade do AM (0 a 7F)";
    strings["1Fxx: Set PM depth (0 to 7F)##sesd"].plurals[0] = "1Fxx: Definir profundidade do PM (0 a 7F)";
    strings["55xy: Set detune 2 (x: operator from 1 to 4 (0 for all ops); y: detune from 0 to 3)##sesd"].plurals[0] = "55xy: Definir desafinação 2 (x: operador de 1 a 4 (0 para todos os ops.); y: desafinação de 0 a 3)";
    strings["24xx: Set LFO 2 speed##sesd"].plurals[0] = "24xx: Definir velocidade do LFO 2";
    strings["25xx: Set LFO 2 waveform (0 saw, 1 square, 2 triangle, 3 noise)##sesd"].plurals[0] = "25xx: Definir forma de onda do LFO 2 (0 serra, 1 quadrada, 2 triângulo, 3 ruído)";
    strings["26xx: Set AM 2 depth (0 to 7F)##sesd"].plurals[0] = "26xx: Definir profundidade do AM 2 (0 ta 7F)";
    strings["27xx: Set PM 2 depth (0 to 7F)##sesd"].plurals[0] = "27xx: Definir profundidade do PM 2 (0 ta 7F)";
    strings["28xy: Set reverb (x: operator from 1 to 4 (0 for all ops); y: reverb from 0 to 7)##sesd"].plurals[0] = "28xy: Definir reverb (x: operador de 1 a 4 (0 para todos os ops.); y: reverb de 0 a 7)";
    strings["2Axy: Set waveform (x: operator from 1 to 4 (0 for all ops); y: waveform from 0 to 7)##sesd0"].plurals[0] = "2Axy: Definir forma de onda (x: operador de 1 a 4 (0 para todos os ops.); y: forma de onda de 0 a 7)";
    strings["2Bxy: Set envelope generator shift (x: operator from 1 to 4 (0 for all ops); y: shift from 0 to 3)##sesd"].plurals[0] = "2Bxy: Definir deslocamento do gerador de envelope x: operador de 1 a 4 (0 para todos os ops.); y: deslocamento de 0 a 3)";
    strings["2Cxy: Set fine multiplier (x: operator from 1 to 4 (0 for all ops); y: fine)##sesd"].plurals[0] = "2Cxy: Definir multiplicador fino (x: operador de 1 a 4 (0 para todos os ops.); y: multiplicador fino)";
    strings["3xyy: Set fixed frequency of operator 1 (x: octave from 0 to 7; y: frequency)##sesd"].plurals[0] = "3xyy: Definir frequência fixa do operador 1 (x: oitava de 0 a 7; y: frequência)";
    strings["3xyy: Set fixed frequency of operator 2 (x: octave from 8 to F; y: frequency)##sesd"].plurals[0] = "3xyy: Definir frequência fixa do operador 2 (x: oitava de 8 a F; y: frequência)";
    strings["4xyy: Set fixed frequency of operator 3 (x: octave from 0 to 7; y: frequency)##sesd"].plurals[0] = "4xyy: Definir frequência fixa do operador 3 (x: oitava de 0 a 7; y: frequência)";
    strings["4xyy: Set fixed frequency of operator 4 (x: octave from 8 to F; y: frequency)##sesd"].plurals[0] = "4xyy: Definir frequência fixa do operador 4 (x: oitava de 8 a F; y: frequência)";
    strings["10xy: Setup LFO (x: enable; y: speed)##sesd"].plurals[0] = "10xy: Configurar LFO (x: habilitar; y: velocidade)";
    strings["55xy: Set SSG envelope (x: operator from 1 to 4 (0 for all ops); y: 0-7 on, 8 off)##sesd"].plurals[0] = "55xy: Definir envelope SSG (x: operador de 1 a 4 (0 para todos os ops.); y: 0-7 ligado, 8 desligado)";
    strings["1Fxx: Set ADPCM-A global volume (0 to 3F)##sesd"].plurals[0] = "1Fxx: Definir volume global do ADPCM-A (0 a 3F)";
    strings["11xx: Set feedback (0 to 7)##sesd1"].plurals[0] = "11xx: Definir feedback (0 a 7)";
    strings["12xx: Set level of operator 1 (0 highest, 3F lowest)##sesd0"].plurals[0] = "12xx: Definir nível do operador 1 (0 é o mais alto, 3F o mais baixo)";
    strings["13xx: Set level of operator 2 (0 highest, 3F lowest)##sesd0"].plurals[0] = "13xx: Definir nível do operador 2 (0 é o mais alto, 3F o mais baixo)";
    strings["16xy: Set operator multiplier (x: operator from 1 to 2; y: multiplier)##sesd"].plurals[0] = "16xy: Definir multiplicador do operador (x: operator de 1 a 2; y: multiplicador)";
    strings["19xx: Set attack of all operators (0 to F)##sesd0"].plurals[0] = "19xx: Definir ataque de todos os operadores (0 a F)";
    strings["1Axx: Set attack of operator 1 (0 to F)##sesd0"].plurals[0] = "1Axx: Definir ataque do operador 1 (0 a F)";
    strings["1Bxx: Set attack of operator 2 (0 to F)##sesd0"].plurals[0] = "1Bxx: Definir ataque do operador 2 (0 a F)";
    strings["50xy: Set AM (x: operator from 1 to 2 (0 for all ops); y: AM)##sesd"].plurals[0] = "50xy: Definir AM (x: operador de 1 a 2 (0 para todos os ops.); y: AM)";
    strings["51xy: Set sustain level (x: operator from 1 to 2 (0 for all ops); y: sustain)##sesd"].plurals[0] = "51xy: Definir nível de sustentação (x: operador de 1 a 2 (0 para todos os ops.); y: sustentação)";
    strings["52xy: Set release (x: operator from 1 to 2 (0 for all ops); y: release)##sesd"].plurals[0] = "52xy: Definir repouso (x: operador de 1 a 2 (0 para todos os ops.); y: repouso)";
    strings["53xy: Set vibrato (x: operator from 1 to 2 (0 for all ops); y: enabled)##sesd"].plurals[0] = "53xy: Definir vibrato (x: operador de 1 a 2 (0 para todos os ops.); y: habilitado)";
    strings["54xy: Set envelope scale (x: operator from 1 to 2 (0 for all ops); y: scale from 0 to 3)##sesd"].plurals[0] = "54xy: Definir escalonamento de envelope (x: operador de 1 a 2 (0 para todos os ops.); y: escala de 0 a 3)";
    strings["55xy: Set envelope sustain (x: operator from 1 to 2 (0 for all ops); y: enabled)##sesd"].plurals[0] = "55xy: Definir modo de sustentação de envelope (x: operador de 1 a 2 (0 para todos os ops.); y: habilitado)";
    strings["56xx: Set decay of all operators (0 to F)##sesd0"].plurals[0] = "56xx: Definir decaimento de todos os operadores (0 a F)";
    strings["57xx: Set decay of operator 1 (0 to F)##sesd0"].plurals[0] = "57xx: Definir decaimento do operador 1 (0 a F)";
    strings["58xx: Set decay of operator 2 (0 to F)##sesd0"].plurals[0] = "58xx: Definir decaimento do operador 2 (0 a F)";
    strings["5Bxy: Set whether key will scale envelope (x: operator from 1 to 2 (0 for all ops); y: enabled)##sesd"].plurals[0] = "5Bxy: Definir se a nota irá escalonar a taxa do envelope (x: operador de 1 a 2 (0 para todos os ops.); y: habilitado)";
    strings["10xx: Set global AM depth (0: 1dB, 1: 4.8dB)##sesd"].plurals[0] = "10xx: Definir profundidade de AM global (0: 1dB, 1: 4.8dB)";
    strings["11xx: Set feedback (0 to 7)##sesd2"].plurals[0] = "11xx: Definir feedback (0 a 7)";
    strings["12xx: Set level of operator 1 (0 highest, 3F lowest)##sesd1"].plurals[0] = "12xx: Definir nível do operador 1 (0 é o mais alto, 3F o mais baixo)";
    strings["13xx: Set level of operator 2 (0 highest, 3F lowest)##sesd1"].plurals[0] = "13xx: Definir nível do operador 2 (0 é o mais alto, 3F o mais baixo)";
    strings["14xx: Set level of operator 3 (0 highest, 3F lowest)##sesd0"].plurals[0] = "14xx: Definir nível do operador 3 (0 é o mais alto, 3F o mais baixo)";
    strings["15xx: Set level of operator 4 (0 highest, 3F lowest)##sesd0"].plurals[0] = "15xx: Definir nível do operador 4 (0 é o mais alto, 3F o mais baixo)";
    strings["16xy: Set operator multiplier (x: operator from 1 to 4; y: multiplier)##sesd1"].plurals[0] = "16xy: Definir multiplicador do operador (x: operador de 1 a 4; y: multiplicador)";
    strings["17xx: Set global vibrato depth (0: normal, 1: double)##sesd"].plurals[0] = "17xx: Definir profundidade de vibrato global (0: normal, 1: dobro)";
    strings["19xx: Set attack of all operators (0 to F)##sesd1"].plurals[0] = "19xx: Definir ataque de todos os operadores (0 a F)";
    strings["1Axx: Set attack of operator 1 (0 to F)##sesd1"].plurals[0] = "1Axx: Definir ataque do operador 1 (0 a F)";
    strings["1Bxx: Set attack of operator 2 (0 to F)##sesd1"].plurals[0] = "1Bxx: Definir ataque do operador 2 (0 a F)";
    strings["1Cxx: Set attack of operator 3 (0 to F)##sesd0"].plurals[0] = "1Cxx: Definir ataque do operador 3 (0 a F)";
    strings["1Dxx: Set attack of operator 4 (0 to F)##sesd0"].plurals[0] = "1Dxx: Definir ataque do operador 4 (0 a F)";
    strings["2Axy: Set waveform (x: operator from 1 to 4 (0 for all ops); y: waveform from 0 to 3 in OPL2 and 0 to 7 in OPL3)##sesd"].plurals[0] = "2Axy: Definir forma de onda (x: operador de 1 a 4 (0 para todos os ops.); y: forma de onda de 0 a 3 no OPL2 e 0 a 7 no OPL3)";
    strings["50xy: Set AM (x: operator from 1 to 4 (0 for all ops); y: AM)##sesd1"].plurals[0] = "50xy: Definir AM (x: operador de 1 a 4 (0 para todos os ops.); y: AM)";
    strings["51xy: Set sustain level (x: operator from 1 to 4 (0 for all ops); y: sustain)##sesd1"].plurals[0] = "51xy: Definir nível de sustentação (x: operador de 1 a 4 (0 para todos os ops.); y: sustentação)";
    strings["52xy: Set release (x: operator from 1 to 4 (0 for all ops); y: release)##sesd1"].plurals[0] = "52xy: Definir repouso (x: operador de 1 a 4 (0 para todos os ops.); y: repouso)";
    strings["53xy: Set vibrato (x: operator from 1 to 4 (0 for all ops); y: enabled)##sesd0"].plurals[0] = "53xy: Definir vibrato (x: operador de 1 a 4 (0 para todos os ops.); y: habilitado)";
    strings["54xy: Set envelope scale (x: operator from 1 to 4 (0 for all ops); y: scale from 0 to 3)##sesd1"].plurals[0] = "54xy: Definir escalonamento de envelope (x: operador de 1 a 4 (0 para todos os ops.); y: escala de 0 a 3)";
    strings["55xy: Set envelope sustain (x: operator from 1 to 4 (0 for all ops); y: enabled)##sesd0"].plurals[0] = "55xy: Definir modo de sustentação de envelope (x: operador de 1 a 4 (0 para todos os ops.); y: habilitado)";
    strings["56xx: Set decay of all operators (0 to F)##sesd1"].plurals[0] = "56xx: Definir decaimento de todos os operadores (0 a F)";
    strings["57xx: Set decay of operator 1 (0 to F)##sesd1"].plurals[0] = "57xx: Definir decaimento do operador 1 (0 a F)";
    strings["58xx: Set decay of operator 2 (0 to F)##sesd1"].plurals[0] = "58xx: Definir decaimento do operador 2 (0 a F)";
    strings["59xx: Set decay of operator 3 (0 to F)##sesd0"].plurals[0] = "59xx: Definir decaimento do operador 3 (0 a F)";
    strings["5Axx: Set decay of operator 4 (0 to F)##sesd0"].plurals[0] = "5Axx: Definir decaimento do operador 4 (0 a F)";
    strings["5Bxy: Set whether key will scale envelope (x: operator from 1 to 4 (0 for all ops); y: enabled)##sesd0"].plurals[0] = "5Bxy: Definir se a nota irá escalonar a taxa do envelope (x: operador de 1 a 4 (0 para todos os ops.); y: habilitado)";
    strings["10xx: Set waveform (bit 0: triangle; bit 1: saw; bit 2: pulse; bit 3: noise)##sesd"].plurals[0] = "10xx: Definir forma de onda (bit 0: triângulo; bit 1: serra; bit 2: pulso; bit 3: ruído)";
    strings["11xx: Set coarse cutoff (not recommended; use 4xxx instead)##sesd"].plurals[0] = "11xx: Definir ponto de corte grosso (não recomendado; use o 4xxx ao invés desse)";
    strings["12xx: Set coarse pulse width (not recommended; use 3xxx instead)##sesd"].plurals[0] = "12xx: Definir largura de pulso grossa (não recomendado, use o 3xxx ao invés desse)";
    strings["13xx: Set resonance (0 to F)##sesd"].plurals[0] = "13xx: Definir ressonância (0 a F)";
    strings["14xx: Set filter mode (bit 0: low pass; bit 1: band pass; bit 2: high pass)##sesd"].plurals[0] = "14xx: Definir modo de filtro (bit 0: passa baixa; bit 1: passa banda; bit 2: passa alta)";
    strings["15xx: Set envelope reset time##sesd"].plurals[0] = "15xx: Definir tempo de reinício do envelope";
    strings["1Axx: Disable envelope reset for this channel (1 disables; 0 enables)##sesd"].plurals[0] = "1Axx: Desabilitar reinício do envelope para este canal (1 desabilita; 0 habilita)";
    strings["1Bxy: Reset cutoff (x: on new note; y: now)##sesd"].plurals[0] = "1Bxy: Redefinir ponto de corte (x: em nova nota; y: agora)";
    strings["1Cxy: Reset pulse width (x: on new note; y: now)##sesd"].plurals[0] = "1Cxy: Redefinir largura de pulso (x: em nova nota; y: agora)";
    strings["1Exy: Change other parameters (LEGACY)##sesd"].plurals[0] = "1Exy: Alterar outros parâmetros (LEGADO)";
    strings["20xy: Set attack/decay (x: attack; y: decay)##sesd"].plurals[0] = "20xy: Definir ataque/decaimento (x: ataque; y: decaimento)";
    strings["21xy: Set sustain/release (x: sustain; y: release)##sesd"].plurals[0] = "21xy: Definir sustentação/repouso (x: sustentação; y: repouso)";
    strings["22xx: Pulse width slide up##sesd"].plurals[0] = "22xx: Slide de largura de pulso para cima";
    strings["23xx: Pulse width slide down##sesd"].plurals[0] = "23xx: Slide de largura de pulso para baixo";
    strings["24xx: Cutoff slide up##sesd"].plurals[0] = "24xx: Slide de ponto de corte para cima";
    strings["25xx: Cutoff slide down##sesd"].plurals[0] = "25xx: Slide de ponto de corte para baixo";
    strings["3xxx: Set pulse width (0 to FFF)##sesd"].plurals[0] = "3xxx: Definir largura de pulso (0 a FFF)";
    strings["4xxx: Set cutoff (0 to 7FF)##sesd"].plurals[0] = "4xxx: Definir ponto de corte (0 to 7FF)";
    strings["10xx: Set waveform##sesd0"].plurals[0] = "10xx: Definir forma de onda";
    strings["13xx: Set waveform (local)##sesd"].plurals[0] = "13xx: Definir forma de onda (local)";
    strings["11xx: Set raw period##sesd"].plurals[0] = "11xx: Definir período (val. registrador) (0-1F)";
    strings["11xx: Set waveform (local)##sesd"].plurals[0] = "11xx: Definir forma de onda (local)";
    strings["20xx: Set PCM frequency##sesd"].plurals[0] = "20xx: Definir frequência do PCM";
    strings["10xy: Set AM depth (x: operator from 1 to 4 (0 for all ops); y: depth (0: 1dB, 1: 4.8dB))##sesd"].plurals[0] = "10xy: Definir profundidade do AM (x: operador de 1 a 4 (0 para todos os ops.); y: profundidade (0: 1dB, 1: 4.8dB))";
    strings["12xx: Set level of operator 1 (0 highest, 3F lowest)##sesd2"].plurals[0] = "12xx: Definir nível do operador 1 (0 é o mais alto, 3F o mais baixo)";
    strings["13xx: Set level of operator 2 (0 highest, 3F lowest)##sesd2"].plurals[0] = "13xx: Definir nível do operador 2 (0 é o mais alto, 3F o mais baixo)";
    strings["14xx: Set level of operator 3 (0 highest, 3F lowest)##sesd1"].plurals[0] = "14xx: Definir nível do operador 3 (0 é o mais alto, 3F o mais baixo)";
    strings["15xx: Set level of operator 4 (0 highest, 3F lowest)##sesd1"].plurals[0] = "15xx: Definir nível do operador 4 (0 é o mais alto, 3F o mais baixo)";
    strings["16xy: Set operator multiplier (x: operator from 1 to 4; y: multiplier)##sesd2"].plurals[0] = "16xy: Definir multiplicador do operador (x: operador de 1 a 4; y: multiplicador)";
    strings["17xy: Set vibrato depth (x: operator from 1 to 4 (0 for all ops); y: depth (0: normal, 1: double))##sesd"].plurals[0] = "17xy: Definir profundidade do vibrato (x: operador de 1 a 4 (0 para todos os ops.); y: profundidade (0: normal, 1: dobro))";
    strings["19xx: Set attack of all operators (0 to F)##sesd"].plurals[0] = "19xx: Definir ataque de todos os operadores (0 a F)";
    strings["1Axx: Set attack of operator 1 (0 to F)##sesd2"].plurals[0] = "1Axx: Definir ataque do operador 1 (0 a F)";
    strings["1Bxx: Set attack of operator 2 (0 to F)##sesd2"].plurals[0] = "1Bxx: Definir ataque do operador 2 (0 a F)";
    strings["1Cxx: Set attack of operator 3 (0 to F)##sesd1"].plurals[0] = "1Cxx: Definir ataque do operador 3 (0 a F)";
    strings["1Dxx: Set attack of operator 4 (0 to F)##sesd1"].plurals[0] = "1Dxx: Definir ataque do operador 4 (0 a F)";
    strings["20xy: Set panning of operator 1 (x: left; y: right)##sesd"].plurals[0] = "20xy: Definir balanço do operador 1 (x: esquerda; y: direita)";
    strings["21xy: Set panning of operator 2 (x: left; y: right)##sesd"].plurals[0] = "21xy: Definir balanço do operador 2 (x: esquerda; y: direita)";
    strings["22xy: Set panning of operator 3 (x: left; y: right)##sesd"].plurals[0] = "22xy: Definir balanço do operador 3 (x: esquerda; y: direita)";
    strings["23xy: Set panning of operator 4 (x: left; y: right)##sesd"].plurals[0] = "23xy: Definir balanço do operador 4 (x: esquerda; y: direita)";
    strings["24xy: Set output level register (x: operator from 1 to 4 (0 for all ops); y: level from 0 to 7)##sesd"].plurals[0] = "24xy: Definir registrador de nível de saída (x: operador de 1 a 4 (0 para todos os ops.); y: nível de 0 a 7)";
    strings["25xy: Set modulation input level (x: operator from 1 to 4 (0 for all ops); y: level from 0 to 7)##sesd"].plurals[0] = "25xy: Definir nível de entrada de modulação (x: operador de 1 a 4 (0 para todos os ops.); y: nível de 0 a 7)";
    strings["26xy: Set envelope delay (x: operator from 1 to 4 (0 for all ops); y: delay from 0 to 7)##sesd"].plurals[0] = "26xy: Definir atraso de envelope (x: operador de 1 a 4 (0 para todos os ops.); y: atraso de 0 a 7)";
    strings["27xx: Set noise mode for operator 4 (x: mode from 0 to 3)##sesd"].plurals[0] = "27xx: Definir modo de ruído do operador 4 (x: modo de 0 a 3)";
    strings["2Axy: Set waveform (x: operator from 1 to 4 (0 for all ops); y: waveform from 0 to 7)##sesd1"].plurals[0] = "2Axy: Definir forma de onda (x: operador de 1 a 4 (0 para todos os ops.); y: forma de onda de 0 a 7)";
    strings["2Fxy: Set fixed frequency block (x: operator from 1 to 4; y: octave from 0 to 7)##sesd"].plurals[0] = "2Fxy: Definir bloco da frequência fixa (x: operador de 1 a 4; y: oitava de 0 a 7)";
    strings["40xx: Set detune of operator 1 (80: center)##sesd"].plurals[0] = "40xx: Definir desafinação do operador 1 (80: centro)";
    strings["41xx: Set detune of operator 2 (80: center)##sesd"].plurals[0] = "41xx: Definir desafinação do operador 2 (80: centro)";
    strings["42xx: Set detune of operator 3 (80: center)##sesd"].plurals[0] = "42xx: Definir desafinação do operador 3 (80: centro)";
    strings["43xx: Set detune of operator 4 (80: center)##sesd"].plurals[0] = "43xx: Definir desafinação do operador 4 (80: centro)";
    strings["50xy: Set AM (x: operator from 1 to 4 (0 for all ops); y: AM)##sesd2"].plurals[0] = "50xy: Set AM (x: operador de 1 a 4 (0 para todos os ops.); y: AM)";
    strings["51xy: Set sustain level (x: operator from 1 to 4 (0 for all ops); y: sustain)##sesd2"].plurals[0] = "51xy: Definir nível de sustentação (x: operador de 1 a 4 (0 para todos os ops.); y: sustentação)";
    strings["52xy: Set release (x: operator from 1 to 4 (0 for all ops); y: release)##sesd2"].plurals[0] = "52xy: Definir repouso (x: operador de 1 a 4 (0 para todos os ops.); y: repouso)";
    strings["53xy: Set vibrato (x: operator from 1 to 4 (0 for all ops); y: enabled)##sesd1"].plurals[0] = "53xy: Definir vibrato (x: operador de 1 a 4 (0 para todos os ops.); y: habilitado)";
    strings["54xy: Set envelope scale (x: operator from 1 to 4 (0 for all ops); y: scale from 0 to 3)##sesd2"].plurals[0] = "54xy: Definir escalonamento de envelope (x: operador de 1 a 4 (0 para todos os ops.); y: escala de 0 a 3)";
    strings["55xy: Set envelope sustain (x: operator from 1 to 4 (0 for all ops); y: enabled)##sesd1"].plurals[0] = "55xy: Definir modo de sustentação de envelope (x: operador de 1 a 4 (0 para todos os ops.); y: habilitado)";
    strings["56xx: Set decay of all operators (0 to F)##sesd2"].plurals[0] = "56xx: Definir decaimento de todos os operadores (0 a F)";
    strings["57xx: Set decay of operator 1 (0 to F)##sesd2"].plurals[0] = "57xx: Definir decaimento do operador 1 (0 a F)";
    strings["58xx: Set decay of operator 2 (0 to F)##sesd2"].plurals[0] = "58xx: Definir decaimento do operador 2 (0 a F)";
    strings["59xx: Set decay of operator 3 (0 to F)##sesd1"].plurals[0] = "59xx: Definir decaimento do operador 3 (0 a F)";
    strings["5Axx: Set decay of operator 4 (0 to F)##sesd1"].plurals[0] = "5Axx: Definir decaimento do operador 4 (0 a F)";
    strings["5Bxy: Set whether key will scale envelope (x: operator from 1 to 4 (0 for all ops); y: enabled)##sesd1"].plurals[0] = "5Bxy: Definir se a nota irá escalonar a taxa do envalope (x: operador de 1 a 4 (0 para todos os ops.); y: habilitado)";
    strings["3xyy: Set fixed frequency F-num of operator 1 (x: high 2 bits from 0 to 3; y: low 8 bits of F-num)##sesd"].plurals[0] = "3xyy: Definir Número-Frequência da frequência fixa do operador 1 (x: 2 bits superiores de 0 a 3; y: 8 bits inferiores)";
    strings["3xyy: Set fixed frequency F-num of operator 2 (x: high 2 bits from 4 to 7; y: low 8 bits of F-num)##sesd"].plurals[0] = "3xyy: Definir Número-Frequência da frequência fixa do operador 2 (x: 2 bits superiores de 4 a 7; y: 8 bits inferiores)";
    strings["3xyy: Set fixed frequency F-num of operator 3 (x: high 2 bits from 8 to B; y: low 8 bits of F-num)##sesd"].plurals[0] = "3xyy: Definir Número-Frequência da frequência fixa do operador 3 (x: 2 bits superiores de 8 a B; y: 8 bits inferiores)";
    strings["3xyy: Set fixed frequency F-num of operator 4 (x: high 2 bits from C to F; y: low 8 bits of F-num)##sesd"].plurals[0] = "3xyy: Definir Número-Frequência da frequência fixa do operador 4 (x: 2 bits superiores de C a F; y: 8 bits inferiores)";
    strings["10xx: Set waveform (bit 0: triangle; bit 1: saw; bit 2: pulse; bit 3: noise)##sesd1"].plurals[0] = "10xx: Definir forma de onda (bit 0: triângulo; bit 1: serra; bit 2: pulso; bit 3: ruído)";
    strings["11xx: Set resonance (0 to FF)##sesd"].plurals[0] = "11xx: Definir ressonância (0 a FF)";
    strings["12xx: Set filter mode (bit 0: low pass; bit 1: band pass; bit 2: high pass)##sesd"].plurals[0] = "12xx: Definir modo de filtro (bit 0: passa baixa; bit 1: passa banda; bit 2: passa alta)";
    strings["13xx: Disable envelope reset for this channel (1 disables; 0 enables)##sesd"].plurals[0] = "13xx: Desabilitar reinício do envelope para este canal (1 desabilita; 0 habilita)";
    strings["14xy: Reset cutoff (x: on new note; y: now)##sesd"].plurals[0] = "14xy: Redefinir ponto de corte (x: em nova nota; y: agora)";
    strings["15xy: Reset pulse width (x: on new note; y: now)##sesd"].plurals[0] = "15xy: Redefinir largura de pulso (x: em nova nota; y: agora)";
    strings["16xy: Change other parameters##sesd"].plurals[0] = "16xy: Alterar outros parâmetros";
    strings["17xx: Pulse width slide up##sesd"].plurals[0] = "17xx: Slide de largura de pulso para cima";
    strings["18xx: Pulse width slide down##sesd"].plurals[0] = "18xx: Slide de largura de pulso para baixo";
    strings["19xx: Cutoff slide up##sesd"].plurals[0] = "19xx: Slide de ponto de corte para cima";
    strings["1Axx: Cutoff slide down##sesd"].plurals[0] = "1Axx: Slide de ponto de corte para baixo";
    strings["3xxx: Set pulse width (0 to FFF)##sesd1"].plurals[0] = "3xxx: Definir largura de pulso (0 a FFF)";
    strings["4xxx: Set cutoff (0 to FFF)##sesd1"].plurals[0] = "4xxx: Definir ponto de corte (0 a FFF)";
    strings["a chip which found its way inside mobile phones in the 2000's.\nas proprietary as it is, it passed away after losing to MP3 in the mobile hardware battle.##sesd"].plurals[0] = "um chip que encontrou seu lugar dentro de celulares nos anos 2000.\nproprietário como era, ele faleceu após perder para o MP3 na batalha de hardware móvel.";
    strings["<COMPOUND SYSTEM!>##sesd0"].plurals[0] = "<SISTEMA COMPOSTO!>";
    strings["Sega Genesis Extended Channel 3##sesd1"].plurals[0] = "Sega Mega Drive com Canal 3 Estendido";
    strings["<COMPOUND SYSTEM!>##sesd1"].plurals[0] = "<SISTEMA COMPOSTO!>";
    strings["a square/noise sound chip found on the Sega Master System, ColecoVision, Tandy, TI's own 99/4A and a few other places.##sesd"].plurals[0] = "um chip de som de onda quadrada e ruído encontrado no Sega Master System, ColecoVision, PCs Tandy, o próprio 99/4A da TI e alguns outros lugares.";
    strings["20xy: Set noise mode (x: preset freq/ch3 freq; y: thin pulse/noise)##sesd"].plurals[0] = "20xy: Definir modo de ruído (x: freq. predefinidas/freq. canal 3; y: pulso fino/ruído)";
    strings["<COMPOUND SYSTEM!>##sesd2"].plurals[0] = "<SISTEMA COMPOSTO!>";
    strings["the most popular portable game console of the era.##sesd"].plurals[0] = "o console de games portátil mais popular da sua época.";
    strings["10xx: Set waveform##sesd1"].plurals[0] = "10xx: Definir forma de onda";
    strings["11xx: Set noise length (0: long; 1: short)##sesd"].plurals[0] = "11xx: Definir comprimento do ruído (0: longo; 1: curto)";
    strings["12xx: Set duty cycle (0 to 3)##sesd"].plurals[0] = "12xx: Definir ciclo de trabalho (0 a 3)";
    strings["13xy: Setup sweep (x: time; y: shift)##sesd"].plurals[0] = "13xy: Configurar sweep (x: tempo; y: deslocamento)";
    strings["14xx: Set sweep direction (0: up; 1: down)##sesd"].plurals[0] = "14xx: Definir direção do sweep (0: para cima; 1: para baixo)";
    strings["15xx: Set waveform (local)##sesd"].plurals[0] = "15xx: Definir forma de onda (local)";
    strings["an '80s game console with a wavetable sound chip, popular in Japan.##sesd"].plurals[0] = "um console de games dos anos 80 com um chip de som wavetable, popular no Japão.";
    strings["10xx: Set waveform##sesd2"].plurals[0] = "10xx: Definir forma de onda";
    strings["11xx: Toggle noise mode##sesd0"].plurals[0] = "11xx: Alternar modo de rúido";
    strings["12xx: Setup LFO (0: disabled; 1: 1x depth; 2: 16x depth; 3: 256x depth)##sesd"].plurals[0] = "12xx: Configurar LFO (0: desabilitado; 1: profundidade 1x; 2: profundidade 16x; 3: profundidade 256x)";
    strings["13xx: Set LFO speed##sesd"].plurals[0] = "13xx: Definir velocidade do LFO";
    strings["17xx: Toggle PCM mode (LEGACY)##sesd1"].plurals[0] = "17xx: Alternar modo PCM (LEGADO)";
    strings["18xx: Set waveform (local)##sesd"].plurals[0] = "18xx: Definir forma de onda (local)";
    strings["also known as Famicom in Japan, it's the most well-known game console of the '80s.##sesd"].plurals[0] = "também conhecido como Famicom no Japão, e também conhecido como Phantom System, Top Game, Turbo Game, Dynavision, Bit System e Polystation no Brasil, é o console de jogos mais conhecido dos anos 80."; // viva os Famiclones!
    strings["11xx: Write to delta modulation counter (0 to 7F)##sesd"].plurals[0] = "11xx: Escrever no contador de modulação delta (DAC) (0 a 7F)";
    strings["12xx: Set duty cycle/noise mode (pulse: 0 to 3; noise: 0 or 1)##sesd0"].plurals[0] = "12xx: Definir ciclo de trabalho/modo de ruído (pulso: 0 a 3; ruído: 0 ou 1)";
    strings["13xy: Sweep up (x: time; y: shift)##sesd"].plurals[0] = "13xy: Realizar sweep de tom para cima (x: tempo; y: deslocamento)";
    strings["14xy: Sweep down (x: time; y: shift)##sesd"].plurals[0] = "14xy: Realizar sweep de tom para baixo (x: tempo; y: deslocamento)";
    strings["15xx: Set envelope mode (0: envelope, 1: length, 2: looping, 3: constant)##sesd"].plurals[0] = "15xx: Definir modo de envelope (0: envelope, 1: comprimento, 2: repetir, 3: constante)";
    strings["16xx: Set length counter (refer to manual for a list of values)##sesd"].plurals[0] = "16xx: Definir contador de comprimento (veja o manual para uma lista de valores)";
    strings["17xx: Set frame counter mode (0: 4-step, 1: 5-step)##sesd"].plurals[0] = "17xx: Definir modo do contador de quadros (0: 4 passos, 1: 5 passos)";
    strings["18xx: Select PCM/DPCM mode (0: PCM; 1: DPCM)##sesd"].plurals[0] = "18xx: Selecionar modo PCM/DPCM (0: PCM; 1: DPCM)";
    strings["19xx: Set triangle linear counter (0 to 7F; 80 and higher halt)##sesd"].plurals[0] = "19xx: Definir contador linear do canal triângulo (0 a 7F; valores 80 e superiores interrompem)";
    strings["20xx: Set DPCM frequency (0 to F)##sesd"].plurals[0] = "20xx: Definir frequência DPCM (0 a F)";
    strings["<COMPOUND SYSTEM!>##sesd3"].plurals[0] = "<SISTEMA COMPOSTO!>";
    strings["<COMPOUND SYSTEM!>##sesd4"].plurals[0] = "<SISTEMA COMPOSTO!>";
    strings["this computer is powered by the SID chip, which had synthesizer features like a filter and ADSR.##sesd"].plurals[0] = "este computador possui o chip de som SID, que possui recursos de sintetizador como um filtro e ADSR.";
    strings["this computer is powered by the SID chip, which had synthesizer features like a filter and ADSR.\nthis is the newer revision of the chip.##sesd"].plurals[0] = "este computador possui o chip de som SID, que possui recursos de sintetizador como um filtro e ADSR.\nessa é uma revisão mais nova do chip.";
    strings["<COMPOUND SYSTEM!>##sesd5"].plurals[0] = "<SISTEMA COMPOSTO!>";
    strings["like Neo Geo, but lacking the ADPCM-B channel since they couldn't connect the pins.##sesd"].plurals[0] = "como o Neo Geo, mas faltando o canal ADPCM-B já que eles não puderam conectar os pinos.";
    strings["Neo Geo CD Extended Channel 2##sesd"].plurals[0] = "Neo Geo CD com Canal 2 estendido";
    strings["like Neo Geo, but lacking the ADPCM-B channel since they couldn't connect the pins.\nthis one is in Extended Channel mode, which turns the second FM channel into four operators with independent notes/frequencies.##sesd"].plurals[0] = "como o Neo Geo, mas faltando o canal ADPCM-B já que eles não puderam conectar os pinos.\nesse está no modo de Canal Estendido, que transforma o segundo canal FM em quatro operadores com notas/frequências independentes.";
    strings["this chip is everywhere! ZX Spectrum, MSX, Amstrad CPC, Intellivision, Vectrex...\nthe discovery of envelope bass helped it beat the SN76489 with ease.##sesd"].plurals[0] = "esse chip está em todo lugar! ZX Spectrum, MSX, Amstrad CPC, Intellivision, Vectrex...\na descoberta do baixo com o envelope ajudou ele a ganhar do SN76489 facilmente.";
    strings["a computer from the '80s with full sampling capabilities, giving it a sound ahead of its time.##sesd"].plurals[0] = "um computador dos anos 80 com capacidade completa de amostragem (sampling), dando a ele um som à frente do seu tempo.";
    strings["10xx: Toggle filter (0 disables; 1 enables)##sesd"].plurals[0] = "10xx: Alternar filtro (0 desabilita; 1 habilita)";
    strings["11xx: Toggle AM with next channel##sesd"].plurals[0] = "11xx: Alternar AM com o canal à direita";
    strings["12xx: Toggle period modulation with next channel##sesd"].plurals[0] = "12xx: Alternar modulação de período com o canal à direita";
    strings["13xx: Set waveform##sesd"].plurals[0] = "13xx: Definir forma de onda";
    strings["14xx: Set waveform (local)##sesd"].plurals[0] = "14xx: Definir forma de onda (local)";
    strings["this was Yamaha's first integrated FM chip.\nit was used in several synthesizers, computers and arcade boards.##sesd"].plurals[0] = "esse foi o primeiro chip FM integrado da Yamaha.\nele foi usado em vários sintetizadores, computadores e placas de fliperama.";
    strings["this chip is mostly known for being in the Sega Genesis (but it also was on the FM Towns computer).##sesd"].plurals[0] = "esse chip é mais conhecido por ter sido utilizado no Sega Mega Drive (mas ele também foi usado no computador FM Towns).";
    strings["it's a challenge to make music on this chip which barely has musical capabilities...##sesd"].plurals[0] = "é um desafio fazer música nesse chip que mal tem capacidades musicais..."; // zói tu num é dj, duvido cê fazer música no atari... mas desafio é desafio zói, cê num é hardicóri???
    strings["supposedly an upgrade from the AY-3-8910, this was present on the Creative Music System (Game Blaster) and SAM Coupé.##sesd"].plurals[0] = "supostamente um upgrade do AY-3-8910, esse chip estava presente na placa Creative Music System (Game Blaster) e no computador SAM Coupé.";
    strings["10xy: Set channel mode (x: noise; y: tone)##sesd"].plurals[0] = "10xy: Definir modo do canal (x: ruído; y: tom)";
    strings["11xx: Set noise frequency##sesd"].plurals[0] = "11xx: Definir frequência do ruído";
    strings["12xx: Setup envelope (refer to docs for more information)##sesd"].plurals[0] = "12xx: Configurar envelope (confira o manual para mais informações)";
    strings["an improved version of the AY-3-8910 with a bigger frequency range, duty cycles, configurable noise and per-channel envelopes!##sesd"].plurals[0] = "uma versão melhorada do AY-3-8910 com uma faixa de frequências maior, ciclos de trabalho, ruído configurável e envelopes por canal!";
    strings["Commodore's successor to the PET.\nits square wave channels are more than just square...##sesd"].plurals[0] = "o sucessor da Commodore para o PET.\nseus canais de onda quadrada são mais do que apenas ondas quadradas...";
    strings["one channel of 1-bit wavetable which is better (and worse) than the PC Speaker.##sesd"].plurals[0] = "um canal de wavetable 1-bit que é melhor (e pior) que o PC Speaker.";
    strings["FM? nah... samples! Nintendo's answer to Sega.##sesd"].plurals[0] = "FM? nah... samples! a resposta da Nintendo para a Sega.";
    strings["18xx: Enable echo buffer##sesd"].plurals[0] = "18xx: Habilitar buffer de eco";
    strings["19xx: Set echo delay (0 to F)##sesd"].plurals[0] = "19xx: Definir atraso de eco (0 a F)";
    strings["1Axx: Set left echo volume##sesd"].plurals[0] = "1Axx: Definir volume esquerdo do eco";
    strings["1Bxx: Set right echo volume##sesd"].plurals[0] = "1Bxx: Definir volume direito do eco";
    strings["1Cxx: Set echo feedback##sesd"].plurals[0] = "1Cxx: Definir retorno do eco";
    strings["1Exx: Set dry output volume (left)##sesd"].plurals[0] = "1Exx: Definir volume de saída não-processado (esquerda)";
    strings["1Fxx: Set dry output volume (right)##sesd"].plurals[0] = "1Fxx: Definir volume de saída não-processado (direita)";
    strings["30xx: Set echo filter coefficient 0##sesd"].plurals[0] = "30xx: Definir coeficiente de filtro do eco 0";
    strings["31xx: Set echo filter coefficient 1##sesd"].plurals[0] = "31xx: Definir coeficiente de filtro do eco 1";
    strings["32xx: Set echo filter coefficient 2##sesd"].plurals[0] = "32xx: Definir coeficiente de filtro do eco 2";
    strings["33xx: Set echo filter coefficient 3##sesd"].plurals[0] = "33xx: Definir coeficiente de filtro do eco 3";
    strings["34xx: Set echo filter coefficient 4##sesd"].plurals[0] = "34xx: Definir coeficiente de filtro do eco 4";
    strings["35xx: Set echo filter coefficient 5##sesd"].plurals[0] = "35xx: Definir coeficiente de filtro do eco 5";
    strings["36xx: Set echo filter coefficient 6##sesd"].plurals[0] = "36xx: Definir coeficiente de filtro do eco 6";
    strings["37xx: Set echo filter coefficient 7##sesd"].plurals[0] = "37xx: Definir coeficiente de filtro do eco 7";
    strings["10xx: Set waveform##sesd3"].plurals[0] = "10xx: Definir forma de onda";
    strings["11xx: Toggle noise mode##sesd1"].plurals[0] = "11xx: Alternar modo de ruído";
    strings["12xx: Toggle echo on this channel##sesd"].plurals[0] = "12xx: Alternar eco neste canal";
    strings["13xx: Toggle pitch modulation##sesd"].plurals[0] = "13xx: Alternar modulação de tom";
    strings["14xy: Toggle invert (x: left; y: right)##sesd"].plurals[0] = "14xy: Alternar inversão de onda (x: esquerda; y: direita)";
    strings["15xx: Set envelope mode (0: ADSR, 1: gain/direct, 2: dec, 3: exp, 4: inc, 5: bent)##sesd"].plurals[0] = "15xx: Definir modo de envelope (0: ADSR, 1: ganho/direto, 2: decr., 3: exp., 4: incr., 5: linha torta)";
    strings["16xx: Set gain (00 to 7F if direct; 00 to 1F otherwise)##sesd"].plurals[0] = "16xx: Definir ganho (00 a 7F se for direto; 00 a 1F caso contrário)";
    strings["17xx: Set waveform (local)##sesd"].plurals[0] = "17xx: Definir forma de onda (local)";
    strings["1Dxx: Set noise frequency (00 to 1F)##sesd"].plurals[0] = "1Dxx: Definir frequência do ruído (00 a 1F)";
    strings["20xx: Set attack (0 to F)##sesd"].plurals[0] = "20xx: Definir ataque (0 a F)";
    strings["21xx: Set decay (0 to 7)##sesd"].plurals[0] = "21xx: Definir decaimento (0 a 7)";
    strings["22xx: Set sustain (0 to 7)##sesd"].plurals[0] = "22xx: Definir sustentação (0 a 7)";
    strings["23xx: Set release (00 to 1F)##sesd"].plurals[0] = "23xx: Definir repouso (00 a 1F)";
    strings["an expansion chip for the Famicom, featuring a quirky sawtooth channel.##sesd"].plurals[0] = "um chip de expansão para o Famicom, contendo um canal dente-de-serra peculiar.";
    strings["12xx: Set duty cycle (pulse: 0 to 7)##sesd"].plurals[0] = "12xx: Definir ciclo de trabalho (pulso: 0 a 7)";
    strings["17xx: Toggle PCM mode (LEGACY)##sesd2"].plurals[0] = "17xx: Alternar modo PCM (LEGADO)";
    strings["cost-reduced version of the OPL with 16 patches and only one of them is user-configurable.##sesd"].plurals[0] = "versão de custo reduzido do chip OPL com 16 timbres selecionáveis, e apenas um deles é configurável pelo usuário.";
    strings["Famicom Disk System (chip)##sesd"].plurals[0] = "Famicom Disk System (chip)";
    strings["a disk drive for the Famicom which also contains one wavetable channel.##sesd"].plurals[0] = "um drive de disquete para o Famicom que também contém um canal de wavetable.";
    strings["10xx: Set waveform##sesd4"].plurals[0] = "10xx: Definir forma de onda";
    strings["11xx: Set modulation depth##sesd"].plurals[0] = "11xx: Definir profundidade de modulação";
    strings["12xy: Set modulation speed high byte (x: enable; y: value)##sesd"].plurals[0] = "12xy: Definir byte superior da velocidade de modulação (x: habilitar; y: valor)";
    strings["13xx: Set modulation speed low byte##sesd"].plurals[0] = "13xx: Definir byte inferior da velocidade de modulação";
    strings["14xx: Set modulator position##sesd"].plurals[0] = "14xx: Definir posição do modulador";
    strings["15xx: Set modulator table to waveform##sesd"].plurals[0] = "15xx: Definir tabela do modulador para uma forma de onda";
    strings["16xx: Set waveform (local)##sesd"].plurals[0] = "16xx: Definir forma de onda (local)";
    strings["an expansion chip for the Famicom, featuring a little-known PCM channel.##sesd"].plurals[0] = "um chip de expansão para o Famicom, contendo um canal PCM pouco conhecido.";
    strings["12xx: Set duty cycle/noise mode (pulse: 0 to 3; noise: 0 or 1)##sesd1"].plurals[0] = "12xx: Definir ciclo de trabalho/modo de ruído (pulso: 0 a 3; ruído: 0 ou 1)";
    strings["an expansion chip for the Famicom, with full wavetable.##sesd"].plurals[0] = "um chip de expansão para o Famicom, com vários canais de wavetable.";
    strings["18xx: Change channel limits (0 to 7, x + 1)##sesd"].plurals[0] = "18xx: Mudar limite de canais (0 a 7, x + 1)";
    strings["20xx: Load a waveform into memory##sesd"].plurals[0] = "20xx: Carregar uma forma de onda na memória";
    strings["21xx: Set position for wave load##sesd"].plurals[0] = "21xx: Definir posição para carregar onda na memória";
    strings["10xx: Select waveform##sesd"].plurals[0] = "10xx: Selecionar forma de onda";
    strings["11xx: Set waveform position in RAM##sesd"].plurals[0] = "11xx: Definir posição da forma de onda na RAM";
    strings["12xx: Set waveform length in RAM (04 to FC in steps of 4)##sesd"].plurals[0] = "12xx: Definir comprimento da forma de onda na RAM (04 a FC em incrementos de 4)";
    strings["15xx: Set waveform load position##sesd"].plurals[0] = "15xx: Definir posição de carregamento de forma de onda";
    strings["16xx: Set waveform load length (04 to FC in steps of 4)##sesd"].plurals[0] = "16xx: Definir comprimento do carregamento da forma de onda (04 a FC em incrementos de 4)";
    strings["17xx: Select waveform (local)##sesd1"].plurals[0] = "17xx: Selecionar forma de onda (local)";
    strings["cost-reduced version of the OPM with a different register layout and no stereo...\n...but it has a built-in AY-3-8910! (actually an YM2149)##sesd"].plurals[0] = "versão de custo reduzido do OPM com um leiaute de registradores diferentes e sem estéreo...\n...mas ele tem um AY-3-8910 embutido! (na verdade é um YM2149)";
    strings["Yamaha YM2203 (OPN) Extended Channel 3##sesd"].plurals[0] = "Yamaha YM2203 (OPN) com Canal 3 Estendido";
    strings["cost-reduced version of the OPM with a different register layout and no stereo...\n...but it has a built-in AY-3-8910! (actually an YM2149)\nthis one is in Extended Channel mode, which turns the third FM channel into four operators with independent notes/frequencies##sesd"].plurals[0] = "versão de custo reduzido do OPM com um leiaute de registradores diferentes e sem estéreo...\n...mas ele tem um AY-3-8910 embutido! (na verdade é um YM2149)\nesse está em modo de Canal Estendido, que transforma o terceiro canal FM em quatro operadores com notas/frequências independentes.";
    strings["cost-reduced version of the OPM with a different register layout and no stereo...\n...but it has a built-in AY-3-8910! (actually an YM2149)\nthis one is in Extended Channel mode, which turns the third FM channel into four operators with independent notes/frequencies.\nthis one includes CSM mode control for special effects on Channel 3.##sesd"].plurals[0] = "versão de custo reduzido do OPM com um leiaute de registradores diferentes e sem estéreo...\n...mas ele tem um AY-3-8910 embutido! (na verdade é um YM2149)\nesse está em modo de Canal Estendido, que transforma o terceiro canal FM em quatro operadores com notas/frequências independentes.\nesse inclui controle do modo CSM para efeitos especiais no Canal 3.";
    strings["OPN but twice the FM channels, stereo makes a come-back and has rhythm and ADPCM channels.##sesd"].plurals[0] = "OPN mas com o dobro dos canais FM, o estéreo está de volta e possui canais de bateria e um canal ADPCM.";
    strings["Yamaha YM2608 (OPNA) Extended Channel 3##sesd"].plurals[0] = "Yamaha YM2608 (OPNA) com Canal 3 Estendido";
    strings["OPN but twice the FM channels, stereo makes a come-back and has rhythm and ADPCM channels.\nthis one is in Extended Channel mode, which turns the third FM channel into four operators with independent notes/frequencies.##sesd"].plurals[0] = "OPN mas com o dobro dos canais FM, o estéreo está de volta e possui canais de bateria e um canal ADPCM.\nesse está em modo de Canal Estendido, que transforma o terceiro canal FM em quatro operadores com notas/frequências independentes.";
    strings["Yamaha YM2608 (OPNA) Extended Channel 3 and CSM##sesd"].plurals[0] = "Yamaha YM2608 (OPNA) com Canal 3 Estendido e CSM";
    strings["OPN but twice the FM channels, stereo makes a come-back and has rhythm and ADPCM channels.\nthis one is in Extended Channel mode, which turns the third FM channel into four operators with independent notes/frequencies.\nthis one includes CSM mode control for special effects on Channel 3.##sesd"].plurals[0] = "OPN mas com o dobro dos canais FM, o estéreo está de volta e possui canais de bateria e um canal ADPCM.\nesse está em modo de Canal Estendido, que transforma o terceiro canal FM em quatro operadores com notas/frequências independentes.\nesse inclui controle do modo CSM para efeitos especiais no Canal 3.";
    strings["OPN, but what if you only had two operators, no stereo, no detune and a lower ADSR parameter range?##sesd"].plurals[0] = "OPN, mas e se você só tivesse dois operadores, não tivesse estéreo, nem desafinação e tivesse um alcance menor nos parâmetros ADSR?"; // mas peraí, o OPN já não tinha estéreo pra começo de conversa!
    strings["OPL, but what if you had more waveforms to choose than the normal sine?##sesd"].plurals[0] = "OPL, mas e se você tivesse mais formas de onda pra escolher do que apenas o seno normal?";
    strings["OPL2, but what if you had twice the channels, 4-op mode, stereo and even more waveforms?##sesd"].plurals[0] = "OPL2, mas e se você tivesse o dobro de canais, modo de 4 operadores, estéreo e ainda mais formas de onda?";
    strings["how many channels of PCM do you want?\nMultiPCM: yes##sesd"].plurals[0] = "quantos canais de PCM você quer?\nMultiPCM: sim";
    strings["PC Speaker##sesd"].plurals[0] = "PC Speaker";
    strings["good luck! you get one square and no volume control.##sesd"].plurals[0] = "boa sorte! você tem uma onda quadrada e nenhum controle de volume.";
    strings["please don't use this chip. it was added as a joke.##sesd"].plurals[0] = "por favor não use esse chip. ele foi adicionado por causa de uma brincadeira.";
    strings["TIA, but better and more flexible.\nused in the Atari 8-bit family of computers (400/800/XL/XE).##sesd"].plurals[0] = "TIA, só que melhor e mais flexível.\nutilizado na família de computadores Atari 8-bit (400/800/XL/XE)."; // e no Atari 5200, né?
    strings["10xx: Set waveform (0 to 7)##sesd0"].plurals[0] = "10xx: Definir forma de onda (0 a 7)";
    strings["11xx: Set AUDCTL##sesd"].plurals[0] = "11xx: Definir AUDCTL";
    strings["12xx: Toggle two-tone mode##sesd"].plurals[0] = "12xx: Alternar modo de dois tons";
    strings["13xx: Set raw period##sesd"].plurals[0] = "13xx: Definir período (val. registrador)";
    strings["14xx: Set raw period (higher byte; only for 16-bit mode)##sesd"].plurals[0] = "14xx: Definir byte superior do período (val. registrador) (somente para o modo 16-bit)";
    strings["this is like SNES' sound chip but without interpolation and the rest of nice bits.##sesd"].plurals[0] = "esse é como o chip de som do SNES só que sem interpolação e o resto das coisas legais.";
    strings["developed by the makers of the Game Boy and the Virtual Boy...##sesd"].plurals[0] = "desenvolvido pelos criadores do Game Boy e do Virtual Boy...";
    strings["10xx: Set waveform##sesd5"].plurals[0] = "10xx: Definir forma de onda";
    strings["11xx: Setup noise mode (0: disabled; 1-8: enabled/tap)##sesd"].plurals[0] = "11xx: Configurar modo de ruído (0: desabilitado; 1-8: habilitado/tap)";
    strings["12xx: Setup sweep period (0: disabled; 1-20: enabled/period)##sesd"].plurals[0] = "12xx: Configurar período de sweep (0: desabilitado; 1-20: habilitado/período)";
    strings["13xx: Set sweep amount##sesd"].plurals[0] = "13xx: Definir quantidade de sweep";
    strings["17xx: Toggle PCM mode (LEGACY)##sesd3"].plurals[0] = "17xx: Alternar modo PCM (LEGADO)";
    strings["18xx: Set waveform (local)##sesd1"].plurals[0] = "18xx: Definir forma de onda (local)";
    strings["like OPM, but with more waveforms, fixed frequency mode and totally... undocumented.\nused in the Yamaha TX81Z and some other synthesizers.##sesd"].plurals[0] = "como o OPM, mas com mais formas de onda, modo de frequência fixa e completamente... não documentado.\nutilizado no Yamaha TX81Z e alguns outros sintetizadores.";
    strings["2Fxx: Toggle hard envelope reset on new notes##sesd"].plurals[0] = "2Fxx: Alternar reinício brusco do envelope em novas notas";
    strings["this one is like PC Speaker but has duty cycles.##sesd"].plurals[0] = "esse é parecido com o PC Speaker só que tem ciclos de trabalho.";
    strings["used in some Sega arcade boards (like OutRun), and usually paired with a YM2151.##sesd"].plurals[0] = "utilizado em algumas placas de fliperama da Sega (como OutRun), e geralmente pareado com um YM2151 (OPM).";
    strings["a console which failed to sell well due to its headache-inducing features.##sesd"].plurals[0] = "um console que foi um desastre de vendas por conta dos seus recursos que causavam dores de cabeça.";
    strings["10xx: Set waveform##sesd6"].plurals[0] = "10xx: Definir forma de onda";
    strings["11xx: Set noise length (0 to 7)##sesd"].plurals[0] = "11xx: Definir comprimento do ruído (0 a 7)";
    strings["12xy: Setup envelope (x: enabled/loop (1: enable, 3: enable+loop); y: speed/direction (0-7: down, 8-F: up))##sesd"].plurals[0] = "12xy: Definir envelope (x: habilitado/repetir (1: habilitado, 3: habilitado+repetir); y: velocidade/direção (0-7: para baixo, 8-F: para cima))";
    strings["13xy: Setup sweep (x: speed; y: shift; channel 5 only)##sesd"].plurals[0] = "13xy: Configurar sweep (x: velocidade; y: deslocamento; apenas no canal 5)";
    strings["14xy: Setup modulation (x: enabled/loop (1: enable, 3: enable+loop); y: speed; channel 5 only)##sesd"].plurals[0] = "14xy: Configurar modulação (x: habilitada/repetir (1: habilitada, 3: habilitada+repetir); y: velocidade; canal 5 somente)";
    strings["15xx: Set modulation waveform (x: wavetable; channel 5 only)##sesd"].plurals[0] = "15xx: Definir forma de onda de modulação (x: wavetable; canal 5 somente)";
    strings["16xx: Set waveform (local)##sesd1"].plurals[0] = "16xx: Definir forma de onda (local)";
    strings["like OPLL, but even more cost reductions applied. three less FM channels, and no drums mode...##sesd"].plurals[0] = "como o OPLL, mas com ainda mais reduções de custo aplicadas: três canais FM a menos, e sem o modo bateria...";
    strings["so Taito asked Yamaha if they could get the two missing FM channels back, and Yamaha gladly provided them with this chip.##sesd"].plurals[0] = "então a Taito pediu à Yamaha se eles poderiam ter os 2 canais FM faltantes de volta, e a Yamaha lhes providenciou de bom grado com este chip.";
    strings["ZX Spectrum Beeper##sesd"].plurals[0] = "Beeper do ZX Spectrum";
    strings["the ZX Spectrum only had a basic beeper capable of...\n...a bunch of thin pulses and tons of other interesting stuff!\nFurnace provides a thin pulse system.##sesd"].plurals[0] = "o ZX Spectrum só tinha um beeper básico capaz de...\n...um monte de pulsos finos e um monte de outras coisas interessantes!\nFurnace provê um sistema de pulsos finos.";
    strings["12xx: Set pulse width##sesd0"].plurals[0] = "12xx: Definir largura de pulso";
    strings["17xx: Trigger overlay drum##sesd"].plurals[0] = "17xx: Disparar tambor sobreposto";
    strings["Yamaha YM2612 (OPN2) Extended Channel 3##sesd"].plurals[0] = "Yamaha YM2612 (OPN2) com Canal 3 Estendido";
    strings["this chip is mostly known for being in the Sega Genesis (but it also was on the FM Towns computer).\nthis one is in Extended Channel mode, which turns the third FM channel into four operators with independent notes/frequencies.##sesd"].plurals[0] = "esse chip é mais conhecido por ter sido utilizado no Sega Mega Drive (mas ele também foi usado no computador FM Towns).\nesse está em modo de Canal Estendido, que transforma o terceiro canal FM em quatro operadores com notas/frequências independentes.";
    strings["this chip is mostly known for being in the Sega Genesis (but it also was on the FM Towns computer).\nthis one includes CSM mode control for special effects on Channel 3.##sesd"].plurals[0] = "esse chip é mais conhecido por ter sido utilizado no Sega Mega Drive (mas ele também foi usado no computador FM Towns).\nesse inclui controle do modo CSM para efeitos especiais no Canal 3.";
    strings["a wavetable chip made by Konami for use with the MSX.\nthe last channel shares its wavetable with the previous one though.##sesd"].plurals[0] = "um chip wavetable feito pela Konami para ser utilizado com o MSX.\no último canal compartilha sua wavetable com o anterior, porém.";
    strings["Yamaha YM3526 (OPL) with drums##sesd"].plurals[0] = "Yamaha YM3526 (OPL) em modo bateria";
    strings["the OPL chip but with drums mode enabled.##sesd"].plurals[0] = "o chip OPL mas com modo bateria habilitado.";
    strings["Yamaha YM3812 (OPL2) with drums##sesd"].plurals[0] = "Yamaha YM3812 (OPL2) em modo bateria";
    strings["the OPL2 chip but with drums mode enabled.##sesd"].plurals[0] = "o chip OPL2 mas com modo bateria habilitado.";
    strings["Yamaha YMF262 (OPL3) with drums##sesd"].plurals[0] = "Yamaha YMF262 (OPL3) em modo bateria";
    strings["the OPL3 chip but with drums mode enabled.##sesd"].plurals[0] = "o chip OPL3 mas com modo bateria habilitado.";
    strings["this chip was used in SNK's Neo Geo arcade board and video game console.\nit's like OPNA but the rhythm channels are ADPCM channels and two FM channels went missing.##sesd"].plurals[0] = "esse chip foi usado no Neo Geo, placa de fliperama e console de videogame da SNK.\né como o OPNA mas os canais de bateria são canais de ADPCM e dois canais de FM foram removidos.";
    strings["Yamaha YM2610 (OPNB) Extended Channel 2##sesd"].plurals[0] = "Yamaha YM2610 (OPNB) com Canal 2 Estendido";
    strings["this chip was used in SNK's Neo Geo arcade board and video game console.\nit's like OPNA but the rhythm channels are ADPCM channels and two FM channels went missing.\nthis one is in Extended Channel mode, which turns the second FM channel into four operators with independent notes/frequencies.##sesd"].plurals[0] = "esse chip foi usado no Neo Geo, placa de fliperama e console de videogame da SNK.\né como o OPNA mas os canais de bateria são canais de ADPCM e dois canais de FM foram removidos.\nesse está em modo de Canal Estendido, que transforma o segundo canal FM em quatro operadores com notas/frequências independentes.";
    strings["this chip was used in SNK's Neo Geo arcade board and video game console.\nit's like OPNA but the rhythm channels are ADPCM channels and two FM channels went missing.\nthis one is in Extended Channel mode, which turns the second FM channel into four operators with independent notes/frequencies.\nthis one includes CSM mode control for special effects on Channel 2.##sesd"].plurals[0] = "esse chip foi usado no Neo Geo, placa de fliperama e console de videogame da SNK.\né como o OPNA mas os canais de bateria são canais de ADPCM e dois canais de FM foram removidos.\nesse está em modo de Canal Estendido, que transforma o segundo canal FM em quatro operadores com notas/frequências independentes.\nesse inclui controle do modo CSM para efeitos especiais no Canal 2.";
    strings["Yamaha YM2413 (OPLL) with drums##sesd"].plurals[0] = "Yamaha YM2413 (OPLL) em modo bateria";
    strings["the OPLL chip but with drums mode turned on.##sesd"].plurals[0] = "o chip OPLL mas com modo bateria habilitado.";
    strings["3xxx: Load LFSR (0 to FFF)##sesd"].plurals[0] = "3xxx: Carregar LFSR (0 a FFF)";
    strings["a portable console made by Atari. it has all of Atari's trademark waveforms.##sesd"].plurals[0] = "um console portátil feito pela Atari. ele possui todas as formas de onda conhecidíssimas dos consoles Atari.";
    strings["10xx: Set echo feedback level (00 to FF)##sesd"].plurals[0] = "10xx: Definir nível de retorno do Eco (00 a FF)";
    strings["11xx: Set channel echo level (00 to FF)##sesd"].plurals[0] = "11xx: Definir nível de eco do canal (00 a FF)";
    strings["12xx: Toggle QSound algorithm (0: disabled; 1: enabled)##sesd"].plurals[0] = "12xx: Alternar algoritmo QSound (0: desabilitado; 1: habilitado)";
    strings["3xxx: Set echo delay buffer length (000 to AA5)##sesd"].plurals[0] = "3xxx: Definir tamanho do buffer de atraso do eco (000 a AA5)";
    strings["used in some of Capcom's arcade boards. surround-like sampled sound with echo.##sesd"].plurals[0] = "utilizado em algumas das placas de fliperama da Capcom. som amostrado estilo surround com eco.";
    strings["the chip used in a computer design created by The 8-Bit Guy.##sesd"].plurals[0] = "o chip utilizado em um computador criado pelo The 8-Bit Guy.";
    strings["20xx: Set waveform##sesd"].plurals[0] = "20xx: Definir forma de onda";
    strings["22xx: Set duty cycle (0 to 3F)##sesd"].plurals[0] = "22xx: Definir ciclo de trabalho (0 a 3F)";
    strings["Yamaha YM2610B (OPNB2) Extended Channel 3##sesd"].plurals[0] = "Yamaha YM2610B (OPNB2) com Canal 3 Estendido";
    strings["so Taito asked Yamaha if they could get the two missing FM channels back, and Yamaha gladly provided them with this chip.\nthis one is in Extended Channel mode, which turns the third FM channel into four operators with independent notes/frequencies.##sesd"].plurals[0] = "então a Taito pediu à Yamaha se eles poderiam ter os 2 canais FM faltantes de volta, e a Yamaha lhes providenciou de bom grado com este chip.\nesse está em modo de Canal Estendido, que transforma o terceiro canal FM em quatro operadores com notas/frequências independentes.";
    strings["so Taito asked Yamaha if they could get the two missing FM channels back, and Yamaha gladly provided them with this chip.\nthis one is in Extended Channel mode, which turns the third FM channel into four operators with independent notes/frequencies.\nthis one includes CSM mode control for special effects on Channel 3.##sesd"].plurals[0] = "então a Taito pediu à Yamaha se eles poderiam ter os 2 canais FM faltantes de volta, e a Yamaha lhes providenciou de bom grado com este chip.\nesse está em modo de Canal Estendido, que transforma o terceiro canal FM em quatro operadores com notas/frequências independentes.\nesse inclui controle do modo CSM para efeitos especiais no Canal 3.";
    strings["SegaPCM (compatible 5-channel mode)##sesd"].plurals[0] = "SegaPCM (modo de compatibilidade de 5 canais)";
    strings["this is the same thing as SegaPCM, but only exposes 5 of the channels for compatibility with DefleMask.##sesd"].plurals[0] = "esse é a mesma coisa que o SegaPCM, mas só expões 5 dos canais para compatibilidade com o DefleMask.";
    strings["a sound chip used in several Seta/Allumer-manufactured arcade boards with too many channels of wavetable sound, which also are capable of sampled sound.##sesd"].plurals[0] = "um chip de som utilizado em várias placa de fliperama fabricadas pela Seta/Allumer, com muitos canais de wavetable que também são capazes de tocar samples.";
    strings["10xx: Set waveform##sesd7"].plurals[0] = "10xx: Definir forma de onda";
    strings["11xx: Set envelope shape##sesd"].plurals[0] = "11xx: Definir formato do envelope";
    strings["12xx: Set sample bank slot (0 to 7)##sesd"].plurals[0] = "12xx: Definir vaga do banco de samples (0 to 7)";
    strings["17xx: Toggle PCM mode (LEGACY)##sesd4"].plurals[0] = "17xx: Alternar modo PCM (LEGADO)";
    strings["18xx: Set waveform (local)##sesd2"].plurals[0] = "18xx: Definir forma de onda (local)";
    strings["20xx: Set PCM frequency (1 to FF)##sesd"].plurals[0] = "20xx: Definir frequência PCM (1 a FF)";
    strings["22xx: Set envelope mode (bit 0: enable; bit 1: one-shot; bit 2: split shape to L/R; bit 3/5: H.invert right/left; bit 4/6: V.invert right/left)##sesd"].plurals[0] = "22xx: Definir modo do envelope (bit 0: habilitar; bit 1: uma vez; bit 2: dividir formato para esquerda/direita; bit 3/5: inverter hor. direita/esquerda; bit 4/6: inverter vert. direita/esquerda)";
    strings["23xx: Set envelope period##sesd"].plurals[0] = "23xx: Definir período do envelope";
    strings["25xx: Envelope slide up##sesd1"].plurals[0] = "25xx: Slide de envelope para cima";
    strings["26xx: Envelope slide down##sesd1"].plurals[0] = "26xx: Slide de envelope para baixo";
    strings["29xy: Set auto-envelope (x: numerator; y: denominator)##sesd1"].plurals[0] = "29xy: Definir envelope automático (x: numerador; y: denominador)";
    strings["this is the wavetable part of the Bubble System, which also had two AY-3-8910s.##sesd"].plurals[0] = "essa é a parte wavetable do Bubble System, que também possuía dois AY-3-8910.";
    strings["like OPL3, but this time it also has a 24-channel version of MultiPCM.##sesd"].plurals[0] = "como o OPL3, mas dessa vez ele também possui uma versão de 24 canais do MultiPCM.";
    strings["Yamaha YMF278B (OPL4) with drums##sesd"].plurals[0] = "Yamaha YMF278B (OPL4) em modo bateria";
    strings["the OPL4 but with drums mode turned on.##sesd"].plurals[0] = "o chip OPL4 mas com modo bateria habilitado.";
    strings["11xx: Set filter mode (00 to 03)##sesd"].plurals[0] = "11xx: Definir modo de filtro (00 to 03)";
    strings["14xx: Set filter coefficient K1 low byte (00 to FF)##sesd"].plurals[0] =   "14xx: Definir byte inferior do coeficiente do filtro K1 (00 a FF)";
    strings["15xx: Set filter coefficient K1 high byte (00 to FF)##sesd"].plurals[0] =  "15xx: Definir byte superior do coeficiente do filtro K1 (00 a FF)";
    strings["16xx: Set filter coefficient K2 low byte (00 to FF)##sesd"].plurals[0] =   "16xx: Definir byte inferior do coeficiente do filtro K2 (00 a FF)";
    strings["17xx: Set filter coefficient K2 high byte (00 to FF)##sesd"].plurals[0] =  "17xx: Definir byte superior do coeficiente do filtro K2 (00 a FF)";
    strings["18xx: Set filter coefficient K1 slide up (00 to FF)##sesd"].plurals[0] =   "18xx: Definir slide para cima do coeficiente do filtro K1 (00 a FF)";
    strings["19xx: Set filter coefficient K1 slide down (00 to FF)##sesd"].plurals[0] = "19xx: Definir slide para baixo do coeficiente do filtro K1 (00 a FF)";
    strings["1Axx: Set filter coefficient K2 slide up (00 to FF)##sesd"].plurals[0] =   "1Axx: Definir slide para cima do coeficiente do filtro K2 (00 a FF)";
    strings["1Bxx: Set filter coefficient K2 slide down (00 to FF)##sesd"].plurals[0] = "1Bxx: Definir slide para baixo do coeficiente do filtro K2 (00 a FF)";
    strings["22xx: Set envelope left volume ramp (signed) (00 to FF)##sesd"].plurals[0] = "22xx: Definir rampa de volume esquerdo do envelope (sinalizado) (00 a FF)";
    strings["23xx: Set envelope right volume ramp (signed) (00 to FF)##sesd"].plurals[0] = "23xx: Definir rampa de volume direito do envelope (sinalizado) (00 a FF)";
    strings["24xx: Set envelope filter coefficient k1 ramp (signed) (00 to FF)##sesd"].plurals[0] = "24xx: Definir rampa do coeficiente do filtro K1 (sinalizado) (00 a FF)";
    strings["25xx: Set envelope filter coefficient k1 ramp (signed, slower) (00 to FF)##sesd"].plurals[0] = "25xx: Definir rampa do coeficiente do filtro K1 (sinalizado, mais lento) (00 a FF)";
    strings["26xx: Set envelope filter coefficient k2 ramp (signed) (00 to FF)##sesd"].plurals[0] = "26xx: Definir rampa do coeficiente do filtro K2 (sinalizado) (00 a FF)";
    strings["27xx: Set envelope filter coefficient k2 ramp (signed, slower) (00 to FF)##sesd"].plurals[0] = "27xx: Definir rampa do coeficiente do filtro K2 (sinalizado, mais lento) (00 a FF)";
    strings["DFxx: Set sample playback direction (0: normal; 1: reverse)##sesd1"].plurals[0] = "DFxx: Definir direção de reprodução da sample (0: normal; 1: reversa)";
    strings["120x: Set pause (bit 0)##sesd"].plurals[0] = "120x: Definir pausa (bit 0)";
    strings["2xxx: Set envelope count (000 to 1FF)##sesd"].plurals[0] = "2xxx: Definir contagem do envelope (000 a 1FF)";
    strings["3xxx: Set filter coefficient K1 (000 to FFF)##sesd"].plurals[0] = "3xxx: Definir coeficiente do filtro K1 (000 a FFF)";
    strings["4xxx: Set filter coefficient K2 (000 to FFF)##sesd"].plurals[0] = "4xxx: Definir coeficiente do filtro K2 (000 a FFF)";
    strings["a sample chip made by Ensoniq, which is the basis for the GF1 chip found in Gravis' Ultrasound cards.##sesd"].plurals[0] = "um chip de sample feito pela Ensoniq, que é a base para o chip GF1 encontrado nas placas Gravis Ultrasound.";
    strings["like OPL but with an ADPCM channel.##sesd"].plurals[0] = "como o OPL, mas com um canal ADPCM.";
    strings["Yamaha Y8950 with drums##sesd"].plurals[0] = "Yamaha Y8950 em modo bateria";
    strings["the Y8950 chip, in drums mode.##sesd"].plurals[0] = "o chip Y8950 mas com modo bateria habilitado.";
    strings["this is a variant of Konami's SCC chip with the last channel's wavetable being independent.##sesd"].plurals[0] = "essa é uma variante do chip Konami SCC com a wavetable do último canal sendo independente.";
    strings["10xx: Set waveform (0 to 7)##sesd1"].plurals[0] = "10xx: Definir waveform (0 a 7)";
    strings["12xx: Set pulse width (0 to 7F)##sesd"].plurals[0] = "12xx: Definir largura de pulso (0 a 7F)";
    strings["13xx: Set resonance (0 to FF)##sesd"].plurals[0] = "13xx: Definir ressonância (0 a FF)";
    strings["14xx: Set filter mode (bit 0: ring mod; bit 1: low pass; bit 2: high pass; bit 3: band pass)##sesd"].plurals[0] = "14xx: Definir modo de filtro (bit 0: modulação anel; bit 1: passa baixa; bit 2: passa alta; bit 3: passa banda)";
    strings["15xx: Set frequency sweep period low byte##sesd"].plurals[0] = "15xx: Definir byte inferior do período do sweep de frequência";
    strings["16xx: Set frequency sweep period high byte##sesd"].plurals[0] = "16xx: Definir byte superior do período de sweep de frequência";
    strings["17xx: Set volume sweep period low byte##sesd"].plurals[0] = "17xx: Definir byte inferior do período do sweep de volume";
    strings["18xx: Set volume sweep period high byte##sesd"].plurals[0] = "18xx: Definir byte superior do período do sweep de volume";
    strings["19xx: Set cutoff sweep period low byte##sesd"].plurals[0] = "19xx: Definir byte inferior do período do sweep de ponto de corte";
    strings["1Axx: Set cutoff sweep period high byte##sesd"].plurals[0] = "1Axx: Definir byte superior do período do sweep de ponto de corte";
    strings["1Bxx: Set frequency sweep boundary##sesd"].plurals[0] = "1Bxx: Definir limite do sweep de frequência";
    strings["1Cxx: Set volume sweep boundary##sesd"].plurals[0] = "1Cxx: Definir limite do sweep de volume";
    strings["1Dxx: Set cutoff sweep boundary##sesd"].plurals[0] = "1Dxx: Definir limite do sweep de ponto de corte";
    strings["1Exx: Set phase reset period low byte##sesd"].plurals[0] = "1Exx: Definir byte inferior do período de redefinição de fase";
    strings["1Fxx: Set phase reset period high byte##sesd"].plurals[0] = "1Fxx: Definir byte superior do período de redefinição de fase";
    strings["20xx: Toggle frequency sweep (bit 0-6: speed; bit 7: direction is up)##sesd"].plurals[0] = "20xx: Alternar sweep de frequência (bit 0-6: velocidade; bit 7: direção é para cima)";
    strings["21xx: Toggle volume sweep (bit 0-4: speed; bit 5: direction is up; bit 6: loop; bit 7: alternate)##sesd"].plurals[0] = "21xx: Alternar sweep de volume (bit 0-4: velocidade; bit 5: direção é para cima; bit 6: repetir; bit 7: alternar)";
    strings["22xx: Toggle cutoff sweep (bit 0-6: speed; bit 7: direction is up)##sesd"].plurals[0] = "22xx: Alternar sweep de ponto de corte (bit 0-6: velocidade; bit 7: direção é para cima)";
    strings["4xxx: Set cutoff (0 to FFF)##sesd"].plurals[0] = "4xxx: Definir ponto de corte (0 a FFF)";
    strings["tildearrow's fantasy sound chip. put SID, AY and VERA in a blender, and you get this!##sesd"].plurals[0] = "chip de som de fantasia do tildearrow. coloque o SID, AY e VERA num liquidificador, e você obtém isso!";
    strings["an ADPCM sound chip manufactured by OKI and used in many arcade boards.##sesd"].plurals[0] = "um chip de som ADPCM fabricado pela OKI e utilizado em muitas placas de fliperama.";
    strings["20xx: Set chip output rate (0: clock/132; 1: clock/165)##sesd"].plurals[0] = "20xx: Definir taxa de amostragem do chip (0: clock/132; 1: clock/165)";
    strings["an ADPCM sound chip manufactured by OKI and used in the Sharp X68000.##sesd"].plurals[0] = "um chip de som ADPCM fabricado pela OKI e utilizado no Sharp X68000.";
    strings["20xx: Set frequency divider (0-2)##sesd"].plurals[0] = "20xx: Definir divisor de frequência (0-2)";
    strings["21xx: Select clock rate (0: full; 1: half)##sesd"].plurals[0] = "21xx: Definir taxa de clock (0: 100%; 1: 50%)";
    strings["used in some arcade boards. Can play back either 4-bit ADPCM, 8-bit PCM or 16-bit PCM.##sesd"].plurals[0] = "utilizado em algumas placas de fliperama. Pode tocar ADPCM 4-bit, PCM 8-bit ou PCM 16-bit.";
    strings["10xx: Set waveform##sesd8"].plurals[0] = "10xx: Definir forma de onda";
    strings["11xx: Select waveform (local)##sesd1"].plurals[0] = "11xx: Selecionar forma de onda (local)";
    strings["10xx: Set waveform##sesd9"].plurals[0] = "10xx: Definir forma de onda";
    strings["11xx: Toggle noise mode##sesd2"].plurals[0] = "11xx: Alternar modo de ruído";
    strings["a wavetable sound chip used in Pac-Man, among other early Namco arcade games.##sesd"].plurals[0] = "um chip de som wavetable utilizado no fliperama Pac-Man, juntamente com outros dos primeiros fliperamas da Namco.";
    strings["successor of the original Namco WSG chip, used in later Namco arcade games.##sesd"].plurals[0] = "sucessos do chip Namco WSG original, utilizado em fliperamas Namco posteriores.";
    strings["like Namco C15 but with stereo sound.##sesd"].plurals[0] = "similar ao Namco C15 mas com som estéreo.";
    strings["a square wave additive synthesis chip made by OKI. used in some arcade machines and instruments.##sesd"].plurals[0] = "um chip de síntese aditiva de ondas quadradas feito pela OKI. utilizado em algumas máquinas de fliperama e instrumentos..";
    strings["10xy: Set group control (x: sustain; y: part toggle bitmask)##sesd"].plurals[0] = "10xy: Definir controle de grupo (x: sustentação; y: máscara de bits para alternar partes)"; // TODO refinar?
    strings["11xx: Set noise mode##sesd0"].plurals[0] = "11xx: Definir modo de ruído"; 
    strings["12xx: Set group attack (0 to 5)##sesd"].plurals[0] = "12xx: Definir ataque do grupo (0 a 5)";
    strings["13xx: Set group decay (0 to 11)##sesd"].plurals[0] = "13xx: Definir decaimento do grupo (0 a 11)";
    strings["Yamaha YM2612 (OPN2) with DualPCM##sesd"].plurals[0] = "Yamaha YM2612 (OPN2) com DualPCM";
    strings["this chip is mostly known for being in the Sega Genesis (but it also was on the FM Towns computer).\nthis system uses software mixing to provide two sample channels.##sesd"].plurals[0] = "esse chip é mais conhecido por ter sido utilizado no Sega Mega Drive (mas ele também foi usado no computador FM Towns).\nesse sistema utiliza mistura de samples em software para prover dois canais de sample.";
    strings["Yamaha YM2612 (OPN2) Extended Channel 3 with DualPCM and CSM##sesd"].plurals[0] = "Yamaha YM2612 (OPN2) Extended Channel 3 com DualPCM e CSM";
    strings["this chip is mostly known for being in the Sega Genesis (but it also was on the FM Towns computer).\nthis system uses software mixing to provide two sample channels.\nthis one is in Extended Channel mode, which turns the third FM channel into four operators with independent notes/frequencies.\nthis one includes CSM mode control for special effects on Channel 3.##sesd"].plurals[0] = "esse chip é mais conhecido por ter sido utilizado no Sega Mega Drive (mas ele também foi usado no computador FM Towns).\nesse sistema utiliza mistura de samples em software para prover dois canais de sample.\nesse está em modo de Canal Estendido, que transforma o terceiro canal FM em quatro operadores com notas/frequências independentes.\nesse inclui controle do modo CSM para efeitos especiais no Canal 3.";
    strings["an SN76489 derivative used in Neo Geo Pocket, has independent stereo volume and noise channel frequency.##sesd"].plurals[0] = "uma derivativa do SN76489 utilizada no Neo Geo Pocket que possui volume estéreo independente e frequência independente para o canal de ruído.";
    strings["20xx: Set noise length (0: short, 1: long)##sesd"].plurals[0] = "20xx: Definir comprimento do ruído (0: curto, 1: longo)";
    strings["Generic PCM DAC##sesd"].plurals[0] = "DAC PCM Genérico";
    strings["as generic sample playback as it gets.##sesd"].plurals[0] = "simplesmente reprodução de samples, o mais genérico possível.";
    strings["this PCM chip was widely used at Konami arcade boards in 1986-1990.##sesd"].plurals[0] = "esse chip PCM foi amplamente utilizado em placas de fliperama da Konami entre 1986 e 1990.";
    strings["yet another PCM chip from Irem. like Amiga, but less pitch resolution and no sample loop.##sesd"].plurals[0] = "mais um chip PCM da Irem. similar ao Amiga, mas com menos resolução de tom e sem repetição de samples.";
    strings["a SoC with wavetable sound hardware.##sesd"].plurals[0] = "um SoC com hardware de som wavetable.";
    strings["a game console with 3 channels of square wave. it's what happens after fusing TIA and VIC together.##sesd"].plurals[0] = "um console de jogo com 3 canais de onda quadrada. é o que acontece quando se funde a TIA e o VIC juntos.";
    strings["10xx: Set ring modulation (0: disable, 1: enable)##sesd"].plurals[0] = "10xx: Alternar modulação anel (0: desabilitar, 1: habilitar)";
    strings["11xx: Raw frequency (0-3E)##sesd"].plurals[0] = "11xx: Definir frequência (valor do registrador, 0-3E)";
    strings["ZX Spectrum Beeper (QuadTone Engine)##sesd"].plurals[0] = "Beeper do ZX Spectrum (Motor QuadTone)";
    strings["another ZX Spectrum beeper system with full PWM pulses and 3-level volume per channel. it also has a pitchable overlay sample channel.##sesd"].plurals[0] = "outro sistema de beeper do ZX Spectrum, dessa vez com pulsos PWM completos e volume de 3 níveis por canal. ele também possui um canal de sobreposição de sample com tom controlável.";
    strings["12xx: Set pulse width##sesd1"].plurals[0] = "12xx: Definir largura de pulso";
    strings["this PCM chip was widely used at Konami arcade boards in 1990-1992.##sesd"].plurals[0] = "esse chip PCM foi largamente utilizado em placas de fliperama da Konami entre 1990 e 1992.";
    strings["DFxx: Set sample playback direction (0: normal; 1: reverse)##sesd2"].plurals[0] = "DFxx: Definir direção de reprodução da sample (0: normal; 1: reversa)";
    strings["two square waves (one may be turned into noise). used in the Commodore Plus/4, 16 and 116.##sesd"].plurals[0] = "duas ondas quadradas (uma pode ser transformada em ruído). utilizado no Commodore Plus/4, 16 e 116.";
    strings["Namco's first PCM chip from 1987. it's pretty good for being so.##sesd"].plurals[0] = "o primeiro chip PCM da Namco de 1987. é muito bom pelo que é.";
    strings["Namco's PCM chip used in their NA-1/2 hardware.\nvery similar to C140, but has noise generator.##sesd"].plurals[0] = "chip de som PCM da Namco utilizado em suas placas de fliperama NA-1/2.\nmuito similar ao C140, porém contém um gerador de ruído.";
    strings["11xx: Set noise mode##sesd1"].plurals[0] = "11xx: Definir modo de ruído";
    strings["12xy: Set invert mode (x: surround; y: invert)##sesd"].plurals[0] = "12xy: Definir modo de inversão (x: surround; y: inverter)";
    strings["ESS ES1xxx series (ESFM)##sesd"].plurals[0] = "ESS série ES1xxx (ESFM)";
    strings["a unique FM synth featured in PC sound cards.\nbased on the OPL3 design, but with lots of its features extended.##sesd"].plurals[0] = "um sintetizador FM único encontrado em placas de som de PC.\nbaseado no design do OPL3, mas com muitos de seus recursos estendidos."; // :3
    strings["2Exx: Toggle hard envelope reset on new notes##sesd"].plurals[0] = "2Exx: Alternar reinício brusco do envelope em novas notas";
    strings["first Ensoniq chip used in their synths and Apple IIGS computer. Has 32 hard-panned 8-bit wavetable/sample channels, can do oscillator sync (like SID) and amplitude modulation. Can have up to 128 KiB (2 banks of 64 KiB) of memory for wavetables/samples.\nAs Robert Yannes (SID chip creator) said, it's more or less what SID chip could be if he was given enough time for its development.##sesd"].plurals[0] = "o primeiro chip da Ensoniq utilizado em seus sintetizadores e no computador Apple IIGS. Contém 32 canais de sample/wavetable 8-bit com balanço binário, é capaz de sincronizar osciladores (como o SID) e realizar modulação de amplitude. Pode endereçar até 128 KiB (2 bancos de 64 KiB) de memória para wavetables/samples.\nComo Robert Yannes (criador do chip SID) disse, é mais ou menos o que o chip SID poderia ter sido se ele tivesse tido tempo suficiente para o seu desenvolvimento.";
    strings["11xx: Set number of enabled oscillators (2-1F)##sesd"].plurals[0] = "11xx: Definir número de osciladores habilitados (2-1F)";
    strings["12xx: Set oscillator output (0-7, 0=left, 1=right)##sesd"].plurals[0] = "12xx: Definir saída do oscilador (0-7, 0=esquerda, 1=direita)";
    strings["13xx: Set wave/sample length (0-7, 0=256, 1=512, 2=1024, etc.)##sesd"].plurals[0] = "13xx: Definir tamanho da onda/sample (0-7, 0=256, 1=512, 2=1024, etc.)";
    strings["14xx: Set wave/sample position in memory (xx*256 offset)##sesd"].plurals[0] = "14xx: Definir posição na memória da onda/sample (deslocamento em xx*256)";
    strings["15xx: Set oscillator mode (0-3)##sesd"].plurals[0] = "15xx: Definir modo do oscilador (0-3)";
    strings["a fantasy sound chip designed by jvsTSX and The Beesh-Spweesh!\nused in the Hexheld fantasy console.##sesd"].plurals[0] = "um chip de som fantasia desenvolvido por jvsTSX e The Beesh-Spweesh!\nutilizado no console fantasia Hexheld.";
    strings["20xx: Load low byte of noise channel LFSR (00 to FF) or slope channel accumulator (00 to 7F)##sesd"].plurals[0] = "20xx: Carregar byte inferior do LFSR do canal de ruído (00 a FF) ou acumulador do canal de rampa (00 a 7F)";
    strings["21xx: Load high byte of noise channel LFSR (00 to FF)##sesd"].plurals[0] = "21xx: Carregar byte superior do LFSR do canal de ruído (00 to FF)";
    strings["22xx: Write to I/O port A##sesd"].plurals[0] = "22xx: Escrever na porta E/S A";
    strings["23xx: Write to I/O port B##sesd"].plurals[0] = "23xx: Escrever na porta E/S B";
    strings["this chip was featured in the Enterprise 128 computer. it is similar to POKEY, but with stereo output, more features and frequency precision and the ability to turn left or right (or both) channel into a 6-bit DAC for sample playback.##sesd"].plurals[0] = "esse chip foi utilizado no computador Enterprise 128. ele é similar ao POKEY, mas com saída estéreo, mais recursos, mais precisão de frequência e com a possibilidade de transformar o canal esquerdo, direito ou ambos em um DAC 6-bit para reprodução de samples.";
    strings["10xx: Set waveform (0 to 4; 0 to 3 on noise)##sesd"].plurals[0] = "10xx: Definir forma de onda (0 a 4; 0 a 3 no canal de ruído)";
    strings["11xx: Set noise frequency source (0: fixed; 1-3: channels 1 to 3)##sesd"].plurals[0] = "11xx: Definir fonte da frequência do ruído (0: fixa; 1-3: canais 1 a 3)";
    strings["12xx: Toggle high-pass with next channel##sesd"].plurals[0] = "12xx: Alternar passa alta com o próximo canal";
    strings["13xx: Toggle ring modulation with channel+2##sesd"].plurals[0] = "13xx: Alternar modulação anel com o canal+2";
    strings["14xx: Toggle swap counters (noise only)##sesd"].plurals[0] = "14xx: Alternar trocar contadores (somente ruído)";
    strings["15xx: Toggle low pass (noise only)##sesd"].plurals[0] = "15xx: Alternar passa baixa (somente ruído)";
    strings["16xx: Set clock divider (0: /2; 1: /3)##sesd"].plurals[0] = "16xx: Definir divisor de clock (0: /2; 1: /3)";
    strings["a fictional sound chip by LTVA. like SID, but with many of its problems fixed. also features extended functionality like more wave mixing modes, tonal noise, filter and volume per channel.##sesd"].plurals[0] = "um chip de som fictício por LTVA. como o SID, mas com muitos de seus problemas consertados. também possui recursos estendidos como mais modos de mistura de formas de onda, ruído tonal, filtro e volume por canal.";
    strings["a fictional sound chip by Euly. similar to Ricoh 2A03, but all the duty cycles are different, noise has 32 pitches instead of 16 and you have four hard-coded waveforms on triangle channel.##sesd"].plurals[0] = "um chip de som fictício por Euly. similar ao Ricoh 2A03, mas todos os ciclos de trabalho são diferentes, o ruído tem 32 tons ao invés de 16 e você possui 4 formas de onda predefinidas no lugar do canal de triângulo.";
    strings["12xx: Set duty cycle/noise mode (pulse: 0 to 3; noise: 0 or 1, wave: 0 to 3)##sesd"].plurals[0] = "12xx: Definir ciclo de trabalho/modo de ruído/onda (pulso: 0 a 3; ruído: 0 ou 1, onda: 0 a 3)";
    strings["19xx: Set wave linear counter (0 to 7F; 80 and higher halt)##sesd"].plurals[0] = "19xx: Definir contador linear do canal de onda (0 a 7F; valores 80 e superiores interrompem)";
    strings["additional PCM FIFO channels in Game Boy Advance driven directly by its DMA hardware.##sesd"].plurals[0] = "canais PCM FIFO adicionais no Game Boy Advance controlados diretamente por seu hardware de DMA.";
    strings["additional PCM FIFO channels in Game Boy Advance driven by software mixing to provide up to sixteen sample channels.##sesd"].plurals[0] = "canais PCM FIFO adicionais no Game Boy Advance controlados por mixing em software para prover até 16 canais de sample.";
    strings["11xy: Set echo channel (x: left/right source; y: delay (0 disables))##sesd"].plurals[0] = "11xy: Definir canal de eco (x: fonte esquerda/direita; y: atraso (0 desabilita))";
    strings["12xy: Toggle invert (x: left; y: right)##sesd"].plurals[0] = "12xy: Alternar inversão (x: lado esquerdo; y: lado direito)";
    strings["a handheld video game console with two screens. it uses a stylus.##sesd"].plurals[0] = "um console de videogame portátil com duas telas. ele usa uma canetinha.";
    strings["12xx: Set duty cycle (pulse: 0 to 7)##sesd"].plurals[0] = "12xx: Definir ciclo de trabalho (pulso: 0 a 7)";
    strings["1Fxx: Set global volume (0 to 7F)##sesd"].plurals[0] = "1Fxx: Definir volume global (0 a 7F)";
    strings["Dummy System##sesd"].plurals[0] = "Sistema Fantoche"; // TODO: Sistema Fictício seria melhor? acho muito literal...
    strings["this is a system designed for testing purposes.##sesd"].plurals[0] = "este é um sistema desenvolvido para propósito de testes.";
    
    //channel names

    strings["Channel 1##sesd"].plurals[0] =  "Canal 1";
    strings["Channel 2##sesd"].plurals[0] =  "Canal 2";
    strings["Channel 3##sesd"].plurals[0] =  "Canal 3";
    strings["Channel 4##sesd"].plurals[0] =  "Canal 4";
    strings["Channel 5##sesd"].plurals[0] =  "Canal 5";
    strings["Channel 6##sesd"].plurals[0] =  "Canal 6";
    strings["Channel 7##sesd"].plurals[0] =  "Canal 7";
    strings["Channel 8##sesd"].plurals[0] =  "Canal 8";
    strings["Channel 9##sesd"].plurals[0] =  "Canal 9";
    strings["Channel 10##sesd"].plurals[0] = "Canal 10";
    strings["Channel 11##sesd"].plurals[0] = "Canal 11";
    strings["Channel 12##sesd"].plurals[0] = "Canal 12";
    strings["Channel 13##sesd"].plurals[0] = "Canal 13";
    strings["Channel 14##sesd"].plurals[0] = "Canal 14";
    strings["Channel 15##sesd"].plurals[0] = "Canal 15";
    strings["Channel 16##sesd"].plurals[0] = "Canal 16";
    strings["Channel 17##sesd"].plurals[0] = "Canal 17";
    strings["Channel 18##sesd"].plurals[0] = "Canal 18";
    strings["Channel 19##sesd"].plurals[0] = "Canal 19";
    strings["Channel 20##sesd"].plurals[0] = "Canal 20";
    strings["Channel 21##sesd"].plurals[0] = "Canal 21";
    strings["Channel 22##sesd"].plurals[0] = "Canal 22";
    strings["Channel 23##sesd"].plurals[0] = "Canal 23";
    strings["Channel 24##sesd"].plurals[0] = "Canal 24";
    strings["Channel 25##sesd"].plurals[0] = "Canal 25";
    strings["Channel 26##sesd"].plurals[0] = "Canal 26";
    strings["Channel 27##sesd"].plurals[0] = "Canal 27";
    strings["Channel 28##sesd"].plurals[0] = "Canal 28";
    strings["Channel 29##sesd"].plurals[0] = "Canal 29";
    strings["Channel 30##sesd"].plurals[0] = "Canal 30";
    strings["Channel 31##sesd"].plurals[0] = "Canal 31";
    strings["Channel 32##sesd"].plurals[0] = "Canal 32";

    strings["Square##sesd"].plurals[0] = "Quadrado";

    strings["Square 1##sesd"].plurals[0] = "Quadrado 1";
    strings["Square 2##sesd"].plurals[0] = "Quadrado 2";
    strings["Square 3##sesd"].plurals[0] = "Quadrado 3";

    strings["Pulse##sesd"].plurals[0] = "Pulso";

    strings["Pulse 1##sesd"].plurals[0] = "Pulso 1";
    strings["Pulse 2##sesd"].plurals[0] = "Pulso 2";

    strings["Wave##sesd"].plurals[0] = "Onda";
    strings["Wavetable##sesd"].plurals[0] = "Wavetable";

    strings["Triangle##sesd"].plurals[0] = "Triângulo";

    strings["PCM##sesd"].plurals[0] = "PCM";

    strings["PCM 1##sesd"].plurals[0] =  "PCM 1";
    strings["PCM 2##sesd"].plurals[0] =  "PCM 2";
    strings["PCM 3##sesd"].plurals[0] =  "PCM 3";
    strings["PCM 4##sesd"].plurals[0] =  "PCM 4";
    strings["PCM 5##sesd"].plurals[0] =  "PCM 5";
    strings["PCM 6##sesd"].plurals[0] =  "PCM 6";
    strings["PCM 7##sesd"].plurals[0] =  "PCM 7";
    strings["PCM 8##sesd"].plurals[0] =  "PCM 8";
    strings["PCM 9##sesd"].plurals[0] =  "PCM 9";
    strings["PCM 10##sesd"].plurals[0] = "PCM 10";
    strings["PCM 11##sesd"].plurals[0] = "PCM 11";
    strings["PCM 12##sesd"].plurals[0] = "PCM 12";
    strings["PCM 13##sesd"].plurals[0] = "PCM 13";
    strings["PCM 14##sesd"].plurals[0] = "PCM 14";
    strings["PCM 15##sesd"].plurals[0] = "PCM 15";
    strings["PCM 16##sesd"].plurals[0] = "PCM 16";
    strings["PCM 17##sesd"].plurals[0] = "PCM 17";
    strings["PCM 18##sesd"].plurals[0] = "PCM 18";
    strings["PCM 19##sesd"].plurals[0] = "PCM 19";
    strings["PCM 20##sesd"].plurals[0] = "PCM 20";
    strings["PCM 21##sesd"].plurals[0] = "PCM 21";
    strings["PCM 22##sesd"].plurals[0] = "PCM 22";
    strings["PCM 23##sesd"].plurals[0] = "PCM 23";
    strings["PCM 24##sesd"].plurals[0] = "PCM 24";

    strings["DPCM##sesd"].plurals[0] = "DPCM";

    strings["ADPCM##sesd"].plurals[0] = "ADPCM";

    strings["ADPCM 1##sesd"].plurals[0] = "ADPCM 1";
    strings["ADPCM 2##sesd"].plurals[0] = "ADPCM 2";
    strings["ADPCM 3##sesd"].plurals[0] = "ADPCM 3";

    strings["ADPCM-A 1##sesd"].plurals[0] = "ADPCM-A 1";
    strings["ADPCM-A 2##sesd"].plurals[0] = "ADPCM-A 2";
    strings["ADPCM-A 3##sesd"].plurals[0] = "ADPCM-A 3";
    strings["ADPCM-A 4##sesd"].plurals[0] = "ADPCM-A 4";
    strings["ADPCM-A 5##sesd"].plurals[0] = "ADPCM-A 5";
    strings["ADPCM-A 6##sesd"].plurals[0] = "ADPCM-A 6";

    strings["ADPCM-B##sesd"].plurals[0] = "ADPCM-B";

    strings["Sample##sesd"].plurals[0] = "Sample";

    strings["DAC Left##sesd"].plurals[0] = "DAC Esquerdo";
    strings["DAC Right##sesd"].plurals[0] = "DAC Direito";

    strings["Noise##sesd"].plurals[0] = "Ruído";

    strings["Noise 1##sesd"].plurals[0] = "Ruído 1";
    strings["Noise 2##sesd"].plurals[0] = "Ruído 2";
    strings["Noise 3##sesd"].plurals[0] = "Ruído 3";

    strings["Slope##sesd"].plurals[0] = "Rampa";

    strings["FM 6/PCM 1##sesd"].plurals[0] = "FM 6/PCM 1";
    strings["CSM Timer##sesd"].plurals[0] = "Timer CSM";

    strings["VRC6 Saw##sesd"].plurals[0] = "Serra VRC6";

    strings["4OP 1##sesd"].plurals[0] = "4OP 1";
    strings["4OP 3##sesd"].plurals[0] = "4OP 3";
    strings["4OP 5##sesd"].plurals[0] = "4OP 5";
    strings["4OP 7##sesd"].plurals[0] = "4OP 7";
    strings["4OP 9##sesd"].plurals[0] = "4OP 9";
    strings["4OP 11##sesd"].plurals[0] = "4OP 11";

    strings["Kick/FM 7##sesd"].plurals[0] = "Bumbo/FM 7";
    strings["Kick/FM 16##sesd"].plurals[0] = "Bumbo/FM 16";
    strings["Kick##sesd"].plurals[0] = "Bumbo";
    strings["Snare##sesd"].plurals[0] = "Caixa";
    strings["Top##sesd"].plurals[0] = "Prato";
    strings["HiHat##sesd"].plurals[0] = "Chimbal";
    strings["Tom##sesd"].plurals[0] = "Surdo";
    strings["Rim##sesd"].plurals[0] = "Aro";

    // channel short names

    strings["CH1##sesd"].plurals[0] = "CN1";
    strings["CH2##sesd"].plurals[0] = "CN2";
    strings["CH3##sesd"].plurals[0] = "CN3";
    strings["CH4##sesd"].plurals[0] = "CN4";
    strings["CH5##sesd"].plurals[0] = "CN5";
    strings["CH6##sesd"].plurals[0] = "CN6";
    strings["CH7##sesd"].plurals[0] = "CN7";
    strings["CH8##sesd"].plurals[0] = "CN8";
    strings["CH9##sesd"].plurals[0] = "CN9";

    strings["NO##sesd"].plurals[0] = "RU";
    strings["N1##sesd"].plurals[0] = "R1";
    strings["N2##sesd"].plurals[0] = "R2";
    strings["N3##sesd"].plurals[0] = "R3";

    strings["SL##sesd"].plurals[0] = "RA";

    strings["S1##sesd"].plurals[0] = "Q1";
    strings["S2##sesd"].plurals[0] = "Q2";
    strings["S3##sesd"].plurals[0] = "Q3";
    strings["S4##sesd"].plurals[0] = "Q4";
    strings["S5##sesd"].plurals[0] = "Q5";
    strings["S6##sesd"].plurals[0] = "Q6";

    strings["P1##sesd"].plurals[0] = "S1";
    strings["P2##sesd"].plurals[0] = "S2";
    strings["P3##sesd"].plurals[0] = "S3";
    strings["P4##sesd"].plurals[0] = "S4";
    strings["P5##sesd"].plurals[0] = "S5";
    strings["P6##sesd"].plurals[0] = "S6";
    strings["P7##sesd"].plurals[0] = "S7";
    strings["P8##sesd"].plurals[0] = "S8";
    strings["P9##sesd"].plurals[0] = "S9";
    strings["P10##sesd"].plurals[0] = "S10";
    strings["P11##sesd"].plurals[0] = "S11";
    strings["P12##sesd"].plurals[0] = "S12";
    strings["P13##sesd"].plurals[0] = "S13";
    strings["P14##sesd"].plurals[0] = "S14";
    strings["P15##sesd"].plurals[0] = "S15";
    strings["P16##sesd"].plurals[0] = "S16";
    strings["P17##sesd"].plurals[0] = "S17";
    strings["P18##sesd"].plurals[0] = "S18";
    strings["P19##sesd"].plurals[0] = "S19";
    strings["P20##sesd"].plurals[0] = "S20";
    strings["P21##sesd"].plurals[0] = "S21";
    strings["P22##sesd"].plurals[0] = "S22";
    strings["P23##sesd"].plurals[0] = "S23";
    strings["P24##sesd"].plurals[0] = "S24";

    strings["BD##sesd"].plurals[0] = "BU";
    strings["SD##sesd"].plurals[0] = "CX";
    strings["TP##sesd"].plurals[0] = "PR";
    strings["HH##sesd"].plurals[0] = "CH";
    strings["TM##sesd"].plurals[0] = "SU";
    strings["RM##sesd"].plurals[0] = "AR";

    strings["P##sesd"].plurals[0] = "S";

    strings["VS##sesd"].plurals[0] = "VS";

    strings["TR##sesd"].plurals[0] = "TR";
    strings["DMC##sesd"].plurals[0] = "DMC";

    strings["WA##sesd"].plurals[0] = "ON";
}
