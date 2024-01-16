/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2023 tildearrow and contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#define _USE_MATH_DEFINES
#include "gui.h"
#include <math.h>

const char* aboutLine[]={
  "tildearrow",
  "and Furnace-B developers##sgab",
  "are proud to present##sgab",
  "",
  ("Furnace " DIV_VERSION),
  "",
  "the biggest multi-system chiptune tracker!##sgab",
  "featuring DefleMask song compatibility.##sgab",
  "",
  "> CREDITS <##sgab",
  "",
  "-- program --##sgab",
  "tildearrow",
  "A M 4 N (intro tune)",
  "akumanatt",
  "cam900",
  "djtuBIG-MaliceX",
  "Eknous",
  "laoo",
  "LTVA1",
  "MooingLemur",
  "OPNA2608",
  "superctr",
  "System64",
  "",
  "-- graphics/UI design --##sgab",
  "tildearrow",
  "BlastBrothers",
  "Electric Keet",
  "Mahbod Karamoozian",
  "Raijin",
  "",
  "-- documentation --##sgab",
  "brickblock369",
  "cam900",
  "DeMOSic",
  "Electric Keet",
  "freq-mod",
  "host12prog",
  "Lunathir",
  "tildearrow",
  "",
  "-- demo songs --##sgab",
  "0x5066",
  "Abstract 64",
  "ActualNK358",
  "airconmanws",
  "akumanatt",
  "AmigaX",
  "AquaDoesStuff",
  "AURORA*FIELDS",
  "battybeats",
  "Bernie",
  "Blaze Weednix",
  "BlueElectric05",
  "breakthetargets",
  "brickblock369",
  "Brightonic",
  "Burnt Fishy",
  "CaptainMalware",
  "Clingojam",
  "ControlleronaHanger",
  "Crisps",
  "DeMOSic",
  "DevEd",
  "Dippy",
  "djtuBIG-MaliceX",
  "dmKaltsit",
  "Dolce",
  "dumbut",
  "Eknous",
  "Electric Keet",
  "EpicTyphlosion",
  "FΛDE",
  "Forte",
  "Fragmare",
  "freq-mod",
  "gtr3qq",
  "Hortus",
  "ifrit05",
  "iyatemu",
  "JayBOB18",
  "Jimmy-DS",
  "Kagamiin~",
  "kleeder",
  "Korbo",
  "jaezu",
  "Laggy",
  "LovelyA72",
  "LTVA1",
  "LunaMoth",
  "Lunathir",
  "LVintageNerd",
  "Mahbod Karamoozian",
  "Marisa Kirisame [DJ MasterSpark]",
  "Martin Demsky",
  "masicbemester",
  "MelonadeM",
  "Miker",
  "NeoWar",
  "niffuM",
  "<nk>",
  "Notakin",
  "NyaongI",
  "Pale Moon",
  "PeyPey",
  "PichuMario",
  "potatoTeto",
  "psxdominator",
  "Raijin",
  "railzen7",
  "RepellantMold",
  "RetroCarrot",
  "RevvoBolt",
  "scooblee",
  "sillygoose",
  "SnugglyBun",
  "SuperJet Spade",
  "Supper_E1",
  "SwapXFO",
  "System64",
  "TakuikaNinja",
  "tapekeep",
  "TCORPStudios",
  "Teuthida",
  "ThaCuber",
  "The Blender Fiddler",
  "TheDuccinator",
  "theloredev",
  "The Beesh-Spweesh!",
  "TheRealHedgehogSonic",
  "tildearrow",
  "Uhrwerk Klockwerx",
  "Ultraprogramer",
  "UserSniper",
  "Weeppiko",
  "Xan",
  "YaIiya",
  "Yuzu4K",
  "Zaxolotl",
  "ZoomTen (Zumi)",
  "",
  "-- additional feedback/fixes --##sgab",
  "Electric Keet",
  "fd",
  "GENATARi",
  "host12prog",
  "jvsTSX",
  "Lumigado",
  "Lunathir",
  "plane",
  "TheEssem",
  "",
  "powered by:##sgab",
  "Dear ImGui by Omar Cornut##sgab",
  "SDL2 by Sam Lantinga##sgab",
#ifdef HAVE_FREETYPE
  "FreeType",
#endif
  "zlib by Jean-loup Gailly##sgab",
  "and Mark Adler##sgab",
  "libsndfile by Erik de Castro Lopo##sgab",
  "Portable File Dialogs by Sam Hocevar##sgab",
  "Native File Dialog by Frogtoss Games##sgab",
  "PortAudio",
  "Weak-JACK by x42##sgab",
  "RtMidi by Gary P. Scavone##sgab",
  "FFTW by Matteo Frigo and Steven G. Johnson##sgab",
  "backward-cpp by Google##sgab",
  "adpcm by superctr##sgab",
  "Nuked-OPL3/OPLL/OPM/OPN2/PSG by nukeykt##sgab",
  "YM3812-LLE, YMF262-LLE and YMF276-LLE by nukeykt##sgab",
  "ymfm by Aaron Giles##sgab",
  "MAME SN76496 by Nicola Salmoria##sgab",
  "MAME AY-3-8910 by Couriersud##sgab",
  "with AY8930 fixes by Eulous, cam900 and Grauw##sgab",
  "MAME SAA1099 by Juergen Buchmueller and Manuel Abadia##sgab",
  "MAME Namco WSG by Nicola Salmoria and Aaron Giles##sgab",
  "MAME RF5C68 core by Olivier Galibert and Aaron Giles##sgab",
  "MAME MSM5232 core by Jarek Burczynski and Hiromitsu Shioya##sgab",
  "MAME MSM6258 core by Barry Rodewald##sgab",
  "MAME YMZ280B core by Aaron Giles##sgab",
  "MAME GA20 core by Acho A. Tang and R. Belmont##sgab",
  "MAME SegaPCM core by Hiromitsu Shioya and Olivier Galibert##sgab",
  "SAASound by Dave Hooper and Simon Owen##sgab",
  "SameBoy by Lior Halphon##sgab",
  "Mednafen PCE, WonderSwan, T6W28 and Virtual Boy audio cores##sgab",
  "SNES DSP core by Blargg##sgab",
  "puNES (NES, MMC5 and FDS) by FHorse##sgab",
  "NSFPlay (NES and FDS) by Brad Smith and Brezza##sgab",
  "reSID by Dag Lem##sgab",
  "reSIDfp by Dag Lem, Antti Lankila##sgab",
  "and Leandro Nini##sgab",
  "dSID by DefleMask Team based on jsSID##sgab",
  "Stella by Stella Team##sgab",
  "QSound emulator by superctr and Valley Bell##sgab",
  "VICE VIC-20 sound core by Rami Rasanen and viznut##sgab",
  "VICE TED sound core by Andreas Boose, Tibor Biczo##sgab",
  "and Marco van den Heuvel##sgab",
  "VERA sound core by Frank van den Hoef##sgab",
  "mzpokeysnd POKEY emulator by Michael Borisov##sgab",
  "ASAP POKEY emulator by Piotr Fusik##sgab",
  "ported by laoo to C++##sgab",
  "vgsound_emu (second version, modified version) by cam900##sgab",
  "SM8521 emulator (modified version) by cam900##sgab",
  "D65010G031 emulator (modified version) by cam900##sgab",
  "Namco C140/C219 emulator (modified version) by cam900##sgab",
  "ESFMu emulator by Kagamiin~##sgab",
  "",
  "greetings to:##sgab",
  "NEOART Costa Rica##sgab",
  "Xenium Demoparty##sgab",
  "all members of Deflers of Noice!##sgab",
  "",
  "copyright © 2021-2023 tildearrow##sgab",
  "(and contributors).##sgab",
  "licensed under GPLv2+! see##sgab",
  "LICENSE for more information.##sgab",
  "",
  "help Furnace grow:##sgab",
  "https://github.com/tildearrow/furnace",
  "help Furnace-B:##sgab",
  "https://github.com/LTVA1/furnace",
  "",
  "contact tildearrow at:##sgab",
  "https://tildearrow.org/?p=contact",
  "",
  "disclaimer:##sgab",
  "despite the fact this program works##sgab",
  "with the .dmf file format, it is NOT##sgab",
  "affiliated with Delek or DefleMask in##sgab",
  "any way, nor it is a replacement for##sgab",
  "the original program.##sgab",
  "",
  "it also comes with ABSOLUTELY NO WARRANTY.##sgab",
  "",
  "thanks to all contributors/bug reporters!##sgab"
};

