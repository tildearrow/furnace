/* brrUtils - BRR audio codec utilities
 * Copyright (C) 2022 tildearrow 
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _BRR_UTILS_H
#define _BRR_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * read len samples from buf, encode in BRR and output to out.
 * @param buf input data.
 * @param out output buffer. shall be at least 9*((15+len)/16) shorts in size (9 more if loopStart is not -1!)
 * @param len input length (should be a multiple of 16. if it isn't, the output will be padded).
 * @param loopStart beginning of loop area (may be -1 for no loop). this is used to ensure the respective block has no filter in order to loop properly.
 * @param emphasis apply filter to compensate for Gaussian interpolation high frequency loss.
 * @return number of written samples.
 */
long brrEncode(short* buf, unsigned char* out, long len, long loopStart, unsigned char emphasis);

/**
 * read len bytes from buf, decode BRR and output to out.
 * @param buf input data.
 * @param out output buffer. shall be at least 16*(len/9) shorts in size.
 * @param len input length (shall be a multiple of 9).
 * @param emphasis apply filter to simulate Gaussian interpolation high frequency loss.
 * @return number of written bytes.
 */
long brrDecode(unsigned char* buf, short* out, long len, unsigned char emphasis);

#ifdef __cplusplus
}
#endif

#endif
