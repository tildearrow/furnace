#pragma once

#include <cstdint>
#include <memory>

namespace AltASAP
{

class PokeyPimpl;

class Pokey
{
public:

  Pokey();
  void init( uint32_t pokeyClock, uint32_t sampleRate );
  ~Pokey();

  void write( uint8_t address, uint8_t value );
  int16_t sampleAudio( short* oscb = nullptr );

  uint8_t const* getRegisterPool();

  void reset();

private:
  uint32_t mPokeyClock;
  uint32_t mSampleRate;
  std::unique_ptr<PokeyPimpl> mPokey;
};

}
