/******************************************************************************/
/* Mednafen Virtual Boy Emulation Module                                      */
/******************************************************************************/
/* vsu.cpp:
**  Copyright (C) 2010-2016 Mednafen Team
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "vsu.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

static const unsigned int Tap_LUT[8] = { 15 - 1, 11 - 1, 14 - 1, 5 - 1, 9 - 1, 7 - 1, 10 - 1, 12 - 1 };

#define MDFN_UNLIKELY(x) x

VSU::VSU()
{
 for(int ch = 0; ch < 6; ch++)
 {
  for(int lr = 0; lr < 2; lr++)
   last_output[ch][lr] = 0;
 }
}

VSU::~VSU()
{

}

void VSU::SetSoundRate(double rate)
{
}

void VSU::Power(void)
{
 SweepControl = 0;
 SweepModCounter = 0;
 SweepModClockDivider = 1;
 ModState = 0;
 ModLock = 0;

 for(int ch = 0; ch < 6; ch++)
 {
  IntlControl[ch] = 0;
  LeftLevel[ch] = 0;
  RightLevel[ch] = 0;
  Frequency[ch] = 0;
  EnvControl[ch] = 0;
  RAMAddress[ch] = 0;

  EffFreq[ch] = 0;
  EnvelopeReload[ch] = 0;
  EnvelopeValue[ch] = 0;
  EnvelopeModMask[ch] = 0;
  WavePos[ch] = 0;
  FreqCounter[ch] = 1;
  IntervalCounter[ch] = 0;
  EnvelopeCounter[ch] = 1;

  EffectsClockDivider[ch] = 4800;
  IntervalClockDivider[ch] = 4;
  EnvelopeClockDivider[ch] = 4;

  LatcherClockDivider[ch] = 120;
 }

 ModWavePos = 0;

 NoiseLatcherClockDivider = 120;
 NoiseLatcher = 0;

 lfsr = 0;

 memset(WaveData, 0, sizeof(WaveData));
 memset(ModData, 0, sizeof(ModData));

 last_ts = 0;
}

void VSU::Write(int timestamp, unsigned int A, unsigned char V)
{
 if(MDFN_UNLIKELY(A & 0x3))
 {
  return;
 }
 //
 //
 A &= 0x7FF;

 Update(timestamp);

 ModLock = 0;

 //printf("VSU Write: %d, %08x %02x\n", timestamp, A, V);

 if(A < 0x280)
  WaveData[A >> 7][(A >> 2) & 0x1F] = V & 0x3F;
 else if(A < 0x400)
 {
  //if(A >= 0x300)
   //printf("Modulation mirror write? %08x %02x\n", A, V);
  ModData[(A >> 2) & 0x1F] = V;
 }
 else if(A < 0x600)
 {
  int ch = (A >> 6) & 0xF;

  //if(ch < 6)
  //printf("Ch: %d, Reg: %d, Value: %02x\n", ch, (A >> 2) & 0xF, V);
 
  if(ch > 5)
  {
   if(A == 0x580 && (V & 1))
   {
    //puts("STOP, HAMMER TIME");
    for(int i = 0; i < 6; i++)
     IntlControl[i] &= ~0x80;
   }
  }
  else
  switch((A >> 2) & 0xF)
  {
   case 0x0: IntlControl[ch] = V & ~0x40;

	     if(V & 0x80)
	     {
	      if(ch == 5)
	       FreqCounter[ch] = 10 * (2048 - EffFreq[ch]);
	      else
	       FreqCounter[ch] = 2048 - EffFreq[ch];
	      IntervalCounter[ch] = (V & 0x1F) + 1;
	      EnvelopeCounter[ch] = (EnvControl[ch] & 0x7) + 1;

	      if(ch == 4)
	      {
	       SweepModCounter = (SweepControl >> 4) & 7;
               SweepModClockDivider = (SweepControl & 0x80) ? 8 : 1;
               ModWavePos = 0;
               ModState = 0;
	      }

	      WavePos[ch] = 0;

	      if(ch == 5)	{ // Not sure if this is correct.
	       lfsr = 1;
        }

        EnvelopeModMask[ch] = 0;
        if(!(EnvControl[ch] & 0x200) && (
           (EnvelopeValue[ch] == 0 && !(EnvControl[ch] & 0x0008)) ||
           (EnvelopeValue[ch] == 0xF && (EnvControl[ch] & 0x0008))))
         EnvelopeModMask[ch] = 1;

	      EffectsClockDivider[ch] = 4800;
	      IntervalClockDivider[ch] = 4;
	      EnvelopeClockDivider[ch] = 4;
	     }
	     break;

   case 0x1: LeftLevel[ch] = (V >> 4) & 0xF;
	     RightLevel[ch] = (V >> 0) & 0xF;
	     break;

   case 0x2: Frequency[ch] &= 0xFF00;
             Frequency[ch] |= V << 0;
             EffFreq[ch] &= 0xFF00;
             EffFreq[ch] |= V << 0;
             ModLock = 1;
	     break;

   case 0x3: Frequency[ch] &= 0x00FF;
             Frequency[ch] |= (V & 0x7) << 8;
             EffFreq[ch] &= 0x00FF;
             EffFreq[ch] |= (V & 0x7) << 8;
             ModLock = 2;
	     break;

   case 0x4: EnvControl[ch] &= 0xFF00;
	     EnvControl[ch] |= V << 0;

	     EnvelopeReload[ch] = (V >> 4) & 0xF;
	     EnvelopeValue[ch] = (V >> 4) & 0xF;

       if(EnvelopeModMask[ch] == 1)
        EnvelopeModMask[ch] = 2;
	     break;

   case 0x5: EnvControl[ch] &= 0x00FF;
	     if(ch == 4)
	      EnvControl[ch] |= (V & 0x73) << 8;
	     else if(ch == 5)
	     {
	      EnvControl[ch] |= (V & 0x73) << 8;
	      lfsr = 1;
	     }
	     else
	      EnvControl[ch] |= (V & 0x03) << 8;

       if(EnvelopeModMask[ch] == 0 && !(EnvControl[ch] & 0x200) && (
          (EnvelopeValue[ch] == 0 && !(EnvControl[ch] & 0x0008)) ||
          (EnvelopeValue[ch] == 0xF && (EnvControl[ch] & 0x0008))))
        EnvelopeModMask[ch] = 1;
       
	     break;

   case 0x6: RAMAddress[ch] = V & 0xF;
	     break;

   case 0x7: if(ch == 4)
	     {
	      SweepControl = V;
	     }
	     break;
  }
 }
}

inline void VSU::CalcCurrentOutput(int ch, int &left, int &right)
{
 if(!(IntlControl[ch] & 0x80))
 {
  left = right = 0;
  return;
 }

 int WD;
 int l_ol, r_ol;

 if(ch == 5)
  WD = NoiseLatcher;	//(NoiseLatcher << 6) - NoiseLatcher;
 else
 {
  if(RAMAddress[ch] > 4)
   WD = 0;
  else
   WD = WaveData[RAMAddress[ch]][WavePos[ch]];	// - 0x20;
 }
 l_ol = EnvelopeValue[ch] * LeftLevel[ch];
 if(l_ol)
 {
  l_ol >>= 3;
  l_ol += 1;
 }

 r_ol = EnvelopeValue[ch] * RightLevel[ch];
 if(r_ol)
 {
  r_ol >>= 3;
  r_ol += 1;
 }

 left = WD * l_ol;
 right = WD * r_ol;
}

void VSU::Update(int timestamp)
{
 //puts("VSU Start");
 int left, right;

 for(int ch = 0; ch < 6; ch++)
 {
  int clocks = timestamp - last_ts;
  int running_timestamp = last_ts;

  // Output sound here
  CalcCurrentOutput(ch, left, right);
  if (left!=last_output[ch][0]) {
    blip_add_delta(bb[0],running_timestamp,left - last_output[ch][0]);
  last_output[ch][0] = left;
  }
  if (right!=last_output[ch][1]) {
    blip_add_delta(bb[1],running_timestamp,right - last_output[ch][1]);
  last_output[ch][1] = right;
  }
  oscBuf[ch]->putSample(running_timestamp,(left+right)*8);

  if(!(IntlControl[ch] & 0x80))
   continue;

  while(clocks > 0)
  {
   int chunk_clocks = clocks;

   if(chunk_clocks > EffectsClockDivider[ch])
    chunk_clocks = EffectsClockDivider[ch];

   if(ch == 5)
   {
    if(chunk_clocks > NoiseLatcherClockDivider)
     chunk_clocks = NoiseLatcherClockDivider;
   }
   else
   {
    if(EffFreq[ch] >= 2040)
    {
     if(chunk_clocks > LatcherClockDivider[ch])
      chunk_clocks = LatcherClockDivider[ch];
    }
    else
    {
     if(chunk_clocks > FreqCounter[ch])
      chunk_clocks = FreqCounter[ch];
    }
   }

   if(ch == 5 && chunk_clocks > NoiseLatcherClockDivider)
    chunk_clocks = NoiseLatcherClockDivider;

   FreqCounter[ch] -= chunk_clocks;
   while(FreqCounter[ch] <= 0)
   {
    if(ch == 5)
    {
     int feedback = ((lfsr >> 7) & 1) ^ ((lfsr >> Tap_LUT[(EnvControl[5] >> 12) & 0x7]) & 1) ^ 1;
     lfsr = ((lfsr << 1) & 0x7FFF) | feedback;
 
     FreqCounter[ch] += 10 * (2048 - EffFreq[ch]);
    }
    else
    {
     FreqCounter[ch] += 2048 - EffFreq[ch];
     WavePos[ch] = (WavePos[ch] + 1) & 0x1F;
    }
   }

   LatcherClockDivider[ch] -= chunk_clocks;
   while(LatcherClockDivider[ch] <= 0)
    LatcherClockDivider[ch] += 120;

   if(ch == 5)
   {
    NoiseLatcherClockDivider -= chunk_clocks;
    if(!NoiseLatcherClockDivider)
    {
     NoiseLatcherClockDivider = 120;
     NoiseLatcher = ((lfsr & 1) << 6) - (lfsr & 1);
    }
   }

   EffectsClockDivider[ch] -= chunk_clocks;
   while(EffectsClockDivider[ch] <= 0)
   {
    EffectsClockDivider[ch] += 4800;

    IntervalClockDivider[ch]--;
    while(IntervalClockDivider[ch] <= 0)
    {
     IntervalClockDivider[ch] += 4;

     if(IntlControl[ch] & 0x20)
     {
      IntervalCounter[ch]--;
      if(!IntervalCounter[ch])
      {
       IntlControl[ch] &= ~0x80;
      }
     }

     EnvelopeClockDivider[ch]--;
     while(EnvelopeClockDivider[ch] <= 0)
     {
      EnvelopeClockDivider[ch] += 4;

      int new_envelope = EnvelopeValue[ch];
      if(EnvelopeValue[ch] < 0xF && (EnvControl[ch] & 0x0008))
       new_envelope++;
      else if(EnvelopeValue[ch] > 0 && !(EnvControl[ch] & 0x0008))
       new_envelope--;
      else if((EnvControl[ch] & 0x200) && EnvelopeModMask[ch] != 2)
       {
        new_envelope = EnvelopeReload[ch];
        EnvelopeModMask[ch] = 0;
       }
      else if(EnvelopeModMask[ch] == 0)
       EnvelopeModMask[ch] = 1;

      if(EnvControl[ch] & 0x0100)	// Enveloping enabled?
      {
       EnvelopeCounter[ch]--;
       if(!EnvelopeCounter[ch])
       {
        EnvelopeCounter[ch] = (EnvControl[ch] & 0x7) + 1;
        if(EnvelopeModMask[ch] == 0)
         EnvelopeValue[ch] = new_envelope;
       }
      }

     } // end while(EnvelopeClockDivider[ch] <= 0)
    } // end while(IntervalClockDivider[ch] <= 0)

    if(ch == 4)
    {

      // Calculate sweep early
      int delta = EffFreq[ch] >> (SweepControl & 0x7);
      int NewSweepFreq = EffFreq[ch] + ((SweepControl & 0x8) ? delta : -delta);

      if(!(EnvControl[ch] & 0x1000))
       {
        if(NewSweepFreq < 0)
         NewSweepFreq = 0;
        else if(NewSweepFreq > 0x7FF)
         IntlControl[ch] &= ~0x80;
       }

     SweepModClockDivider--;
     while(SweepModClockDivider <= 0)
     {
      SweepModClockDivider += (SweepControl & 0x80) ? 8 : 1;

      if(((SweepControl >> 4) & 0x7) && (EnvControl[ch] & 0x4000))
      {
       if(SweepModCounter)
        SweepModCounter--;

       if(!SweepModCounter)
       {
        SweepModCounter = (SweepControl >> 4) & 0x7;

        if(EnvControl[ch] & 0x1000)        // Modulation
        {
         if(ModState == 0 || (EnvControl[ch] & 0x2000))
          EffFreq[ch] = (Frequency[ch] + (signed char)ModData[ModWavePos]) & 0x7FF;
         if(ModState == 1)
          ModState = 2;

         // Hardware bug: writing to S5FQ* locks the relevant byte when modulating
         if(ModLock == 1)
          EffFreq[ch] = (EffFreq[ch] & 0x700) | (Frequency[ch] & 0xFF);
         else if(ModLock == 2)
          EffFreq[ch] = (EffFreq[ch] & 0xFF) | (Frequency[ch] & 0x700);
        }
        else if(ModState < 2)               // Sweep
        {
          EffFreq[ch] = NewSweepFreq;
        }

        if(++ModWavePos >= 32)
         {
          if(ModState == 0)
           ModState = 1;
          ModWavePos = 0;
         }
       }
      }
     } // end while(SweepModClockDivider <= 0)
    } // end if(ch == 4)
   } // end while(EffectsClockDivider[ch] <= 0)
   clocks -= chunk_clocks;
   running_timestamp += chunk_clocks;

   // Output sound here too.
   CalcCurrentOutput(ch, left, right);
   if (left!=last_output[ch][0]) {
     blip_add_delta(bb[0],running_timestamp,left - last_output[ch][0]);
   last_output[ch][0] = left;
   }
   if (right!=last_output[ch][1]) {
     blip_add_delta(bb[1],running_timestamp,right - last_output[ch][1]);
   last_output[ch][1] = right;
   }
   oscBuf[ch]->putSample(running_timestamp,(left+right)*8);
  }
 }
 last_ts = timestamp;
 //puts("VSU End");
}

int VSU::EndFrame(int timestamp)
{
 int ret = 0;

 Update(timestamp);
 last_ts = 0;

 return ret;
}

unsigned char VSU::PeekWave(const unsigned int which, unsigned int Address)
{
 assert(which <= 4);

 Address &= 0x1F;

 return(WaveData[which][Address]);
}

void VSU::PokeWave(const unsigned int which, unsigned int Address, unsigned char value)
{
 assert(which <= 4);

 Address &= 0x1F;

 WaveData[which][Address] = value & 0x3F;
}

unsigned char VSU::PeekModWave(unsigned int Address)
{
 Address &= 0x1F;
 return(ModData[Address]);
}

void VSU::PokeModWave(unsigned int Address, unsigned char value)
{
 Address &= 0x1F;

 ModData[Address] = value & 0xFF;
}
