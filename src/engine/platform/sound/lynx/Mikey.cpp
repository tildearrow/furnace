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

#include "Mikey.hpp"
#include <array>
#include <cstdint>
#include <vector>
#include <functional>
#include <cassert>
#include <algorithm>
#include <limits>

#if defined( _MSC_VER )

#include <intrin.h>

static uint32_t popcnt_generic( uint32_t x )
{
  int v = 0;
  while ( x != 0 )
  {
    x &= x - 1;
    v++;
  }
  return v;
}

#if defined( _M_IX86 ) || defined( _M_X64 )

static uint32_t popcnt_intrinsic( uint32_t x )
{
  return __popcnt( x );
}

static uint32_t( *popcnt )( uint32_t );

//detecting popcnt availability on msvc intel
static void selectPOPCNT()
{
  int info[4];
  __cpuid( info, 1 );
  if ( ( info[2] & ( (int)1 << 23 ) ) != 0 )
  {
    popcnt = &popcnt_intrinsic;
  }
  else
  {
    popcnt = &popcnt_generic;
  }
}

#else //defined( _M_IX86 ) || defined( _M_X64 )

//MSVC non INTEL should use generic implementation
inline void selectPOPCNT()
{
}

#define popcnt popcnt_generic

#endif

#else //defined( _MSC_VER )

//non MVSC should use builtin implementation

inline void selectPOPCNT()
{
}

#define popcnt __builtin_popcount

#endif

namespace Lynx
{

namespace
{

static constexpr int64_t CNT_MAX = std::numeric_limits<int64_t>::max() & ~15;

int32_t clamp( int32_t v, int32_t lo, int32_t hi )
{
  return v < lo ? lo : ( v > hi ? hi : v );
}

class Timer
{
public:
  Timer() : mValueUpdateTick{}, mAudShift {}, mEnableReload{}, mEnableCount{}, mTimerDone{}, mBackup{ 0 }, mValue{ 0 }
  {
  }

  int64_t setBackup( int64_t tick, uint8_t backup )
  {
    mBackup = backup;
    return computeAction( tick );
  }

  int64_t setControlA( int64_t tick, uint8_t controlA )
  {
    mTimerDone ^= ( controlA & CONTROLA::RESET_DONE ) != 0;
    mEnableReload = ( controlA & CONTROLA::ENABLE_RELOAD ) != 0;
    mEnableCount = ( controlA & CONTROLA::ENABLE_COUNT ) != 0;
    mAudShift = controlA & CONTROLA::AUD_CLOCK_MASK;

    return computeAction( tick );
  }

  int64_t setCount( int64_t tick, uint8_t value )
  {
    return computeTriggerTime( tick );
  }

  void setControlB( uint8_t controlB )
  {
    mTimerDone = ( controlB & CONTROLB::TIMER_DONE ) != 0;
  }

  int64_t fireAction( int64_t tick )
  {
    mTimerDone = true;

    return computeAction( tick );
  }

  uint8_t getBackup() const
  {
    return mBackup;
  }

  uint8_t getCount( int64_t tick )
  {
    updateValue( tick );
    return mValue;
  }

private:

  int64_t scaleDiff( int64_t older, int64_t newer ) const
  {
    int64_t const mask = (int64_t)( ~0ull << ( mAudShift + 4 ) );
    return ( ( newer & mask ) - ( older & mask ) ) >> ( mAudShift + 4 );
  }

  void updateValue( int64_t tick )
  {
    if ( mEnableCount )
      mValue = (uint8_t)std::max( (int64_t)0, mValue - scaleDiff( mValueUpdateTick, tick ) );
    mValueUpdateTick = tick;
  }

  int64_t computeTriggerTime( int64_t tick )
  {
    if ( mEnableCount && mValue != 0 )
    {
      //tick value is increased by multipy of 16 (1 MHz resolution) lower bits are unchanged
      return tick + ( 1ull + mValue ) * ( 1ull << ( mAudShift + 4 ) );
    }
    else
    {
      return CNT_MAX;  //infinite
    }
  }

