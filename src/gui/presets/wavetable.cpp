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

void initSystemPresetsWavetable(std::vector<FurnaceGUISysCategory>& sysCategories) {
  CATEGORY_BEGIN(_("Wavetable"),_("chips which use user-specified waveforms to generate sound."));
  ENTRY(
    _("PC Engine"), {
      CH(DIV_SYSTEM_PCE, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Konami Bubble System WSG"), {
      CH(DIV_SYSTEM_BUBSYS_WSG, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Konami SCC"), {
      CH(DIV_SYSTEM_SCC, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Konami SCC+"), {
      CH(DIV_SYSTEM_SCC_PLUS, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Namco WSG"), {
      CH(DIV_SYSTEM_NAMCO, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Namco Pole Position WSG (8-channel quadraphonic)"), {
      CH(DIV_SYSTEM_NAMCO_POLEPOS, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Namco C15 (8-channel mono)"), {
      CH(DIV_SYSTEM_NAMCO_15XX, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Namco C30 (8-channel stereo)"), {
      CH(DIV_SYSTEM_NAMCO_CUS30, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Namco 163"), {
      CH(DIV_SYSTEM_N163, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("Famicom Disk System (chip)"), {
      CH(DIV_SYSTEM_FDS, 1.0f, 0, "")
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
    _("Seta/Allumer X1-010"), {
      CH(DIV_SYSTEM_X1_010, 1.0f, 0, "")
    }
  );
  CATEGORY_END;
}
