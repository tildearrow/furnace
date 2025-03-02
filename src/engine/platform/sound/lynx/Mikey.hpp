#pragma once

#include <cstdint>
#include <memory>

// can you forgive me
#include "../../../dispatch.h"

namespace Lynx
{

class MikeyPimpl;
class ActionQueue;

class Mikey
{
public:


  Mikey( uint32_t sampleRate );
  ~Mikey();

  void write( uint8_t address, uint8_t value );
  void sampleAudio( int16_t* bufL, int16_t* bufR, size_t size, int* oscb = NULL );

  uint8_t const* getRegisterPool();

private:
  void enqueueSampling();

private:

  std::unique_ptr<MikeyPimpl> mMikey;
  std::unique_ptr<ActionQueue> mQueue;
  uint64_t mTick;
  uint64_t mNextTick;
  uint32_t mSampleRate;
  uint32_t mSamplesRemainder;
  std::pair<uint32_t, uint32_t> mTicksPerSample;
};

}
