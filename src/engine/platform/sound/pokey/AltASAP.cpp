/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2022 tildearrow and contributors
 * 
 * Original author: Piotr Fusik (http://asap.sourceforge.net)
 * Rewritten based on Mikey emulation by Waldemar Pawlaszek
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

#include "AltASAP.hpp"
#include <array>
#include <vector>
#include <cassert>
#include <algorithm>
#include <limits>

namespace AltASAP
{

namespace
{

static constexpr int64_t CNT_MAX = std::numeric_limits<int64_t>::max() & ~7;
static constexpr int MuteFrequency = 1;
static constexpr int MuteInit = 2;
static constexpr int MuteSerialInput = 8;
//just some magick value to match the audio level of mzpokeysnd
static constexpr int16_t MAGICK_VOLUME_BOOSTER = 160;
static constexpr int16_t MAGICK_OSC_VOLUME_BOOSTER = 4;

struct PokeyBase
{
  int64_t mPolyIndex;
  int mAudctl;
  int mSkctl;
  bool mInit;
  std::array<uint8_t, 511> mPoly9Lookup;
  std::array<uint8_t, 16385> mPoly17Lookup;


  PokeyBase() : mPolyIndex{ 15 * 31 * 131071 }, mAudctl{ 0 }, mSkctl{ 3 }, mInit{ false }, mPoly9Lookup{}, mPoly17Lookup{}
  {
    int reg = 0x1ff;
    for ( int i = 0; i < 511; i++ )
    {
      reg = ( ( ( reg >> 5 ^ reg ) & 1 ) << 8 ) + ( reg >> 1 );
      mPoly9Lookup[i] = reg & 0xff;
    }
    reg = 0x1ffff;
    for ( int i = 0; i < 16385; i++ )
    {
      reg = ( ( ( reg >> 5 ^ reg ) & 0xff ) << 9 ) + ( reg >> 8 );
      mPoly17Lookup[i] = reg >> 1 & 0xff;
    }

  }
};


/*
  "Queue" holding event timepoints.
  - 4 channel timer fire points
  - 1 sample point
  Three LSBs are used to encode event kind: 0-3 are channels, 4 is sampling.
   Channel sequence is 2,3,0,1
*/
class ActionQueue
{
public:

  ActionQueue() : mTab{ CNT_MAX | 0, CNT_MAX | 1, CNT_MAX | 2, CNT_MAX | 3, CNT_MAX | 4 }
  {
  }

  void insert( int idx, int64_t value )
  {
    assert( idx < (int)mTab.size() );
    idx ^= 2;
    mTab[idx] = value | idx;
  }

  void insertSampling( int64_t value )
  {
    mTab[4] = value | 4;
  }

  void disable( int idx )
  {
    assert( idx < (int)mTab.size() );
    idx ^= 2;
    mTab[idx] = CNT_MAX | idx;
  }

  bool enqueued( int idx )
  {
    assert( idx < (int)mTab.size() );
    idx ^= 2;
    return mTab[idx] < CNT_MAX;
  }

  int64_t pop()
  {
    int64_t min1 = std::min( mTab[0], mTab[1] );
    int64_t min2 = std::min( mTab[2], mTab[3] );
    int64_t min3 = std::min( min1, mTab[4] );
    int64_t min4 = std::min( min2, min3 );

    return min4 ^ 2;
  }

private:
  std::array<int64_t, 5> mTab;
};

class AudioChannel
{
public:
  AudioChannel( ActionQueue& queue, uint32_t number ) : mQueue{ queue }, mNumber{ number }, mAudf{ 0 }, mAudc{ 0 }, mPeriodCycles{ 28 }, mMute{ MuteFrequency }, mDelta{ 0 }, mOut{ 0 }
  {
  }

  int16_t getOutput() const
  {
    return (int16_t)mDelta;
  }

  void fillRegisterPool( uint8_t* regs )
  {
    regs[0] = (uint8_t)mAudf;
    regs[1] = (uint8_t)mAudc;
  }

  void renewTrigger( int64_t cycle )
  {
    overrideTrigger( cycle + mPeriodCycles );
  }

  void overrideTrigger( int64_t cycle )
  {
    mQueue.insert( mNumber, cycle << 3 );
  }

  void doStimer( int64_t cycle )
  {
    if ( mQueue.enqueued( mNumber ) )
      renewTrigger( cycle );
  }

  void trigger( int64_t cycle, PokeyBase const& pokey )
  {
    renewTrigger( cycle );
    if ( ( mAudc & 0xb0 ) == 0xa0 )
      mOut ^= 1;
    else if ( ( mAudc & 0x10 ) != 0 || pokey.mInit )
      return;
    else
    {
      int64_t poly = cycle + pokey.mPolyIndex - mNumber;
      if ( mAudc < 0x80 && ( 0x65bd44e0 & 1 << ( poly % 31 ) ) == 0 ) // 0000011100100010101111011010011
        return;
      if ( ( mAudc & 0x20 ) != 0 )
        mOut ^= 1;
      else
      {
        uint32_t newOut;
        if ( ( mAudc & 0x40 ) != 0 )
          newOut = 0x5370u >> (int)( poly % 15 ); // 000011101100101
        else if ( pokey.mAudctl < 0x80 )
        {
          poly %= 131071;
          newOut = pokey.mPoly17Lookup[poly >> 3] >> ( poly & 7 );
        }
        else
          newOut = pokey.mPoly9Lookup[poly % 511];
        newOut &= 1;
        if ( mOut == newOut )
          return;
        mOut = newOut;
      }
    }
    toggle();
  }

  uint32_t mute() const
  {
    return mMute;
  }

  uint32_t audf() const
  {
    return mAudf;
  }

  uint32_t audc() const
  {
    return mAudc;
  }

  void setAudf( uint32_t value )
  {
    mAudf = value;
  }

  void setAudc( int64_t cycle, uint32_t value )
  {
    if ( mAudc == value )
      return;
    mAudc = value;
    int32_t volume = value & 0x0f;
    if ( ( value & 0x10 ) != 0 )
    {
      mDelta = volume * MAGICK_VOLUME_BOOSTER;
    }
    else
    {
      muteUltrasound( cycle );
      if ( mDelta > 0 )
        mDelta = volume * MAGICK_VOLUME_BOOSTER;
      else
        mDelta = -volume * MAGICK_VOLUME_BOOSTER;
    }
  }

  void toggle()
  {
    mDelta = -mDelta;
  }

  void setPeriodCycles( int64_t value )
  {
    mPeriodCycles = value;
  }

  void setMute( bool enable, int mask, int64_t cycle )
  {
    if ( enable )
    {
      mMute |= mask;
      mQueue.disable( mNumber );
    }
    else
    {
      mMute &= ~mask;
      if ( mMute == 0 && !mQueue.enqueued( mNumber ) )
        overrideTrigger( cycle );
    }
  }

  void muteUltrasound( int64_t cycle )
  {
    static constexpr int UltrasoundCycles = 112;
    setMute( mPeriodCycles <= UltrasoundCycles && ( mAudc & 0xb0 ) == 0xa0, MuteFrequency, cycle );
  }

private:
  ActionQueue& mQueue;
  uint32_t mNumber;
  uint32_t mAudf;
  uint32_t mAudc;
  int64_t mPeriodCycles;

  uint32_t mMute;
  int32_t mDelta;
  uint32_t mOut;
};

}


class PokeyPimpl : public PokeyBase
{
public:

  PokeyPimpl( uint32_t pokeyClock, uint32_t sampleRate ) : PokeyBase{}, mQueue{ std::make_unique<ActionQueue>() },
    mAudioChannels{ AudioChannel{ *mQueue, 0u }, AudioChannel{ *mQueue, 1u }, AudioChannel{ *mQueue, 2u }, AudioChannel{ *mQueue, 3u } },
    mRegisterPool{}, mTick{}, mNextTick{},
    mReloadCycles1{ 28 }, mReloadCycles3{ 28 }, mDivCycles{ 28 }, mSampleRate{ sampleRate }, mSamplesRemainder{},
    mTicksPerSample{ ( pokeyClock * 8 ) / mSampleRate, ( pokeyClock * 8 ) % mSampleRate }
  {
    std::fill_n( mRegisterPool.data(), mRegisterPool.size(), (uint8_t)0xff );
    enqueueSampling();
  }

  ~PokeyPimpl() {}

  void write( uint8_t address, uint8_t value )
  {
    auto cycle = mTick >> 3;

    switch ( address & 0xf )
    {
    case 0x00:
      if ( value == mAudioChannels[0].audf() )
        break;
      mAudioChannels[0].setAudf( value );
      switch ( mAudctl & 0x50 )
      {
      case 0x00:
        mAudioChannels[0].setPeriodCycles( mDivCycles * ( value + 1 ) );
        break;
      case 0x10:
        mAudioChannels[1].setPeriodCycles( mDivCycles * ( value + ( mAudioChannels[1].audf() << 8 ) + 1 ) );
        mReloadCycles1 = mDivCycles * ( value + 1 );
        mAudioChannels[1].muteUltrasound( cycle );
        break;
      case 0x40:
        mAudioChannels[0].setPeriodCycles( value + 4 );
        break;
      case 0x50:
        mAudioChannels[1].setPeriodCycles( value + ( mAudioChannels[1].audf() << 8 ) + 7 );
        mReloadCycles1 = value + 4;
        mAudioChannels[1].muteUltrasound( cycle );
        break;
      default:
        assert( false );
      }
      mAudioChannels[0].muteUltrasound( cycle );
      break;
    case 0x01:
      mAudioChannels[0].setAudc( cycle, value );
      break;
    case 0x02:
      if ( value == mAudioChannels[1].audf() )
        break;
      mAudioChannels[1].setAudf( value );
      switch ( mAudctl & 0x50 )
      {
      case 0x00:
      case 0x40:
        mAudioChannels[1].setPeriodCycles( mDivCycles * ( value + 1 ) );
        break;
      case 0x10:
        mAudioChannels[1].setPeriodCycles( mDivCycles * ( mAudioChannels[0].audf() + ( value << 8 ) + 1 ) );
        break;
      case 0x50:
        mAudioChannels[1].setPeriodCycles( mAudioChannels[0].audf() + ( value << 8 ) + 7 );
        break;
      default:
        assert( false );
      }
      mAudioChannels[1].muteUltrasound( cycle );
      break;
    case 0x03:
      mAudioChannels[1].setAudc( cycle, value );
      break;
    case 0x04:
      if ( value == mAudioChannels[2].audf() )
        break;
      mAudioChannels[2].setAudf( value );
      switch ( mAudctl & 0x28 )
      {
      case 0x00:
        mAudioChannels[2].setPeriodCycles( mDivCycles * ( value + 1 ) );
        break;
      case 0x08:
        mAudioChannels[3].setPeriodCycles( mDivCycles * ( value + ( mAudioChannels[3].audf() << 8 ) + 1 ) );
        mReloadCycles3 = mDivCycles * ( value + 1 );
        mAudioChannels[3].muteUltrasound( cycle );
        break;
      case 0x20:
        mAudioChannels[2].setPeriodCycles( value + 4 );
        break;
      case 0x28:
        mAudioChannels[3].setPeriodCycles( value + ( mAudioChannels[3].audf() << 8 ) + 7 );
        mReloadCycles3 = value + 4;
        mAudioChannels[3].muteUltrasound( cycle );
        break;
      default:
        assert( false );
      }
      mAudioChannels[2].muteUltrasound( cycle );
      break;
    case 0x05:
      mAudioChannels[2].setAudc( cycle, value );
      break;
    case 0x06:
      if ( value == mAudioChannels[3].audf() )
        break;
      mAudioChannels[3].setAudf( value );
      switch ( mAudctl & 0x28 )
      {
      case 0x00:
      case 0x20:
        mAudioChannels[3].setPeriodCycles( mDivCycles * ( value + 1 ) );
        break;
      case 0x08:
        mAudioChannels[3].setPeriodCycles( mDivCycles * ( mAudioChannels[2].audf() + ( value << 8 ) + 1 ) );
        break;
      case 0x28:
        mAudioChannels[3].setPeriodCycles( mAudioChannels[2].audf() + ( value << 8 ) + 7 );
        break;
      default:
        assert( false );
      }
      mAudioChannels[3].muteUltrasound( cycle );
      break;
    case 0x07:
      mAudioChannels[3].setAudc( cycle, value );
      break;
    case 0x08:
      if ( value == mAudctl )
        break;
      mAudctl = value;
      mDivCycles = ( value & 1 ) != 0 ? 114 : 28;
      switch ( value & 0x50 )
      {
      case 0x00:
        mAudioChannels[0].setPeriodCycles( mDivCycles * ( mAudioChannels[0].audf() + 1 ) );
        mAudioChannels[1].setPeriodCycles( mDivCycles * ( mAudioChannels[1].audf() + 1 ) );
        break;
      case 0x10:
        mAudioChannels[0].setPeriodCycles( (int64_t)mDivCycles << 8 );
        mAudioChannels[1].setPeriodCycles( mDivCycles * ( mAudioChannels[0].audf() + ( mAudioChannels[1].audf() << 8 ) + 1 ) );
        mReloadCycles1 = mDivCycles * ( mAudioChannels[0].audf() + 1 );
        break;
      case 0x40:
        mAudioChannels[0].setPeriodCycles( mAudioChannels[0].audf() + 4 );
        mAudioChannels[1].setPeriodCycles( mDivCycles * ( mAudioChannels[1].audf() + 1 ) );
        break;
      case 0x50:
        mAudioChannels[0].setPeriodCycles( 256 );
        mAudioChannels[1].setPeriodCycles( mAudioChannels[0].audf() + ( mAudioChannels[1].audf() << 8 ) + 7 );
        mReloadCycles1 = mAudioChannels[0].audf() + 4;
        break;
      default:
        assert( false );
      }
      mAudioChannels[0].muteUltrasound( cycle );
      mAudioChannels[1].muteUltrasound( cycle );
      switch ( value & 0x28 )
      {
      case 0x00:
        mAudioChannels[2].setPeriodCycles( mDivCycles * ( mAudioChannels[2].audf() + 1 ) );
        mAudioChannels[3].setPeriodCycles( mDivCycles * ( mAudioChannels[3].audf() + 1 ) );
        break;
      case 0x08:
        mAudioChannels[2].setPeriodCycles( (int64_t)mDivCycles << 8 );
        mAudioChannels[3].setPeriodCycles( mDivCycles * ( mAudioChannels[2].audf() + ( mAudioChannels[3].audf() << 8 ) + 1 ) );
        mReloadCycles3 = mDivCycles * ( mAudioChannels[2].audf() + 1 );
        break;
      case 0x20:
        mAudioChannels[2].setPeriodCycles( mAudioChannels[2].audf() + 4 );
        mAudioChannels[3].setPeriodCycles( mDivCycles * ( mAudioChannels[3].audf() + 1 ) );
        break;
      case 0x28:
        mAudioChannels[2].setPeriodCycles( 256 );
        mAudioChannels[3].setPeriodCycles( mAudioChannels[2].audf() + ( mAudioChannels[3].audf() << 8 ) + 7 );
        mReloadCycles3 = mAudioChannels[2].audf() + 4;
        break;
      default:
        assert( false );
      }
      mAudioChannels[2].muteUltrasound( cycle );
      mAudioChannels[3].muteUltrasound( cycle );
      initMute( cycle );
      break;
    case 0x09:
      for ( int i = 0; i < 4; i++ )
        mAudioChannels[i].doStimer( cycle );
      break;
    case 0x0f:
    {
      if ( value == mSkctl )
        break;
      mSkctl = value;
      bool init = ( value & 3 ) == 0;
      if ( mInit && !init )
        mPolyIndex = ( ( mAudctl & 0x80 ) != 0 ? 15 * 31 * 511 - 1 : 15 * 31 * 131071 - 1 ) - cycle;
      mInit = init;
      initMute( cycle );
      mAudioChannels[2].setMute( ( value & 0x10 ) != 0, MuteSerialInput, cycle );
      mAudioChannels[3].setMute( ( value & 0x10 ) != 0, MuteSerialInput, cycle );
      break;
    }
    default:
      break;
    }
  }

  int16_t sampleAudio( DivDispatchOscBuffer** oscb )
  {
    for ( ;; )
    {
      int64_t value = mQueue->pop();
      if ( ( value & 7 ) == 6 ) // 6 == 4 ^ 2
      {
        int16_t ch0 = mAudioChannels[0].getOutput();
        int16_t ch1 = mAudioChannels[1].getOutput();
        int16_t ch2 = mAudioChannels[2].getOutput();
        int16_t ch3 = mAudioChannels[3].getOutput();

        if ( oscb != nullptr )
        {
          oscb[0]->data[oscb[0]->needle++]=ch0 * MAGICK_OSC_VOLUME_BOOSTER;
          oscb[1]->data[oscb[1]->needle++]=ch1 * MAGICK_OSC_VOLUME_BOOSTER;
          oscb[2]->data[oscb[2]->needle++]=ch2 * MAGICK_OSC_VOLUME_BOOSTER;
          oscb[3]->data[oscb[3]->needle++]=ch3 * MAGICK_OSC_VOLUME_BOOSTER;
        }

        enqueueSampling();
        return ch0 + ch1 + ch2 + ch3;
      }
      else
      {
        fireTimer( value );
      }
    }
  }

  uint8_t const* getRegisterPool()
  {
    for ( size_t i = 0; i < mAudioChannels.size(); ++i )
    {
      mAudioChannels[i].fillRegisterPool( mRegisterPool.data() + 2 * i );
    }

    mRegisterPool[8] = mAudctl;

    return  mRegisterPool.data();
  }

private:

  void initMute( int64_t cycle )
  {
    mAudioChannels[0].setMute( mInit && ( mAudctl & 0x40 ) == 0, MuteInit, cycle );
    mAudioChannels[1].setMute( mInit && ( mAudctl & 0x50 ) != 0x50, MuteInit, cycle );
    mAudioChannels[2].setMute( mInit && ( mAudctl & 0x20 ) == 0, MuteInit, cycle );
    mAudioChannels[3].setMute( mInit && ( mAudctl & 0x28 ) != 0x28, MuteInit, cycle );
  }

  void fireTimer( int64_t tick )
  {
    mTick = tick & ~7;
    size_t ch = tick & 3;
    auto cycle = tick >> 3;

    switch ( ch )
    {
    case 0:
      if ( ( mSkctl & 0x88 ) == 8 ) // two-tone, sending 1 (i.e. timer1)
        mAudioChannels[1].renewTrigger( cycle );
      mAudioChannels[0].trigger( cycle, *this );
      break;
    case 1:
      if ( ( mAudctl & 0x10 ) != 0 )
        mAudioChannels[0].overrideTrigger( cycle + mReloadCycles1 );
      else if ( ( mSkctl & 8 ) != 0 ) // two-tone
        mAudioChannels[0].renewTrigger( cycle );
      mAudioChannels[1].trigger( cycle, *this );
      break;
    case 2:
      if ( ( mAudctl & 4 ) != 0 && mAudioChannels[0].getOutput() > 0 && mAudioChannels[0].mute() == 0 )
        mAudioChannels[0].toggle();
      mAudioChannels[2].trigger( cycle, *this );
      break;
    case 3:
      if ( ( mAudctl & 8 ) != 0 )
        mAudioChannels[2].overrideTrigger( cycle + mReloadCycles3 );
      if ( ( mAudctl & 2 ) != 0 && mAudioChannels[1].getOutput() > 0 && mAudioChannels[1].mute() == 0 )
        mAudioChannels[1].toggle();
      mAudioChannels[3].trigger( cycle, *this );
      break;
    default:
      break;
    }
  }

  void enqueueSampling()
  {
    mTick = mNextTick & ~7;
    mNextTick = mNextTick + mTicksPerSample.first;
    mSamplesRemainder += mTicksPerSample.second;
    if ( mSamplesRemainder > mSampleRate )
    {
      mSamplesRemainder %= mSampleRate;
      mNextTick += 1;
    }

    mQueue->insertSampling( mNextTick & ~7 );
  }


private:

  std::unique_ptr<ActionQueue> mQueue;
  std::array<AudioChannel, 4> mAudioChannels;

  std::array<uint8_t, 4 * 2 + 1> mRegisterPool;

  uint64_t mTick;
  uint64_t mNextTick;
  int64_t mReloadCycles1;
  int64_t mReloadCycles3;
  int64_t mDivCycles;
  uint32_t mSampleRate;
  uint32_t mSamplesRemainder;
  std::pair<uint32_t, uint32_t> mTicksPerSample;
};

//Initializing periods with safe defaults
Pokey::Pokey() : mPokeyClock{ (uint32_t)COLOR_NTSC / 2 }, mSampleRate{ mPokeyClock / 7 }, mPokey{}
{
}

void Pokey::init( uint32_t pokeyClock, uint32_t sampleRate )
{
  mPokey.reset();
  mPokeyClock = pokeyClock;
  mSampleRate = sampleRate;
}

void Pokey::reset()
{
  mPokey = std::make_unique<PokeyPimpl>( mPokeyClock, mSampleRate );
}

Pokey::~Pokey()
{
}

void Pokey::write( uint8_t address, uint8_t value )
{
  assert( mPokey );
  mPokey->write( address, value );
}

int16_t Pokey::sampleAudio( DivDispatchOscBuffer** oscb )
{
  assert( mPokey );
  return mPokey->sampleAudio( oscb );
}

uint8_t const* Pokey::getRegisterPool()
{
  assert( mPokey );
  return mPokey->getRegisterPool();
}

}
