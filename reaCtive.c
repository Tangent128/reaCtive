
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
        context->dispose(context);
        context->flags |= REAc_DISPOSED;
        /* autofree observable */
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

/* Default implementations of Observable methods */
static void reaC_default_init(Observable *context) { /* noop */ }
static void reaC_default_next(Observable *context, uintptr_t a, uintptr_t b) { /* noop */ }
static void reaC_default_error(Observable *context, uintptr_t a, uintptr_t b)
{
    reaC_emit_error(context, a, b);
}
static void reaC_default_finish(Observable *context, uintptr_t a, uintptr_t b)
{
    reaC_emit_finish(context, a, b);
}
static void reaC_default_dispose(Observable *context) { /* noop */ }

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

    /* Patch with default functions */
    if(consumer->init == NULL) consumer->init = reaC_default_init;
    if(consumer->next == NULL) consumer->next = reaC_default_next;
    if(consumer->error == NULL) consumer->error = reaC_default_error;
    if(consumer->finish == NULL) consumer->finish = reaC_default_finish;
    if(consumer->dispose == NULL) consumer->dispose = reaC_default_dispose;

    /* Note parent now in case init() ends the sequence */
    Observable *producer = consumer->producer;

    consumer->init(consumer);
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
        context->consumer->next(context->consumer, a, b);
        if(reaC_cleanup(context->consumer)) {
            return REAc_ECANCELLED;
        } else {
            return 0;
        }
    } else {
        return REAc_ENO_LISTENER;
    }
}

reaC_err reaC_emit_error(Observable *context, uintptr_t a, uintptr_t b)
{
    if(reaC_validate_context(context) < 0) return REAc_EINVAL;

    /* end of pipeline, mark self for reaping once action complete */
    context->flags |= REAc_CANCELLING;

    if(context->consumer != NULL) {
        context->consumer->error(context->consumer, a, b);
        reaC_cleanup(context->consumer);
        return 0;
    } else {
        return REAc_ENO_LISTENER;
    }
}

reaC_err reaC_emit_finish(Observable *context, uintptr_t a, uintptr_t b)
{
    if(reaC_validate_context(context) < 0) return REAc_EINVAL;

    /* end of pipeline, mark self for reaping once action complete */
    context->flags |= REAc_CANCELLING;

    if(context->consumer != NULL) {
        context->consumer->finish(context->consumer, a, b);
        reaC_cleanup(context->consumer);
        return 0;
    } else {
        return REAc_ENO_LISTENER;
    }
}
