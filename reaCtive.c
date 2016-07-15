
#include <stdbool.h>
#include <stdlib.h>

#include "reaCtive.h"

static bool reaC_validate_context(Observable *context)
{
    if(context == NULL) return REAc_EINVAL;
    if(context->flags & REAc_DISPOSED) return REAc_EINVAL;
}

/* Hook two Observables together as part of a chain. 
 */
reaC_err reaC_subscribe(Observable *producer, Observable *consumer, unsigned int flags)
{
    if(reaC_validate_context(producer) < 0) return REAc_EINVAL;
    if(reaC_validate_context(consumer) < 0) return REAc_EINVAL;

    producer->consumer = consumer;
    consumer->producer = producer;
}

/* Run after any Observable init/next/error/finish call;
 * disposes / frees the Observable if appropriate.
 * Returns true if the consumer has been disposed.
 */
static bool reaC_cleanup(Observable *context)
{
    if(context->flags & REAc_CANCELLING) {
        /* cancel parent */
        if(context->producer != NULL) {
            context->producer->consumer = NULL;
            context->producer->flags |= REAc_CANCELLING;
        }
        /* free resources */
        if(context->dispose != NULL) {
            context->dispose(context);
        }
        context->flags |= REAc_DISPOSED;
        if(!(context->flags & REAc_PINNED)) {
            free(context);
        }
        return true;
    } else {
        return false;
    }
}

/* Indicates that this pipeline is no longer interesting;
 * children will be immediately disposed without any error
 * or finish signals; parents will be disposed asap.
 */
reaC_err reaC_cancel(Observable *context)
{
    if(reaC_validate_context(context) < 0) return REAc_EINVAL;

    /* mark self for reaping once action complete */
    context->flags |= REAc_CANCELLING;

    /* reap downstream */
    if(context->consumer != NULL) {
        reaC_cancel(context->consumer);
        reaC_cleanup(context->consumer);
    }

    return 0;
}

/* Initialize the chain terminated by `consumer`;
 * at this point, all Observables in the chain may execute
 * as soon as the head of the chain produces values, either
 * synchronously or through some sort of a scheduler/event loop.
 * reaCtive will handle freeing the Observables after the chain
 * completes, except for any with the REAc_PINNED flag set.
 */
reaC_err reaC_start(Observable *consumer)
{
    if(reaC_validate_context(consumer) < 0) return REAc_EINVAL;

    /* Note parent now in case init() ends the sequence */
    Observable *producer = consumer->producer;

    if(consumer->init != NULL) {
        consumer->init(consumer);
    }
    reaC_cleanup(consumer);

    if(producer != NULL) {
        return reaC_start(consumer->producer);
    }
}

/* Send a data item to the subscriber.
 */
reaC_err reaC_emit_next(Observable *context, uintptr_t a, uintptr_t b)
{
    if(reaC_validate_context(context) < 0) return REAc_EINVAL;

    if(context->consumer != NULL) {
        if(context->consumer->next) {
            context->consumer->next(context->consumer, a, b);
        }
        if(reaC_cleanup(context->consumer)) {
            return REAc_ECANCELLED;
        } else {
            return 0;
        }
    } else {
        return REAc_ENO_LISTENER;
    }
}
