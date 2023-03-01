/*
 * This file is part of a modification to libsidplayfp, a SID player engine.
 * The applied modification fixes compilation under recent version of GCC,
 * which have tougher warnings against memory access problems.
 *
 *  Copyright (C) 2011-2014 Leandro Nini
 *  Copyright (C) 2023 tildearrow
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "array.h"

std::mutex counterOps;
