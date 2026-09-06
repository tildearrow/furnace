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

void initSystemPresetsGameConsoles(std::vector<FurnaceGUISysCategory>& sysCategories) {
  CATEGORY_BEGIN(_("Game consoles"),_("let's play some chiptune making games!"));
  ENTRY(
    _("Sega Genesis"), {
      CH(DIV_SYSTEM_YM2612, 1.0f, 0, ""),
      CH(DIV_SYSTEM_SMS, 0.5f, 0, "")
    }
  );
    SUB_ENTRY(
      _("Sega Genesis (extended channel 3)"), {
        CH(DIV_SYSTEM_YM2612_EXT, 1.0f, 0, ""),
        CH(DIV_SYSTEM_SMS, 0.5f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Sega Genesis (CSM)"), {
        CH(DIV_SYSTEM_YM2612_CSM, 1.0f, 0, ""),
        CH(DIV_SYSTEM_SMS, 0.5f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Sega Genesis (DualPCM)"), {
        CH(DIV_SYSTEM_YM2612_DUALPCM, 1.0f, 0, ""),
        CH(DIV_SYSTEM_SMS, 0.5f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Sega Genesis (DualPCM, extended channel 3)"), {
        CH(DIV_SYSTEM_YM2612_DUALPCM_EXT, 1.0f, 0, ""),
        CH(DIV_SYSTEM_SMS, 0.5f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Sega Genesis (with Sega CD)"), {
        CH(DIV_SYSTEM_YM2612, 1.0f, 0, ""),
        CH(DIV_SYSTEM_SMS, 0.5f, 0, ""),
        CH(DIV_SYSTEM_RF5C68, 1.0f, 0,
          "clockSel=2\n"
          "chipType=1\n"
        )
      }
    );
    SUB_ENTRY(
      _("Sega Genesis (extended channel 3 with Sega CD)"), {
        CH(DIV_SYSTEM_YM2612_EXT, 1.0f, 0, ""),
        CH(DIV_SYSTEM_SMS, 0.5f, 0, ""),
        CH(DIV_SYSTEM_RF5C68, 1.0f, 0,
          "clockSel=2\n"
          "chipType=1\n"
        )
      }
    );
    SUB_ENTRY(
      _("Sega Genesis (CSM with Sega CD)"), {
        CH(DIV_SYSTEM_YM2612_CSM, 1.0f, 0, ""),
        CH(DIV_SYSTEM_SMS, 0.5f, 0, ""),
        CH(DIV_SYSTEM_RF5C68, 1.0f, 0,
          "clockSel=2\n"
          "chipType=1\n"
        )
      }
    );
  ENTRY(
    _("Sega Master System"), {
      CH(DIV_SYSTEM_SMS, 1.0f, 0, "")
    }
  );
    SUB_ENTRY(
      _("Sega Master System (with FM expansion)"), {
        CH(DIV_SYSTEM_SMS, 1.0f, 0, ""),
        CH(DIV_SYSTEM_OPLL, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Sega Master System (with FM expansion in drums mode)"), {
        CH(DIV_SYSTEM_SMS, 1.0f, 0, ""),
        CH(DIV_SYSTEM_OPLL_DRUMS, 1.0f, 0, "")
      }
    );
  ENTRY(
    _("Sega Game Gear"), {
      CH(DIV_SYSTEM_SMS, 1.0f, 0, "chipType=3")
    }
  );
  ENTRY(
    _("Game Boy"), {
      CH(DIV_SYSTEM_GB, 1.0f, 0, "")
    }
  );
  ENTRY(
     _("Game Boy Advance"), {}
  );
  SUB_ENTRY(
    _("Game Boy Advance (no software mixing)"), {
      CH(DIV_SYSTEM_GB, 1.0f, 0, "chipType=3"),
      CH(DIV_SYSTEM_GBA_DMA, 0.5f, 0, ""),
    }
  );
  SUB_ENTRY(
    _("Game Boy Advance (with MinMod)"), {
      CH(DIV_SYSTEM_GB, 1.0f, 0, "chipType=3"),
      CH(DIV_SYSTEM_GBA_MINMOD, 0.5f, 0, ""),
    }
  );
  ENTRY(
    _("Neo Geo Pocket"), {
      CH(DIV_SYSTEM_T6W28, 1.0f, 0, ""),
      CH(DIV_SYSTEM_PCM_DAC, 1.0f, -1.0f, 
        "rate=11025\n"
        "outDepth=5\n"
      ),
      CH(DIV_SYSTEM_PCM_DAC, 1.0f, 1.0f, 
        "rate=11025\n"
        "outDepth=5\n"
      ) // don't know what the actual sample rate is
    }
  );
  ENTRY(
    _("NEC PC Engine/TurboGrafx-16"), {
      CH(DIV_SYSTEM_PCE, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("NES"), {
      CH(DIV_SYSTEM_NES, 1.0f, 0, "")
    }
  );
    SUB_ENTRY(
      _("Famicom with Konami VRC6"), {
        CH(DIV_SYSTEM_NES, 1.0f, 0, ""),
        CH(DIV_SYSTEM_VRC6, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Famicom with Konami VRC7"), {
        CH(DIV_SYSTEM_NES, 1.0f, 0, ""),
        CH(DIV_SYSTEM_VRC7, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Famicom with MMC5"), {
        CH(DIV_SYSTEM_NES, 1.0f, 0, ""),
        CH(DIV_SYSTEM_MMC5, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Famicom with Sunsoft 5B"), {
        CH(DIV_SYSTEM_NES, 1.0f, 0, ""),
        CH(DIV_SYSTEM_AY8910, 1.0f, 0, "chipType=2")
      }
    );
    SUB_ENTRY(
      _("Famicom with Namco 163"), {
        CH(DIV_SYSTEM_NES, 1.0f, 0, ""),
        CH(DIV_SYSTEM_N163, 1.0f, 0, "channels=7")
      }
    );
    SUB_ENTRY(
      _("Famicom Disk System"), {
        CH(DIV_SYSTEM_NES, 1.0f, 0, ""),
        CH(DIV_SYSTEM_FDS, 1.0f, 0, "")
      }
    );
  ENTRY(
    _("SNES"), {
      CH(DIV_SYSTEM_SNES, 1.0f, 0, "")
    }
  );
    SUB_ENTRY(
      _("Super Game Boy"), {
        CH(DIV_SYSTEM_GB, 1.0f, 0, "customClock=4295455"),
        CH(DIV_SYSTEM_SNES, 1.0f, 0, "")
      },
      "tickRate=61.44697015935724"
    );
    SUB_ENTRY(
      _("Super Game Boy 2"), {
        CH(DIV_SYSTEM_GB, 1.0f, 0, ""),
        CH(DIV_SYSTEM_SNES, 1.0f, 0, "")
      }
    );
  ENTRY(
    _("Mattel Intellivision"), {
      CH(DIV_SYSTEM_AY8910, 1.0f, 0, "chipType=3")
    }
  );
  ENTRY(
    _("Vectrex"), {
      CH(DIV_SYSTEM_AY8910, 1.0f, 0, "clockSel=4")
    }
  );
  ENTRY(
    _("Neo Geo AES"), {
      CH(DIV_SYSTEM_YM2610_FULL, 1.0f, 0, "clockSel=1")
    }
  );
    SUB_ENTRY(
      _("Neo Geo AES (extended channel 2)"), {
        CH(DIV_SYSTEM_YM2610_FULL_EXT, 1.0f, 0, "clockSel=1")
      }
    );
    SUB_ENTRY(
      _("Neo Geo AES (CSM)"), {
        CH(DIV_SYSTEM_YM2610_CSM, 1.0f, 0, "clockSel=1")
      }
    );
  ENTRY(
    _("Atari 2600/7800"), {
      CH(DIV_SYSTEM_TIA, 1.0f, 0, "")
    }
  );
    SUB_ENTRY(
      _("Atari 2600/7800 (with software pitch driver)"), {
        CH(DIV_SYSTEM_TIA, 1.0f, 0, "softwarePitch=1")
      }
    );
  ENTRY(
    _("Atari 7800 + Ballblazer/Commando"), {
      CH(DIV_SYSTEM_TIA, 1.0f, 0, ""),
      CH(DIV_SYSTEM_POKEY, 1.0f, 0, "")
    }
  );
    SUB_ENTRY(
      _("Atari 7800 (with software pitch driver) + Ballblazer/Commando"), {
        CH(DIV_SYSTEM_TIA, 1.0f, 0, "softwarePitch=1"),
        CH(DIV_SYSTEM_POKEY, 1.0f, 0, "")
      }
    );
  ENTRY(
    _("Atari Lynx"), {
      CH(DIV_SYSTEM_LYNX, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("WonderSwan"), {
      CH(DIV_SYSTEM_SWAN, 1.0f, 0, "")
    },
    "tickRate=75.47169811320754716981"
  );
  ENTRY(
    _("Virtual Boy"), {
      CH(DIV_SYSTEM_VBOY, 1.0f, 0, "")
    },
    "tickRate=50.2734877734878"
  );
  ENTRY(
    _("Gamate"), {
      CH(DIV_SYSTEM_AY8910, 1.0f, 0,
        "clockSel=9\n"
        "chipType=0\n"
        "stereo=true\n"
      )
    }
  );
  ENTRY(
    _("Pokémon Mini"), {
      CH(DIV_SYSTEM_POKEMINI, 0.5f, 0, "")
    }
  );
  ENTRY(
    _("Tiger Game.com"), {
      CH(DIV_SYSTEM_SM8521, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Casio PV-1000"), {
      CH(DIV_SYSTEM_PV1000, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Nintendo DS"), {
      CH(DIV_SYSTEM_NDS, 1.0f, 0, "")
    }
  );
  ENTRY(
    "Watara Supervision", {
      CH(DIV_SYSTEM_SUPERVISION, 1.0f, 0, "")
    },
    "tickRate=50.81300813008130081301"
  );
  CATEGORY_END;
}
