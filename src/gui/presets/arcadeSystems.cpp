/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
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

#include "presets.h"

void initSystemPresetsArcadeSystems(std::vector<FurnaceGUISysCategory>& sysCategories) {
  CATEGORY_BEGIN(_("Arcade systems"),_("INSERT COIN"));
  // MANUFACTURERS
  ENTRY(
    _("Alpha Denshi"), {}
  );
    SUB_ENTRY(
      _("Alpha Denshi Alpha-68K"), {
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=3"), // 3MHz
        CH(DIV_SYSTEM_OPLL, 1.0f, 0, "clockSel=0"), // 3.58MHz
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0,
          "rate=7614\n"
          "outDepth=7\n"
        ) // software controlled 8 bit DAC
      }
    );
    SUB_ENTRY(
      _("Alpha Denshi Alpha-68K (extended channel 3)"), {
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=3"), // 3MHz
        CH(DIV_SYSTEM_OPLL, 1.0f, 0, "clockSel=0"), // 3.58MHz
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0,
          "rate=7614\n"
          "outDepth=7\n"
        ) // software controlled 8 bit DAC
      }
    );
    SUB_ENTRY(
      _("Alpha Denshi Alpha-68K (CSM)"), {
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=3"), // 3MHz
        CH(DIV_SYSTEM_OPLL, 1.0f, 0, "clockSel=0"), // 3.58MHz
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0,
          "rate=7614\n"
          "outDepth=7\n"
        ) // software controlled 8 bit DAC
      }
    );
    SUB_ENTRY(
      _("Alpha Denshi Alpha-68K (drums mode)"), {
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=3"), // 3MHz
        CH(DIV_SYSTEM_OPLL_DRUMS, 1.0f, 0, "clockSel=0"), // 3.58MHz
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0,
          "rate=7614\n"
          "outDepth=7\n"
        ) // software controlled 8 bit DAC
      }
    );
    SUB_ENTRY(
      _("Alpha Denshi Alpha-68K (extended channel 3; drums mode)"), {
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=3"), // 3MHz
        CH(DIV_SYSTEM_OPLL_DRUMS, 1.0f, 0, "clockSel=0"), // 3.58MHz
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0,
          "rate=7614\n"
          "outDepth=7\n"
        ) // software controlled 8 bit DAC
      }
    );
    SUB_ENTRY(
      _("Alpha Denshi Alpha-68K (CSM; drums mode)"), {
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=3"), // 3MHz
        CH(DIV_SYSTEM_OPLL_DRUMS, 1.0f, 0, "clockSel=0"), // 3.58MHz
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0,
          "rate=7614\n"
          "outDepth=7\n"
        ) // software controlled 8 bit DAC
      }
    );
    SUB_ENTRY(
      _("Alpha Denshi Equites"), {
        CH(DIV_SYSTEM_MSM5232, 1.0f, 0, "customClock=6144000"),
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=14"),
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0, 
           "rate=11025\n" 
           "outDepth=5\n"
        ),
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0, 
           "rate=11025\n" 
           "outDepth=5\n"
        ) // don't know what the actual sample rate is
      }
    );

  ENTRY(
    _("Atari"), {}
  );
    SUB_ENTRY(
      _("Atari Klax"), { 
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "clockSel=7") // 0.895MHz (3.579545MHz / 4)
      }
    );
    SUB_ENTRY(
      _("Atari Rampart"), {
        CH(DIV_SYSTEM_OPLL, 1.0f, 0, ""), // 3.579545MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "clockSel=14") // 1.193MHz (3.579545MHz / 3)
      }
    );
    SUB_ENTRY(
      _("Atari Rampart (drums mode)"), { 
        CH(DIV_SYSTEM_OPLL_DRUMS, 1.0f, 0, ""), // 3.579545MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "clockSel=14") // 1.193MHz (3.579545MHz / 3)
      }
    );
    SUB_ENTRY(
      _("Atari JSA IIIs"), { 
        CH(DIV_SYSTEM_YM2151, 1.0f, 0, ""), // 3.579545MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, -1.0f, "clockSel=14"), // 1.193MHz (3.579545MHz / 3), Left output
        CH(DIV_SYSTEM_MSM6295, 1.0f, 1.0f, "clockSel=14") // 1.193MHz (3.579545MHz / 3), Right output
      }
    );
    SUB_ENTRY(
      _("Atari Marble Madness"), {
        CH(DIV_SYSTEM_YM2151, 1.0f, 0, ""),
        CH(DIV_SYSTEM_POKEY, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Atari Championship Sprint"), {
        CH(DIV_SYSTEM_YM2151, 1.0f, 0, ""),
        CH(DIV_SYSTEM_POKEY, 1.0f, 0, ""),
        CH(DIV_SYSTEM_POKEY, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Atari Tetris"), {
        CH(DIV_SYSTEM_POKEY, 1.0f, 0, ""),
        CH(DIV_SYSTEM_POKEY, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Atari I, Robot"), {
        CH(DIV_SYSTEM_POKEY, 1.0f, 0, "customClock=1512000"),
        CH(DIV_SYSTEM_POKEY, 1.0f, 0, "customClock=1512000"),
        CH(DIV_SYSTEM_POKEY, 1.0f, 0, "customClock=1512000"),
        CH(DIV_SYSTEM_POKEY, 1.0f, 0, "customClock=1512000")
      }
    );

  ENTRY(
    _("Capcom"), {}
  );
    SUB_ENTRY(
      _("Capcom Exed Exes"), {
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=4"), // 1.5MHz
        CH(DIV_SYSTEM_SMS, 1.0f, 0,
          "clockSel=4\n"
          "chipType=1\n"
        ), // SN76489 3MHz
        CH(DIV_SYSTEM_SMS, 1.0f, 0,
          "clockSel=4\n"
          "chipType=1\n"
        ) // SN76489 3MHz
      }
    );
    SUB_ENTRY(
      _("Capcom Arcade"), { // 1943, Side arms, etc
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=5"), // 4 or 1.5MHz; various per games
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=5")
      }
    );
    SUB_ENTRY(
      _("Capcom Arcade (extended channel 3 on first OPN)"), {
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=5"),
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=5")
      }
    );
    SUB_ENTRY(
      _("Capcom Arcade (extended channel 3 on second OPN)"), {
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=5"),
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=5")
      }
    );
    SUB_ENTRY(
      _("Capcom Arcade (extended channel 3 on both OPNs)"), {
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=5"),
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=5")
      }
    );
    SUB_ENTRY(
      _("Capcom Arcade (CSM on first OPN)"), {
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=5"),
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=5")
      }
    );
    SUB_ENTRY(
      _("Capcom Arcade (CSM on second OPN)"), {
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=5"),
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=5")
      }
    );
    SUB_ENTRY(
      _("Capcom Arcade (CSM on both OPNs)"), {
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=5"),
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=5")
      }
    );
    SUB_ENTRY(
      _("Capcom CPS-1"), { 
        CH(DIV_SYSTEM_YM2151, 1.0f, 0, ""), // 3.58MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Capcom CPS-2 (QSound)"), {
        CH(DIV_SYSTEM_QSOUND, 1.0f, 0, "")
      }
    );

  ENTRY(
    _("Data East"), {}
  );
    SUB_ENTRY(
      _("Data East Karnov"), {
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=5"), // 1.5MHz
        CH(DIV_SYSTEM_OPL, 1.0f, 0, "clockSel=3") // 3MHz
      }
    );
    SUB_ENTRY(
      _("Data East Karnov (extended channel 3)"), {
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=5"), // 1.5MHz
        CH(DIV_SYSTEM_OPL, 1.0f, 0, "clockSel=3") // 3MHz
      }
    );
    SUB_ENTRY(
      _("Data East Karnov (CSM)"), {
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=5"), // 1.5MHz
        CH(DIV_SYSTEM_OPL, 1.0f, 0, "clockSel=3") // 3MHz
      }
    );
    SUB_ENTRY(
      _("Data East Karnov (drums mode)"), {
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=5"), // 1.5MHz
        CH(DIV_SYSTEM_OPL_DRUMS, 1.0f, 0, "clockSel=3") // 3MHz
      }
    );
    SUB_ENTRY(
      _("Data East Karnov (extended channel 3; drums mode)"), {
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=5"), // 1.5MHz
        CH(DIV_SYSTEM_OPL_DRUMS, 1.0f, 0, "clockSel=3") // 3MHz
      }
    );
    SUB_ENTRY(
      _("Data East Karnov (CSM; drums mode)"), {
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=5"), // 1.5MHz
        CH(DIV_SYSTEM_OPL_DRUMS, 1.0f, 0, "clockSel=3") // 3MHz
      }
    );
    SUB_ENTRY(
      _("Data East Arcade"), { // Bad Dudes, RoboCop, etc
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=5"), // 1.5MHz
        CH(DIV_SYSTEM_OPL2, 1.0f, 0, "clockSel=3"), // 3MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "") // 1 to 1.056MHz; various per games or optional
      }
    );
    SUB_ENTRY(
      _("Data East Arcade (extended channel 3)"), { // Bad Dudes, RoboCop, etc
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=5"), // 1.5MHz
        CH(DIV_SYSTEM_OPL2, 1.0f, 0, "clockSel=3"), // 3MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "") // 1 to 1.056MHz; various per games or optional
      }
    );
    SUB_ENTRY(
      _("Data East Arcade (CSM)"), { // Bad Dudes, RoboCop, etc
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=5"), // 1.5MHz
        CH(DIV_SYSTEM_OPL2, 1.0f, 0, "clockSel=3"), // 3MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "") // 1 to 1.056MHz; various per games or optional
      }
    );
    SUB_ENTRY(
      _("Data East Arcade (drums mode)"), { // Bad Dudes, RoboCop, etc
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=5"), // 1.5MHz
        CH(DIV_SYSTEM_OPL2_DRUMS, 1.0f, 0, "clockSel=3"), // 3MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "") // 1 to 1.056MHz; various per games or optional
      }
    );
    SUB_ENTRY(
      _("Data East Arcade (extended channel 3; drums mode)"), { // Bad Dudes, RoboCop, etc
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=5"), // 1.5MHz
        CH(DIV_SYSTEM_OPL2_DRUMS, 1.0f, 0, "clockSel=3"), // 3MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "") // 1 to 1.056MHz; various per games or optional
      }
    );
    SUB_ENTRY(
      _("Data East Arcade (CSM; drums mode)"), { // Bad Dudes, RoboCop, etc
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=5"), // 1.5MHz
        CH(DIV_SYSTEM_OPL2_DRUMS, 1.0f, 0, "clockSel=3"), // 3MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "") // 1 to 1.056MHz; various per games or optional
      }
    );
    SUB_ENTRY(
      _("Data East PCX"), {
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=5"), // 1.5MHz
        CH(DIV_SYSTEM_PCE, 1.0f, 0, "")
        // software controlled MSM5205
      }
    );
    SUB_ENTRY(
      _("Data East PCX (extended channel 3)"), {
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=5"), // 1.5MHz
        CH(DIV_SYSTEM_PCE, 1.0f, 0, "")
        // software controlled MSM5205
      }
    );
    SUB_ENTRY(
      _("Data East PCX (CSM)"), {
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=5"), // 1.5MHz
        CH(DIV_SYSTEM_PCE, 1.0f, 0, "")
        // software controlled MSM5205
      }
    );
    SUB_ENTRY(
      _("Data East Dark Seal"), { // Dark Seal, Crude Buster, Vapor Trail, etc
        CH(DIV_SYSTEM_YM2151, 1.0f, 0, ""), // 3.580MHz (32.22MHz / 9)
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=2"), // 4.0275MHz (32.22MHz / 8); optional
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, ""), // 1.007MHz (32.22MHz / 32)
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "clockSel=8") // 2.014MHz (32.22MHz / 16); optional
        // HuC6280 is for controlling them; internal sound isn't used
      }
    );
    SUB_ENTRY(
      _("Data East Dark Seal (extended channel 3)"), { // Dark Seal, Crude Buster, Vapor Trail, etc
        CH(DIV_SYSTEM_YM2151, 1.0f, 0, ""), // 3.580MHz (32.22MHz / 9)
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=2"), // 4.0275MHz (32.22MHz / 8); optional
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, ""), // 1.007MHz (32.22MHz / 32)
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "clockSel=8") // 2.014MHz (32.22MHz / 16); optional
        // HuC6280 is for controlling them; internal sound isn't used
      }
    );
    SUB_ENTRY(
      _("Data East Dark Seal (CSM)"), { // Dark Seal, Crude Buster, Vapor Trail, etc
        CH(DIV_SYSTEM_YM2151, 1.0f, 0, ""), // 3.580MHz (32.22MHz / 9)
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=2"), // 4.0275MHz (32.22MHz / 8); optional
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, ""), // 1.007MHz (32.22MHz / 32)
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "clockSel=8") // 2.014MHz (32.22MHz / 16); optional
        // HuC6280 is for controlling them; internal sound isn't used
      }
    );
    SUB_ENTRY(
      _("Data East Deco 156"), {
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "clockSel=0"), // 1 or 1.007MHz (32.22MHz / 32); various per games
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "clockSel=8") // 1 or 2 or 2.014MHz (32.22MHz / 16); various per games
      }
    );
    SUB_ENTRY(
      _("Data East MLC"), {
        CH(DIV_SYSTEM_YMZ280B, 1.0f, 0, "clockSel=5") // 14MHz
      }
    );

  ENTRY(
    _("Irem"), {}
  );
    SUB_ENTRY(
      _("Irem M72"), {
        CH(DIV_SYSTEM_YM2151, 1.0f, 0, ""),
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0,
          "rate=7812\n"
          "outDepth=7\n"
        )
      }
    );
    SUB_ENTRY(
      _("Irem M92/M107"), {
        CH(DIV_SYSTEM_YM2151, 1.0f, 0, ""),
        CH(DIV_SYSTEM_GA20, 1.0f, 0, "")
      }
    );

  ENTRY(
    _("Jaleco"), {}
  );
    SUB_ENTRY(
      _("Jaleco Ginga NinkyouDen"), {
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "chipType=1"), // 1.79MHz
        CH(DIV_SYSTEM_Y8950, 1.0f, 0, "") // 3.58MHz
      }
    );
    SUB_ENTRY(
      _("Jaleco Ginga NinkyouDen (drums mode)"), {
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "chipType=1"), // 1.79MHz
        CH(DIV_SYSTEM_Y8950_DRUMS, 1.0f, 0, "") // 3.58MHz
      }
    );
    SUB_ENTRY(
      _("Jaleco Mega System 1"), {
        CH(DIV_SYSTEM_YM2151, 1.0f, 0, "clockSel=1"), // 3.5MHz (7MHz / 2)
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "clockSel=2"), // 4MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "clockSel=2") // 4MHz
      }
    );

  ENTRY(
    _("Kaneko"), {}
  );
    SUB_ENTRY(
      _("Kaneko DJ Boy"), {
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=3"), // 3MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, -1.0f, "clockSel=12"), // 1.5MHz, Left output
        CH(DIV_SYSTEM_MSM6295, 1.0f, 1.0f, "clockSel=12"), // 1.5MHz, Right output
      }
    );
    SUB_ENTRY(
      _("Kaneko DJ Boy (extended channel 3)"), {
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=3"), // 3MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, -1.0f, "clockSel=12"), // 1.5MHz, Left output
        CH(DIV_SYSTEM_MSM6295, 1.0f, 1.0f, "clockSel=12") // 1.5MHz, Right output
      }
    );
    SUB_ENTRY(
      _("Kaneko DJ Boy (CSM)"), {
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=3"), // 3MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, -1.0f, "clockSel=12"), // 1.5MHz, Left output
        CH(DIV_SYSTEM_MSM6295, 1.0f, 1.0f, "clockSel=12") // 1.5MHz, Right output
      }
    );
    SUB_ENTRY(
      _("Kaneko Air Buster"), {
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=3"), // 3MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0,
          "clockSel=13\n"
          "rateSel=true\n"
        ) // 3MHz
      }
    );
    SUB_ENTRY(
      _("Kaneko Air Buster (extended channel 3)"), {
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=3"), // 3MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0,
          "clockSel=13\n"
          "rateSel=true\n"
        ) // 3MHz
      }
    );
    SUB_ENTRY(
      _("Kaneko Air Buster (CSM)"), {
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=3"), // 3MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0,
          "clockSel=13\n"
          "rateSel=true\n"
        ) // 3MHz
      }
    );
    SUB_ENTRY(
      _("Kaneko Toybox System"), {
        CH(DIV_SYSTEM_AY8910, 1.0f, 0,
          "clockSel=3\n"
          "chipType=1\n"
        ), // YM2149 2MHz
        CH(DIV_SYSTEM_AY8910, 1.0f, 0,
          "clockSel=3\n"
          "chipType=1\n"
        ), // ^^
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "clockSel=8") // 2MHz
      }
    );
    SUB_ENTRY(
      _("Kaneko Jackie Chan"), {
        CH(DIV_SYSTEM_YMZ280B, 1.0f, 0, "clockSel=3") // 16MHz
      }
    );
    SUB_ENTRY(
      _("Super Kaneko Nova System"), {
        CH(DIV_SYSTEM_YMZ280B, 1.0f, 0, "clockSel=4") // 16.67MHz (33.33MHz / 2)
      }
    );

  ENTRY(
    _("Konami"), {}
  );
    SUB_ENTRY(
      _("Konami Gyruss"), {
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, ""),
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, ""),
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, ""),
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, ""),
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "")
        // additional discrete sound logics
      }
    );
    SUB_ENTRY(
      _("Konami Bubble System"), {
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, ""),
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, ""),
        CH(DIV_SYSTEM_BUBSYS_WSG, 1.0f, 0, "")
        // VLM5030 exists but not used for music at all
      }
    );
    SUB_ENTRY(
      _("Konami MX5000"), {
        CH(DIV_SYSTEM_YM2151, 1.0f, 0, ""), // 3.58MHz
        CH(DIV_SYSTEM_K007232, 1.0f, 0, "")  // ""
      }
    );
    SUB_ENTRY(
      _("Konami Battlantis"), {
        CH(DIV_SYSTEM_OPL2, 1.0f, 0, "clockSel=3"), // 3MHz
        CH(DIV_SYSTEM_OPL2, 1.0f, 0, "clockSel=3")  // ""
      }
    );
    SUB_ENTRY(
      _("Konami Battlantis (drums mode on first OPL2)"), {
        CH(DIV_SYSTEM_OPL2_DRUMS, 1.0f, 0, "clockSel=3"), // 3MHz
        CH(DIV_SYSTEM_OPL2, 1.0f, 0, "clockSel=3") // ""
      }
    );
    SUB_ENTRY(
      _("Konami Battlantis (drums mode on second OPL2)"), {
        CH(DIV_SYSTEM_OPL2, 1.0f, 0, "clockSel=3"), // 3MHz
        CH(DIV_SYSTEM_OPL2_DRUMS, 1.0f, 0, "clockSel=3") // ""
      }
    );
    SUB_ENTRY(
      _("Konami Battlantis (drums mode on both OPL2s)"), {
        CH(DIV_SYSTEM_OPL2_DRUMS, 1.0f, 0, "clockSel=3"), // 3MHz
        CH(DIV_SYSTEM_OPL2_DRUMS, 1.0f, 0, "clockSel=3") // ""
      }
    );
    SUB_ENTRY(
      _("Konami Fast Lane"), {
        CH(DIV_SYSTEM_K007232, 1.0f, 0, ""),  // 3.58MHz
        CH(DIV_SYSTEM_K007232, 1.0f, 0, "")  // ""
      }
    );
    SUB_ENTRY(
      _("Konami Chequered Flag"), {
        CH(DIV_SYSTEM_YM2151, 1.0f, 0, ""), // 3.58MHz
        CH(DIV_SYSTEM_K007232, 1.0f, 0, "stereo=true"),  // ""
        CH(DIV_SYSTEM_K007232, 1.0f, 0, "")  // ""
      }
    );
    SUB_ENTRY(
      _("Konami Haunted Castle"), {
        CH(DIV_SYSTEM_OPL2, 1.0f, 0, ""), // 3.58MHz
        CH(DIV_SYSTEM_SCC, 1.0f, 0, ""),  // ""
        CH(DIV_SYSTEM_K007232, 1.0f, 0, "")  // ""
      }
    );
    SUB_ENTRY(
      _("Konami Haunted Castle (drums mode)"), {
        CH(DIV_SYSTEM_OPL2_DRUMS, 1.0f, 0, ""), // 3.58MHz
        CH(DIV_SYSTEM_SCC, 1.0f, 0, ""), // ""
        CH(DIV_SYSTEM_K007232, 1.0f, 0, "")  // ""
      }
    );
    SUB_ENTRY(
      _("Konami Hot Chase"), {
        CH(DIV_SYSTEM_K007232, 1.0f, 0, "stereo=true"),  // 3.58MHz
        CH(DIV_SYSTEM_K007232, 1.0f, 0, "stereo=true"),  // ""
        CH(DIV_SYSTEM_K007232, 1.0f, 0, "stereo=true")  // ""
      }
    );
    SUB_ENTRY(
      _("Konami S.P.Y."), {
        CH(DIV_SYSTEM_OPL2, 1.0f, 0, ""), // 3.58MHz
        CH(DIV_SYSTEM_K007232, 1.0f, 0, ""),  // ""
        CH(DIV_SYSTEM_K007232, 1.0f, 0, "")  // ""
      }
    );
    SUB_ENTRY(
      _("Konami S.P.Y. (drums mode)"), {
        CH(DIV_SYSTEM_OPL2_DRUMS, 1.0f, 0, ""), // 3.58MHz
        CH(DIV_SYSTEM_K007232, 1.0f, 0, ""), // ""
        CH(DIV_SYSTEM_K007232, 1.0f, 0, "")  // ""
      }
    );
    SUB_ENTRY(
      _("Konami Rollergames"), {
        CH(DIV_SYSTEM_OPL2, 1.0f, 0, ""), // 3.58MHz
        CH(DIV_SYSTEM_K053260, 1.0f, 0, ""),  // ""
      }
    );
    SUB_ENTRY(
      _("Konami Rollergames (drums mode)"), {
        CH(DIV_SYSTEM_OPL2_DRUMS, 1.0f, 0, ""), // 3.58MHz
        CH(DIV_SYSTEM_K053260, 1.0f, 0, ""), // ""
      }
    );
    SUB_ENTRY(
      _("Konami Golfing Greats"), {
        CH(DIV_SYSTEM_K053260, 1.0f, 0, ""), // 3.58MHz
      }
    );
    SUB_ENTRY(
      _("Konami Lightning Fighters"), {
        CH(DIV_SYSTEM_YM2151, 1.0f, 0, ""), // 3.58MHz
        CH(DIV_SYSTEM_K053260, 1.0f, 0, ""), // ""
      }
    );
    SUB_ENTRY(
      _("Konami Over Drive"), {
        CH(DIV_SYSTEM_YM2151, 1.0f, 0, ""), // 3.58MHz
        CH(DIV_SYSTEM_K053260, 1.0f, 0, ""), // ""
        CH(DIV_SYSTEM_K053260, 1.0f, 0, ""), // ""
      }
    );
    SUB_ENTRY(
      _("Konami Asterix"), {
        CH(DIV_SYSTEM_YM2151, 1.0f, 0, "clockSel=2"), // 4MHz
        CH(DIV_SYSTEM_K053260, 1.0f, 0, "clockSel=1"), // ""
      }
    );
    SUB_ENTRY(
      _("Konami Hexion"), {
        CH(DIV_SYSTEM_SCC, 1.0f, 0, "clockSel=2"), // 1.5MHz (3MHz input)
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "clockSel=1")
      }
    );

  ENTRY(
    _("Namco"), {}
  );
    SUB_ENTRY(
      _("Namco (3-channel WSG)"), { // Pac-Man, Galaga, Xevious, etc
        CH(DIV_SYSTEM_NAMCO, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Namco Pole Position"), { // Pole position/2
        CH(DIV_SYSTEM_NAMCO_POLEPOS, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Namco Mappy"), { // Mappy, Super Pac-Man, Libble Rabble, etc
        CH(DIV_SYSTEM_NAMCO_15XX, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Namco Pac-Land"), { // Pac-Land, Baraduke, Sky kid, etc
        CH(DIV_SYSTEM_NAMCO_CUS30, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Namco System 86"), { // without expansion board case; Hopping Mappy, etc
        CH(DIV_SYSTEM_YM2151, 1.0f, 0, ""),
        CH(DIV_SYSTEM_NAMCO_CUS30, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Namco Thunder Ceptor"), {
        CH(DIV_SYSTEM_YM2151, 1.0f, 0, ""),
        CH(DIV_SYSTEM_NAMCO_CUS30, 1.0f, 0, ""),
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0, 
          "rate=8000\n"
          "outDepth=7\n"
        ) // M65C02 software driven, correct sample rate?
      }
    );
    SUB_ENTRY(
      _("Namco System 1"), {
        CH(DIV_SYSTEM_YM2151, 1.0f, 0, ""),
        CH(DIV_SYSTEM_NAMCO_CUS30, 1.0f, 0, ""),
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0,
          "rate=6000\n"
          "outDepth=7\n"
        ), // sample rate verified from https://github.com/mamedev/mame/blob/master/src/devices/sound/n63701x.cpp
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0,
          "rate=6000\n"
          "outDepth=7\n"
        ) // ""
      }
    );
    SUB_ENTRY(
      _("Namco System 2"), {
        CH(DIV_SYSTEM_YM2151, 1.0f, 0, ""),
        CH(DIV_SYSTEM_C140, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Namco NA-1/2"), {
        CH(DIV_SYSTEM_C219, 1.0f, 0, "")
      }
    );

  ENTRY(
    _("Psikyo"), {}
  );
    SUB_ENTRY(
      _("Psikyo 68EC020 hardware with OPL4"), {
        CH(DIV_SYSTEM_OPL4, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Psikyo 68EC020 hardware with OPL4 (drums mode)"), {
        CH(DIV_SYSTEM_OPL4_DRUMS, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Psikyo SH-2 hardware"), {
        CH(DIV_SYSTEM_OPL4, 1.0f, 0, "clockSel=1")
      }
    );
    SUB_ENTRY(
      _("Psikyo SH-2 hardware (drums mode)"), {
        CH(DIV_SYSTEM_OPL4_DRUMS, 1.0f, 0, "clockSel=1")
      }
    );

  ENTRY(
    _("Sega"), {}
  );
    SUB_ENTRY(
      _("Sega Kyugo"), {
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=14"),
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=14")
      }
    );
    SUB_ENTRY(
      _("Sega System 1"), {
        CH(DIV_SYSTEM_SMS, 1.0f, 0,
          "clockSel=2\n"
          "chipType=4\n"
        ), // SN76489A 4MHz
        CH(DIV_SYSTEM_SMS, 1.0f, 0,
          "clockSel=5\n"
          "chipType=4\n"
        ) // SN76489A 2MHz
      }
    );
    SUB_ENTRY(
      _("Sega System E"), {
        CH(DIV_SYSTEM_SMS, 1.0f, 0, ""),
        CH(DIV_SYSTEM_SMS, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Sega System E (with FM expansion)"), {
        CH(DIV_SYSTEM_SMS, 1.0f, 0, ""),
        CH(DIV_SYSTEM_SMS, 1.0f, 0, ""),
        CH(DIV_SYSTEM_OPLL, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Sega System E (with FM expansion in drums mode)"), {
        CH(DIV_SYSTEM_SMS, 1.0f, 0, ""),
        CH(DIV_SYSTEM_SMS, 1.0f, 0, ""),
        CH(DIV_SYSTEM_OPLL_DRUMS, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Sega Hang-On"), {
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=2"), // 4MHz
        CH(DIV_SYSTEM_SEGAPCM, 1.0f, 0, "") // discrete logics, 62.5KHz output rate
      }
    );
    SUB_ENTRY(
      _("Sega Hang-On (extended channel 3)"), {
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=2"), // 4MHz
        CH(DIV_SYSTEM_SEGAPCM, 1.0f, 0, "") // discrete logics, 62.5KHz output rate
      }
    );
    SUB_ENTRY(
      _("Sega Hang-On (CSM)"), {
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=2"), // 4MHz
        CH(DIV_SYSTEM_SEGAPCM, 1.0f, 0, "") // discrete logics, 62.5KHz output rate
      }
    );
    SUB_ENTRY(
      _("Sega OutRun/X Board"), {
        CH(DIV_SYSTEM_YM2151, 1.0f, 0, "clockSel=2"), // 4MHz
        CH(DIV_SYSTEM_SEGAPCM, 1.0f, 0, "") // ASIC, 31.25KHz output rate
      }
    );
    SUB_ENTRY(
      _("Sega System 24"), {
        CH(DIV_SYSTEM_YM2151, 1.0f, 0, "clockSel=2"), // 4MHz
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0,
          "rate=61500\n"
          "outDepth=7\n"
        ) // software controlled, variable rate via configurable timers
      }
    );
    SUB_ENTRY(
      _("Sega System 18"), {
        CH(DIV_SYSTEM_YM2612, 1.0f, 0, 
          "clockSel=2\n"
          "chipType=0\n"
        ), // discrete 8MHz YM3438
        CH(DIV_SYSTEM_YM2612, 1.0f, 0, 
          "clockSel=2\n"
          "chipType=0\n"
        ), // ^^
        CH(DIV_SYSTEM_RF5C68, 1.0f, 0, "clockSel=1") // 10MHz
      }
    );
    SUB_ENTRY(
      _("Sega System 18 (extended channel 3 on first OPN2C)"), {
        CH(DIV_SYSTEM_YM2612_EXT, 1.0f, 0,
          "clockSel=2\n"
          "chipType=0\n"
        ), // discrete 8MHz YM3438
        CH(DIV_SYSTEM_YM2612, 1.0f, 0, 
          "clockSel=2\n"
          "chipType=0\n"
        ), // ^^
        CH(DIV_SYSTEM_RF5C68, 1.0f, 0, "clockSel=1") // 10MHz
      }
    );
    SUB_ENTRY(
      _("Sega System 18 (extended channel 3 on second OPN2C)"), {
        CH(DIV_SYSTEM_YM2612, 1.0f, 0, 
          "clockSel=2\n"
          "chipType=0\n"
        ), // discrete 8MHz YM3438
        CH(DIV_SYSTEM_YM2612_EXT, 1.0f, 0,
          "clockSel=2\n"
          "chipType=0\n"
        ), // ^^
        CH(DIV_SYSTEM_RF5C68, 1.0f, 0, "clockSel=1") // 10MHz
      }
    );
    SUB_ENTRY(
      _("Sega System 18 (extended channel 3 on both OPN2Cs)"), {
        CH(DIV_SYSTEM_YM2612_EXT, 1.0f, 0, 
          "clockSel=2\n"
          "chipType=0\n"
        ), // discrete 8MHz YM3438
        CH(DIV_SYSTEM_YM2612_EXT, 1.0f, 0, 
          "clockSel=2\n"
          "chipType=0\n"
        ), // ^^
        CH(DIV_SYSTEM_RF5C68, 1.0f, 0, "clockSel=1") // 10MHz
      }
    );
    SUB_ENTRY(
      _("Sega System 18 (CSM on first OPN2C)"), {
        CH(DIV_SYSTEM_YM2612_CSM, 1.0f, 0, 
          "clockSel=2\n"
          "chipType=0\n"
        ), // discrete 8MHz YM3438
        CH(DIV_SYSTEM_YM2612, 1.0f, 0, 
          "clockSel=2\n"
          "chipType=0\n"
        ), // ^^
        CH(DIV_SYSTEM_RF5C68, 1.0f, 0, "clockSel=1") // 10MHz
      }
    );
    SUB_ENTRY(
      _("Sega System 18 (CSM on second OPN2C)"), {
        CH(DIV_SYSTEM_YM2612, 1.0f, 0, 
          "clockSel=2\n"
          "chipType=0\n"
        ), // discrete 8MHz YM3438
        CH(DIV_SYSTEM_YM2612_CSM, 1.0f, 0, 
          "clockSel=2\n"
          "chipType=0\n"
        ), // ^^
        CH(DIV_SYSTEM_RF5C68, 1.0f, 0, "clockSel=1") // 10MHz
      }
    );
    SUB_ENTRY(
      _("Sega System 18 (CSM on both OPN2Cs)"), {
        CH(DIV_SYSTEM_YM2612_CSM, 1.0f, 0, 
          "clockSel=2\n"
          "chipType=0\n"
        ), // discrete 8MHz YM3438
        CH(DIV_SYSTEM_YM2612_CSM, 1.0f, 0, 
          "clockSel=2\n"
          "chipType=0\n"
        ), // ^^
        CH(DIV_SYSTEM_RF5C68, 1.0f, 0, "clockSel=1") // 10MHz
      }
    );
    SUB_ENTRY(
      _("Sega System 32"), {
        CH(DIV_SYSTEM_YM2612, 1.0f, 0, 
          "clockSel=4\n"
          "chipType=0\n"
        ), // discrete 8.05MHz YM3438
        CH(DIV_SYSTEM_YM2612, 1.0f, 0, 
          "clockSel=4\n"
          "chipType=0\n"
        ), // ^^
        CH(DIV_SYSTEM_RF5C68, 1.0f, 0, 
          "clockSel=2\n" 
          "chipType=1\n"
        ) // 12.5MHz
      }
    );
    SUB_ENTRY(
      _("Sega System 32 (extended channel 3 on first OPN2C)"), {
        CH(DIV_SYSTEM_YM2612_EXT, 1.0f, 0, 
          "clockSel=4\n"
          "chipType=0\n"
        ), // discrete 8.05MHz YM3438
        CH(DIV_SYSTEM_YM2612, 1.0f, 0, 
          "clockSel=4\n"
          "chipType=0\n"
        ), // ^^
        CH(DIV_SYSTEM_RF5C68, 1.0f, 0, 
          "clockSel=2\n" 
          "chipType=1\n"
        ) // 12.5MHz
      }
    );
    SUB_ENTRY(
      _("Sega System 32 (extended channel 3 on second OPN2C)"), {
        CH(DIV_SYSTEM_YM2612, 1.0f, 0, 
          "clockSel=4\n"
          "chipType=0\n"
        ), // discrete 8.05MHz YM3438
        CH(DIV_SYSTEM_YM2612_EXT, 1.0f, 0, 
          "clockSel=4\n"
          "chipType=0\n"
        ), // ^^
        CH(DIV_SYSTEM_RF5C68, 1.0f, 0, 
          "clockSel=2\n" 
          "chipType=1\n"
        ) // 12.5MHz
      }
    );
    SUB_ENTRY(
      _("Sega System 32 (extended channel 3 on both OPN2Cs)"), {
        CH(DIV_SYSTEM_YM2612_EXT, 1.0f, 0, 
          "clockSel=4\n"
          "chipType=0\n"
        ), // discrete 8.05MHz YM3438
        CH(DIV_SYSTEM_YM2612_EXT, 1.0f, 0, 
          "clockSel=4\n"
          "chipType=0\n"
        ), // ^^
        CH(DIV_SYSTEM_RF5C68, 1.0f, 0, 
          "clockSel=2\n" 
          "chipType=1\n"
        ) // 12.5MHz
      }
    );
    SUB_ENTRY(
      _("Sega System 32 (CSM on first OPN2C)"), {
        CH(DIV_SYSTEM_YM2612_CSM, 1.0f, 0, 
          "clockSel=4\n"
          "chipType=0\n"
        ), // discrete 8.05MHz YM3438
        CH(DIV_SYSTEM_YM2612, 1.0f, 0, 
          "clockSel=4\n"
          "chipType=0\n"
        ), // ^^
        CH(DIV_SYSTEM_RF5C68, 1.0f, 0, 
          "clockSel=2\n" 
          "chipType=1\n"
        ) // 12.5MHz
      }
    );
    SUB_ENTRY(
      _("Sega System 32 (CSM on second OPN2C)"), {
        CH(DIV_SYSTEM_YM2612, 1.0f, 0, 
          "clockSel=4\n"
          "chipType=0\n"
        ), // discrete 8.05MHz YM3438
        CH(DIV_SYSTEM_YM2612_CSM, 1.0f, 0, 
          "clockSel=4\n"
          "chipType=0\n"
        ), // ^^
        CH(DIV_SYSTEM_RF5C68, 1.0f, 0, 
          "clockSel=2\n" 
          "chipType=1\n"
        ) // 12.5MHz
      }
    );
    SUB_ENTRY(
      _("Sega System 32 (CSM on both OPN2Cs)"), {
        CH(DIV_SYSTEM_YM2612_CSM, 1.0f, 0, 
          "clockSel=4\n"
          "chipType=0\n"
        ), // discrete 8.05MHz YM3438
        CH(DIV_SYSTEM_YM2612_CSM, 1.0f, 0, 
          "clockSel=4\n"
          "chipType=0\n"
        ), // ^^
        CH(DIV_SYSTEM_RF5C68, 1.0f, 0, 
          "clockSel=2\n" 
          "chipType=1\n"
        ) // 12.5MHz
      }
    );
    SUB_ENTRY(
      _("Sega System Multi 32"), {
        CH(DIV_SYSTEM_YM2612, 1.0f, 0, 
          "clockSel=2\n"
          "chipType=0\n"
        ), // discrete 8MHz YM3438
        CH(DIV_SYSTEM_MULTIPCM, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Sega System Multi 32 (extended channel 3)"), {
        CH(DIV_SYSTEM_YM2612_EXT, 1.0f, 0, 
          "clockSel=2\n"
          "chipType=0\n"
        ), // discrete 8MHz YM3438
        CH(DIV_SYSTEM_MULTIPCM, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Sega System Multi 32 (CSM)"), {
        CH(DIV_SYSTEM_YM2612_CSM, 1.0f, 0, 
          "clockSel=2\n"
          "chipType=0\n"
        ), // discrete 8MHz YM3438
        CH(DIV_SYSTEM_MULTIPCM, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Sega Model 1/2"), {
        CH(DIV_SYSTEM_YM2612, 1.0f, 0, 
          "clockSel=2\n"
          "chipType=0\n"
        ), // discrete 8MHz YM3438
        CH(DIV_SYSTEM_MULTIPCM, 1.0f, 0, ""),
        CH(DIV_SYSTEM_MULTIPCM, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Sega Model 1/2 (extended channel 3)"), {
        CH(DIV_SYSTEM_YM2612_EXT, 1.0f, 0, 
          "clockSel=2\n"
          "chipType=0\n"
        ), // discrete 8MHz YM3438
        CH(DIV_SYSTEM_MULTIPCM, 1.0f, 0, ""),
        CH(DIV_SYSTEM_MULTIPCM, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Sega Model 1/2 (CSM)"), {
        CH(DIV_SYSTEM_YM2612_CSM, 1.0f, 0, 
          "clockSel=2\n"
          "chipType=0\n"
        ), // discrete 8MHz YM3438
        CH(DIV_SYSTEM_MULTIPCM, 1.0f, 0, ""),
        CH(DIV_SYSTEM_MULTIPCM, 1.0f, 0, "")
      }
    );

  ENTRY(
    _("Seta"), {}
  );
    SUB_ENTRY(
      _("Seta 1"), {
        CH(DIV_SYSTEM_X1_010, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Seta 1 + FM add-on"), {
        CH(DIV_SYSTEM_X1_010, 1.0f, 0, ""),
        CH(DIV_SYSTEM_YM2612, 1.0f, 0, 
          "clockSel=2\n"
          "chipType=0\n"
        ) // Discrete YM3438
      }
    );
    SUB_ENTRY(
      _("Seta 1 + FM add-on (extended channel 3)"), {
        CH(DIV_SYSTEM_X1_010, 1.0f, 0, ""),
        CH(DIV_SYSTEM_YM2612_EXT, 1.0f, 0, 
          "clockSel=2\n"
          "chipType=0\n"
        ) // Discrete YM3438
      }
    );
    SUB_ENTRY(
      _("Seta 1 + FM add-on (CSM)"), {
        CH(DIV_SYSTEM_X1_010, 1.0f, 0, ""),
        CH(DIV_SYSTEM_YM2612_CSM, 1.0f, 0, 
          "clockSel=2\n"
          "chipType=0\n"
        ) // Discrete YM3438
      }
    );
    SUB_ENTRY(
      _("Seta 2"), {
        CH(DIV_SYSTEM_X1_010, 1.0f, 0,
           "clockSel=1\n"
           "isBanked=true\n"
        )
      }
    );
    SUB_ENTRY(
      _("Sammy/Seta/Visco SSV"), {
        CH(DIV_SYSTEM_ES5506, 1.0f, 0, "channels=31")
      }
    );

  ENTRY(
    _("SNK"), {}
  );
    SUB_ENTRY(
      _("Neo Geo MVS"), {
        CH(DIV_SYSTEM_YM2610_FULL, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Neo Geo MVS (extended channel 2)"), {
        CH(DIV_SYSTEM_YM2610_FULL_EXT, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Neo Geo MVS (CSM)"), {
        CH(DIV_SYSTEM_YM2610_CSM, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("SNK Ikari Warriors"), {
        CH(DIV_SYSTEM_OPL, 1.0f, 0, "clockSel=2"),
        CH(DIV_SYSTEM_OPL, 1.0f, 0, "clockSel=2")
      }
    );
    SUB_ENTRY(
      _("SNK Ikari Warriors (drums mode on first OPL)"), {
        CH(DIV_SYSTEM_OPL_DRUMS, 1.0f, 0, "clockSel=2"),
        CH(DIV_SYSTEM_OPL, 1.0f, 0, "clockSel=2")
      }
    );
    SUB_ENTRY(
      _("SNK Ikari Warriors (drums mode on second OPL)"), {
        CH(DIV_SYSTEM_OPL, 1.0f, 0, "clockSel=2"),
        CH(DIV_SYSTEM_OPL_DRUMS, 1.0f, 0, "clockSel=2")
      }
    );
    SUB_ENTRY(
      _("SNK Ikari Warriors (drums mode on both OPLs)"), {
        CH(DIV_SYSTEM_OPL_DRUMS, 1.0f, 0, "clockSel=2"),
        CH(DIV_SYSTEM_OPL_DRUMS, 1.0f, 0, "clockSel=2")
      }
    );
    SUB_ENTRY(
      _("SNK Triple Z80"), {
        CH(DIV_SYSTEM_Y8950, 1.0f, 0, "clockSel=2"),
        CH(DIV_SYSTEM_OPL, 1.0f, 0, "clockSel=2")
      }
    );
    SUB_ENTRY(
      _("SNK Triple Z80 (drums mode on Y8950)"), {
        CH(DIV_SYSTEM_Y8950_DRUMS, 1.0f, 0, "clockSel=2"),
        CH(DIV_SYSTEM_OPL, 1.0f, 0, "clockSel=2")
      }
    );
    SUB_ENTRY(
      _("SNK Triple Z80 (drums mode on OPL)"), {
        CH(DIV_SYSTEM_Y8950, 1.0f, 0, "clockSel=2"),
        CH(DIV_SYSTEM_OPL_DRUMS, 1.0f, 0, "clockSel=2")
      }
    );
    SUB_ENTRY(
      _("SNK Triple Z80 (drums mode on Y8950 and OPL)"), {
        CH(DIV_SYSTEM_Y8950_DRUMS, 1.0f, 0, "clockSel=2"),
        CH(DIV_SYSTEM_OPL_DRUMS, 1.0f, 0, "clockSel=2")
      }
    );
    SUB_ENTRY(
      _("SNK Chopper I"), {
        CH(DIV_SYSTEM_Y8950, 1.0f, 0, "clockSel=2"),
        CH(DIV_SYSTEM_OPL2, 1.0f, 0, "clockSel=2")
      }
    );
    SUB_ENTRY(
      _("SNK Chopper I (drums mode on Y8950)"), {
        CH(DIV_SYSTEM_Y8950_DRUMS, 1.0f, 0, "clockSel=2"),
        CH(DIV_SYSTEM_OPL2, 1.0f, 0, "clockSel=2")
      }
    );
    SUB_ENTRY(
      _("SNK Chopper I (drums mode on OPL2)"), {
        CH(DIV_SYSTEM_Y8950, 1.0f, 0, "clockSel=2"),
        CH(DIV_SYSTEM_OPL2_DRUMS, 1.0f, 0, "clockSel=2")
      }
    );
    SUB_ENTRY(
      _("SNK Chopper I (drums mode on Y8950 and OPL2)"), {
        CH(DIV_SYSTEM_Y8950_DRUMS, 1.0f, 0, "clockSel=2"),
        CH(DIV_SYSTEM_OPL2_DRUMS, 1.0f, 0, "clockSel=2")
      }
    );
    SUB_ENTRY(
      _("SNK Touchdown Fever"), {
        CH(DIV_SYSTEM_OPL, 1.0f, 0, "clockSel=2"),
        CH(DIV_SYSTEM_Y8950, 1.0f, 0, "clockSel=2")
      }
    );
    SUB_ENTRY(
      _("SNK Touchdown Fever (drums mode on OPL)"), {
        CH(DIV_SYSTEM_OPL_DRUMS, 1.0f, 0, "clockSel=2"),
        CH(DIV_SYSTEM_Y8950, 1.0f, 0, "clockSel=2")
      }
    );
    SUB_ENTRY(
      _("SNK Touchdown Fever (drums mode on Y8950)"), {
        CH(DIV_SYSTEM_OPL, 1.0f, 0, "clockSel=2"),
        CH(DIV_SYSTEM_Y8950_DRUMS, 1.0f, 0, "clockSel=2")
      }
    );
    SUB_ENTRY(
      _("SNK Touchdown Fever (drums mode on OPL and Y8950)"), {
        CH(DIV_SYSTEM_OPL_DRUMS, 1.0f, 0, "clockSel=2"),
        CH(DIV_SYSTEM_Y8950_DRUMS, 1.0f, 0, "clockSel=2")
      }
    );

  ENTRY(
    _("Sunsoft"), {}
  );
    SUB_ENTRY(
      _("Sunsoft Shanghai 3"), {
        CH(DIV_SYSTEM_AY8910, 1.0f, 0,
          "clockSel=4\n"
          "chipType=1\n"
        ), // YM2149 1.5MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "clockSel=1") // 1.056MHz
      }
    );
    SUB_ENTRY(
      _("Sunsoft Arcade"), {
        CH(DIV_SYSTEM_YM2612, 1.0f, 0, 
          "clockSel=2\n"
          "chipType=0\n"
        ), // discrete YM3438 8MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "clockSel=1") // 1.056MHz
      }
    );
    SUB_ENTRY(
      _("Sunsoft Arcade (extended channel 3)"), {
        CH(DIV_SYSTEM_YM2612_EXT, 1.0f, 0, 
          "clockSel=2\n"
          "chipType=0\n"
        ), // discrete YM3438 8MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "clockSel=1") // 1.056MHz
      }
    );
    SUB_ENTRY(
      _("Sunsoft Arcade (CSM)"), {
        CH(DIV_SYSTEM_YM2612_CSM, 1.0f, 0, 
          "clockSel=2\n"
          "chipType=0\n"
        ), // discrete YM3438 8MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "clockSel=1") // 1.056MHz
      }
    );

  ENTRY(
    _("Taito"), {}
  );
    SUB_ENTRY(
      _("Taito Arcade"), {
        CH(DIV_SYSTEM_YM2610B, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Taito Arcade (extended channel 3)"), {
        CH(DIV_SYSTEM_YM2610B_EXT, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Taito Arcade (CSM)"), {
        CH(DIV_SYSTEM_YM2610B_CSM, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Taito Metal Soldier Isaac II"), {
        CH(DIV_SYSTEM_MSM5232, 1.0f, 0, ""),
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=3"),
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=3")
      }
    );
    SUB_ENTRY(
      _("Taito The Fairyland Story"), {
        CH(DIV_SYSTEM_MSM5232, 1.0f, 0, ""),
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, 
           "clockSel=3\n"
           "chipType=1\n"
        ),
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0, 
           "rate=11025\n" 
           "outDepth=7\n"
        ) // don't know what the actual sample rate is
      }
    );
    SUB_ENTRY(
      _("Taito Wyvern F-0"), {
        CH(DIV_SYSTEM_MSM5232, 1.0f, 0, ""),
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, 
           "clockSel=3\n"
           "chipType=1\n"
        ),
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, 
           "clockSel=3\n"
           "chipType=1\n"
        ),
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0, 
           "rate=11025\n" 
           "outDepth=7\n"
        ) // don't know what the actual sample rate is
      }
    );

  ENTRY(
    _("Tecmo"), {}
  );
    SUB_ENTRY(
      _("Tecmo Ninja Gaiden"), { // Ninja Gaiden, Raiga, etc
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=2"), // 4MHz
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=2"), // 4MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "") // 1MHz
      }
    );
    SUB_ENTRY(
      _("Tecmo Ninja Gaiden (extended channel 3 on first OPN)"), { // Ninja Gaiden, Raiga, etc
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=2"), // 4MHz
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=2"), // 4MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "") // 1MHz
      }
    );
    SUB_ENTRY(
      _("Tecmo Ninja Gaiden (extended channel 3 on second OPN)"), { // Ninja Gaiden, Raiga, etc
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=2"), // 4MHz
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=2"), // 4MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "") // 1MHz
      }
    );
    SUB_ENTRY(
      _("Tecmo Ninja Gaiden (extended channel 3 on both OPNs)"), { // Ninja Gaiden, Raiga, etc
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=2"), // 4MHz
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=2"), // 4MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "") // 1MHz
      }
    );
    SUB_ENTRY(
      _("Tecmo Ninja Gaiden (CSM on first OPN)"), { // Ninja Gaiden, Raiga, etc
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=2"), // 4MHz
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=2"), // 4MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "") // 1MHz
      }
    );
    SUB_ENTRY(
      _("Tecmo Ninja Gaiden (CSM on second OPN)"), { // Ninja Gaiden, Raiga, etc
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=2"), // 4MHz
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=2"), // 4MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "") // 1MHz
      }
    );
    SUB_ENTRY(
      _("Tecmo Ninja Gaiden (CSM on both OPNs)"), { // Ninja Gaiden, Raiga, etc
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=2"), // 4MHz
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=2"), // 4MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "") // 1MHz
      }
    );
    SUB_ENTRY(
      _("Tecmo System"), {
        CH(DIV_SYSTEM_OPL3, 1.0f, 0, ""),
        CH(DIV_SYSTEM_YMZ280B, 1.0f, 0, ""),
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "clockSel=8") // 2MHz
      }
    );
    SUB_ENTRY(
      _("Tecmo System (drums mode)"), {
        CH(DIV_SYSTEM_OPL3_DRUMS, 1.0f, 0, ""),
        CH(DIV_SYSTEM_YMZ280B, 1.0f, 0, ""),
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "clockSel=8") // 2MHz
      }
    );
    SUB_ENTRY(
      _("Seibu Kaihatsu Raiden"), { // Raiden, Seibu Cup Soccer, Zero Team, etc
        CH(DIV_SYSTEM_OPL2, 1.0f, 0, ""), // YM3812 3.58MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "") // 1 or 1.023MHz (28.636363MHz / 28); various per games
      }
    );
    SUB_ENTRY(
      _("Seibu Kaihatsu Raiden (drums mode)"), { // Raiden, Seibu Cup Soccer, Zero Team, etc
        CH(DIV_SYSTEM_OPL2_DRUMS, 1.0f, 0, ""), // YM3812 3.58MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "") // 1 or 1.023MHz (28.636363MHz / 28); various per games
      }
    );

  ENTRY(
    _("Other"), {}
  );

  // UNSORTED
    SUB_ENTRY(
      _("Bally Midway MCR"), {
        // SSIO sound board
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=3"), // 2MHz
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=3") // 2MHz
        // additional sound boards, mostly software controlled DAC
      }
    );
    SUB_ENTRY(
      _("Williams/Midway Y/T unit w/ADPCM sound board"), {
        // ADPCM sound board
        CH(DIV_SYSTEM_YM2151, 1.0f, 0, ""),
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0,
          "rate=15625\n"
          "outDepth=7\n"
        ), // variable via OPM timer?
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("NMK 16-bit Arcade"), {
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=5"), // 1.5MHz; optional
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0,
          "clockSel=2\n"
          "rateSel=true\n"
        ), // 4MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0,
          "clockSel=2\n"
          "rateSel=true\n"
        ) // ^^
      }
    );
    SUB_ENTRY(
      _("NMK 16-bit Arcade (extended channel 3)"), {
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=5"), // 1.5MHz; optional
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0,
          "clockSel=2\n"
          "rateSel=true\n"
        ), // 4MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0,
          "clockSel=2\n"
          "rateSel=true\n"
        ) // ^^
      }
    );
    SUB_ENTRY(
      _("NMK 16-bit Arcade (CSM)"), {
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=5"), // 1.5MHz; optional
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0,
          "clockSel=2\n"
          "rateSel=true\n"
        ), // 4MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0,
          "clockSel=2\n"
          "rateSel=true\n"
        ) // ^^
      }
    );
    SUB_ENTRY(
      _("NMK 16-bit Arcade (w/NMK112 bankswitching)"), {
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=5"), // 1.5MHz; optional
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0,
          "clockSel=2\n"
          "rateSel=true\n"
          "isBanked=true\n"
        ), // 4MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0,
          "clockSel=2\n"
          "rateSel=true\n"
          "isBanked=true\n"
        ) // ^^
      }
    );
    SUB_ENTRY(
      _("NMK 16-bit Arcade (w/NMK112 bankswitching, extended channel 3)"), {
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=5"), // 1.5MHz; optional
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0,
          "clockSel=2\n"
          "rateSel=true\n"
          "isBanked=true\n"
        ), // 4MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0,
          "clockSel=2\n"
          "rateSel=true\n"
          "isBanked=true\n"
        ) // ^^
      }
    );
    SUB_ENTRY(
      _("NMK 16-bit Arcade (w/NMK112 bankswitching, CSM)"), {
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=5"), // 1.5MHz; optional
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0,
          "clockSel=2\n"
          "rateSel=true\n"
          "isBanked=true\n"
        ), // 4MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0,
          "clockSel=2\n"
          "rateSel=true\n"
          "isBanked=true\n"
        ) // ^^
      }
    );
    SUB_ENTRY(
      _("Atlus Power Instinct 2"), {
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=2"), // 4MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0,
          "clockSel=13\n"
          "rateSel=true\n"
          "isBanked=true\n"
        ), // 3MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0,
          "clockSel=13\n"
          "rateSel=true\n"
          "isBanked=true\n"
        ) // ^^
      }
    );
    SUB_ENTRY(
      _("Atlus Power Instinct 2 (extended channel 3)"), {
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=2"), // 4MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0,
          "clockSel=13\n"
          "rateSel=true\n"
          "isBanked=true\n"
        ), // 3MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0,
          "clockSel=13\n"
          "rateSel=true\n"
          "isBanked=true\n"
        ) // ^^
      }
    );
    SUB_ENTRY(
      _("Atlus Power Instinct 2 (CSM)"), {
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=2"), // 4MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0,
          "clockSel=13\n"
          "rateSel=true\n"
          "isBanked=true\n"
        ), // 3MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0,
          "clockSel=13\n"
          "rateSel=true\n"
          "isBanked=true\n"
        ) // ^^
      }
    );
    SUB_ENTRY(
      _("Raizing/Eighting Battle Garegga"), {
        CH(DIV_SYSTEM_YM2151, 1.0f, 0, "clockSel=2"), // 4MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0,
          "clockSel=8\n"
          "isBanked=true\n"
        ) // 2MHz
      }
    );
    SUB_ENTRY(
      _("Raizing/Eighting Batrider"), {
        CH(DIV_SYSTEM_YM2151, 1.0f, 0, "clockSel=2"), // 4MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0,
          "clockSel=15\n"
          "isBanked=true\n"
        ), // 3.2MHz
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0,
          "clockSel=15\n"
          "rateSel=true\n"
          "isBanked=true\n"
        ) // 3.2MHz
      }
    );
    SUB_ENTRY(
      _("Nichibutsu Mag Max"), {
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=13"),
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=13"),
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=13")
      }
    );
    SUB_ENTRY(
      _("Cave 68000"), {
        CH(DIV_SYSTEM_YMZ280B, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Coreland Cyber Tank"), {
        CH(DIV_SYSTEM_Y8950, 1.0f, -1.0f, ""), // 3.58MHz, Left output
        CH(DIV_SYSTEM_Y8950, 1.0f, 1.0f, "") // 3.58MHz, Right output
      }
    );
    SUB_ENTRY(
      _("Coreland Cyber Tank (drums mode)"), {
        CH(DIV_SYSTEM_Y8950_DRUMS, 1.0f, -1.0f, ""), // 3.58MHz, Left output
        CH(DIV_SYSTEM_Y8950_DRUMS, 1.0f, 1.0f, "") // 3.58MHz, Right output
      }
    );
    SUB_ENTRY(
      _("ICE Skimaxx"), {
        CH(DIV_SYSTEM_MSM6295, 1.0f, -1.0f,
          "clockSel=2\n"
          "rateSel=true\n"
        ), // 4MHz, Left output
        CH(DIV_SYSTEM_MSM6295, 1.0f, 1.0f,
          "clockSel=2\n"
          "rateSel=true\n"
        ), // 4MHz, Right output
        CH(DIV_SYSTEM_MSM6295, 1.0f, -1.0f, "clockSel=8"), // 2MHz, Left output
        CH(DIV_SYSTEM_MSM6295, 1.0f, 1.0f, "clockSel=8") // 2MHz, Right output
      }
    );
    SUB_ENTRY(
      _("Toaplan 1"), {
        CH(DIV_SYSTEM_OPL2, 1.0f, 0, "clockSel=5") // 3.5MHz
      }
    );
    SUB_ENTRY(
      _("Toaplan 1 (drums mode)"), {
        CH(DIV_SYSTEM_OPL2_DRUMS, 1.0f, 0, "clockSel=5") // 3.5MHz
      }
    );
    SUB_ENTRY(
      _("Dynax/Nakanihon 3rd generation hardware"), {
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, ""), // AY or YM, optional - 1.79MHz or 3.58MHz; various per game
        CH(DIV_SYSTEM_OPLL, 1.0f, 0, ""),
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "clockSel=6") // 1.023MHz mostly
      }
    );
    SUB_ENTRY(
      _("Dynax/Nakanihon 3rd generation hardware (drums mode)"), {
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, ""), // AY or YM, optional - 1.79MHz or 3.58MHz; various per game
        CH(DIV_SYSTEM_OPLL_DRUMS, 1.0f, 0, ""),
        CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "clockSel=6") // 1.023MHz mostly
      }
    );
    SUB_ENTRY(
      _("Dynax/Nakanihon Real Break"), {
        CH(DIV_SYSTEM_OPLL, 1.0f, 0, ""),
        CH(DIV_SYSTEM_YMZ280B, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Dynax/Nakanihon Real Break (drums mode)"), {
        CH(DIV_SYSTEM_OPLL_DRUMS, 1.0f, 0, ""),
        CH(DIV_SYSTEM_YMZ280B, 1.0f, 0, "")
      }
    );
  CATEGORY_END;
}
