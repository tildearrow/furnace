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

#include "gui.h"
#include "image.h"
#include "../ta-log.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_STDIO
#include "stb_image.h"

const unsigned char* imageData[GUI_IMAGE_MAX]={
  image_icon_data,
#ifdef NO_INTRO
  NULL, NULL, NULL,
  NULL, NULL, NULL
#else
  image_talogo_data,
  image_tachip_data,
  image_logo_data,
  image_wordmark_data,
  image_introbg_data,
  image_pat_data
#endif
};

const unsigned int imageLen[GUI_IMAGE_MAX]={
  image_icon_size,
#ifdef NO_INTRO
  0, 0, 0,
  0, 0, 0
#else
  image_talogo_size,
  image_tachip_size,
  image_logo_size,
  image_wordmark_size,
  image_introbg_size,
  image_pat_size
#endif
};

FurnaceGUITexture* FurnaceGUI::getTexture(FurnaceGUIImages image, FurnaceGUIBlendMode blendMode) {
  FurnaceGUIImage* img=getImage(image);

  if (img==NULL) return NULL;
  if (img->data==NULL) return NULL;
  if (img->width<=0 || img->height<=0) return NULL;

  bool createTex=false;

  if (img->tex==NULL) {
    createTex=true;
  } else {
    if (!rend->isTextureValid(img->tex)) {
      rend->destroyTexture(img->tex);
      img->tex=NULL;
      createTex=true;
    }
  }

  if (createTex) {
    img->tex=rend->createTexture(false,img->width,img->height,true,bestTexFormat);
    if (img->tex==NULL) {
      logE("error while creating image %d texture! %s",(int)image,SDL_GetError());
      return NULL;
    }
    rend->setTextureBlendMode(img->tex,blendMode);

    if (!rend->updateTexture(img->tex,img->data,img->width*4)) {
      logE("error while updating texture of image %d! %s",(int)image,SDL_GetError());
    }
  }

  return img->tex;
}

FurnaceGUIImage* FurnaceGUI::getImage(FurnaceGUIImages image) {
  FurnaceGUIImage* ret=NULL;
  auto retPos=images.find(image);
  if (retPos!=images.cend()) {
    ret=retPos->second;
  } else {
    ret=new FurnaceGUIImage;
    logV("loading image %d to pool.",(int)image);
    ret->data=stbi_load_from_memory(imageData[image],imageLen[image],&ret->width,&ret->height,&ret->ch,STBI_rgb_alpha);

    if (ret->data==NULL) {
      logE("could not load image %d!",(int)image);
      delete ret;
      return NULL;
    }

    logV("%dx%d",ret->width,ret->height);

#ifdef TA_BIG_ENDIAN
    if (ret->ch==4) {
      size_t total=ret->width*ret->height*ret->ch;
      for (size_t i=0; i<total; i+=4) {
        ret->data[i]^=ret->data[i|3];
        ret->data[i|3]^=ret->data[i];
        ret->data[i]^=ret->data[i|3];
        ret->data[i|1]^=ret->data[i|2];
        ret->data[i|2]^=ret->data[i|1];
        ret->data[i|1]^=ret->data[i|2];
      }
    }
#endif

    if (ret->ch==4) {
      size_t total=ret->width*ret->height*ret->ch;
      switch (bestTexFormat) {
        case GUI_TEXFORMAT_ARGB32:
          for (size_t i=0; i<total; i+=4) {
            ret->data[i]^=ret->data[i|2];
            ret->data[i|2]^=ret->data[i];
            ret->data[i]^=ret->data[i|2];
          }
          break;
        case GUI_TEXFORMAT_BGRA32:
          for (size_t i=0; i<total; i+=4) {
            ret->data[i]^=ret->data[i|3];
            ret->data[i|3]^=ret->data[i];
            ret->data[i]^=ret->data[i|3];
            ret->data[i|1]^=ret->data[i|2];
            ret->data[i|2]^=ret->data[i|1];
            ret->data[i|1]^=ret->data[i|2];
            ret->data[i|1]^=ret->data[i|3];
            ret->data[i|3]^=ret->data[i|1];
            ret->data[i|1]^=ret->data[i|3];
          }
          break;
        case GUI_TEXFORMAT_RGBA32:
          for (size_t i=0; i<total; i+=4) {
            ret->data[i]^=ret->data[i|3];
            ret->data[i|3]^=ret->data[i];
            ret->data[i]^=ret->data[i|3];
            ret->data[i|1]^=ret->data[i|2];
            ret->data[i|2]^=ret->data[i|1];
            ret->data[i|1]^=ret->data[i|2];
          }
          break;
        default:
          break;
      }
    }

    images[image]=ret;
  }

  // warning silencers
  stbi__addints_valid(2,2);
  stbi__mul2shorts_valid(2,2);

  return ret;
}

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