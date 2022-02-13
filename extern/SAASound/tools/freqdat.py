# Part of SAASound copyright 1998-2018 Dave Hooper <dave@beermex.com>
#
# freqdat.py
# You can use this to generate an alternative SAAFreq.dat
#
# By default, the SAA-1099 generator will compute a frequency table at runtime
# but you can set the SAA_FIXED_CLOCKRATE define flag and supply your own (fixed,
# precompiled) frequency table named SAAFreq.dat
#
# This is not super-useful for general-purpose computing devices (e.g PCs), but is
# useful for embedded or low-capability devices, or for hardware-based implementations
#
# To use this simply run the file and pipe the output into src/SAAFreq.dat

BASE = 8000000
SCALE = 4096

for octave in range(0,8):
    for offset in range(0,256):
        f = 2 * SCALE * (BASE/8000000) * 15625 * (2**octave) / (511-offset)
        print(int(f), ',')
