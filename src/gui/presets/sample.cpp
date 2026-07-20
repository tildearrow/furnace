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

void initSystemPresetsSample(std::vector<FurnaceGUISysCategory>& sysCategories) {
  CATEGORY_BEGIN(_("Sample"),_("chips/systems which use PCM or ADPCM samples for sound synthesis."));
  ENTRY(
    _("Amiga"), {
      CH(DIV_SYSTEM_AMIGA, 1.0f, 0, "clockSel=1")
    },
    "tickRate=50"
  );
  ENTRY(
    _("SegaPCM"), {
      CH(DIV_SYSTEM_SEGAPCM, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Capcom QSound"), {
      CH(DIV_SYSTEM_QSOUND, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Seta/Allumer X1-010"), {
      CH(DIV_SYSTEM_X1_010, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Yamaha YMZ280B (PCMD8)"), {
      CH(DIV_SYSTEM_YMZ280B, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Ricoh RF5C68"), {
      CH(DIV_SYSTEM_RF5C68, 1.0f, 0, "")
    }
  ); 
  ENTRY(
    _("OKI MSM6258"), {
      CH(DIV_SYSTEM_MSM6258, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("OKI MSM6295"), {
      CH(DIV_SYSTEM_MSM6295, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("SNES"), {
      CH(DIV_SYSTEM_SNES, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Konami K007232"), {
      CH(DIV_SYSTEM_K007232, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Irem GA20"), {
      CH(DIV_SYSTEM_GA20, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Generic PCM DAC"), {
      CH(DIV_SYSTEM_PCM_DAC, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Ensoniq ES5506 (OTTO)"), {
      CH(DIV_SYSTEM_ES5506, 1.0f, 0, "channels=31")
    }
  );
  ENTRY(
    _("Konami K053260"), {
      CH(DIV_SYSTEM_K053260, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Namco C140"), {
      CH(DIV_SYSTEM_C140, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Namco C219"), {
      CH(DIV_SYSTEM_C219, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Nintendo DS"), {
      CH(DIV_SYSTEM_NDS, 1.0f, 0, "")
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
  ENTRY(
    "Yamaha YMW258-F (MultiPCM)", {
      CH(DIV_SYSTEM_MULTIPCM, 1.0f, 0, "")
    }
  );
  CATEGORY_END;
}
