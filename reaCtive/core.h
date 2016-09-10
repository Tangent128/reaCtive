
#pragma once
#if ! defined(REAcTIVE_H)
#define REAcTIVE_H

#include <stdint.h>

/*
 * Error codes & return type; all errors are negative
 * --------------------------------------------------
 */
typedef int reaC_err;

/* nothing wrong */
#define REAc_OK 0

/* normal stream termination */
#define REAc_DONE 1

/*
 * Flags
 * --------------------------------------------------
 */

/* Reader should simply be free()d when canceled;
 * This removes a simple reader function's need to
 * handle its cleanup case.
 */
#define REAc_AUTOFREE 0x02

/* Pull Streams.
 * --------------------------------------------------
 */

typedef struct ReaC_Writer ReaC_Writer;
typedef struct ReaC_Reader ReaC_Reader;

typedef void ReaC_Writer_Func(ReaC_Writer *context, reaC_err end, uintptr_t a, uintptr_t b);
typedef void ReaC_Reader_Func(ReaC_Reader *context, reaC_err end, ReaC_Writer *callback, uintptr_t control);

struct ReaC_Writer {
    ReaC_Writer_Func *func;
};

/* A Reader is a closure of a source or filter's state.
 * When called with a zero error code and a Writer,
 * it arranges to eventually:
 *
 * - call the Writer with a single data element
 * - call the Writer with an error code indicating end-of-stream
 *
 * The Reader will not receive another such call until
 * it has responded to the Writer.
 *
 * However, at any time, whether a Writer is queued or not,
 * the Reader may be called with a nonzero error code indicating
 * it should dispose itself. Once such a call returns:
 *
 * - any queued Writer will never be called
 * - all resources used by the Reader, including wrapped Readers
 *   and the Reader's struct itself, shall be freed.
 *
 * A Reader function may also take a control argument; its
 * interpretation is specific to specific Reader implementations.
 * Zero must be a sane default value for control in any case.
 * --------------------------------------------------
 */
struct ReaC_Reader {
    ReaC_Reader_Func *func;

    uintptr_t flags;
};

/* Public API functions.
 * --------------------------------------------------
 */
void reaC_read  (ReaC_Reader *context, reaC_err end, ReaC_Writer *callback, uintptr_t control);
void reaC_write (ReaC_Writer *context, reaC_err end, uintptr_t a, uintptr_t b);

void reaC_drain (ReaC_Reader *source);

/* Streamline the common "filter" case
 * --------------------------------------------------
 */

typedef struct ReaC_Filter ReaC_Filter;
typedef struct ReaC_Filter_Reader ReaC_Filter_Reader;

typedef void ReaC_Filter_Destroy_Func(ReaC_Filter *context);

struct ReaC_Filter {
    ReaC_Writer writer;
    ReaC_Writer *output;
    ReaC_Filter_Destroy_Func *destroy;
};

struct ReaC_Filter_Reader {
    ReaC_Reader reader;
    ReaC_Reader *input;
    ReaC_Filter filter[];
};

ReaC_Filter_Reader *reaC_new_filter (ReaC_Reader *source, ReaC_Writer_Func *filter, size_t size, ReaC_Filter_Destroy_Func *destroy);

#endif
