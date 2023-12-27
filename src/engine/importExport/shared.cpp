#include "shared.h"

bool stringNotBlank(String& str) {
  return str.size() > 0 && str.find_first_not_of(' ') != String::npos;
}

// detune needs extra translation from register to furnace format
uint8_t fmDtRegisterToFurnace(uint8_t&& dtNative) {
  return (dtNative>=4) ? (7-dtNative) : (dtNative+3);
}

void readSbiOpData(sbi_t& sbi, SafeReader& reader) {
  sbi.Mcharacteristics = reader.readC();
  sbi.Ccharacteristics = reader.readC();
  sbi.Mscaling_output = reader.readC();
  sbi.Cscaling_output = reader.readC();
  sbi.Meg_AD = reader.readC();
  sbi.Ceg_AD = reader.readC();
  sbi.Meg_SR = reader.readC();
  sbi.Ceg_SR = reader.readC();
  sbi.Mwave = reader.readC();
  sbi.Cwave = reader.readC();
  sbi.FeedConnect = reader.readC();
}