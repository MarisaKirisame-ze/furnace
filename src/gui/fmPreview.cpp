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

#define _USE_MATH_DEFINES
#include "gui.h"
#include "../../extern/opn/ym3438.h"
#include "../../extern/opm/opm.h"
#include "../../extern/opl/opl3.h"
#include "../../extern/Nuked-OPLL/opll.h"
#include "../engine/platform/sound/ymfm/ymfm_opz.h"

#define OPN_WRITE(addr,val) \
  OPN2_Write((ym3438_t*)fmPreviewOPN,0,(addr)); \
  do { \
    OPN2_Clock((ym3438_t*)fmPreviewOPN,out); \
  } while (((ym3438_t*)fmPreviewOPN)->write_busy); \
  OPN2_Write((ym3438_t*)fmPreviewOPN,1,(val)); \
  do { \
    OPN2_Clock((ym3438_t*)fmPreviewOPN,out); \
  } while (((ym3438_t*)fmPreviewOPN)->write_busy);

const unsigned char dtTableFMP[8]={
  7,6,5,0,1,2,3,4
};

void FurnaceGUI::renderFMPreviewOPN(const DivInstrumentFM& params, int pos) {
  if (fmPreviewOPN==NULL) {
    fmPreviewOPN=new ym3438_t;
    pos=0;
  }
  short out[2];
  int aOut=0;
  bool mult0=false;

  if (pos==0) {
    OPN2_Reset((ym3438_t*)fmPreviewOPN);
    OPN2_SetChipType((ym3438_t*)fmPreviewOPN,ym3438_mode_opn);

    // set params
    for (int i=0; i<4; i++) {
      if ((params.op[i].mult&15)==0) {
        mult0=true;
        break;
      }
    }
    for (int i=0; i<4; i++) {
      const DivInstrumentFM::Operator& op=params.op[i];
      unsigned short baseAddr=i*4;
      OPN_WRITE(baseAddr+0x40,op.tl);
      OPN_WRITE(baseAddr+0x30,(op.mult&15)|(dtTableFMP[op.dt&7]<<4));
      OPN_WRITE(baseAddr+0x50,(op.ar&31)|(op.rs<<6));
      OPN_WRITE(baseAddr+0x60,(op.dr&31)|(op.am<<7));
      OPN_WRITE(baseAddr+0x70,op.d2r&31);
      OPN_WRITE(baseAddr+0x80,(op.rr&15)|(op.sl<<4));
      OPN_WRITE(baseAddr+0x90,op.ssgEnv&15);
    }
    OPN_WRITE(0xb0,(params.alg&7)|((params.fb&7)<<3));
    OPN_WRITE(0xb4,0xc0|(params.fms&7)|((params.ams&3)<<4));
    OPN_WRITE(0xa4,mult0?0x1c:0x14); // frequency
    OPN_WRITE(0xa0,0);
    OPN_WRITE(0x28,0xf0); // key on
  }

  // render
  for (int i=0; i<FM_PREVIEW_SIZE; i++) {
    aOut=0;
    for (int j=0; j<24; j++) {
      OPN2_Clock((ym3438_t*)fmPreviewOPN,out);
    }
    aOut+=((ym3438_t*)fmPreviewOPN)->ch_out[0];
    if (aOut<-32768) aOut=-32768;
    if (aOut>32767) aOut=32767;
    fmPreview[i]=aOut;
  }
}

#define OPM_WRITE(addr,val) \
  OPM_Write((opm_t*)fmPreviewOPM,0,(addr)); \
  do { \
    OPM_Clock((opm_t*)fmPreviewOPM,out,NULL,NULL,NULL); \
    OPM_Clock((opm_t*)fmPreviewOPM,out,NULL,NULL,NULL); \
  } while (((opm_t*)fmPreviewOPM)->write_busy); \
  OPM_Write((opm_t*)fmPreviewOPM,1,(val)); \
  do { \
    OPM_Clock((opm_t*)fmPreviewOPM,out,NULL,NULL,NULL); \
    OPM_Clock((opm_t*)fmPreviewOPM,out,NULL,NULL,NULL); \
  } while (((opm_t*)fmPreviewOPM)->write_busy);

void FurnaceGUI::renderFMPreviewOPM(const DivInstrumentFM& params, int pos) {
  if (fmPreviewOPM==NULL) {
    fmPreviewOPM=new opm_t;
    pos=0;
  }
  int out[2];
  int aOut=0;
  bool mult0=false;

  if (pos==0) {
    OPM_Reset((opm_t*)fmPreviewOPM);

    // set params
    for (int i=0; i<4; i++) {
      if ((params.op[i].mult&15)==0) {
        mult0=true;
        break;
      }
    }
    for (int i=0; i<4; i++) {
      const DivInstrumentFM::Operator& op=params.op[i];
      unsigned short baseAddr=i*8;
      OPM_WRITE(baseAddr+0x40,(op.mult&15)|(dtTableFMP[op.dt&7]<<4));
      OPM_WRITE(baseAddr+0x60,op.tl);
      OPM_WRITE(baseAddr+0x80,(op.ar&31)|(op.rs<<6));
      OPM_WRITE(baseAddr+0xa0,(op.dr&31)|(op.am<<7));
      OPM_WRITE(baseAddr+0xc0,(op.d2r&31)|(op.dt2<<6));
      OPM_WRITE(baseAddr+0xe0,(op.rr&15)|(op.sl<<4));
    }
    OPM_WRITE(0x20,(params.alg&7)|((params.fb&7)<<3)|0xc0);
    OPM_WRITE(0x38,((params.fms&7)<<4)|(params.ams&3));
    OPM_WRITE(0x28,mult0?0x39:0x29); // frequency
    OPM_WRITE(0x30,0xe6);
    OPM_WRITE(0x08,0x78); // key on
  }

  // render
  for (int i=0; i<FM_PREVIEW_SIZE; i++) {
    aOut=0;
    for (int j=0; j<32; j++) {
      OPM_Clock((opm_t*)fmPreviewOPM,out,NULL,NULL,NULL);
    }
    aOut+=out[0];
    if (aOut<-32768) aOut=-32768;
    if (aOut>32767) aOut=32767;
    fmPreview[i]=aOut;
  }
}

void FurnaceGUI::renderFMPreviewOPLL(const DivInstrumentFM& params, int pos) {
}

void FurnaceGUI::renderFMPreviewOPL(const DivInstrumentFM& params, int pos) {
}

void FurnaceGUI::renderFMPreviewOPZ(const DivInstrumentFM& params, int pos) {
}

void FurnaceGUI::renderFMPreview(const DivInstrument* ins, int pos) {
  switch (ins->type) {
    case DIV_INS_FM:
      renderFMPreviewOPN(ins->fm,pos);
      break;
    case DIV_INS_OPM:
      renderFMPreviewOPM(ins->fm,pos);
      break;
    case DIV_INS_OPLL:
      renderFMPreviewOPLL(ins->fm,pos);
      break;
    case DIV_INS_OPL:
      renderFMPreviewOPL(ins->fm,pos);
      break;
    case DIV_INS_OPZ:
      renderFMPreviewOPZ(ins->fm,pos);
      break;
    default:
      break;
  }
}
