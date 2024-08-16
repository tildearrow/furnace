/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2024 tildearrow and contributors
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

#define _USE_MATH_DEFINES
#include "gui.h"
#include "../ta-log.h"
#include "imgui_internal.h"
#include <fmt/printf.h>

void FurnaceGUI::drawImage(ImDrawList* dl, FurnaceGUIImages image, const ImVec2& pos, const ImVec2& scale, double rotate, const ImVec2& uvMin, const ImVec2& uvMax, const ImVec4& imgColor) {
  FurnaceGUIImage* imgI=getImage(image);
  FurnaceGUITexture* img=getTexture(image);

  if (img==NULL) return;

  float squareSize=MAX(introMax.x-introMin.x,introMax.y-introMin.y);
  float uDiff=uvMax.x-uvMin.x;
  float vDiff=uvMax.y-uvMin.y;

  ImVec2 rectMin=ImVec2(
    -imgI->width*0.5*scale.x*(squareSize/2048.0)*uDiff,
    -imgI->height*0.5*scale.y*(squareSize/2048.0)*vDiff
  );
  ImVec2 rectMax=ImVec2(
    -rectMin.x,
    -rectMin.y
  );

  ImVec2 posAbs=ImLerp(
    ImVec2(introMin.x+((introMax.x-introMin.x)-squareSize)*0.5,introMin.y+((introMax.y-introMin.y)-squareSize)*0.5),
    ImVec2(introMin.x+((introMax.x-introMin.x)+squareSize)*0.5,introMin.y+((introMax.y-introMin.y)+squareSize)*0.5),
    pos
  );

  ImVec2 quad0=ImVec2(
    posAbs.x+rectMin.x*cos(rotate)-rectMin.y*sin(rotate),
    posAbs.y+rectMin.x*sin(rotate)+rectMin.y*cos(rotate)
  );
  ImVec2 quad1=ImVec2(
    posAbs.x+rectMax.x*cos(rotate)-rectMin.y*sin(rotate),
    posAbs.y+rectMax.x*sin(rotate)+rectMin.y*cos(rotate)
  );
  ImVec2 quad2=ImVec2(
    posAbs.x+rectMax.x*cos(rotate)-rectMax.y*sin(rotate),
    posAbs.y+rectMax.x*sin(rotate)+rectMax.y*cos(rotate)
  );
  ImVec2 quad3=ImVec2(
    posAbs.x+rectMin.x*cos(rotate)-rectMax.y*sin(rotate),
    posAbs.y+rectMin.x*sin(rotate)+rectMax.y*cos(rotate)
  );

  float uScale=rend->getTextureU(img);
  float vScale=rend->getTextureV(img);

  ImVec2 uv0=ImVec2(uvMin.x*uScale,uvMin.y*vScale);
  ImVec2 uv1=ImVec2(uvMax.x*uScale,uvMin.y*vScale);
  ImVec2 uv2=ImVec2(uvMax.x*uScale,uvMax.y*vScale);
  ImVec2 uv3=ImVec2(uvMin.x*uScale,uvMax.y*vScale);

  ImU32 colorConverted=ImGui::GetColorU32(imgColor);

  dl->AddImageQuad(rend->getTextureID(img),quad0,quad1,quad2,quad3,uv0,uv1,uv2,uv3,colorConverted);
}

void FurnaceGUI::endIntroTune() {
  if (introStopped) return;
  logV("ending intro");
  stop();
  if (curFileName.empty()) {
    e->createNewFromDefaults();
  } else { // load pending song
    if (load(curFileName)>0) {
      showError(fmt::sprintf(_("Error while loading file! (%s)"),lastError));
      curFileName="";
      e->createNewFromDefaults();
    }
  }
  undoHist.clear();
  redoHist.clear();
  modified=false;
  curNibble=false;
  orderNibble=false;
  orderCursor=-1;
  samplePos=0;
  updateSampleTex=true;
  selStart=SelectionPoint();
  selEnd=SelectionPoint();
  cursor=SelectionPoint();
  updateWindowTitle();
  updateScroll(0);
  introStopped=true;
}