  int64_t computeAction( int64_t tick )
  {
    updateValue( tick );
    if ( mValue == 0 && mEnableReload )
    {
      mValue = mBackup;
    }

    return computeTriggerTime( tick );
  }

private:
  struct CONTROLA
  {
    static constexpr uint8_t RESET_DONE = 0b01000000;
    static constexpr uint8_t ENABLE_RELOAD = 0b00010000;
    static constexpr uint8_t ENABLE_COUNT = 0b00001000;
    static constexpr uint8_t AUD_CLOCK_MASK = 0b00000111;
  };
  struct CONTROLB
  {
    static constexpr uint8_t TIMER_DONE = 0b00001000;
  };

private:
  int64_t mValueUpdateTick;
  int mAudShift;
  bool mEnableReload;
  bool mEnableCount;
  bool mTimerDone;
  uint8_t mBackup;
  uint8_t mValue;
};

class AudioChannel
{
public:
  AudioChannel( uint32_t number ) : mTimer{}, mNumber{ number }, mShiftRegister{}, mTapSelector{}, mEnableIntegrate{}, mVolume{}, mOutput{}, mCtrlA{}
  {
  }

  int64_t fireAction( int64_t tick )
  {
    trigger();
    return adjust( mTimer.fireAction( tick ) );
  }

  void setVolume( int8_t value )
  {
    mVolume = value;
  }

  void setFeedback( uint8_t value )
  {
    mTapSelector = ( mTapSelector & 0b0011'1100'0000 ) | ( value & 0b0011'1111 ) | ( ( (int)value & 0b1100'0000 ) << 4 );
  }

  void setOutput( uint8_t value )
  {
    mOutput = value;
  }

  void setShift( uint8_t value )
  {
    mShiftRegister = ( mShiftRegister & 0xff00 ) | value;
  }

  int64_t setBackup( int64_t tick, uint8_t value )
  {
    return adjust( mTimer.setBackup( tick, value ) );
  }

  int64_t setControl( int64_t tick, uint8_t value )
  {
    if ( mCtrlA == value )
      return 0;
    mCtrlA = value;

    mTapSelector = ( mTapSelector & 0b1111'0111'1111 ) | ( value & FEEDBACK_7 );
    mEnableIntegrate = ( value & ENABLE_INTEGRATE ) != 0;
    return adjust( mTimer.setControlA( tick, value & ~( FEEDBACK_7 | ENABLE_INTEGRATE ) ) );
  }

  int64_t setCounter( int64_t tick, uint8_t value )
  {
    return adjust( mTimer.setCount( tick, value ) );
  }

