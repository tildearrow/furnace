/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2023 tildearrow and contributors
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
 *
 * this license only applies to the code. for the license of each font used,
 * see `papers/`.
 */

#include "gui.h"
#include "image.h"
#include "../ta-log.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_STDIO
#include "stb_image.h"

const unsigned char* imageData[GUI_IMAGE_MAX]={
  image_icon_data
};

const unsigned int imageLen[GUI_IMAGE_MAX]={
  image_icon_size
};

const FurnaceGUIImage* FurnaceGUI::getImage(FurnaceGUIImages image) {
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

    images[image]=ret;
  }

  // warning silencers
  stbi__addints_valid(2,2);
  stbi__mul2shorts_valid(2,2);

  return ret;
}
