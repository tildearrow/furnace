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

void initSystemPresetsFM(std::vector<FurnaceGUISysCategory>& sysCategories) {
  CATEGORY_BEGIN(_("FM"),_("chips which use frequency modulation (FM) to generate sound.\nsome of these also pack more (like square and sample channels)."));
  ENTRY(
    _("Yamaha YM2151 (OPM)"), {
      CH(DIV_SYSTEM_YM2151, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Yamaha YM2203 (OPN)"), {
      CH(DIV_SYSTEM_YM2203, 1.0f, 0, "clockSel=3")
    }
  );
    SUB_ENTRY(
      _("Yamaha YM2203 (extended channel 3)"), {
        CH(DIV_SYSTEM_YM2203_EXT, 1.0f, 0, "clockSel=3")
      }
    );
    SUB_ENTRY(
      _("Yamaha YM2203 (CSM)"), {
        CH(DIV_SYSTEM_YM2203_CSM, 1.0f, 0, "clockSel=3")
      }
    );
  ENTRY(
    _("Yamaha YM2608 (OPNA)"), {
      CH(DIV_SYSTEM_YM2608, 1.0f, 0, "")
    }
  );
    SUB_ENTRY(
      _("Yamaha YM2608 (extended channel 3)"), {
        CH(DIV_SYSTEM_YM2608_EXT, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Yamaha YM2608 (CSM)"), {
        CH(DIV_SYSTEM_YM2608_CSM, 1.0f, 0, "")
      }
    );
  ENTRY(
    _("Yamaha YM2610 (OPNB)"), {
      CH(DIV_SYSTEM_YM2610_FULL, 1.0f, 0, "")
    }
  );
    SUB_ENTRY(
      _("Yamaha YM2610 (extended channel 2)"), {
        CH(DIV_SYSTEM_YM2610_FULL_EXT, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Yamaha YM2610 (CSM)"), {
        CH(DIV_SYSTEM_YM2610_CSM, 1.0f, 0, "")
      }
    );
  ENTRY(
    _("Yamaha YM2610B (OPNB2)"), {
      CH(DIV_SYSTEM_YM2610B, 1.0f, 0, "")
    }
  );
    SUB_ENTRY(
      _("Yamaha YM2610B (extended channel 3)"), {
        CH(DIV_SYSTEM_YM2610B_EXT, 1.0f, 0, "")
      }
    );
    SUB_ENTRY(
      _("Yamaha YM2610B (CSM)"), {
        CH(DIV_SYSTEM_YM2610B_CSM, 1.0f, 0, "")
      }
    );
  ENTRY(
    _("Yamaha YM2612 (OPN2)"), {
      CH(DIV_SYSTEM_YM2612, 1.0f, 0, "chipType=1")
    }
  );
    SUB_ENTRY(
      _("Yamaha YM2612 (extended channel 3)"), {
        CH(DIV_SYSTEM_YM2612_EXT, 1.0f, 0, "chipType=1")
      }
    );
    SUB_ENTRY(
      _("Yamaha YM2612 (OPN2) CSM"), {
        CH(DIV_SYSTEM_YM2612_CSM, 1.0f, 0, "chipType=1")
      }
    );
    SUB_ENTRY(
      _("Yamaha YM2612 (OPN2) with DualPCM"), {
        CH(DIV_SYSTEM_YM2612_DUALPCM, 1.0f, 0, "chipType=1")
      }
    );
    SUB_ENTRY(
      _("Yamaha YM2612 (extended channel 3) with DualPCM"), {
        CH(DIV_SYSTEM_YM2612_DUALPCM_EXT, 1.0f, 0, "chipType=1")
      }
    );
  ENTRY(
    _("Yamaha YMF276 (OPN2L)"), {
      CH(DIV_SYSTEM_YM2612, 1.0f, 0, "chipType=2")
    }
  );
    SUB_ENTRY(
      _("Yamaha YMF276 (extended channel 3)"), {
        CH(DIV_SYSTEM_YM2612_EXT, 1.0f, 0, "chipType=2")
      }
    );
    SUB_ENTRY(
      _("Yamaha YMF276 (OPN2L) CSM"), {
        CH(DIV_SYSTEM_YM2612_CSM, 1.0f, 0, "chipType=2")
      }
    );
    SUB_ENTRY(
      _("Yamaha YMF276 (OPN2L) with DualPCM"), {
        CH(DIV_SYSTEM_YM2612_DUALPCM, 1.0f, 0, "chipType=2")
      }
    );
    SUB_ENTRY(
      _("Yamaha YMF276 (extended channel 3) with DualPCM"), {
        CH(DIV_SYSTEM_YM2612_DUALPCM_EXT, 1.0f, 0, "chipType=2")
      }
    );
  ENTRY(
    _("Yamaha YM2413 (OPLL)"), {
      CH(DIV_SYSTEM_OPLL, 1.0f, 0, "")
    }
  );
    SUB_ENTRY(
      _("Yamaha YM2413 (drums mode)"), {
        CH(DIV_SYSTEM_OPLL_DRUMS, 1.0f, 0, "")
      }
    );
  ENTRY(
    _("Yamaha YM2414 (OPZ)"), {
      CH(DIV_SYSTEM_OPZ, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Yamaha YM3438 (OPN2C)"), {
      CH(DIV_SYSTEM_YM2612, 1.0f, 0, "chipType=0")
    }
  );
    SUB_ENTRY(
      _("Yamaha YM3438 (extended channel 3)"), {
        CH(DIV_SYSTEM_YM2612_EXT, 1.0f, 0, "chipType=0")
      }
    );
    SUB_ENTRY(
      _("Yamaha YM3438 (OPN2C) CSM"), {
        CH(DIV_SYSTEM_YM2612_CSM, 1.0f, 0, "chipType=0")
      }
    );
    SUB_ENTRY(
      _("Yamaha YM3438 (OPN2C) with DualPCM"), {
        CH(DIV_SYSTEM_YM2612_DUALPCM, 1.0f, 0, "chipType=0")
      }
    );
    SUB_ENTRY(
      _("Yamaha YM3438 (extended channel 3) with DualPCM"), {
        CH(DIV_SYSTEM_YM2612_DUALPCM_EXT, 1.0f, 0, "chipType=0")
      }
    );
  ENTRY(
    _("Yamaha YM3526 (OPL)"), {
      CH(DIV_SYSTEM_OPL, 1.0f, 0, "")
    }
  );
    SUB_ENTRY(
      _("Yamaha YM3526 (drums mode)"), {
        CH(DIV_SYSTEM_OPL_DRUMS, 1.0f, 0, "")
      }
    );
  ENTRY(
    _("Yamaha Y8950"), {
      CH(DIV_SYSTEM_Y8950, 1.0f, 0, "")
    }
  );
    SUB_ENTRY(
      _("Yamaha Y8950 (drums mode)"), {
        CH(DIV_SYSTEM_Y8950_DRUMS, 1.0f, 0, "")
      }
    );
  ENTRY(
    _("Yamaha YM3812 (OPL2)"), {
      CH(DIV_SYSTEM_OPL2, 1.0f, 0, "")
    }
  );
    SUB_ENTRY(
      _("Yamaha YM3812 (drums mode)"), {
        CH(DIV_SYSTEM_OPL2_DRUMS, 1.0f, 0, "")
      }
    );
  ENTRY(
    _("Yamaha YMF262 (OPL3)"), {
      CH(DIV_SYSTEM_OPL3, 1.0f, 0, "")
    }
  );
    SUB_ENTRY(
      _("Yamaha YMF262 (drums mode)"), {
        CH(DIV_SYSTEM_OPL3_DRUMS, 1.0f, 0, "")
      }
    );
  ENTRY(
    _("Yamaha YMF289B (OPL3-L)"), {
      CH(DIV_SYSTEM_OPL3, 1.0f, 0, 
         "clockSel=5\n"
         "chipType=1\n"
      )
    }
  );
    SUB_ENTRY(
      _("Yamaha YMF289B (drums mode)"), {
        CH(DIV_SYSTEM_OPL3_DRUMS, 1.0f, 0, 
           "clockSel=5\n"
           "chipType=1\n"
        )
      }
    );
  ENTRY(
    _("ESS ES1xxx series (ESFM)"), {
      CH(DIV_SYSTEM_ESFM, 1.0f, 0, "")
    }
  );
  ENTRY(
    "Yamaha YMF278B (OPL4)", {
      CH(DIV_SYSTEM_OPL4, 1.0f, 0, "")
    }
  );
    SUB_ENTRY(
      "Yamaha YMF278B (drums mode)", {
        CH(DIV_SYSTEM_OPL4_DRUMS, 1.0f, 0, "")
      }
    );
  CATEGORY_END;
}
