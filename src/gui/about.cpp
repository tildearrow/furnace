/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2025 tildearrow and contributors
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
  _N("is proud to present"),
  "",
  ("Furnace " DIV_VERSION),
  "",
  _N("the biggest multi-system chiptune tracker!"),
  _N("featuring DefleMask song compatibility."),
  "",
  _N("> CREDITS <"),
  "",
  _N("-- program --"),
  "tildearrow",
  _N("A M 4 N (intro tune)"),
  "Adam Lederer",
  "akumanatt",
  "asiekierka",
  "cam900",
  "djtuBIG-MaliceX",
  "Eknous",
  "Kagamiin~",
  "laoo",
  "LTVA",
  "MooingLemur",
  "OPNA2608",
  "scratchminer",
  "superctr",
  "System64",
  "techmetx11",
  "",
  _N("-- graphics/UI design --"),
  "tildearrow",
  "BlastBrothers",
  "Electric Keet",
  "Mahbod Karamoozian",
  "Raijin",
  "",
  _N("-- documentation --"),
  "brickblock369",
  "cam900",
  "DeMOSic",
  "Electric Keet",
  "freq-mod",
  "host12prog",
  "Lunathir",
  "tildearrow",
  "",
  _N("-- localization/translation team --"),
  "Bahasa Indonesia: ZoomTen (Zumi)",
  "Español: CrimsonZN, ThaCuber, tildearrow",
  "Հայերեն: Eknous",
  "한국어: Heemin, leejh20, Nicknamé",
  "Nederlands: Lunathir",
  "Polski: freq-mod, PoznańskiSzybkowiec",
  "Português (Brasil): Kagamiin~",
  "Русский: Background2982, LTVA",
  "Slovenčina: Wegfrei",
  "Svenska: RevvoBolt",
  "ไทย: akumanatt",
  "",
  _N("-- additional feedback/fixes --"),
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
  _N("-- Metal backend test team --"),
  "Diggo",
  "konard",
  "NaxeCode",
  "scratchminer",
  "",
  _N("-- DirectX 9 backend test team --"),
  "EpicTyphlosion",
  "Lunathir",
  "Mr. Hassium",
  "wbcbz7",
  "Yuzu4K",
  "",
  _N("powered by:"),
  _N("Dear ImGui by Omar Cornut"),
  _N("SDL2 by Sam Lantinga"),
#ifdef HAVE_FREETYPE
  "FreeType",
#endif
  _N("zlib by Jean-loup Gailly"),
  _N("and Mark Adler"),
  _N("libsndfile by Erik de Castro Lopo"),
  _N("Portable File Dialogs by Sam Hocevar"),
  _N("Native File Dialog by Frogtoss Games"),
  "PortAudio",
  _N("Weak-JACK by x42"),
  _N("RtMidi by Gary P. Scavone"),
  _N("FFTW by Matteo Frigo and Steven G. Johnson"),
  _N("backward-cpp by Google"),
  _N("adpcm by superctr"),
  _N("adpcm-xq by David Bryant"),
  _N("Nuked-OPL3/OPLL/OPM/OPN2/PSG by nukeykt"),
  _N("YM3812-LLE, YMF262-LLE, YMF276-LLE and YM2608-LLE by nukeykt"),
  _N("ESFMu (modified version) by Kagamiin~"),
  _N("ymfm by Aaron Giles"),
  _N("emu2413 by Digital Sound Antiques"),
  _N("MAME SN76496 by Nicola Salmoria"),
  _N("MAME AY-3-8910 by Couriersud"),
  _N("with AY8930 fixes by Eulous, cam900 and Grauw"),
  _N("MAME SAA1099 by Juergen Buchmueller and Manuel Abadia"),
  _N("MAME Namco WSG by Nicola Salmoria and Aaron Giles"),
  _N("MAME RF5C68 core by Olivier Galibert and Aaron Giles"),
  _N("MAME MSM5232 core by Jarek Burczynski and Hiromitsu Shioya"),
  _N("MAME MSM6258 core by Barry Rodewald"),
  _N("MAME YMZ280B core by Aaron Giles"),
  _N("MAME GA20 core by Acho A. Tang, R. Belmont and Valley Bell (modified version)"),
  _N("MAME SegaPCM core by Hiromitsu Shioya and Olivier Galibert"),
  _N("MAME µPD1771C-017 HLE core by David Viens"),
  _N("SAASound by Dave Hooper and Simon Owen"),
  _N("SameBoy by Lior Halphon"),
  _N("Mednafen PCE, WonderSwan, T6W28 and Virtual Boy audio cores"),
  _N("WonderSwan new core by asiekierka"),
  _N("SNES DSP core by Blargg"),
  _N("puNES (NES, MMC5 and FDS) by FHorse (modified version)"),
  _N("NSFPlay (NES and FDS) by Brad Smith and Brezza"),
  _N("reSID by Dag Lem"),
  _N("reSIDfp by Dag Lem, Antti Lankila"),
  _N("and Leandro Nini"),
  _N("dSID by DefleMask Team based on jsSID"),
  _N("Stella by Stella Team"),
  _N("QSound emulator by superctr and Valley Bell"),
  _N("VICE VIC-20 sound core by Rami Rasanen and viznut"),
  _N("VICE TED sound core by Andreas Boose, Tibor Biczo"),
  _N("and Marco van den Heuvel"),
  _N("VERA sound core by Frank van den Hoef"),
  _N("mzpokeysnd POKEY emulator by Michael Borisov"),
  _N("ASAP POKEY emulator by Piotr Fusik"),
  _N("ported by laoo to C++"),
  _N("vgsound_emu (second version, modified version) by cam900"),
  _N("Impulse Tracker GUS volume table by Jeffrey Lim"),
  _N("Schism Tracker IT sample decompression"),
  _N("SM8521 emulator (modified version) by cam900"),
  _N("D65010G031 emulator (modified version) by cam900"),
  _N("Namco C140/C219 emulator (modified version) by cam900"),
  _N("PowerNoise emulator by scratchminer"),
  _N("ep128emu by Istvan Varga"),
  _N("NDS sound emulator by cam900"),
  _N("Adlib-related formats import routines adapted from"),
  _N("adlib2vgm by SudoMaker"),
  _N("openMSX YMF278 emulator (modified version) by the openMSX developers"),
  _N("SID2 emulator by LTVA (modification of reSID emulator)"),
  _N("SID3 emulator by LTVA"),
  "",
  _N("greetings to:"),
  "floxy!",
  "NEOART Costa Rica",
  "Xenium Demoparty",
  "@party",
  _N("all members of Deflers of Noice!"),
  "",
  _N("copyright © 2021-2025 tildearrow"),
  _N("(and contributors)."),
  _N("licensed under GPLv2+! see"),
  _N("LICENSE for more information."),
  "",
  _N("help Furnace grow:"),
  "https://github.com/tildearrow/furnace",
  "",
  _N("contact tildearrow at:"),
  "https://tildearrow.org/?p=contact",
  "",
  _N("disclaimer:"),
  _N("despite the fact this program works"),
  _N("with the .dmf file format, it is NOT"),
  _N("affiliated with Delek or DefleMask in"),
  _N("any way, nor it is a replacement for"),
  _N("the original program."),
  "",
  _N("it also comes with ABSOLUTELY NO WARRANTY."),
  "",
  _N("thanks to all contributors/bug reporters!")
};

