#include <Cocoa/Cocoa.h>
#include "macstuff.h"

double getMacDPIScale() {
  CGFloat val=[[NSScreen mainScreen] backingScaleFactor];
  return (double)val;
}
