#ifndef _ASSERT_BY_KES_H_
#define _ASSERT_BY_KES_H_

#include <assert.h>


#define ASSERT(expr, false_func)							\
  ((expr)								\
   ? __ASSERT_VOID_CAST (0)						\
   : ((false_func), __assert_fail (#expr, __FILE__, __LINE__, __ASSERT_FUNCTION)) )


#endif // _ASSERT_BY_KES_H_