const size_t aboutCount=sizeof(aboutLine)/sizeof(aboutLine[0]);

void FurnaceGUI::drawAbout() {
  // do stuff
  if (ImGui::Begin("About Furnace",NULL,ImGuiWindowFlags_Modal|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoDocking|ImGuiWindowFlags_NoTitleBar,_("About Furnace"))) {
    ImGui::SetWindowPos(ImVec2(0,0));
    ImGui::SetWindowSize(ImVec2(canvasW,canvasH));
    ImGui::PushFont(bigFont);
    ImDrawList* dl=ImGui::GetWindowDrawList();
    float r=0;
    float g=0;
    float b=0;
    float peakMix=0.3;
    ImGui::ColorConvertHSVtoRGB(aboutHue,1.0,0.475,r,g,b);
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
      // don't localize tildearrow, the version or an empty line
      const char* nextLine=(i==0 || i==3 || aboutLine[i][0]==0)?aboutLine[i]:_(aboutLine[i]);
      double posX=(canvasW/2.0)+(sin(double(i)*0.5+double(aboutScroll)/(90.0*dpiScale))*120*dpiScale)-(ImGui::CalcTextSize(nextLine).x*0.5);
      double posY=(canvasH-aboutScroll+42*i*dpiScale);
      if (posY<-80*dpiScale || posY>canvasH) continue;
      dl->AddText(bigFont,bigFont->FontSize,
                  ImVec2(posX+dpiScale,posY+dpiScale),
                  0xff000000,nextLine);
      dl->AddText(bigFont,bigFont->FontSize,
                  ImVec2(posX+dpiScale,posY-dpiScale),
                  0xff000000,nextLine);
      dl->AddText(bigFont,bigFont->FontSize,
                  ImVec2(posX-dpiScale,posY+dpiScale),
                  0xff000000,nextLine);
      dl->AddText(bigFont,bigFont->FontSize,
                  ImVec2(posX-dpiScale,posY-dpiScale),
                  0xff000000,nextLine);
      dl->AddText(bigFont,bigFont->FontSize,
                  ImVec2(posX,posY),
                  0xffffffff,nextLine);
    }
    ImGui::PopFont();

    float timeScale=60.0f*ImGui::GetIO().DeltaTime;

    aboutHue+=(0.001+peakMix*0.004)*timeScale;
    aboutScroll+=(2+(peakMix>0.78)*3)*timeScale*dpiScale;
    aboutSin+=(1+(peakMix>0.75)*2)*timeScale;

    while (aboutHue>1) aboutHue--;
    while (aboutSin>=2400) aboutSin-=2400;
    if (aboutScroll>(42*dpiScale*aboutCount+canvasH)) aboutScroll=-20*dpiScale;

    if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
      aboutOpen=false;
      if (modified) {
        showWarning(_("Unsaved changes! Save changes before playing?"),GUI_WARN_CV);
      } else {
        cvOpen=true;
        cvNotSerious=true;
      }
    }

    WAKE_UP;
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_ABOUT;
  ImGui::End();
}
