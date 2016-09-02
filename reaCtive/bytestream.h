
#pragma once
#if ! defined(REAcTIVE_BYTESTREAM_H)
#define REAcTIVE_BYTESTREAM_H

#include "core.h"

/* TEARDOWN: runs a function when the pipeline is being disposed
 */
typedef void reaC_op_teardown_func(void *context);
Observable *reaC_op_teardown(Observable *producer, void *context, reaC_op_teardown_func *dispose);

// bytestream -> ASCII codepoints?
// bytestream<UTF-8> -> Unicode codepoints?

#endif
