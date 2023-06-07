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
 */

#include "gui.h"
#include "../ta-log.h"
#ifdef HAVE_RENDER_SDL
#include "render/renderSDL.h"
#endif
#ifdef HAVE_RENDER_GL
#include "render/renderGL.h"
#endif

bool FurnaceGUI::initRender() {
  if (rend!=NULL) return false;

  if (settings.renderBackend=="OpenGL") {
    renderBackend=GUI_BACKEND_GL;
  } else if (settings.renderBackend=="SDL") {
    renderBackend=GUI_BACKEND_SDL;
  } else {
    renderBackend=GUI_BACKEND_DEFAULT;
  }
  
  switch (renderBackend) {
#ifdef HAVE_RENDER_GL
    case GUI_BACKEND_GL:
      logI("render backend: OpenGL");
      rend=new FurnaceGUIRenderGL;
      break;
#endif
#ifdef HAVE_RENDER_SDL
    case GUI_BACKEND_SDL:
      logI("render backend: SDL_Renderer");
      rend=new FurnaceGUIRenderSDL;
      break;
#endif
    default:
      logE("invalid render backend!");
      return false;
      break;
  }
  
  return true;
}

bool FurnaceGUI::quitRender() {
  if (rend==NULL) return false;
  bool ret=rend->quit();
  delete rend;
  rend=NULL;
  return ret;
}
