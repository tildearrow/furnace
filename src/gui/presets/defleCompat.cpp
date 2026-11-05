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

void initSystemPresetsDefleCompat(std::vector<FurnaceGUISysCategory>& sysCategories) {
  CATEGORY_BEGIN(_("DefleMask-compatible"),_("these configurations are compatible with DefleMask.\nselect this if you need to save as .dmf or work with that program."));
  ENTRY(
    _("Sega Genesis"), {
      CH(DIV_SYSTEM_YM2612, 1.0f, 0, ""),
      CH(DIV_SYSTEM_SMS, 0.5f, 0, "")
    }
  );
  ENTRY(
    _("Sega Genesis (extended channel 3)"), {
      CH(DIV_SYSTEM_YM2612_EXT, 1.0f, 0, ""),
      CH(DIV_SYSTEM_SMS, 0.5f, 0, "")
    }
  );
  ENTRY(
    _("Sega Master System"), {
      CH(DIV_SYSTEM_SMS, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Sega Master System (with FM expansion)"), {
      CH(DIV_SYSTEM_SMS, 1.0f, 0, ""),
      CH(DIV_SYSTEM_OPLL, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Game Boy"), {
      CH(DIV_SYSTEM_GB, 1.0f, 0, "")
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
  ENTRY(
    _("Famicom with Konami VRC7"), {
      CH(DIV_SYSTEM_NES, 1.0f, 0, ""),
      CH(DIV_SYSTEM_VRC7, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Famicom Disk System"), {
      CH(DIV_SYSTEM_NES, 1.0f, 0, ""),
      CH(DIV_SYSTEM_FDS, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Commodore 64 (6581 SID)"), {
      CH(DIV_SYSTEM_C64_6581, 1.0f, 0, "clockSel=1")
    },
    "tickRate=50.1245421"
  );
  ENTRY(
    _("Commodore 64 (8580 SID)"), {
      CH(DIV_SYSTEM_C64_8580, 1.0f, 0, "clockSel=1")
    },
    "tickRate=50.1245421"
  );
  ENTRY(
    _("Arcade (YM2151 and SegaPCM)"), {
      CH(DIV_SYSTEM_YM2151, 1.0f, 0, ""),
      CH(DIV_SYSTEM_SEGAPCM, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Neo Geo"), {
      CH(DIV_SYSTEM_YM2610_FULL, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Neo Geo (extended channel 2)"), {
      CH(DIV_SYSTEM_YM2610_FULL_EXT, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("MSX + SCC"), {
      CH(DIV_SYSTEM_AY8910, 1.0f, 0, "chipType=1"),
      CH(DIV_SYSTEM_SCC, 0.67f, 0, "")
    }
  );
  CATEGORY_END;
}
