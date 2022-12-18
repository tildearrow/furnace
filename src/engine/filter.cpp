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

#define _USE_MATH_DEFINES
#include <math.h>
#include "filter.h"
#include "../ta-log.h"

float* DivFilterTables::cubicTable=NULL;
float* DivFilterTables::sincTable=NULL;
float* DivFilterTables::sincTable8=NULL;
float* DivFilterTables::sincIntegralTable=NULL;

// portions from Schism Tracker (scripts/lutgen.c)
// licensed under same license as this program.
float* DivFilterTables::getCubicTable() {
  if (cubicTable==NULL) {
    logD("initializing cubic spline table.");
    cubicTable=new float[4096];

    for (int i=0; i<1024; i++) {
      float x=(float)i/1024.0;
      cubicTable[(i<<2)]=-0.5*pow(x,3)+1.0*pow(x,2)-0.5*x;
      cubicTable[1+(i<<2)]=1.5*pow(x,3)-2.5*pow(x,2)+1.0;
      cubicTable[2+(i<<2)]=-1.5*pow(x,3)+2.0*pow(x,2)+0.5*x;
      cubicTable[3+(i<<2)]=0.5*pow(x,3)-0.5*pow(x,2);
    }
  }
  return cubicTable;
}

float* DivFilterTables::getSincTable() {
  if (sincTable==NULL) {
    logD("initializing sinc table.");
    sincTable=new float[65536];

    sincTable[0]=1.0f;
    for (int i=1; i<65536; i++) {
      int mapped=((i&8191)<<3)|(i>>13);
      double x=(double)i*M_PI/8192.0;
      sincTable[mapped]=sin(x)/x;
    }

    for (int i=0; i<65536; i++) {
      int mapped=((i&8191)<<3)|(i>>13);
      sincTable[mapped]*=pow(cos(M_PI*(double)i/131072.0),2.0);
    }
  }
  return sincTable;
}

float* DivFilterTables::getSincTable8() {
  if (sincTable8==NULL) {
    logD("initializing sinc table (8).");
    sincTable8=new float[32768];

    sincTable8[0]=1.0f;
    for (int i=1; i<32768; i++) {
      int mapped=((i&8191)<<2)|(i>>13);
      double x=(double)i*M_PI/8192.0;
      sincTable8[mapped]=sin(x)/x;
    }

    for (int i=0; i<32768; i++) {
      int mapped=((i&8191)<<2)|(i>>13);
      sincTable8[mapped]*=pow(cos(M_PI*(double)i/65536.0),2.0);
    }
  }
  return sincTable8;
}

float* DivFilterTables::getSincIntegralTable() {
  if (sincIntegralTable==NULL) {
    logD("initializing sinc integral table.");
    sincIntegralTable=new float[65536];

    sincIntegralTable[0]=-0.5f;
    for (int i=1; i<65536; i++) {
      int mapped=((i&8191)<<3)|(i>>13);
      int mappedPrev=(((i-1)&8191)<<3)|((i-1)>>13);
      double x=(double)i*M_PI/8192.0;
      double sinc=sin(x)/x;
      sincIntegralTable[mapped]=sincIntegralTable[mappedPrev]+(sinc/8192.0);
    }

    for (int i=0; i<65536; i++) {
      int mapped=((i&8191)<<3)|(i>>13);
      sincIntegralTable[mapped]*=pow(cos(M_PI*(double)i/131072.0),2.0);
    }
  }
  return sincIntegralTable;
}
