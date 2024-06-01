/* Momo - portable gettext() implementation
 * Copyright (C) 2024 tildearrow 
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

#ifdef __cplusplus
extern "C" {
#endif

#define MOMO_FORMATARG __attribute__((format_arg(1)))

const char* momo_setlocale(int type, const char* locale);
const char* momo_bindtextdomain(const char* domainName, const char* dirName);
const char* momo_textdomain(const char* domainName);

const char* momo_gettext(const char* str) MOMO_FORMATARG;
const char* momo_ngettext(const char* str1, const char* str2, unsigned long amount);

#ifdef __cplusplus
}
#endif

#ifdef MOMO_LIBINTL
#define setlocale momo_setlocale
#define bindtextdomain momo_bindtextdomain
#define textdomain momo_textdomain

#define gettext momo_gettext
#define ngettext momo_ngettext
#endif
