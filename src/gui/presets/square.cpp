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

void initSystemPresetsSquare(std::vector<FurnaceGUISysCategory>& sysCategories) {
  CATEGORY_BEGIN(_("Square"),_("these chips generate square/pulse tones only (but may include noise)."));
  ENTRY(
    _("TI SN76489"), {
      CH(DIV_SYSTEM_SMS, 1.0f, 0, "chipType=1")
    }
  );
    SUB_ENTRY(
      _("TI SN76489A"), {
        CH(DIV_SYSTEM_SMS, 1.0f, 0, "chipType=4")
      }
    );
    SUB_ENTRY(
      _("TI SN76496"), {
        CH(DIV_SYSTEM_SMS, 1.0f, 0, "chipType=5")
      }
    );
    SUB_ENTRY(
      _("NCR 8496"), {
        CH(DIV_SYSTEM_SMS, 1.0f, 0, "chipType=6")
      }
    );
    SUB_ENTRY(
      _("Tandy PSSJ 3-voice sound"), {
        CH(DIV_SYSTEM_SMS, 1.0f, 0, "chipType=7")
        // 8 bit DAC
      }
    );
    SUB_ENTRY(
      _("Sega PSG (SN76489-like)"), {
        CH(DIV_SYSTEM_SMS, 1.0f, 0, ""),
      }
    );
    SUB_ENTRY(
      _("Sega PSG (SN76489-like, Stereo)"), {
        CH(DIV_SYSTEM_SMS, 1.0f, 0, "chipType=3")
      }
    );
    SUB_ENTRY(
      _("TI SN94624"), {
        CH(DIV_SYSTEM_SMS, 1.0f, 0,
          "clockSel=6\n"
          "chipType=8\n"
        )
      }
    );
    SUB_ENTRY(
      _("TI SN76494"), {
        CH(DIV_SYSTEM_SMS, 1.0f, 0,
          "clockSel=6\n"
          "chipType=9\n"
        )
      }
    );
  ENTRY(
    _("Toshiba T6W28"), {
      CH(DIV_SYSTEM_T6W28, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("AY-3-8910"), {
      CH(DIV_SYSTEM_AY8910, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("AY-3-8914"), {
      CH(DIV_SYSTEM_AY8910, 1.0f, 0, "chipType=3")
    }
  );
  ENTRY(
    _("Yamaha YM2149(F)"), {
      CH(DIV_SYSTEM_AY8910, 1.0f, 0, "chipType=1")
    }
  );
  ENTRY(
    _("Philips SAA1099"), {
      CH(DIV_SYSTEM_SAA1099, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("PC Speaker"), {
      CH(DIV_SYSTEM_PCSPKR, 0.5f, 0, "")
    }
  );
  ENTRY(
    _("Pokémon Mini"), {
      CH(DIV_SYSTEM_POKEMINI, 0.5f, 0, "")
    }
  );
  ENTRY(
    _("Commodore VIC"), {
      CH(DIV_SYSTEM_VIC20, 1.0f, 0, "clockSel=1")
    }
  );
  ENTRY(
    _("OKI MSM5232"), {
      CH(DIV_SYSTEM_MSM5232, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("NEC D65010G031"), {
      CH(DIV_SYSTEM_PV1000, 1.0f, 0, "")
    }
  );
  ENTRY(
    _("MOS Technology TED"), {
      CH(DIV_SYSTEM_TED, 1.0f, 0, "clockSel=1")
    }
  );
  CATEGORY_END;
}
