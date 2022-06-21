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

#include "gui.h"
#include "imgui.h"
#include <math.h>

void Gradient2D::render() {
  ImU32* g=grad.get();
  ImU32 bgColorU=ImGui::ColorConvertFloat4ToU32(bgColor);

  // 1. fill with background color
  for (size_t i=0; i<width*height; i++) {
    g[i]=bgColorU;
  }

  // 2. insert points
  for (Gradient2DPoint& i: points) {
    float pDistSquared=i.distance*i.distance;
    printf("shading this point %f %f\n",i.x,i.y);
    for (size_t j=0; j<height; j++) {
      float jFloat=(float)j/(float)height;
      float distY=jFloat-i.y;
      for (size_t k=0; k<width; k++) {
        float kFloat=(float)k/(float)width;
        float distX=kFloat-i.x;
        float distSquared=(distX*distX)+(distY*distY);

        if (distSquared>=pDistSquared) continue;

        float dist=(1.0-(sqrt(distSquared)/i.distance))-i.spread;
        if (dist<0) dist=0;
        if (dist>1) dist=1;

        ImU32 shadeColor=ImGui::ColorConvertFloat4ToU32(
          ImVec4(
            i.color.x*i.color.w*dist,
            i.color.y*i.color.w*dist,
            i.color.z*i.color.w*dist,
            1.0f
          )
        );

        ImU32 origColor=g[j*width+k];
        g[j*width+k]=(
          (MIN(  0xff,    (origColor&0xff)  +  (shadeColor&0xff)  )) | // R
          (MIN( 0xff00,  (origColor&0xff00) + (shadeColor&0xff00) )) | // G
          (MIN(0xff0000,(origColor&0xff0000)+(shadeColor&0xff0000))) | // B
          (origColor&0xff000000)                                       // A
        );
      }
    }
  }
}