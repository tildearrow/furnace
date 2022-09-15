/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2015 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
 * Copyright 2004,2010 Dag Lem
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef OPAMP_H
#define OPAMP_H

#include <memory>
#include <vector>

#include "Spline.h"

#include "sidcxx11.h"

namespace reSIDfp
{

/**
 * Find output voltage in inverting gain and inverting summer SID op-amp
 * circuits, using a combination of Newton-Raphson and bisection.
 *
 *               +---R2--+
 *               |       |
 *     vi ---R1--o--[A>--o-- vo
 *               vx
 *
 * From Kirchoff's current law it follows that
 *
 *     IR1f + IR2r = 0
 *
 * Substituting the triode mode transistor model K*W/L*(Vgst^2 - Vgdt^2)
 * for the currents, we get:
 *
 *     n*((Vddt - vx)^2 - (Vddt - vi)^2) + (Vddt - vx)^2 - (Vddt - vo)^2 = 0
 *
 * Our root function f can thus be written as:
 *
 *     f = (n + 1)*(Vddt - vx)^2 - n*(Vddt - vi)^2 - (Vddt - vo)^2 = 0
 *
 * Using substitution constants
 *
 *     a = n + 1
 *     b = Vddt
 *     c = n*(Vddt - vi)^2
 *
 * the equations for the root function and its derivative can be written as:
 *
 *     f = a*(b - vx)^2 - c - (b - vo)^2
 *     df = 2*((b - vo)*dvo - a*(b - vx))
 */
class OpAmp
{
private:
    /// Current root position (cached as guess to speed up next iteration)
    mutable double x;

    const double Vddt;
    const double vmin;
    const double vmax;

    std::unique_ptr<Spline> const opamp;

public:
    /**
     * Opamp input -> output voltage conversion
     *
     * @param opamp opamp mapping table as pairs of points (in -> out)
     * @param opamplength length of the opamp array
     * @param kVddt transistor dt parameter (in volts)
     */
    OpAmp(const std::vector<Spline::Point> &opamp, double Vddt) :
        x(0.),
        Vddt(Vddt),
        vmin(opamp.front().x),
        vmax(opamp.back().x),
        opamp(new Spline(opamp)) {}

    void reset() const
    {
        x = vmin;
    }

    /**
     * Solve the opamp equation for input vi in loading context n
     *
     * @param n the ratio of input/output loading
     * @param vi input
     * @return vo
     */
    double solve(double n, double vi) const;
};

} // namespace reSIDfp

#endif