  void setOther( uint8_t value )
  {
    mShiftRegister = ( mShiftRegister & 0b0000'1111'1111 ) | ( ( (int)value & 0b1111'0000 ) << 4 );
    mTimer.setControlB( value & 0b0000'1111 );
  }

  int8_t getOutput() const
  {
    return mOutput;
  }

  void fillRegisterPool( int64_t tick, uint8_t* regs )
  {
    regs[0] = mVolume;
    regs[1] = mTapSelector & 0xff;
    regs[2] = mOutput;
    regs[3] = mShiftRegister & 0xff;
    regs[4] = mTimer.getBackup();
    regs[5] = mCtrlA;
    regs[6] = mTimer.getCount( tick );
    regs[7] = ( ( mShiftRegister >> 4 ) & 0xf0 );
  }

private:

  int64_t adjust( int64_t tick ) const
  {
    //ticks are advancing in 1 MHz resolution, so lower 4 bits are unused.
    //timer number is encoded on lowest 2 bits.
    return tick | mNumber;
  }

  void trigger()
  {
    uint32_t xorGate = mTapSelector & mShiftRegister;
    uint32_t parity = popcnt( xorGate ) & 1;
    uint32_t newShift = ( mShiftRegister << 1 ) | ( parity ^ 1 );
    mShiftRegister = newShift;

    if ( mEnableIntegrate )
    {
      int32_t temp = mOutput + ( ( newShift & 1 ) ? mVolume : -mVolume );
      mOutput = (int8_t)clamp( temp, (int32_t)std::numeric_limits<int8_t>::min(), (int32_t)std::numeric_limits<int8_t>::max() );
    }
    else
    {
      mOutput = ( newShift & 1 ) ? mVolume : -mVolume;
    }
  }

private:
  static constexpr uint8_t FEEDBACK_7 = 0b10000000;
  static constexpr uint8_t ENABLE_INTEGRATE = 0b00100000;

private:
  Timer mTimer;
  uint32_t mNumber;

  uint32_t mShiftRegister;
  uint32_t mTapSelector;
  bool mEnableIntegrate;
  int8_t mVolume;
  int8_t mOutput;
  uint8_t mCtrlA;
};

}


/*
  "Queue" holding event timepoints.
  - 4 channel timer fire points
  - 1 sample point
  Time is in 16 MHz units but only with 1 MHz resolution.
  Four LSBs are used to encode event kind 0-3 are channels, 4 is sampling.
*/
class ActionQueue
{
public:


  ActionQueue() : mTab{ CNT_MAX | 0, CNT_MAX | 1, CNT_MAX | 2, CNT_MAX | 3, CNT_MAX | 4 }
  {
  }

  void push( int64_t value )
  {
    size_t idx = value & 15;
    if ( idx < mTab.size() )
    {
      if ( value & ~15 )
      {
        //writing only non-zero values
        mTab[idx] = value;
      }
    }
  }

  int64_t pop()
  {
    int64_t min1 = std::min( mTab[0], mTab[1] );
    int64_t min2 = std::min( mTab[2], mTab[3] );
    int64_t min3 = std::min( min1, mTab[4] );
    int64_t min4 = std::min( min2, min3 );

    assert( ( min4 & 15 ) < (int64_t)mTab.size() );
    mTab[min4 & 15] = CNT_MAX | ( min4 & 15 );

    return min4;
  }

private:
  std::array<int64_t, 5> mTab;
};


class MikeyPimpl
{
public:

  struct AudioSample
  {
    int16_t left;
    int16_t right;
  };

  static constexpr uint16_t VOLCNTRL = 0x0;
  static constexpr uint16_t FEEDBACK = 0x1;
  static constexpr uint16_t OUTPUT = 0x2;
  static constexpr uint16_t SHIFT = 0x3;
  static constexpr uint16_t BACKUP = 0x4;
  static constexpr uint16_t CONTROL = 0x5;
  static constexpr uint16_t COUNTER = 0x6;
  static constexpr uint16_t OTHER = 0x7;

  static constexpr uint16_t ATTENREG0 = 0x40;
  static constexpr uint16_t ATTENREG1 = 0x41;
  static constexpr uint16_t ATTENREG2 = 0x42;
  static constexpr uint16_t ATTENREG3 = 0x43;
  static constexpr uint16_t MPAN = 0x44;
  static constexpr uint16_t MSTEREO = 0x50;

  MikeyPimpl() : mAudioChannels{ AudioChannel{0}, AudioChannel{1}, AudioChannel{2}, AudioChannel{3} },
    mAttenuationLeft{ 0x3c, 0x3c, 0x3c, 0x3c },
    mAttenuationRight{ 0x3c, 0x3c, 0x3c, 0x3c },
    mRegisterPool{}, mPan{ 0xff }, mStereo{}
  {
    std::fill_n( mRegisterPool.data(), mRegisterPool.size(), (uint8_t)0xff );
  }

  ~MikeyPimpl() {}

  int64_t write( int64_t tick, uint8_t address, uint8_t value )
  {
    assert( address >= 0x20 );

    if ( address < 0x40 )
    {
      size_t idx = ( address >> 3 ) & 3;
      switch ( address & 0x7 )
      {
      case VOLCNTRL:
        mAudioChannels[idx].setVolume( (int8_t)value );
        break;
      case FEEDBACK:
        mAudioChannels[idx].setFeedback( value );
        break;
      case OUTPUT:
        mAudioChannels[idx].setOutput( value );
        break;
      case SHIFT:
        mAudioChannels[idx].setShift( value );
        break;
      case BACKUP:
        return mAudioChannels[idx].setBackup( tick, value );
      case CONTROL:
        return mAudioChannels[idx].setControl( tick, value );
      case COUNTER:
        return mAudioChannels[idx].setCounter( tick, value );
      case OTHER:
        mAudioChannels[idx].setOther( value );
        break;
      }
    }
    else
    {
      int idx = address & 3;
      switch ( address )
      {
      case ATTENREG0:
      case ATTENREG1:
      case ATTENREG2:
      case ATTENREG3:
        mRegisterPool[8*4+idx] = value;
        mAttenuationRight[idx] = ( value & 0x0f ) << 2;
        mAttenuationLeft[idx] = ( value & 0xf0 ) >> 2;
        break;
      case MPAN:
        mPan = value;
        break;
      case MSTEREO:
        mStereo = value;
        break;
      default:
        break;
      }
    }
    return 0;
  }

  int64_t fireTimer( int64_t tick )
  {
    size_t timer = tick & 0x0f;
    assert( timer < 4 );
    return mAudioChannels[timer].fireAction( tick );
  }

  AudioSample sampleAudio( int* oscb ) const
  {
    int left{};
    int right{};

    for ( size_t i = 0; i < 4; ++i )
    {
      int oscbWrite = 0;

      if ( ( mStereo & ( (uint8_t)0x01 << i ) ) == 0 )
      {
        const int attenuation = ( mPan & ( (uint8_t)0x01 << i ) ) != 0 ? mAttenuationLeft[i] : 0x3c;
        left += mAudioChannels[i].getOutput() * attenuation;
        oscbWrite += mAudioChannels[i].getOutput() * attenuation;
      }

      if ( ( mStereo & ( (uint8_t)0x10 << i ) ) == 0 )
      {
        const int attenuation = ( mPan & ( (uint8_t)0x01 << i ) ) != 0 ? mAttenuationRight[i] : 0x3c;
        right += mAudioChannels[i].getOutput() * attenuation;
        oscbWrite += mAudioChannels[i].getOutput() * attenuation;
      }

      if (oscb!=NULL) {
        oscb[i]=oscbWrite;
      }
    }

    return { (int16_t)left, (int16_t)right };
  }

  uint8_t const* getRegisterPool( int64_t tick )
  {
    for ( size_t i = 0; i < mAudioChannels.size(); ++i )
    {
      mAudioChannels[i].fillRegisterPool( tick, mRegisterPool.data() + 8 * i );
    }

    return  mRegisterPool.data();
  }

private:

  std::array<AudioChannel, 4> mAudioChannels;
  std::array<int, 4> mAttenuationLeft;
  std::array<int, 4> mAttenuationRight;
  std::array<uint8_t, 4 * 8 + 4> mRegisterPool;

  uint8_t mPan;
  uint8_t mStereo;
};


Mikey::Mikey( uint32_t sampleRate ) : mMikey{ std::make_unique<MikeyPimpl>() }, mQueue{ std::make_unique<ActionQueue>() }, mTick{}, mNextTick{}, mSampleRate{ sampleRate }, mSamplesRemainder{}, mTicksPerSample{ 16000000 / mSampleRate, 16000000 % mSampleRate }
{
  selectPOPCNT();
  enqueueSampling();
}

Mikey::~Mikey()
{
}

void Mikey::write( uint8_t address, uint8_t value )
{
  if ( auto action = mMikey->write( mTick, address, value ) )
  {
    mQueue->push( action );
  }
}

void Mikey::enqueueSampling()
{
  mTick = mNextTick & ~15;
  mNextTick = mNextTick + mTicksPerSample.first;
  mSamplesRemainder += mTicksPerSample.second;
  if ( mSamplesRemainder > mSampleRate )
  {
    mSamplesRemainder %= mSampleRate;
    mNextTick += 1;
  }

  mQueue->push( ( mNextTick & ~15 ) | 4 );
}

void Mikey::sampleAudio( int16_t* bufL, int16_t* bufR, size_t size, int* oscb )
{
  size_t i = 0;
  while ( i < size )
  {
    int64_t value = mQueue->pop();
    if ( ( value & 4 ) == 0 )
    {
      if ( auto newAction = mMikey->fireTimer( value ) )
      {
        mQueue->push( newAction );
      }
    }
    else
    {
      auto sample = mMikey->sampleAudio( oscb );
      bufL[i] = sample.left;
      bufR[i] = sample.right;
      i += 1;
      enqueueSampling();
    }
  }
}

uint8_t const* Mikey::getRegisterPool()
{
  return mMikey->getRegisterPool( mTick );
}

}
