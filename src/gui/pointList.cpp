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
#include <imgui.h>

#define POINT_POS(_x) \
  ImVec2( \
    (float)((int)_x.x-(int)editorOffset)/(float)editorZoomX, \
    1.0-((_x.y-editorMinY)/editorMaxY) \
  )

#define POINT_RADIUS 8.0f

float* PointList::compile(size_t& length) {
  if (points.size()<1) return NULL;
  length=points[points.size()-1].x+1;
  float* ret=new float[length];

  ret[0]=points[0].y;

  for (size_t i=0; i<points.size()-1; i++) {
    Point& point=points[i];
    Point& nextPoint=points[i+1];

    unsigned int dist=nextPoint.x-point.x;
    if (dist==0) continue;

    for (unsigned int j=0, j_index=points[i].x; j<=dist; j++, j_index++) {
      ret[j_index]=point.y+(nextPoint.y-point.y)*((float)j/(float)dist);
    }
  }

  return ret;
}

void PointList::drawEditor(const char* itemID, bool frame, const ImVec2& size) {
  ImVec2 itemSize=size;
  ImGuiID id=ImGui::GetID(itemID);
  if (size.x==0.0f) {
    itemSize.x=ImGui::GetContentRegionAvail().x;
  }
  if (size.y==0.0f) {
    itemSize.y=ImGui::GetContentRegionAvail().y;
  }

  ImDrawList* dl=ImGui::GetWindowDrawList();
  ImGuiWindow* window=ImGui::GetCurrentWindow();
  ImGuiStyle& style=ImGui::GetStyle();

  ImVec2 minArea=window->DC.CursorPos;
  ImVec2 maxArea=ImVec2(
    minArea.x+itemSize.x,
    minArea.y+itemSize.y
  );
  ImRect rect=ImRect(minArea,maxArea);

  ImGui::ItemSize(itemSize,style.FramePadding.y);
  if (ImGui::ItemAdd(rect,id)) {
    bool hovered=ImGui::ItemHoverable(rect,id,0);
    int hoveredPoint=-1;
    int nearestPoint=-1;

    ImVec2 relMousePos=ImGui::GetMousePos()-minArea;
    int resultingX=editorOffset+(int)round(editorZoomX*(relMousePos.x/itemSize.x));
    float resultingY=editorMinY+(editorMaxY-editorMinY)*(1.0-(relMousePos.y/itemSize.y));

    if (resultingY<editorMinY) resultingY=editorMinY;
    if (resultingY>editorMaxY) resultingY=editorMaxY;
 
    if (frame) {
      ImGui::RenderFrame(minArea,maxArea,ImGui::GetColorU32(ImGuiCol_FrameBg),true,style.FrameRounding);
    }
    
    for (size_t i=0; i<points.size(); i++) {
      Point& point=points[i];
      ImVec2 pointPos=ImLerp(minArea,maxArea,POINT_POS(point));
      const ImVec2 mouseDist=(ImGui::GetMousePos()-pointPos);

      if (i>0) {
        Point& prevPoint=points[i-1];
        ImVec2 prevPointPos=ImLerp(minArea,maxArea,POINT_POS(prevPoint));
        dl->AddLine(prevPointPos,pointPos,0xffffffff);
      }

      dl->AddCircle(pointPos,POINT_RADIUS,0xffffffff);

      if (mouseDist.x>=0) nearestPoint=i;

      if (hovered) {
        float mouseDistSquared=mouseDist.x*mouseDist.x+mouseDist.y*mouseDist.y;
        if (mouseDistSquared<=(POINT_RADIUS*POINT_RADIUS)) {
          hoveredPoint=i;
        }
      }
    }
    
    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
      ImGui::InhibitInertialScroll();
      if (hoveredPoint!=-1) {
        selectedPoint=hoveredPoint;
      }
    }

    if (selectedPoint>=0 && selectedPoint<(int)points.size()) {
      // move the point
      if (selectedPoint==0) {
        points[selectedPoint].x=0;
      } else if (selectedPoint==(int)points.size()-1 && constrainX>0) {
        points[selectedPoint].x=constrainX;
      } else {
        points[selectedPoint].x=resultingX;
        if (points[selectedPoint].x<points[selectedPoint-1].x) points[selectedPoint].x=points[selectedPoint-1].x;
        if (selectedPoint<(int)points.size()-1) {
          if (points[selectedPoint].x>points[selectedPoint+1].x) points[selectedPoint].x=points[selectedPoint+1].x;
        }
      }

      points[selectedPoint].y=resultingY;
    } else if (hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
      // create a point
      if (nearestPoint>=0) {
        points.insert(points.begin()+nearestPoint+1,Point(resultingX,resultingY));
        selectedPoint=nearestPoint+1;
      }
    }

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
      selectedPoint=-1;
    }

    String debugText=fmt::sprintf("hoveredPoint: %d\nselectedPoint: %d\nnearestPoint: %d",hoveredPoint,selectedPoint,nearestPoint);
    if (selectedPoint>=0 && selectedPoint<(int)points.size()) {
      debugText+=fmt::sprintf("\npoint: %d, %f",points[selectedPoint].x,points[selectedPoint].y);
    }
    dl->AddText(minArea,0xffffffff,debugText.c_str());
  } else {
    selectedPoint=-1;
  }
}

void PointList::setXConstraints(unsigned int maxX) {
  constrainX=maxX;
  if (points.size()<1) {
    points.push_back((Point(maxX,0)));
  }
}

PointList::PointList():
  editorOffset(0),
  editorOffsetFine(0.0f),
  editorZoomX(100),
  editorZoomFineX(0.0f),
  editorMinY(0.0),
  editorMaxY(1.0),
  constrainX(0),
  selectedPoint(-1) {
  points.push_back(Point(0,0));
}
