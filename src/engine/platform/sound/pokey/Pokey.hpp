#pragma once

#include <cstdint>
#include <memory>

// can you forgive me
#include "../../../dispatch.h"

namespace Test
{

class PokeyPimpl;

class Pokey
{
public:

  Pokey( uint32_t pokeyClock, uint32_t sampleRate );
  ~Pokey();

  void write( uint8_t address, uint8_t value );
  void sampleAudio( int16_t* buf, size_t size, DivDispatchOscBuffer** oscb = NULL );

  uint8_t const* getRegisterPool();

  void reset();

private:

  std::unique_ptr<PokeyPimpl> mPokey;
  uint32_t mPokeyClock;
  uint32_t mSampleRate;
};

}
