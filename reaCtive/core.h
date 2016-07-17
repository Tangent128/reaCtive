
#pragma once
#if ! defined(REAcTIVE_H)
#define REAcTIVE_H

#include <stdint.h>

/*
 * Error codes & return type; all errors are negative
 * --------------------------------------------------
 */
typedef int reaC_err;

/* bad arguments given to a function */
#define REAc_EINVAL -1

/* tried to emit a value with nobody subscribed */
#define REAc_ENO_LISTENER -2

/* the subscriber cancelled their subscription;
 * the calling function should wrap up quickly,
 * and not waste time generating ignored items,
 * to allow running dispose() asap.
 */
#define REAc_ECANCELLED -3

/*
 * Flags
 * --------------------------------------------------
 */

/* Observable should not be auto-freed when the pipeline stops;
 * dispose() still runs.
 */
#define REAc_PINNED 0x01

/* Observable is ready to free */
#define REAc_DISPOSED 0x10

/* Observable should be disposed asap */
#define REAc_CANCELLING 0x20

/* Core data structure; an Observable is a component in the
 * pipeline that may be subscribed to by one other Observable,
 * which receives next, error, and finish events.
 * --------------------------------------------------
 */
typedef struct Observable Observable; 
struct Observable {
    unsigned int flags;

    /* Links */
    Observable *producer;
    Observable *consumer;

    /* Implementation methods */
    void (*init)    (Observable *context);
    void (*next)    (Observable *context, uintptr_t a, uintptr_t b);
    void (*error)   (Observable *context, uintptr_t a, uintptr_t b);
    void (*finish)  (Observable *context, uintptr_t a, uintptr_t b);
    void (*dispose) (Observable *context);

    /* Arbitrary state */
    void *userdata;
};

/* Public API functions.
 * --------------------------------------------------
 */
reaC_err reaC_subscribe(Observable *producer, Observable *consumer, unsigned int flags);
reaC_err reaC_start(Observable *consumer);

/* Public API functions within an Observable's execution.
 * --------------------------------------------------
 */
reaC_err reaC_emit_next(Observable *context, uintptr_t a, uintptr_t b);
reaC_err reaC_emit_error(Observable *context, uintptr_t a, uintptr_t b);
reaC_err reaC_emit_finish(Observable *context, uintptr_t a, uintptr_t b);
reaC_err reaC_cancel(Observable *context);

#endif
