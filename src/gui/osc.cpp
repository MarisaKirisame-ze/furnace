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
#include "imgui_internal.h"
#include <imgui.h>
#include <math.h>

void FurnaceGUI::readOsc() {
  int writePos=e->oscWritePos;
  int readPos=e->oscReadPos;
  int avail=0;
  int total=0;
  if (writePos>=readPos) {
    avail=writePos-readPos;
  } else {
    avail=writePos-readPos+32768;
  }
  if (oscTotal==0) {
    oscTotal=ImGui::GetIO().DeltaTime*e->getAudioDescGot().rate;
  } else {
    oscTotal=(oscTotal+(int)round(ImGui::GetIO().DeltaTime*e->getAudioDescGot().rate))>>1;
  }
  int bias=avail-oscTotal-e->getAudioDescGot().bufsize;
  if (bias<0) bias=0;
  total=oscTotal+(bias>>6);
  if (total>avail) total=avail;
  //printf("total: %d. avail: %d bias: %d\n",total,avail,bias);
  for (int i=0; i<512; i++) {
    int pos=(readPos+(i*total/512))&0x7fff;
    oscValues[i]=(e->oscBuf[0][pos]+e->oscBuf[1][pos])*0.5f;
  }

  float peakDecay=0.05f*60.0f*ImGui::GetIO().DeltaTime;
  for (int i=0; i<2; i++) {
    peak[i]*=1.0-peakDecay;
    if (peak[i]<0.0001) peak[i]=0.0;
    float newPeak=peak[i];
    for (int j=0; j<total; j++) {
      int pos=(readPos+j)&0x7fff;
      if (fabs(e->oscBuf[i][pos])>newPeak) {
        newPeak=fabs(e->oscBuf[i][pos]);
      }
    }
    peak[i]+=(newPeak-peak[i])*0.9;
  }

  readPos=(readPos+total)&0x7fff;
  e->oscReadPos=readPos;
}

void FurnaceGUI::drawOsc() {
  if (nextWindow==GUI_WINDOW_OSCILLOSCOPE) {
    oscOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!oscOpen) return;
  ImGui::SetNextWindowSizeConstraints(ImVec2(64.0f*dpiScale,32.0f*dpiScale),ImVec2(scrW*dpiScale,scrH*dpiScale));
  /*ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,ImVec2(0,0));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,ImVec2(0,0));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing,ImVec2(0,0));*/
  if (ImGui::Begin("Oscilloscope",&oscOpen)) {
    if (oscZoomSlider) {
      if (ImGui::VSliderFloat("##OscZoom",ImVec2(20.0f*dpiScale,ImGui::GetContentRegionAvail().y),&oscZoom,0.5,2.0)) {
        if (oscZoom<0.5) oscZoom=0.5;
        if (oscZoom>2.0) oscZoom=2.0;
      }
      ImGui::SameLine();
    }

    ImDrawList* dl=ImGui::GetWindowDrawList();
    ImGuiWindow* window=ImGui::GetCurrentWindow();

    ImVec2 waveform[512];
    ImVec2 size=ImGui::GetContentRegionAvail();

    ImVec2 minArea=window->DC.CursorPos;
    ImVec2 maxArea=ImVec2(
      minArea.x+size.x,
      minArea.y+size.y
    );
    ImRect rect=ImRect(minArea,maxArea);
    ImGuiStyle& style=ImGui::GetStyle();
    ImU32 color=ImGui::GetColorU32(isClipping?uiColors[GUI_COLOR_OSC_WAVE_PEAK]:uiColors[GUI_COLOR_OSC_WAVE]);
    ImU32 borderColor=ImGui::GetColorU32(uiColors[GUI_COLOR_OSC_BORDER]);
    ImGui::ItemSize(size,style.FramePadding.y);
    if (ImGui::ItemAdd(rect,ImGui::GetID("wsDisplay"))) {
      // https://github.com/ocornut/imgui/issues/3710
      // TODO: improve
      const int v0 = dl->VtxBuffer.Size;
      dl->AddRectFilled(rect.Min,rect.Max,0xffffffff,8.0f*dpiScale);
      const int v1 = dl->VtxBuffer.Size;

      for (int i=v0; i<v1; i++) {
        ImDrawVert* v=&dl->VtxBuffer.Data[i];
        ImVec4 col0=uiColors[GUI_COLOR_OSC_BG1];
        ImVec4 col1=uiColors[GUI_COLOR_OSC_BG3];
        ImVec4 col2=uiColors[GUI_COLOR_OSC_BG2];
        ImVec4 col3=uiColors[GUI_COLOR_OSC_BG4];
        
        float shadeX=(v->pos.x-rect.Min.x)/(rect.Max.x-rect.Min.x);
        float shadeY=(v->pos.y-rect.Min.y)/(rect.Max.y-rect.Min.y);
        if (shadeX<0.0f) shadeX=0.0f;
        if (shadeX>1.0f) shadeX=1.0f;
        if (shadeY<0.0f) shadeY=0.0f;
        if (shadeY>1.0f) shadeY=1.0f;

        col0.x+=(col2.x-col0.x)*shadeX;
        col0.y+=(col2.y-col0.y)*shadeX;
        col0.z+=(col2.z-col0.z)*shadeX;
        col0.w+=(col2.w-col0.w)*shadeX;

        col1.x+=(col3.x-col1.x)*shadeX;
        col1.y+=(col3.y-col1.y)*shadeX;
        col1.z+=(col3.z-col1.z)*shadeX;
        col1.w+=(col3.w-col1.w)*shadeX;

        col0.x+=(col1.x-col0.x)*shadeY;
        col0.y+=(col1.y-col0.y)*shadeY;
        col0.z+=(col1.z-col0.z)*shadeY;
        col0.w+=(col1.w-col0.w)*shadeY;

        v->col=ImGui::ColorConvertFloat4ToU32(col0);
      }

      for (size_t i=0; i<512; i++) {
        float x=(float)i/512.0f;
        float y=oscValues[i]*oscZoom;
        if (y<-0.5f) y=-0.5f;
        if (y>0.5f) y=0.5f;
        waveform[i]=ImLerp(rect.Min,rect.Max,ImVec2(x,0.5f-y));
      }
      dl->AddPolyline(waveform,512,color,ImDrawFlags_None,dpiScale);
      dl->AddRect(rect.Min,rect.Max,borderColor,8.0f*dpiScale,0,dpiScale);
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
      oscZoomSlider=!oscZoomSlider;
    }
  }
  //ImGui::PopStyleVar(3);
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_OSCILLOSCOPE;
  ImGui::End();
}
