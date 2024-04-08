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

#include "renderSoftware.h"
#include "imgui_sw.hpp"
#include "../../ta-log.h"

class FurnaceSoftwareTexture: public FurnaceGUITexture {
  public:
  SWTexture* tex;
  FurnaceSoftwareTexture():
    tex(NULL) {}
};

ImTextureID FurnaceGUIRenderSoftware::getTextureID(FurnaceGUITexture* which) {
  FurnaceSoftwareTexture* t=(FurnaceSoftwareTexture*)which;
  return t->tex;
}

bool FurnaceGUIRenderSoftware::lockTexture(FurnaceGUITexture* which, void** data, int* pitch) {
  FurnaceSoftwareTexture* t=(FurnaceSoftwareTexture*)which;
  if (!t->tex->managed) return false;
  *data=t->tex->pixels;
  *pitch=t->tex->width*(t->tex->isAlpha?1:4);
  return true;
}

bool FurnaceGUIRenderSoftware::unlockTexture(FurnaceGUITexture* which) {
  return true;
}

bool FurnaceGUIRenderSoftware::updateTexture(FurnaceGUITexture* which, void* data, int pitch) {
  FurnaceSoftwareTexture* t=(FurnaceSoftwareTexture*)which;
  if (!t->tex->managed) return false;
  memcpy(t->tex->pixels,data,pitch*t->tex->height);
  return true;
}

FurnaceGUITexture* FurnaceGUIRenderSoftware::createTexture(bool dynamic, int width, int height, bool interpolate) {
  FurnaceSoftwareTexture* ret=new FurnaceSoftwareTexture;
  ret->tex=new SWTexture(width,height);
  return ret;
}

bool FurnaceGUIRenderSoftware::destroyTexture(FurnaceGUITexture* which) {
  FurnaceSoftwareTexture* t=(FurnaceSoftwareTexture*)which;

  delete t->tex;
  delete t;
  return true;
}

void FurnaceGUIRenderSoftware::setTextureBlendMode(FurnaceGUITexture* which, FurnaceGUIBlendMode mode) {
  // TODO
}

void FurnaceGUIRenderSoftware::setBlendMode(FurnaceGUIBlendMode mode) {
  // TODO
}

void FurnaceGUIRenderSoftware::clear(ImVec4 color) {
  // TODO
}

bool FurnaceGUIRenderSoftware::newFrame() {
  return ImGui_ImplSW_NewFrame();
}

bool FurnaceGUIRenderSoftware::canVSync() {
  return false;
}

void FurnaceGUIRenderSoftware::createFontsTexture() {
  ImGui_ImplSW_CreateFontsTexture();
}

void FurnaceGUIRenderSoftware::destroyFontsTexture() {
  ImGui_ImplSW_DestroyFontsTexture();
}

void FurnaceGUIRenderSoftware::renderGUI() {
  ImGui_ImplSW_RenderDrawData(ImGui::GetDrawData());
}

void FurnaceGUIRenderSoftware::wipe(float alpha) {
  // TODO
}

void FurnaceGUIRenderSoftware::present() {
  // TODO?
}

bool FurnaceGUIRenderSoftware::getOutputSize(int& w, int& h) {
  SDL_Surface* surf=SDL_GetWindowSurface(sdlWin);
  if (surf==NULL) return false;
  w=surf->w;
  h=surf->h;
  return true;
}

int FurnaceGUIRenderSoftware::getWindowFlags() {
  return 0;
}

void FurnaceGUIRenderSoftware::setSwapInterval(int swapInterval) {
}

void FurnaceGUIRenderSoftware::preInit() {
}

bool FurnaceGUIRenderSoftware::init(SDL_Window* win, int swapInterval) {
  sdlWin=win;
  return true;
}

void FurnaceGUIRenderSoftware::initGUI(SDL_Window* win) {
  // hack
  ImGui_ImplSDL2_InitForMetal(win);
  ImGui_ImplSW_Init(win);
}

void FurnaceGUIRenderSoftware::quitGUI() {
  ImGui_ImplSW_Shutdown();
}

bool FurnaceGUIRenderSoftware::quit() {
  return true;
}
