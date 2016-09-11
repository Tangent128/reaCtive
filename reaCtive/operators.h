
#pragma once
#if ! defined(REAcTIVE_OPERATORS_H)
#define REAcTIVE_OPERATORS_H

#include "core.h"

/* COUNT: generates an endless (until overflow) stream of ints */
ReaC_Reader *reaC_count_source();

/* CONSTANT: generates an endless stream of a specific value */
ReaC_Reader *reaC_constant_source(uintptr_t a, uintptr_t b);

/* TAKE: stops a sequence after a number of results */
ReaC_Reader *reaC_take(ReaC_Reader *source, uintmax_t max);

/* MAP: allows applying a transform function to each element in a
 * stream; a & b are passed by reference, to allow modifying them.
 * The transform function may be given a context pointer; be advised
 * that this context pointer is not auto-freed.
 */
typedef void reaC_map_func(void *context, uintptr_t *a, uintptr_t *b);
ReaC_Reader *reaC_map(ReaC_Reader *source, void *context, reaC_map_func *transform);

/* ON_EOF: runs a function when a termination code comes from upstream.
 * Possibly useful for error detection, but does not run on cancel.
 * The handler function may be given a context pointer; be advised
 * that this context pointer is not auto-freed.
 */
typedef void reaC_on_eof_func(void *context, reaC_err end, uintptr_t a, uintptr_t b);
ReaC_Reader *reaC_on_eof(ReaC_Reader *source, void *context, reaC_on_eof_func *handler);

/* ON_CLEANUP: runs a function upon being canceled. The function
 * receives a context pointer, which is not auto-freed. However,
 * a cleanup function is a suitable place to free context pointers
 * used, for example, by map transform functions.
 */
typedef void reaC_on_cleanup_func(void *context, reaC_err end);
ReaC_Reader *reaC_on_cleanup(ReaC_Reader *source, void *context, reaC_on_cleanup_func *handler);

#endif
