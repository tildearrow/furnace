/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2025 tildearrow and contributors
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
#include <fmt/printf.h>
#include <math.h>

ImU32 Gradient2D::get(float x, float y) {
  int xi=round(x*width);
  int yi=round(y*height);
  if (xi<0) xi=0;
  if (xi>=(int)width) xi=width-1;
  if (yi<0) yi=0;
  if (yi>=(int)height) yi=height-1;
  return grad[yi*width+xi];
}

String Gradient2D::toString() {
  String ret=fmt::sprintf("GRAD #%.2X%.2X%.2X%.2X",(unsigned char)(bgColor.x*255.0f),(unsigned char)(bgColor.y*255.0f),(unsigned char)(bgColor.z*255.0f),(unsigned char)(bgColor.w*255.0f));
  for (Gradient2DPoint& i: points) {
    ret+=fmt::sprintf(" %f,%f:%f,%f:#%.2X%.2X%.2X%.2X",i.x,i.y,i.distance,i.spread,(unsigned char)(i.color.x*255.0f),(unsigned char)(i.color.y*255.0f),(unsigned char)(i.color.z*255.0f),(unsigned char)(i.color.w*255.0f));
  }
  return ret;
}

bool Gradient2D::fromString(String val) {
  std::vector<String> split;
  String cur;
  for (char i: val) {
    if (i==' ') {
      if (!cur.empty()) {
        split.push_back(cur);
        cur="";
      }
    } else {
      cur+=i;
    }
  }
  if (!cur.empty()) {
    split.push_back(cur);
  }

  if (split.size()<2) return false;

  if (split[0]!="GRAD") return false;

  ImU32 bgColorH=0;
  if (sscanf(split[1].c_str(),"#%X",&bgColorH)!=1) return false;
  bgColorH=(bgColorH>>24)|((bgColorH>>8)&0xff00)|((bgColorH<<8)&0xff0000)|(bgColorH<<24);

  bgColor=ImGui::ColorConvertU32ToFloat4(bgColorH);

  for (size_t i=2; i<split.size(); i++) {
    Gradient2DPoint point;
    ImU32 colorH=0;
    if (sscanf(split[i].c_str(),"%f,%f:%f,%f:#%X",&point.x,&point.y,&point.distance,&point.spread,&colorH)!=5) {
      return false;
    }
    colorH=(colorH>>24)|((colorH>>8)&0xff00)|((colorH<<8)&0xff0000)|(colorH<<24);

    point.color=ImGui::ColorConvertU32ToFloat4(colorH);
    points.push_back(point);
  }
  return true;
}

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
    for (size_t j=0; j<height; j++) {
      float jFloat=(float)j/(float)height;
      float distY=jFloat-i.y;
      for (size_t k=0; k<width; k++) {
        float kFloat=(float)k/(float)width;
        float distX=kFloat-i.x;
        float distSquared=(distX*distX)+(distY*distY)-(i.spread*i.spread);
        if (distSquared<0) distSquared=0;

        if (distSquared>=pDistSquared) continue;

        float dist=(1.0-(sqrt(distSquared)/i.distance));
        if (dist<0) dist=0;
        if (dist>1) dist=1;

        ImU32 shadeColor=ImGui::ColorConvertFloat4ToU32(
          // ps: multiplying dist to the color channels fixes old
          // mixing, but breaks if the bg is 0. and vice versa (not
          // multiplying fixes 0 bg mixing)
          ImVec4(
            i.color.x*i.color.w*dist,
            i.color.y*i.color.w*dist,
            i.color.z*i.color.w*dist,
            1.0f*dist // this idk
          )
        );

        ImU32 origColor=g[j*width+k];
        // note: this really breaks the color mixing if theres a background
        // and the bitshifts are necessary to avoid overflow (but prob only for alpha)
        // this needs to be redone, its a temporary proof-of-concept solution anyway
        g[j*width+k]=(
          (MIN(0xff,(origColor    &0xff)+(shadeColor    &0xff))    ) | // R
          (MIN(0xff,(origColor>> 8&0xff)+(shadeColor>> 8&0xff))<< 8) | // G
          (MIN(0xff,(origColor>>16&0xff)+(shadeColor>>16&0xff))<<16) | // B
          (MIN(0xff,(origColor>>24&0xff)+(shadeColor>>24&0xff))<<24)   // A
        );
        // ps 2: replacing this with ImAlphaBlendColors doesnt work
      }
    }
  }
}
