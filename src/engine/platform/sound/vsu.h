/******************************************************************************/
/* Mednafen Virtual Boy Emulation Module                                      */
/******************************************************************************/
/* vsu.h:
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

#ifndef __VB_VSU_H
#define __VB_VSU_H

class VSU
{
 public:

 int last_output[6][2];

 VSU();
 ~VSU();

 void SetSoundRate(double rate);

 void Power(void);

 void Write(int timestamp, unsigned int A, unsigned char V);

 int EndFrame(int timestamp);

 unsigned char PeekWave(const unsigned int which, unsigned int Address);
 void PokeWave(const unsigned int which, unsigned int Address, unsigned char value);

 unsigned char PeekModWave(unsigned int Address);
 void PokeModWave(unsigned int Address, unsigned char value);

 private:

 void CalcCurrentOutput(int ch, int &left, int &right);

 void Update(int timestamp);

 unsigned char IntlControl[6];
 unsigned char LeftLevel[6];
 unsigned char RightLevel[6];
 unsigned short Frequency[6];
 unsigned short EnvControl[6];	// Channel 5/6 extra functionality tacked on too.

 unsigned char RAMAddress[6];

 unsigned char SweepControl;

 unsigned char WaveData[5][0x20];

 unsigned char ModData[0x20];

 //
 //
 //
 int EffFreq[6];
 int Envelope[6];

 int WavePos[6];
 int ModWavePos;

 int LatcherClockDivider[6];

 int FreqCounter[6];
 int IntervalCounter[6];
 int EnvelopeCounter[6];
 int SweepModCounter;

 int EffectsClockDivider[6];
 int IntervalClockDivider[6];
 int EnvelopeClockDivider[6];
 int SweepModClockDivider;

 int NoiseLatcherClockDivider;
 unsigned int NoiseLatcher;

 unsigned int lfsr;

 int last_ts;
};

#endif
