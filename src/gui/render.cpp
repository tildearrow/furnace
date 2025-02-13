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
#include "../ta-log.h"
#ifdef HAVE_RENDER_SDL
#include "render/renderSDL.h"
#endif
#ifdef HAVE_RENDER_GL
#include "render/renderGL.h"
#endif
#ifdef HAVE_RENDER_GL1
#include "render/renderGL1.h"
#endif
#ifdef HAVE_RENDER_DX11
#include "render/renderDX11.h"
#endif
#ifdef HAVE_RENDER_DX9
#include "render/renderDX9.h"
#endif
#ifdef HAVE_RENDER_METAL
#include "render/renderMetal.h"
#endif
#include "render/renderSoftware.h"

bool FurnaceGUI::initRender() {
  if (rend!=NULL) return false;

  logV("requested backend: %s",settings.renderBackend);

  if (safeMode) {
    renderBackend=GUI_BACKEND_SOFTWARE;
  } else if (settings.renderBackend=="OpenGL" || settings.renderBackend=="OpenGL 3.0" || settings.renderBackend=="OpenGL ES 2.0") {
    renderBackend=GUI_BACKEND_GL3;
  } else if (settings.renderBackend=="OpenGL 2.0") {
    renderBackend=GUI_BACKEND_GL2;
  } else if (settings.renderBackend=="OpenGL 1.1") {
    renderBackend=GUI_BACKEND_GL1;
  } else if (settings.renderBackend=="DirectX 11") {
    renderBackend=GUI_BACKEND_DX11;
  } else if (settings.renderBackend=="DirectX 9") {
    renderBackend=GUI_BACKEND_DX9;
  } else if (settings.renderBackend=="Metal") {
    renderBackend=GUI_BACKEND_METAL;
  } else if (settings.renderBackend=="SDL") {
    renderBackend=GUI_BACKEND_SDL;
  } else if (settings.renderBackend=="Software") {
    renderBackend=GUI_BACKEND_SOFTWARE;
  } else {
    renderBackend=GUI_BACKEND_DEFAULT;
  }
  
  switch (renderBackend) {
#ifdef HAVE_RENDER_GL
#ifdef USE_GLES
    case GUI_BACKEND_GL3:
    case GUI_BACKEND_GL2:
      logI("render backend: OpenGL ES 2.0");
      rend=new FurnaceGUIRenderGL;
      ((FurnaceGUIRenderGL*)rend)->setVersion(3);
      break;
#else
    case GUI_BACKEND_GL3:
      logI("render backend: OpenGL 3.0");
      rend=new FurnaceGUIRenderGL;
      ((FurnaceGUIRenderGL*)rend)->setVersion(3);
      break;
    case GUI_BACKEND_GL2:
      logI("render backend: OpenGL 2.0");
      rend=new FurnaceGUIRenderGL;
      ((FurnaceGUIRenderGL*)rend)->setVersion(2);
      break;
#endif
#endif
#ifdef HAVE_RENDER_GL1
    case GUI_BACKEND_GL1:
      logI("render backend: OpenGL 1.1");
      rend=new FurnaceGUIRenderGL1;
      break;
#endif
#ifdef HAVE_RENDER_DX11
    case GUI_BACKEND_DX11:
      logI("render backend: DirectX 11");
      rend=new FurnaceGUIRenderDX11;
      break;
#endif
#ifdef HAVE_RENDER_DX9
    case GUI_BACKEND_DX9:
      logI("render backend: DirectX 9");
      rend=new FurnaceGUIRenderDX9;
      break;
#endif
#ifdef HAVE_RENDER_METAL
    case GUI_BACKEND_METAL:
      logI("render backend: Metal");
      rend=new FurnaceGUIRenderMetal;
      break;
#endif
#ifdef HAVE_RENDER_SDL
    case GUI_BACKEND_SDL:
      logI("render backend: SDL_Renderer");
      rend=new FurnaceGUIRenderSDL;
      break;
#endif
    case GUI_BACKEND_SOFTWARE:
      logI("render backend: Software");
      rend=new FurnaceGUIRenderSoftware;
      break;
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
