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

void initSystemPresetsSpecialized(std::vector<FurnaceGUISysCategory>& sysCategories) {
  CATEGORY_BEGIN(_("Specialized"),_("chips/systems with unique sound synthesis methods."));
  ENTRY(
    _("MOS Technology SID (6581)"), {
      CH(DIV_SYSTEM_C64_6581, 1.0f, 0, "clockSel=1")
    },
    "tickRate=50.1245421"
  );
  ENTRY(
    _("MOS Technology SID (8580)"), {
      CH(DIV_SYSTEM_C64_8580, 1.0f, 0, "clockSel=1")
    },
    "tickRate=50.1245421"
  );
  ENTRY(
    _("Commodore PET (pseudo-wavetable)"), {
      CH(DIV_SYSTEM_PET, 1.0f, 0, "")
    },
    "tickRate=50"
  );
  ENTRY(
    _("Konami VRC6"), {
      CH(DIV_SYSTEM_VRC6, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("MMC5"), {
      CH(DIV_SYSTEM_MMC5, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Microchip AY8930"), {
      CH(DIV_SYSTEM_AY8930, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Game Boy"), {
      CH(DIV_SYSTEM_GB, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Atari Lynx"), {
      CH(DIV_SYSTEM_LYNX, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("POKEY"), {
      CH(DIV_SYSTEM_POKEY, 1.0f, 0, "clockSel=1")
    },
    "tickRate=50"
  );
  ENTRY(
    _("Atari TIA"), {
      CH(DIV_SYSTEM_TIA, 1.0f, 0, "")
    }
  );
    SUB_ENTRY(
      _("Atari TIA (with software pitch driver)"), {
        CH(DIV_SYSTEM_TIA, 1.0f, 0, "softwarePitch=1")
      }
    );
  ENTRY(
    _("NES (Ricoh 2A03)"), {
      CH(DIV_SYSTEM_NES, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Commander X16 (VERA only)"), {
      CH(DIV_SYSTEM_VERA, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("ZX Spectrum (beeper only, SFX-like engine)"), {
      CH(DIV_SYSTEM_SFX_BEEPER, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("ZX Spectrum (beeper only, QuadTone engine)"), {
      CH(DIV_SYSTEM_SFX_BEEPER_QUADTONE, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Sharp SM8521"), {
      CH(DIV_SYSTEM_SM8521, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("tildearrow Sound Unit"), {
      CH(DIV_SYSTEM_SOUND_UNIT, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("PowerNoise"), {
      CH(DIV_SYSTEM_POWERNOISE, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Dave"), {
      CH(DIV_SYSTEM_DAVE, 1.0f, 0, "")
    },
    "tickRate=50"
  );
  ENTRY(
    _("Nintendo DS"), {
      CH(DIV_SYSTEM_NDS, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Bifurcator"), {
      CH(DIV_SYSTEM_BIFURCATOR, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("SID2"), {
      CH(DIV_SYSTEM_SID2, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("SID3"), {
      CH(DIV_SYSTEM_SID3, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Watara Supervision"), {
      CH(DIV_SYSTEM_SUPERVISION, 1.0f, 0, "")
    }
  );
  CATEGORY_END;
}
