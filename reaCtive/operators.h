
#pragma once
#if ! defined(REAcTIVE_OPERATORS_H)
#define REAcTIVE_OPERATORS_H

#include "core.h"

/* COUNT: generates an endless (until overflow) stream of ints */
Observable *reaC_new_count();

/* LIMIT: stops a sequence after a number of results */
Observable *reaC_op_limit(Observable *producer, int max);

/* MAP: allows applying a transform function to each element in a
 * stream; a & b are passed by reference, to allow modifying them.
 * The transform function may be given a context pointer; be advised
 * that this context pointer is not auto-freed.
 */
typedef void reaC_op_map_func(void *context, uintptr_t *a, uintptr_t *b);
Observable *reaC_op_map(Observable *producer, void *context, reaC_op_map_func *transform);
Observable *reaC_op_map_error(Observable *producer, void *context, reaC_op_map_func *transform);
Observable *reaC_op_map_finish(Observable *producer, void *context, reaC_op_map_func *transform);

/* TEARDOWN: runs a function when the pipeline is being disposed,
 * whether due to error, finishing, or cancellation. The function
 * receives a context pointer, which is not auto-freed. However,
 * a teardown function is a suitable place to free context pointers
 * used, for example, by map transform functions.
 */
typedef void reaC_op_teardown_func(void *context);
Observable *reaC_op_teardown(Observable *producer, void *context, reaC_op_teardown_func *dispose);

#endif