void FurnaceGUI::drawIntro(double introTime, bool monitor) {
  if (monitor) {
    return;
  }
  if (introPos<(shortIntro?1.0:11.0) || monitor) {
    if (!monitor) {
      WAKE_UP;
      nextWindow=GUI_WINDOW_NOTHING;
      ImGui::SetNextWindowPos(ImVec2(0,0));
      ImGui::SetNextWindowSize(ImVec2(canvasW,canvasH));
      if (introPos<0.1) ImGui::SetNextWindowFocus();
    }
    if (ImGui::Begin(monitor?"IntroMon X":"Intro",NULL,monitor?globalWinFlags:(ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoDocking|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoBackground),monitor?_("IntroMon X"):_("Intro"))) {
      if (monitor) {
        if (ImGui::Button(_("Preview"))) {
          introPos=0;
          tutorial.introPlayed=false;
          shortIntro=false;
          introSkipDo=false;
          introSkip=0.0;
          e->setOrder(0);
          e->setRepeatPattern(false);
          play();
        }
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0,introTime<10.0?1.0:0.0,introTime<10.0?1.0:0.0,1.0),"%.2f",introTime);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::ProgressBar(introTime/11.0,ImVec2(-FLT_MIN,0),"##IntroP");
      }

      ImDrawList* dl=monitor?ImGui::GetWindowDrawList():ImGui::GetForegroundDrawList();
      ImVec2 top=monitor?ImGui::GetCursorScreenPos():ImVec2(0.0f,0.0f);
      ImVec2 bottom=monitor?ImGui::GetContentRegionAvail():ImVec2(canvasW,canvasH);
      if (monitor) {
        bottom.x+=top.x;
        bottom.y+=top.y;
      }

      introMin=top;
      introMax=bottom;

      if (monitor) dl->PushClipRect(top,bottom);

      if (shortIntro && !monitor) {
        // background
        float bgAlpha=CLAMP((1.0-introTime)*4.0,0.0,1.0);
        bgAlpha=3.0*pow(bgAlpha,2.0)-2.0*pow(bgAlpha,3.0);
        ImU32 bgColor=ImGui::GetColorU32(ImVec4(0.0f,0.0f,0.0f,bgAlpha));

        dl->AddRectFilled(top,bottom,bgColor);

        drawImage(dl,GUI_IMAGE_INTROBG,ImVec2(0.125,0.25-(introTime)*0.05),ImVec2(18.0,18.0),0.0,ImVec2(0.0,0.0),ImVec2(1.0,1.0),ImVec4(0.4,0.8,1.0,0.4*CLAMP(introTime*2.0,0.0,1.0)*bgAlpha));
        drawImage(dl,GUI_IMAGE_INTROBG,ImVec2(0.4,0.25-(introTime)*0.08),ImVec2(18.0,18.0),0.0,ImVec2(0.0,0.0),ImVec2(1.0,1.0),ImVec4(0.1,0.2,1.0,0.2*CLAMP(introTime*3.0,0.0,1.0)*bgAlpha));
        drawImage(dl,GUI_IMAGE_INTROBG,ImVec2(0.7,0.25-(introTime)*0.03),ImVec2(20.0,20.0),0.0,ImVec2(0.0,0.0),ImVec2(1.0,1.0),ImVec4(0.7,1.0,0.7,0.1*CLAMP(introTime*3.0,0.0,1.0)*bgAlpha));

        drawImage(dl,GUI_IMAGE_LOGO,ImVec2(0.5,0.5+pow(1.0-CLAMP(introTime*2.0,0.0,1.0),4.0)*0.125),ImVec2(0.67,0.67),0.0f,ImVec2(0.0,0.0),ImVec2(1.0,1.0),ImVec4(1.0,1.0,1.0,CLAMP(introTime*3.0,0.0,1.0)*bgAlpha));
      } else {
        // preload textures
        getTexture(GUI_IMAGE_TALOGO);
        getTexture(GUI_IMAGE_TACHIP);
        getTexture(GUI_IMAGE_LOGO);
        getTexture(GUI_IMAGE_INTROBG,GUI_BLEND_MODE_ADD);

        if (monitor) {
          ImVec2 textPos=ImLerp(top,bottom,ImVec2(0.5,0.5));
          textPos.x-=ImGui::CalcTextSize(_("SORRY  NOTHING")).x*0.5;
          textPos.y-=ImGui::CalcTextSize(_("SORRY  NOTHING")).y*0.5;
          dl->AddText(textPos,ImGui::GetColorU32(uiColors[GUI_COLOR_TEXT]),_("SORRY  NOTHING"));
        }

        if (introSkip<0.5 || monitor) {
          // background
          float bgAlpha=CLAMP(11.0-introTime,0.0,1.0);
          bgAlpha=3.0*pow(bgAlpha,2.0)-2.0*pow(bgAlpha,3.0);
          ImU32 bgColor=ImGui::GetColorU32(ImVec4(0.0f,0.0f,0.0f,bgAlpha));

          dl->AddRectFilled(top,bottom,bgColor);

          // part 1 - talogo
          if (introTime<2.3) {
            drawImage(dl,GUI_IMAGE_TALOGO,ImVec2(0.5,0.5),ImVec2(0.67,0.67),0.0f,ImVec2(0.0,0.0),ImVec2(1.0,1.0),ImVec4(1.0,1.0,1.0,MAX(0.01,1.0-pow(MAX(0.0,1.0-introTime*2.0),3.0))));

            for (int i=0; i<16; i++) {
              double chipCenter=0.22+pow(MAX(0.0,1.5-introTime*0.8-((double)i/36.0)),2.0)+pow(sin(-introTime*2.2-(double)i*0.44),24)*0.05;
              ImVec2 chipPos=ImVec2(
                0.5+chipCenter*cos(2.0*M_PI*(double)i/16.0-pow(introTime,2.2)),
                0.5+chipCenter*sin(2.0*M_PI*(double)i/16.0-pow(introTime,2.2))
              );
              drawImage(dl,GUI_IMAGE_TACHIP,chipPos,ImVec2(0.25,0.25),0.0f,ImVec2(0.0,0.0),ImVec2(1.0,0.5),ImVec4(1.0,1.0,1.0,1.0));
            }
          }

          // background after part 1
          if (introTime>2.3) {
            float s1a=CLAMP((introTime-3.2)*1.3,0.0f,1.0f);
            float s2a=CLAMP((introTime-4.5)*1.0,0.0f,1.0f);
            float addition=(3*pow(s1a,2)-2*pow(s1a,3)+(3*pow(s2a,2)-2*pow(s2a,3))*1.5)*3.5;
            drawImage(dl,GUI_IMAGE_INTROBG,ImVec2(0.125,0.25-(introTime+addition)*0.05),ImVec2(18.0,18.0),0.0,ImVec2(0.0,0.0),ImVec2(1.0,1.0),ImVec4(0.2,0.1+0.7*s1a,1.0*s1a,0.5*bgAlpha));
            drawImage(dl,GUI_IMAGE_INTROBG,ImVec2(0.4,0.25-(introTime+addition)*0.08),ImVec2(18.0,18.0),0.0,ImVec2(0.0,0.0),ImVec2(1.0,1.0),ImVec4(0.1*s1a,0.2+0.4*s1a,1.0*s1a,0.6*bgAlpha));
            drawImage(dl,GUI_IMAGE_INTROBG,ImVec2(0.7,0.25-(introTime+addition)*0.03),ImVec2(20.0,20.0),0.0,ImVec2(0.0,0.0),ImVec2(1.0,1.0),ImVec4(0.2+0.5*s1a,0.2,1.0,(0.5-0.4*s1a)*bgAlpha));
          }

          const double fallPatX[]={
            0.0,
            272.0,
            470.0,
            742.0,
            1013.0
          };

          // part 2 - falling patterns
          if (introTime>2.3 && introTime<4.5) {
            for (int i=0; i<48; i++) {
              ImVec2 uv0=ImVec2(
                fallPatX[i&3],
                (double)(i>>2)*36.0
              );
              ImVec2 uv1=ImVec2(
                fallPatX[1+(i&3)]-2.0,
                uv0.y+36.0
              );
              uv0.x/=1024.0;
              uv0.y/=512.0;
              uv1.x/=1024.0;
              uv1.y/=512.0;

              bool left=(i%6)>=3;
              double t=(introTime-2.5)*(0.77+(cos(i+7)*0.05));
              double alteration=pow(t,0.4)-(0.55*t)+0.55*pow(t,6.0);

              drawImage(dl,GUI_IMAGE_PAT,ImVec2((left?0:1)+sin(cos(i*3.67))*0.35+((alteration+((introTime-2.3)*(0.08*(double)(1+(i&3)))))*(left?1.0:-1.0)),0.5+sin(i*6.74)*0.3-pow(CLAMP(introTime-3.0,0.0,1.0),4.0)*1.5),ImVec2(1.5,1.5),0.0f,uv0,uv1,ImVec4(1.0,1.0,1.0,1.0));
            }
          }

          // transition
          float transitionPos=CLAMP(introTime*4.0-8,-1.5,3.5);
          dl->AddQuadFilled(
            ImVec2(
              top.x+(transitionPos-1.5)*(bottom.x-top.x),
              top.y
            ),
            ImVec2(
              top.x+(transitionPos)*(bottom.x-top.x),
              top.y
            ),
            ImVec2(
              top.x+(transitionPos-0.2)*(bottom.x-top.x),
              bottom.y
            ),
            ImVec2(
              top.x+(transitionPos-1.7)*(bottom.x-top.x),
              bottom.y
            ),
            ImGui::GetColorU32(ImVec4(0.35,0.4,0.5,1.0))
          );

          // part 3 - falling chips
          if (introTime>3.0 && introTime<6.0) {
            for (int i=0; i<40; i++) {
              float blah=(introTime-4.25)*1.3;
              ImVec2 chipPos=ImVec2(
                0.5+sin(i)*0.4,
                0.1-(1.1*pow(blah,2.0)-1.3*pow(blah,2.0)+pow(blah,5.0))+i*0.02+((introTime-3.75)*1.3*(fabs(sin(i*1.3))*0.28))
              );
              drawImage(dl,GUI_IMAGE_TACHIP,chipPos,ImVec2(0.33,0.33),0.5*M_PI,ImVec2(0.0,0.0),ImVec2(1.0,0.5),ImVec4(1.0,1.0,1.0,1.0));
            }
          }

          // part 4 - logo end
          if (introTime>5.0) {
            drawImage(
              dl,
              GUI_IMAGE_WORDMARK,
              ImVec2(0.36+0.3*(1.0-pow(1.0-CLAMP(introTime-6.0,0.0,1.0),6.0)),0.5+pow(1.0-CLAMP(introTime-5.0,0.0,1.0),4.0)),
              ImVec2(1.0,1.0),
              0.0f,
              ImVec2(pow(1.0-CLAMP(introTime-6.0,0.0,1.0),8.0),0.0),
              ImVec2(1.0,1.0),
              ImVec4(1.0,1.0,1.0,bgAlpha)
            );
            drawImage(dl,GUI_IMAGE_LOGO,ImVec2(0.5-0.25*(1.0-pow(1.0-CLAMP(introTime-6.0,0.0,1.0),6.0)),0.5+pow(1.0-CLAMP(introTime-5.0,0.0,1.0),4.0)),ImVec2(0.67,0.67),0.0f,ImVec2(0.0,0.0),ImVec2(1.0,1.0),ImVec4(1.0,1.0,1.0,bgAlpha));
          }
        }

        // intro skip fade
        if (!monitor) {
          if (introSkipDo) {
            introSkip+=ImGui::GetIO().DeltaTime;
            if (introSkip>=0.5) {
              if (!shortIntro) endIntroTune();
              introPos=0.1;
              if (introSkip>=0.75) introPos=12.0;
            }
          } else {
            introSkip-=ImGui::GetIO().DeltaTime*4.0f;
            if (introSkip<0.0) introSkip=0.0;
          }

          dl->AddRectFilled(top,bottom,ImGui::GetColorU32(ImVec4(0.0,0.0,0.0,CLAMP(introSkip*2.0,0.0,1.0)-CLAMP((introSkip-0.5)*4,0.0,1.0))));
          if (introSkip<0.5) dl->AddText(ImVec2(8.0*dpiScale,8.0*dpiScale),ImGui::GetColorU32(ImVec4(1.0,1.0,1.0,CLAMP(introSkip*8.0,0.0,1.0))),_("hold to skip"));
        }
      }

      if (monitor) dl->PopClipRect();

      // workaround to texture issue
      dl->AddText(ImVec2(bottom.x-1,bottom.y-1),ImGui::ColorConvertFloat4ToU32(ImVec4(0.0,0.0,0.1,0.01)),"A");
    }
    ImGui::End();

    if (mustClear<=0 && !monitor) {
      if (!shortIntro && introPos<=0.0) {
        e->setOrder(0);
        e->setRepeatPattern(false);
        play();
      }
      if (introPos>=10.0 && !shortIntro) endIntroTune();
      introPos+=ImGui::GetIO().DeltaTime;
      if (introPos>=(shortIntro?1.0:11.0)) {
        introPos=12.0;
        tutorial.introPlayed=true;
        commitTutorial();
      }
    }
  } else if (!shortIntro) {
    endIntroTune();
  }
}
