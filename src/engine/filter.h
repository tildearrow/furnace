/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2022 tildearrow and contributors
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

class DivFilterTables {
  public:
    static float* cubicTable;
    static float* sincTable;
    static float* sincTable8;
    static float* sincIntegralTable;

    /**
     * get a 1024x4 cubic spline table.
     * @return the table.
     */
    static float* getCubicTable();

    /**
     * get a 8192x8 one-side sine-windowed sinc table.
     * @return the table.
     */
    static float* getSincTable();

    /**
     * get a 8192x4 one-side sine-windowed sinc table.
     * @return the table.
     */
    static float* getSincTable8();

    /**
     * get a 8192x8 one-side sine-windowed sinc integral table.
     * @return the table.
     */
    static float* getSincIntegralTable();
};