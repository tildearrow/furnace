//  ---------------------------------------------------------------------------
//  This file is part of reSID, a MOS6581_2 SID2 emulator engine.
//  Copyright (C) 2004  Dag Lem <resid@nimrod.no>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//  ---------------------------------------------------------------------------

#define __VOICE_CC__
#include "voice.h"

// ----------------------------------------------------------------------------
// Constructor.
// ----------------------------------------------------------------------------
Voice2::Voice2()
{
  set_chip_model(MOS6581_2);
}

// ----------------------------------------------------------------------------
// Set chip model.
// ----------------------------------------------------------------------------
void Voice2::set_chip_model(chip_model2 model)
{
  wave.set_chip_model(model);
    wave_zero = 0x800;
    voice_DC = 0;
}

// ----------------------------------------------------------------------------
// Set sync source.
// ----------------------------------------------------------------------------
void Voice2::set_sync_source(Voice2* source)
{
  wave.set_sync_source(&source->wave);
}

// ----------------------------------------------------------------------------
// Register functions.
// ----------------------------------------------------------------------------
void Voice2::writeCONTROL_REG(reg8 control)
{
  wave.writeCONTROL_REG(control);
  envelope.writeCONTROL_REG(control);
}

// ----------------------------------------------------------------------------
// SID2 reset.
// ----------------------------------------------------------------------------
void Voice2::reset()
{
  wave.reset();
  envelope.reset();
}
