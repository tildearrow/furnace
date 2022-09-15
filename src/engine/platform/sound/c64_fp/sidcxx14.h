/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 *  Copyright 2014-2015 Leandro Nini
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

#ifndef SIDCXX14_H
#define SIDCXX14_H

#include "sidcxx11.h"

#define MAKE_UNIQUE(type, ...) std::make_unique<type>(__VA_ARGS__)
#define HAVE_CXX14

#endif