const size_t aboutCount=sizeof(aboutLine)/sizeof(aboutLine[0]);

void FurnaceGUI::drawAbout() {
  // do stuff
  if (ImGui::Begin("About Furnace",NULL,ImGuiWindowFlags_Modal|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoDocking|ImGuiWindowFlags_NoTitleBar, _L("About Furnace###About Furnace"))) {
    ImGui::SetWindowPos(ImVec2(0,0));
    ImGui::SetWindowSize(ImVec2(canvasW,canvasH));
    ImGui::PushFont(bigFont);
    ImDrawList* dl=ImGui::GetWindowDrawList();
    float r=0;
    float g=0;
    float b=0;
    float peakMix=settings.partyTime?0:0.3;
    if (settings.partyTime) {
      for (int j=0; j<e->getAudioDescGot().outChans; j++) {
        peakMix+=peak[j];
      }
      peakMix/=e->getAudioDescGot().outChans;
    }
    ImGui::ColorConvertHSVtoRGB(aboutHue,1.0,0.25+MIN(0.75f,peakMix*0.75f),r,g,b);
    dl->AddRectFilled(ImVec2(0,0),ImVec2(canvasW,canvasH),0xff000000);
    bool skip=false;
    bool skip2=false;
    for (int i=(-80-sin(double(aboutSin)*2*M_PI/120.0)*80.0)*2*dpiScale; i<canvasW; i+=160*dpiScale) {
      skip2=!skip2;
      skip=skip2;
      for (int j=(-80-cos(double(aboutSin)*2*M_PI/150.0)*80.0)*2*dpiScale; j<canvasH; j+=160*dpiScale) {
        skip=!skip;
        if (skip) continue;
        dl->AddRectFilled(ImVec2(i,j),ImVec2(i+160*dpiScale,j+160*dpiScale),ImGui::GetColorU32(ImVec4(r*0.25,g*0.25,b*0.25,1.0)));
      }
    }

    skip=false;
    skip2=false;
    for (int i=(-80-cos(double(aboutSin)*2*M_PI/120.0)*80.0)*2*dpiScale; i<canvasW; i+=160*dpiScale) {
      skip2=!skip2;
      skip=skip2;
      for (int j=(-80-sin(double(aboutSin)*2*M_PI/150.0)*80.0)*2*dpiScale; j<canvasH; j+=160*dpiScale) {
        skip=!skip;
        if (skip) continue;
        dl->AddRectFilled(ImVec2(i,j),ImVec2(i+160*dpiScale,j+160*dpiScale),ImGui::GetColorU32(ImVec4(r*0.5,g*0.5,b*0.5,1.0)));
      }
    }

    skip=false;
    skip2=false;
    for (int i=(-160+fmod(aboutSin*2,160))*2*dpiScale; i<canvasW; i+=160*dpiScale) {
      skip2=!skip2;
      skip=skip2;
      for (int j=(-240-cos(double(aboutSin*M_PI/300.0))*240.0)*2*dpiScale; j<canvasH; j+=160*dpiScale) {
        skip=!skip;
        if (skip) continue;
        dl->AddRectFilled(ImVec2(i,j),ImVec2(i+160*dpiScale,j+160*dpiScale),ImGui::GetColorU32(ImVec4(r*0.75,g*0.75,b*0.75,1.0)));
      }
    }

    for (size_t i=0; i<aboutCount; i++) {
      double posX=(canvasW/2.0)+(sin(double(i)*0.5+double(aboutScroll)/(90.0*dpiScale))*120*dpiScale)-(ImGui::CalcTextSize(_L(aboutLine[i])).x*0.5);
      double posY=(canvasH-aboutScroll+42*i*dpiScale);
      if (posY<-80*dpiScale || posY>canvasH) continue;
      dl->AddText(bigFont,bigFont->FontSize,
                  ImVec2(posX+dpiScale,posY+dpiScale),
                  0xff000000,_L(aboutLine[i]));
      dl->AddText(bigFont,bigFont->FontSize,
                  ImVec2(posX+dpiScale,posY-dpiScale),
                  0xff000000,_L(aboutLine[i]));
      dl->AddText(bigFont,bigFont->FontSize,
                  ImVec2(posX-dpiScale,posY+dpiScale),
                  0xff000000,_L(aboutLine[i]));
      dl->AddText(bigFont,bigFont->FontSize,
                  ImVec2(posX-dpiScale,posY-dpiScale),
                  0xff000000,_L(aboutLine[i]));
      dl->AddText(bigFont,bigFont->FontSize,
                  ImVec2(posX,posY),
                  0xffffffff,_L(aboutLine[i]));
    }
    ImGui::PopFont();

    float timeScale=60.0f*ImGui::GetIO().DeltaTime;

    aboutHue+=(0.001+peakMix*0.004)*timeScale;
    aboutScroll+=(2+(peakMix>0.78)*3)*timeScale*dpiScale;
    aboutSin+=(1+(peakMix>0.75)*2)*timeScale;

    while (aboutHue>1) aboutHue--;
    while (aboutSin>=2400) aboutSin-=2400;
    if (aboutScroll>(42*dpiScale*aboutCount+canvasH)) aboutScroll=-20*dpiScale;

    WAKE_UP;
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_ABOUT;
  ImGui::End();
}
