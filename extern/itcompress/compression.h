#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

uint32_t it_decompress8(void *dest, uint32_t len, const void *file, uint32_t filelen, int it215, int channels);
uint32_t it_decompress16(void *dest, uint32_t len, const void *file, uint32_t filelen, int it215, int channels);
