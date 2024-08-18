
/* Include the primary system assert.h */
#include_next <assert.h>


/* now add the missing definition of static_assert for C11 code, added to the 10.11 SDK. */
/* if a newer assert.h header has already done this in a modern SDK, then */
/* _ASSERT_H_ will be defined and will block this */

#ifndef _ASSERT_H_
#define _ASSERT_H_

#ifndef __cplusplus
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define static_assert _Static_assert
#endif /* __STDC_VERSION__ */
#endif /* !__cplusplus */

#endif /* _ASSERT_H_ */
