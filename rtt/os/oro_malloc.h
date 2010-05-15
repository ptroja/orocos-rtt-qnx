/**
 * @file
 * Wraps the oro_rt_ allocator functions to standard C malloc function family
 * or TLSF if available.
 */
#ifndef _ORO_MALLOC_H_
#define _ORO_MALLOC_H_


#include "../rtt-config.h"
#ifdef OS_RT_MALLOC
#include "tlsf/tlsf.h"
#else

#include <cstdlib>
#define oro_rt_malloc std::malloc
#define oro_rt_free std::free
#define oro_rt_realloc std::realloc
#define oro_rt_calloc std::rt_calloc

#endif

#endif
