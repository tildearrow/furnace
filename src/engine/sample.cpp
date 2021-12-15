#include "sample.h"

DivSample::~DivSample() {
  if (data) delete data;
  if (rendData) delete rendData;
  if (adpcmRendData) delete adpcmRendData;
}
