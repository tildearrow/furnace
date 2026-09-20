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

void initSystemPresetsComputers(std::vector<FurnaceGUISysCategory>& sysCategories) {
  CATEGORY_BEGIN(_("Computers"),_("let's get to work on chiptune today."));
  ENTRY(
    _("Commodore PET"), {
      CH(DIV_SYSTEM_PET, 1.0f, 0, "")
    },
    "tickRate=50"
  );
  ENTRY(
    _("Commodore VIC-20"), {
      CH(DIV_SYSTEM_VIC20, 1.0f, 0, "clockSel=1")
    },
    "tickRate=50"
  );
  ENTRY(
    _("Commodore Plus/4"), {
      CH(DIV_SYSTEM_TED, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Commodore 64 (SID)"), {}
  );
    SUB_ENTRY(
      _("Commodore 64 (6581 SID)"), {
        CH(DIV_SYSTEM_C64_6581, 1.0f, 0, "clockSel=1")
      },
      "tickRate=50.1245421"
    );
      SUB_SUB_ENTRY(
        _("Commodore 64 (6581 SID + Sound Expander)"), {
          CH(DIV_SYSTEM_C64_6581, 1.0f, 0, "clockSel=1"),
          CH(DIV_SYSTEM_OPL, 1.0f, 0, "")
        },
        "tickRate=50.1245421"
      );
      SUB_SUB_ENTRY(
        _("Commodore 64 (6581 SID + Sound Expander in drums mode)"), {
          CH(DIV_SYSTEM_C64_6581, 1.0f, 0, "clockSel=1"),
          CH(DIV_SYSTEM_OPL_DRUMS, 1.0f, 0, "")
        },
        "tickRate=50.1245421"
      );
      SUB_SUB_ENTRY(
        _("Commodore 64 (6581 SID + FM-YAM)"), {
          CH(DIV_SYSTEM_C64_6581, 1.0f, 0, "clockSel=1"),
          CH(DIV_SYSTEM_OPL2, 1.0f, 0, "")
        },
        "tickRate=50.1245421"
      );
      SUB_SUB_ENTRY(
        _("Commodore 64 (6581 SID + FM-YAM in drums mode)"), {
          CH(DIV_SYSTEM_C64_6581, 1.0f, 0, "clockSel=1"),
          CH(DIV_SYSTEM_OPL2_DRUMS, 1.0f, 0, "")
        },
        "tickRate=50.1245421"
      );
    SUB_ENTRY(
      _("Commodore 64 (8580 SID)"), {
        CH(DIV_SYSTEM_C64_8580, 1.0f, 0, "clockSel=1")
      },
      "tickRate=50.1245421"
    );
      SUB_SUB_ENTRY(
        _("Commodore 64 (8580 SID + Sound Expander)"), {
          CH(DIV_SYSTEM_C64_8580, 1.0f, 0, "clockSel=1"),
          CH(DIV_SYSTEM_OPL, 1.0f, 0, "")
        },
        "tickRate=50.1245421"
      );
      SUB_SUB_ENTRY(
        _("Commodore 64 (8580 SID + Sound Expander in drums mode)"), {
          CH(DIV_SYSTEM_C64_8580, 1.0f, 0, "clockSel=1"),
          CH(DIV_SYSTEM_OPL_DRUMS, 1.0f, 0, "")
        },
        "tickRate=50.1245421"
      );
      SUB_SUB_ENTRY(
        _("Commodore 64 (8580 SID + FM-YAM)"), {
          CH(DIV_SYSTEM_C64_8580, 1.0f, 0, "clockSel=1"),
          CH(DIV_SYSTEM_OPL2, 1.0f, 0, "")
        },
        "tickRate=50.1245421"
      );
      SUB_SUB_ENTRY(
        _("Commodore 64 (8580 SID + FM-YAM in drums mode)"), {
          CH(DIV_SYSTEM_C64_8580, 1.0f, 0, "clockSel=1"),
          CH(DIV_SYSTEM_OPL2_DRUMS, 1.0f, 0, "")
        },
        "tickRate=50.1245421"
      );
    SUB_ENTRY(
      _("Commodore 64 (6581 SID) with software PCM"), {
        CH(DIV_SYSTEM_C64_PCM, 1.0f, 0, "clockSel=1")
      },
      "tickRate=50.1245421"
    );
  ENTRY(
    _("Amiga"), {
      CH(DIV_SYSTEM_AMIGA, 1.0f, 0, "clockSel=1")
    },
    "tickRate=50"
  );
  ENTRY(
    _("MSX"), {
      CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=0\nchipType=1")
    }
  );
    SUB_ENTRY(
      _("MSX + SFG-01"), {
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "chipType=1"),
        CH(DIV_SYSTEM_YM2151, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("MSX + MSX-AUDIO"), {
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "chipType=1"),
        CH(DIV_SYSTEM_Y8950, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("MSX + MSX-AUDIO (drums mode)"), {
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "chipType=1"),
        CH(DIV_SYSTEM_Y8950_DRUMS, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("MSX + MSX-MUSIC"), {
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "chipType=1"),
        CH(DIV_SYSTEM_OPLL, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("MSX + MSX-MUSIC (drums mode)"), {
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "chipType=1"),
        CH(DIV_SYSTEM_OPLL_DRUMS, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("MSX + Darky"), {
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "chipType=1"),
        CH(DIV_SYSTEM_AY8930, 1.0f, 0,
          "clockSel=11\n"
          "halfClock=true\n"
        ), // 3.58MHz
        CH(DIV_SYSTEM_AY8930, 1.0f, 0,
          "clockSel=11\n"
          "halfClock=true\n"
        ) // 3.58MHz or 3.6MHz selectable via register
        // per-channel mixer (soft panning, post processing) isn't emulated at all
      }
    );
      SUB_ENTRY(
      _("MSX + Playsoniq"), {
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "chipType=1"),
        CH(DIV_SYSTEM_SMS, 1.0f, 0, ""), // Sega VDP
        CH(DIV_SYSTEM_C64_8580, 1.0f, 0, ""),
        CH(DIV_SYSTEM_SCC_PLUS, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("MSX + SCC"), {
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "chipType=1"),
        CH(DIV_SYSTEM_SCC, 0.67f, 0, "")
      }
    );
    SUB_ENTRY(
      _("MSX + SCC+"), {
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "chipType=1"),
        CH(DIV_SYSTEM_SCC_PLUS, 0.67f, 0, "")
      }
    );
    SUB_ENTRY(
      _("MSX + Neotron"), {
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "chipType=1"),
        CH(DIV_SYSTEM_YM2610_FULL, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("MSX + Neotron (extended channel 2)"), {
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "chipType=1"),
        CH(DIV_SYSTEM_YM2610_FULL_EXT, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("MSX + Neotron (CSM)"), {
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "chipType=1"),
        CH(DIV_SYSTEM_YM2610_CSM, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("MSX + Neotron (with YM2610B)"), {
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "chipType=1"),
        CH(DIV_SYSTEM_YM2610B, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("MSX + Neotron (with YM2610B; extended channel 3)"), {
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "chipType=1"),
        CH(DIV_SYSTEM_YM2610B_EXT, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("MSX + Neotron (with YM2610B; CSM)"), {
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "chipType=1"),
        CH(DIV_SYSTEM_YM2610B_CSM, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("MSX + SIMPL"), {
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "chipType=1"),
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0,
          "rate=55930\n"
          "outDepth=7\n"
        ) // variable rate, Mono DAC
      }
    );
    SUB_ENTRY(
      "MSX + MoonSound", {
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "chipType=1"),
        CH(DIV_SYSTEM_OPL4, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      "MSX + MoonSound (drums mode)", {
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "chipType=1"),
        CH(DIV_SYSTEM_OPL4_DRUMS, 1.0f, 0, "")
      }
    );
  ENTRY(
      _("NEC PC-6001"), {
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "customClock=3993600")
      }
    );
  ENTRY(
    _("NEC PC-88"), {}
  );
    SUB_ENTRY(
      _("NEC PC-88 (with PC-8801-10)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=15"), // external
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=15") // ""
      }
    );
    SUB_ENTRY(
      _("NEC PC-88 (with PC-8801-11)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2203, 1.0f, 0,
          "clockSel=4\n"
          "ssgVol=141\n"
        ) // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-88 (with PC-8801-11; extended channel 3)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0,
          "clockSel=4\n"
          "ssgVol=141\n"
        ) // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-88 (with PC-8801-11; CSM)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0,
          "clockSel=4\n"
          "ssgVol=141\n"
        ) // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-88 (with PC-8801-23)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2608, 1.0f, 0,
          "clockSel=1\n"
          "ssgVol=149\n"
        ) // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-88 (with PC-8801-23; extended channel 3)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2608_EXT, 1.0f, 0,
          "clockSel=1\n"
          "ssgVol=149\n"
        ) // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-88 (with PC-8801-23; CSM)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2608_CSM, 1.0f, 0,
          "clockSel=1\n"
          "ssgVol=149\n"
        ) // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-88 (with HMB-20 HIBIKI-8800)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2151, 1.0f, 0, "clockSel=2") // external; 4.0000MHz
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801mk2SR (with PC-8801-10)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2203, 1.0f, 0,
          "clockSel=4\n"
          "ssgVol=154\n"
        ), // internal
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=15"), // external
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=15") // ""
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801mk2SR (with PC-8801-10; extended channel 3)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0,
          "clockSel=4\n"
          "ssgVol=154\n"
        ), // internal
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=15"), // external
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=15") // ""
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801mk2SR (with PC-8801-10; CSM)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0,
          "clockSel=4\n"
          "ssgVol=154\n"
        ), // internal
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=15"), // external
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=15") // ""
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801mk2SR (with PC-8801-11)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2203, 1.0f, 0,
          "clockSel=4\n"
          "ssgVol=154\n"
        ), // internal
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=4") // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801mk2SR (with PC-8801-11; extended channel 3 on internal OPN)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0,
          "clockSel=4\n"
          "ssgVol=154\n"
        ), // internal
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=4") // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801mk2SR (with PC-8801-11; extended channel 3 on external OPN)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2203, 1.0f, 0,
          "clockSel=4\n"
          "ssgVol=154\n"
        ), // internal
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=4") // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801mk2SR (with PC-8801-11; extended channel 3 on both OPNs)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0,
          "clockSel=4\n"
          "ssgVol=154\n"
        ), // internal
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=4") // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801mk2SR (with PC-8801-11; CSM on internal OPN)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0,
          "clockSel=4\n"
          "ssgVol=154\n"
        ), // internal
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=4") // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801mk2SR (with PC-8801-11; CSM on external OPN)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2203, 1.0f, 0,
          "clockSel=4\n"
          "ssgVol=154\n"
        ), // internal
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=4") // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801mk2SR (with PC-8801-11; CSM on both OPNs)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0,
          "clockSel=4\n"
          "ssgVol=154\n"
        ), // internal
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=4") // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801mk2SR (with PC-8801-23)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2203, 1.0f, 0,
          "clockSel=4\n"
          "ssgVol=154\n"
        ), // internal
        CH(DIV_SYSTEM_YM2608, 1.0f, 0, "clockSel=1") // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801mk2SR (with PC-8801-23; extended channel 3 on internal OPN)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0,
          "clockSel=4\n"
          "ssgVol=154\n"
        ), // internal
        CH(DIV_SYSTEM_YM2608, 1.0f, 0, "clockSel=1") // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801mk2SR (with PC-8801-23; extended channel 3 on external OPN)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2203, 1.0f, 0,
          "clockSel=4\n"
          "ssgVol=154\n"
        ), // internal
        CH(DIV_SYSTEM_YM2608_EXT, 1.0f, 0, "clockSel=1") // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801mk2SR (with PC-8801-23; extended channel 3 on both OPNs)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0,
          "clockSel=4\n"
          "ssgVol=154\n"
        ), // internal
        CH(DIV_SYSTEM_YM2608_EXT, 1.0f, 0, "clockSel=1") // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801mk2SR (with PC-8801-23; CSM on internal OPN)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0,
          "clockSel=4\n"
          "ssgVol=154\n"
        ), // internal
        CH(DIV_SYSTEM_YM2608, 1.0f, 0, "clockSel=1") // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801mk2SR (with PC-8801-23; CSM on external OPN)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2203, 1.0f, 0,
          "clockSel=4\n"
          "ssgVol=154\n"
        ), // internal
        CH(DIV_SYSTEM_YM2608_CSM, 1.0f, 0, "clockSel=1") // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801mk2SR (with PC-8801-23; CSM on both OPNs)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0,
          "clockSel=4\n"
          "ssgVol=154\n"
        ), // internal
        CH(DIV_SYSTEM_YM2608_CSM, 1.0f, 0, "clockSel=1") // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801mk2SR (with HMB-20 HIBIKI-8800)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2203, 1.0f, 0,
          "clockSel=4\n"
          "ssgVol=154\n"
        ), // internal
        CH(DIV_SYSTEM_YM2151, 1.0f, 0, "clockSel=2") // external; 4.0000MHz
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801mk2SR (with HMB-20 HIBIKI-8800; extended channel 3)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0,
          "clockSel=4\n"
          "ssgVol=154\n"
        ), // internal
        CH(DIV_SYSTEM_YM2151, 1.0f, 0, "clockSel=2") // external; 4.0000MHz
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801FA (with PC-8801-10)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2608, 1.0f, 0, "clockSel=4"), // internal
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=15"), // external
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=15") // ""
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801FA (with PC-8801-10; extended channel 3)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2608_EXT, 1.0f, 0, "clockSel=4"), // internal
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=15"), // external
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=15") // ""
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801FA (with PC-8801-11)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2608, 1.0f, 0, "clockSel=1"), // internal
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=4") // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801FA (with PC-8801-11; extended channel 3 on internal OPN)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2608_EXT, 1.0f, 0, "clockSel=1"), // internal
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=4") // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801FA (with PC-8801-11; extended channel 3 on external OPN)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2608, 1.0f, 0, "clockSel=1"), // internal
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=4") // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801FA (with PC-8801-11; extended channel 3 on both OPNs)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2608_EXT, 1.0f, 0, "clockSel=1"), // internal
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=4") // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801FA (with PC-8801-11; CSM on internal OPN)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2608_CSM, 1.0f, 0, "clockSel=1"), // internal
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=4") // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801FA (with PC-8801-11; CSM on external OPN)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2608, 1.0f, 0, "clockSel=1"), // internal
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=4") // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801FA (with PC-8801-11; CSM on both OPNs)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2608_CSM, 1.0f, 0, "clockSel=1"), // internal
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=4") // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801FA (with PC-8801-23)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2608, 1.0f, 0, "clockSel=1"), // internal
        CH(DIV_SYSTEM_YM2608, 1.0f, 0, "clockSel=1") // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801FA (with PC-8801-23; extended channel 3 on internal OPNA)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2608_EXT, 1.0f, 0, "clockSel=1"), // internal
        CH(DIV_SYSTEM_YM2608, 1.0f, 0, "clockSel=1") // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801FA (with PC-8801-23; extended channel 3 on external OPNA)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2608, 1.0f, 0, "clockSel=1"), // internal
        CH(DIV_SYSTEM_YM2608_EXT, 1.0f, 0, "clockSel=1") // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801FA (with PC-8801-23; extended channel 3 on both OPNAs)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2608_EXT, 1.0f, 0, "clockSel=1"), // internal
        CH(DIV_SYSTEM_YM2608_EXT, 1.0f, 0, "clockSel=1") // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801FA (with PC-8801-23; CSM on internal OPNA)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2608_CSM, 1.0f, 0, "clockSel=1"), // internal
        CH(DIV_SYSTEM_YM2608, 1.0f, 0, "clockSel=1") // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801FA (with PC-8801-23; CSM on external OPNA)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2608, 1.0f, 0, "clockSel=1"), // internal
        CH(DIV_SYSTEM_YM2608_CSM, 1.0f, 0, "clockSel=1") // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801FA (with PC-8801-23; CSM on both OPNAs)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2608_CSM, 1.0f, 0, "clockSel=1"), // internal
        CH(DIV_SYSTEM_YM2608_CSM, 1.0f, 0, "clockSel=1") // external
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801FA (with HMB-20 HIBIKI-8800)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2608, 1.0f, 0, "clockSel=1"), // internal
        CH(DIV_SYSTEM_YM2151, 1.0f, 0, "clockSel=2") // external; 4.0000MHz
      }
    );
    SUB_ENTRY(
      _("NEC PC-8801FA (with HMB-20 HIBIKI-8800; extended channel 3)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2608_EXT, 1.0f, 0, "clockSel=1"), // internal
        CH(DIV_SYSTEM_YM2151, 1.0f, 0, "clockSel=2") // external; 4.0000MHz
      }
    );
  ENTRY(
    _("NEC PC-98"), {}
  );
    SUB_ENTRY(
      _("NEC PC-98 (with PC-9801-26/K)"), {
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=4\nssgVol=77"), // 3.9936MHz but some compatible card has 4MHz
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1")
      }
    );
    SUB_ENTRY(
      _("NEC PC-98 (with PC-9801-26/K; extended channel 3)"), {
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=4\nssgVol=77"), // 3.9936MHz but some compatible card has 4MHz
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1")
      }
    );
    SUB_ENTRY(
      _("NEC PC-98 (with PC-9801-26/K; CSM)"), {
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=4\nssgVol=77"), // 3.9936MHz but some compatible card has 4MHz
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1")
      }
    );
    SUB_ENTRY(
      _("NEC PC-98 (with Sound Orchestra)"), {
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=4"),
        CH(DIV_SYSTEM_OPL2, 1.0f, 0, "clockSel=4"),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1")
      }
    );
    SUB_ENTRY(
      _("NEC PC-98 (with Sound Orchestra; extended channel 3)"), {
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=4"),
        CH(DIV_SYSTEM_OPL2, 1.0f, 0, "clockSel=4"),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1")
      }
    );
    SUB_ENTRY(
      _("NEC PC-98 (with Sound Orchestra; CSM)"), {
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=4"),
        CH(DIV_SYSTEM_OPL2, 1.0f, 0, "clockSel=4"),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1")
      }
    );
    SUB_ENTRY(
      _("NEC PC-98 (with Sound Orchestra in drums mode)"), {
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=4"),
        CH(DIV_SYSTEM_OPL2_DRUMS, 1.0f, 0, "clockSel=4"),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1")
      }
    );
    SUB_ENTRY(
      _("NEC PC-98 (with Sound Orchestra in drums mode; extended channel 3)"), {
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=4"),
        CH(DIV_SYSTEM_OPL2_DRUMS, 1.0f, 0, "clockSel=4"),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1")
      }
    );
    SUB_ENTRY(
      _("NEC PC-98 (with Sound Orchestra in drums mode; CSM)"), {
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=4"),
        CH(DIV_SYSTEM_OPL2_DRUMS, 1.0f, 0, "clockSel=4"),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1")
      }
    );
    SUB_ENTRY(
      _("NEC PC-98 (with Sound Orchestra V)"), {
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=4"),
        CH(DIV_SYSTEM_Y8950, 1.0f, 0, "clockSel=4"),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1")
      }
    );
    SUB_ENTRY(
      _("NEC PC-98 (with Sound Orchestra V; extended channel 3)"), {
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=4"),
        CH(DIV_SYSTEM_Y8950, 1.0f, 0, "clockSel=4"),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1")
      }
    );
    SUB_ENTRY(
      _("NEC PC-98 (with Sound Orchestra V; CSM)"), {
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=4"),
        CH(DIV_SYSTEM_Y8950, 1.0f, 0, "clockSel=4"),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1")
      }
    );
    SUB_ENTRY(
      _("NEC PC-98 (with Sound Orchestra V in drums mode)"), {
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=4"),
        CH(DIV_SYSTEM_Y8950_DRUMS, 1.0f, 0, "clockSel=4"),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1")
      }
    );
    SUB_ENTRY(
      _("NEC PC-98 (with Sound Orchestra V in drums mode; extended channel 3)"), {
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=4"),
        CH(DIV_SYSTEM_Y8950_DRUMS, 1.0f, 0, "clockSel=4"),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1")
      }
    );
    SUB_ENTRY(
      _("NEC PC-98 (with Sound Orchestra V in drums mode; CSM)"), {
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=4"),
        CH(DIV_SYSTEM_Y8950_DRUMS, 1.0f, 0, "clockSel=4"),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1")
      }
    );
    // note: PC-9801-86 has no ADPCM-B RAM
    SUB_ENTRY(
      _("NEC PC-98 (with PC-9801-86)"), { // -73 also has OPNA
        CH(DIV_SYSTEM_YM2608, 1.0f, 0, "clockSel=1\nssgVol=64"),
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0, // 2x 16-bit Burr Brown DAC
          "rate=44100\n"
          "outDepth=15\n"
        ),
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0,
          "rate=44100\n"
          "outDepth=15\n"
        ),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1")
      }
    );
    SUB_ENTRY(
      _("NEC PC-98 (with PC-9801-86; extended channel 3)"), { // -73 also has OPNA
        CH(DIV_SYSTEM_YM2608_EXT, 1.0f, 0, "clockSel=1\nssgVol=64"),
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0,
          "rate=44100\n"
          "outDepth=15\n"
        ),
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0,
          "rate=44100\n"
          "outDepth=15\n"
        ),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1")
      }
    );
    SUB_ENTRY(
      _("NEC PC-98 (with PC-9801-86; CSM)"), { // -73 also has OPNA
        CH(DIV_SYSTEM_YM2608_CSM, 1.0f, 0, "clockSel=1\nssgVol=64"),
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0,
          "rate=44100\n"
          "outDepth=15\n"
        ),
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0,
          "rate=44100\n"
          "outDepth=15\n"
        ),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1")
      }
    );
    SUB_ENTRY(
      _("NEC PC-98 (with PC-9801-86) stereo"), { // -73 also has OPNA
        CH(DIV_SYSTEM_YM2608, 1.0f, 0, "clockSel=1\nssgVol=64"),
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, -1.0f, // 2x 16-bit Burr Brown DAC
          "rate=44100\n"
          "outDepth=15\n"
        ),
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 1.0f,
          "rate=44100\n"
          "outDepth=15\n"
        ),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1")
      }
    );
    SUB_ENTRY(
      _("NEC PC-98 (with PC-9801-86; extended channel 3) stereo"), { // -73 also has OPNA
        CH(DIV_SYSTEM_YM2608_EXT, 1.0f, 0, "clockSel=1\nssgVol=64"),
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, -1.0f,
          "rate=44100\n"
          "outDepth=15\n"
        ),
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 1.0f,
          "rate=44100\n"
          "outDepth=15\n"
        ),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1")
      }
    );
    SUB_ENTRY(
      _("NEC PC-98 (with PC-9801-86; CSM) stereo"), { // -73 also has OPNA
        CH(DIV_SYSTEM_YM2608_CSM, 1.0f, 0, "clockSel=1\nssgVol=64"),
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, -1.0f,
          "rate=44100\n"
          "outDepth=15\n"
        ),
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 1.0f,
          "rate=44100\n"
          "outDepth=15\n"
        ),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1")
      }
    );
    // note: PC-9801-73 has ADPCM-B RAM, PCM DAC AND DSP
    SUB_ENTRY(
      _("NEC PC-98 (with PC-9801-73)"), {
        CH(DIV_SYSTEM_YM2608, 1.0f, 0, "clockSel=1\nssgVol=64"),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1")
      }
    );
    SUB_ENTRY(
      _("NEC PC-98 (with PC-9801-73; extended channel 3)"), {
        CH(DIV_SYSTEM_YM2608_EXT, 1.0f, 0, "clockSel=1\nssgVol=64"),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1")
      }
    );
    SUB_ENTRY(
      _("NEC PC-98 (with PC-9801-73; CSM)"), {
        CH(DIV_SYSTEM_YM2608_CSM, 1.0f, 0, "clockSel=1\nssgVol=64"),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1")
      }
    );
    SUB_ENTRY(
      _("NEC PC-98 (with Sound Blaster 16 for PC-9800 w/PC-9801-26/K compatible)"), {
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=2\nssgVol=77"), // 4MHz
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0,
          "rate=44100\n"
          "outDepth=15\n"
          "stereo=true\n"
        ),
        CH(DIV_SYSTEM_OPL3, 1.0f, 0, "clockSel=2"),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1")
      }
    );
    SUB_ENTRY(
      _("NEC PC-98 (with Sound Blaster 16 for PC-9800 w/PC-9801-26/K compatible; extended channel 3)"), {
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=2\nssgVol=77"), // 4MHz
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0,
          "rate=44100\n"
          "outDepth=15\n"
          "stereo=true\n"
        ),
        CH(DIV_SYSTEM_OPL3, 1.0f, 0, "clockSel=2"),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1")
      }
    );
    SUB_ENTRY(
      _("NEC PC-98 (with Sound Blaster 16 for PC-9800 w/PC-9801-26/K compatible; CSM)"), {
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=2\nssgVol=77"), // 4MHz
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0,
          "rate=44100\n"
          "outDepth=15\n"
          "stereo=true\n"
        ),
        CH(DIV_SYSTEM_OPL3, 1.0f, 0, "clockSel=2"),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1")
      }
    );
    SUB_ENTRY(
      _("NEC PC-98 (with Sound Blaster 16 for PC-9800 w/PC-9801-26/K compatible in drums mode)"), {
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=2\nssgVol=77"), // 4MHz
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0,
          "rate=44100\n"
          "outDepth=15\n"
          "stereo=true\n"
        ),
        CH(DIV_SYSTEM_OPL3_DRUMS, 1.0f, 0, "clockSel=2"),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1")
      }
    );
    SUB_ENTRY(
      _("NEC PC-98 (with Sound Blaster 16 for PC-9800 w/PC-9801-26/K compatible in drums mode; extended channel 3)"), {
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=2\nssgVol=77"), // 4MHz
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0,
          "rate=44100\n"
          "outDepth=15\n"
          "stereo=true\n"
        ),
        CH(DIV_SYSTEM_OPL3_DRUMS, 1.0f, 0, "clockSel=2"),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1")
      }
    );
    SUB_ENTRY(
      _("NEC PC-98 (with Sound Blaster 16 for PC-9800 w/PC-9801-26/K compatible in drums mode; CSM)"), {
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=2\nssgVol=77"), // 4MHz
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0,
          "rate=44100\n"
          "outDepth=15\n"
          "stereo=true\n"
        ),
        CH(DIV_SYSTEM_OPL3_DRUMS, 1.0f, 0, "clockSel=2"),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "clockSel=1")
      }
    );
  ENTRY(
    _("ZX Spectrum (48K) beeper"), {}
  );
    SUB_ENTRY(
      _("ZX Spectrum (48K, SFX-like engine)"), {
        CH(DIV_SYSTEM_SFX_BEEPER, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("ZX Spectrum (48K, QuadTone engine)"), {
        CH(DIV_SYSTEM_SFX_BEEPER_QUADTONE, 1.0f, 0, "")
      }
    );
  ENTRY(
    _("ZX Spectrum (128K)"), {
      CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=1") //beeper was also included
    }
  );
    SUB_ENTRY(
      _("ZX Spectrum (128K) with TurboSound FM"), {
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=1")
      }
    );
      SUB_SUB_ENTRY(
        _("ZX Spectrum (128K) with TurboSound FM (extended channel 3 on first OPN)"), {
          CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=1"),
          CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=1")
        }
      );
      SUB_SUB_ENTRY(
        _("ZX Spectrum (128K) with TurboSound FM (extended channel 3 on second OPN)"), {
          CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=1"),
          CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=1")
        }
      );
      SUB_SUB_ENTRY(
        _("ZX Spectrum (128K) with TurboSound FM (extended channel 3 on both OPNs)"), {
          CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=1"),
          CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=1")
        }
      );
      SUB_SUB_ENTRY(
        _("ZX Spectrum (128K) with TurboSound FM (CSM on first OPN)"), {
          CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=1"),
          CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=1")
        }
      );
      SUB_SUB_ENTRY(
        _("ZX Spectrum (128K) with TurboSound FM (CSM on second OPN)"), {
          CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=1"),
          CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=1")
        }
      );
      SUB_SUB_ENTRY(
        _("ZX Spectrum (128K) with TurboSound FM (CSM on both OPNs)"), {
          CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=1"),
          CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=1")
        }
      );
    SUB_ENTRY(
      _("ZX Spectrum (128K) with TurboSound"), {
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=1"), // or YM2149
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=1") // or YM2149
      }
    );
  ENTRY(
    _("Amstrad CPC"), {
      CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=5")
    }
  );
  ENTRY(
    _("Atari 800"), {
      CH(DIV_SYSTEM_POKEY, 1.0f, 0, "clockSel=1")
    },
    "tickRate=50"
  );
    SUB_ENTRY(
      _("Atari 800 (stereo)"), {
        CH(DIV_SYSTEM_POKEY, 1.0f, -1.0f, "clockSel=1"),
        CH(DIV_SYSTEM_POKEY, 1.0f, 1.0f, "clockSel=1"),
      },
      "tickRate=50"
    );
  ENTRY(
    _("Atari ST"), {
      CH(DIV_SYSTEM_AY8910, 1.0f, 0,
        "clockSel=3\n"
        "chipType=1\n"
      )
    }
  );
  ENTRY(
    _("Atari STE"), {
      CH(DIV_SYSTEM_AY8910, 1.0f, 0,
        "clockSel=3\n"
        "chipType=1\n"
      ),
      CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0,
        "rate=50668\n"
        "outDepth=7\n"
      ),
      CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0,
        "rate=50668\n"
        "outDepth=7\n"
      )
    }
  );
  ENTRY(
    _("SAM Coupé"), {
      CH(DIV_SYSTEM_SAA1099, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Enterprise 128"), {
      CH(DIV_SYSTEM_DAVE, 1.0f, 0, "")
    },
    "tickRate=50"
  );
  ENTRY(
    _("BBC Micro"), {
      CH(DIV_SYSTEM_SMS, 1.0f, 0,
        "clockSel=2\n"
        "chipType=4\n" // SN76489A 4MHz
      )
    }
  );
  ENTRY(
    _("IBM PC"), {}
  );
    SUB_ENTRY(
      _("PC (barebones)"), {
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("IBM PCjr"), {
        // it can be enable sound output at once
        CH(DIV_SYSTEM_SMS, 1.0f, 0, "chipType=5") // SN76496
      }
    );
    SUB_ENTRY(
      _("Tandy 1000"), {
        CH(DIV_SYSTEM_SMS, 1.0f, 0, "chipType=5"), // NCR 8496 or SN76496 or Tandy PSSJ (with 8 bit DAC)
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("PC + Covox Sound Master"), {
        CH(DIV_SYSTEM_AY8930, 1.0f, 0, "clockSel=3"),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("PC + SSI 2001"), {
        CH(DIV_SYSTEM_C64_6581, 1.0f, 0, "clockSel=2"),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("PC + Game Blaster"), {
        CH(DIV_SYSTEM_SAA1099, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_SAA1099, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("PC + AdLib"), {
        CH(DIV_SYSTEM_OPL2, 1.0f, 0, ""),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("PC + AdLib (drums mode)"), {
        CH(DIV_SYSTEM_OPL2_DRUMS, 1.0f, 0, ""),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("PC + Sound Blaster"), {
        CH(DIV_SYSTEM_OPL2, 1.0f, 0, ""),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, ""),
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0, 
          "rate=22050\n"
          "outDepth=7\n"
        )
      }
    );
    SUB_ENTRY(
      _("PC + Sound Blaster (drums mode)"), {
        CH(DIV_SYSTEM_OPL2_DRUMS, 1.0f, 0, ""),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, ""),
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0, 
          "rate=22050\n"
          "outDepth=7\n"
        )
      }
    );
    SUB_ENTRY(
      _("PC + Sound Blaster w/Game Blaster Compatible"), {
        CH(DIV_SYSTEM_OPL2, 1.0f, 0, ""),
        CH(DIV_SYSTEM_SAA1099, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_SAA1099, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0, 
          "rate=22050\n"
          "outDepth=7\n"
        ),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("PC + Sound Blaster w/Game Blaster Compatible (drums mode)"), {
        CH(DIV_SYSTEM_OPL2_DRUMS, 1.0f, 0, ""),
        CH(DIV_SYSTEM_SAA1099, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_SAA1099, 1.0f, 0, "clockSel=1"),
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0, 
          "rate=22050\n"
          "outDepth=7\n"
        ),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("PC + Sound Blaster Pro"), {
        CH(DIV_SYSTEM_OPL2, 1.0f, -1.0f, ""),
        CH(DIV_SYSTEM_OPL2, 1.0f, 1.0f, ""),
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0, 
          "rate=22050\n"
          "outDepth=7\n"
          "stereo=true\n"
        ), //alternatively 44.1 khz mono
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("PC + Sound Blaster Pro (drums mode)"), {
        CH(DIV_SYSTEM_OPL2_DRUMS, 1.0f, -1.0f, ""),
        CH(DIV_SYSTEM_OPL2_DRUMS, 1.0f, 1.0f, ""),
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0, 
          "rate=22050\n"
          "outDepth=7\n"
          "stereo=true\n"
        ), //alternatively 44.1 khz mono
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("PC + Sound Blaster Pro 2"), {
        CH(DIV_SYSTEM_OPL3, 1.0f, 0, ""),
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0, 
          "rate=44100\n"
          "outDepth=15\n"
          "stereo=true\n"
        ),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("PC + Sound Blaster Pro 2 (drums mode)"), {
        CH(DIV_SYSTEM_OPL3_DRUMS, 1.0f, 0, ""),
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0, 
          "rate=44100\n"
          "outDepth=15\n"
          "stereo=true\n"
        ),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("PC + ESS AudioDrive ES1488 (native ESFM mode)"), {
        CH(DIV_SYSTEM_ESFM, 1.0f, 0, ""),
        CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0, 
          "rate=44100\n"
          "outDepth=15\n"
          "stereo=true\n"
        ),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("PC + PC-FXGA"), {
        CH(DIV_SYSTEM_PCE, 1.0f, 0, ""), // HuC6230 (WSG from HuC6280 but with built in 2 OKI ADPCM playback engines)
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("PC + SAAYM"), {
        CH(DIV_SYSTEM_YM2151, 1.0f, 0, ""), // 3.58MHz or 4MHz selectable via jumper
        CH(DIV_SYSTEM_SAA1099, 1.0f, 0, "clockSel=1"), // 7.16MHz or 8MHz selectable via jumper
        CH(DIV_SYSTEM_SAA1099, 1.0f, 0, "clockSel=1"), // ""
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      "Sega TeraDrive", {
        CH(DIV_SYSTEM_YM2612, 1.0f, 0, "chipType=0"), // YM3438
        CH(DIV_SYSTEM_SMS, 0.5f, 0, ""),
        CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "")
      }
    );
      SUB_SUB_ENTRY(
        "Sega TeraDrive (extended channel 3)", {
          CH(DIV_SYSTEM_YM2612_EXT, 1.0f, 0, "chipType=0"), // YM3438
          CH(DIV_SYSTEM_SMS, 0.5f, 0, ""),
          CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "")
        }
    );
      SUB_SUB_ENTRY(
        "Sega TeraDrive (CSM)", {
          CH(DIV_SYSTEM_YM2612_CSM, 1.0f, 0, "chipType=0"), // YM3438
          CH(DIV_SYSTEM_SMS, 0.5f, 0, ""),
          CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "")
        }
    );
      SUB_SUB_ENTRY(
        "Sega TeraDrive (DualPCM)", {
          CH(DIV_SYSTEM_YM2612_DUALPCM, 1.0f, 0, "chipType=0"), // YM3438
          CH(DIV_SYSTEM_SMS, 0.5f, 0, ""),
          CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "")
        }
    );
      SUB_SUB_ENTRY(
        "Sega TeraDrive (DualPCM, extended channel 3)", {
          CH(DIV_SYSTEM_YM2612_DUALPCM_EXT, 1.0f, 0, "chipType=0"), // YM3438
          CH(DIV_SYSTEM_SMS, 0.5f, 0, ""),
          CH(DIV_SYSTEM_PCSPKR, 1.0f, 0, "")
        }
    );
  ENTRY(
    _("Sharp X1"), {
      CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=3")
    }
  );
    SUB_ENTRY(
      _("Sharp X1 + FM add-on"), {
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=3"),
        CH(DIV_SYSTEM_YM2151, 1.0f, 0, "clockSel=2")
      }
    );
  ENTRY(
    _("Sharp X68000"), {
      CH(DIV_SYSTEM_YM2151, 1.0f, 0, "clockSel=2"),
      CH(DIV_SYSTEM_MSM6258, 1.0f, 0, "clockSel=2")
    }
  );
  ENTRY(
    _("FM-7"), {
      CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=12"),
      CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=5")
    }
  );
   SUB_ENTRY(
     _("FM-7 (extended channel 3)"), {
       CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=12"),
       CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=5")
     }
  );
   SUB_ENTRY(
     _("FM-7 (CSM)"), {
       CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=12"),
       CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=5")
     }
  );
  ENTRY(
    _("FM Towns"), {
      CH(DIV_SYSTEM_YM2612, 1.0f, 0, 
              "clockSel=2\n"
              "chipType=0\n"
      ), // YM3438
      CH(DIV_SYSTEM_RF5C68, 1.0f, 0, "")
    }
  );
    SUB_ENTRY(
      _("FM Towns (extended channel 3)"), {
        CH(DIV_SYSTEM_YM2612_EXT, 1.0f, 0, 
          "clockSel=2\n"
          "chipType=0\n"
        ), // YM3438
        CH(DIV_SYSTEM_RF5C68, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("FM Towns (CSM)"), {
        CH(DIV_SYSTEM_YM2612_CSM, 1.0f, 0, 
          "clockSel=2\n"
          "chipType=0\n"
        ), // YM3438
        CH(DIV_SYSTEM_RF5C68, 1.0f, 0, "")
      }
    );
  ENTRY(
    _("Commander X16"), {
      CH(DIV_SYSTEM_VERA, 1.0f, 0, ""),
      CH(DIV_SYSTEM_YM2151, 1.0f, 0, "")
    }
  );
    SUB_ENTRY(
      _("Commander X16 (VERA only)"), {
        CH(DIV_SYSTEM_VERA, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Commander X16 (with Twin OPL3)"), {
        CH(DIV_SYSTEM_VERA, 1.0f, 0, ""),
        CH(DIV_SYSTEM_OPL3, 1.0f, 0, ""),
        CH(DIV_SYSTEM_OPL3, 1.0f, 0, "")
      }
    );
  ENTRY(
    _("TI-99/4A"), {
      CH(DIV_SYSTEM_SMS, 1.0f, 0,
        "clockSel=6\n"
        "chipType=8\n" // SN94624 447KHz
      )
    }
  );
  ENTRY(
    _("Sord M5"), {
      CH(DIV_SYSTEM_SMS, 1.0f, 0,
        "customClock=1773447\n"
        "chipType=1\n"
      )
    }
  );
  CATEGORY_END;
}
