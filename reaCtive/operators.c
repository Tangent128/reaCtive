
#include <stdlib.h>

#include "operators.h"

/* COUNT: generates an endless (until overflow) stream of ints */

static void count_init(Observable *context)
{
    int i;
    for(i = 0; 1; i++) {
        if(reaC_emit_next(context, i, 0)) {
            /* some error downstream, stop */
            break;
        }
    }
    reaC_emit_error(context, 0, 0);
};
Observable *reaC_new_count()
{
    Observable *obsv = calloc(1, sizeof(Observable));

    obsv->init = &count_init;

    return obsv;
}

/* LIMIT: stops a sequence after a number of results */

struct limit_state {
    int seen;
    int max;
};
static void limit_next(Observable *context, uintptr_t a, uintptr_t b)
{
    struct limit_state *state = context->userdata;

    /* pass along unchanged */
    reaC_emit_next(context, a, b);

    /* stop if at end */
    state->seen++;
    if(state->seen == state->max) {
        reaC_emit_finish(context, 0, 0);
    }
};
Observable *reaC_op_limit(Observable *producer, int max)
{
    void *limiter = calloc(1, sizeof(Observable) + sizeof(struct limit_state));

    Observable *obsv = limiter;
    struct limit_state *state = limiter + sizeof(Observable);

    obsv->next = limit_next;
    obsv->userdata = state;
    state->max = max;

    reaC_subscribe(producer, obsv, 0);

    return obsv;
}

/* MAP: allows applying a transform function to each element in a
 * stream; a & b are passed by reference, to allow modifying them.
 * The transform function may be given a context pointer; be advised
 * that this context pointer is not auto-freed.
 */
struct map_state {
    reaC_err (*emit)(Observable *context, uintptr_t a, uintptr_t b);
    void *context;
    reaC_op_map_func *transform;
};
static void map_action(Observable *context, uintptr_t a, uintptr_t b)
{
    struct map_state *state = context->userdata;

    state->transform(state->context, &a, &b);

    state->emit(context, a, b);
}
static Observable *op_map_common(Observable *producer, void *context, reaC_op_map_func *transform)
{
    void *mapper = calloc(1, sizeof(Observable) + sizeof(struct map_state));

    Observable *obsv = mapper;
    struct map_state *state = mapper + sizeof(Observable);

    obsv->userdata = state;
    state->context = context;
    state->transform = transform;

    reaC_subscribe(producer, obsv, 0);

    return obsv;
}
Observable *reaC_op_map(Observable *producer, void *context, reaC_op_map_func *transform)
{
    Observable *obsv = op_map_common(producer, context, transform);

    ((struct map_state*) obsv->userdata)->emit = reaC_emit_next;
    obsv->next = map_action;

    return obsv;
}
Observable *reaC_op_map_error(Observable *producer, void *context, reaC_op_map_func *transform)
{
    Observable *obsv = op_map_common(producer, context, transform);

    ((struct map_state*) obsv->userdata)->emit = reaC_emit_error;
    obsv->error = map_action;

    return obsv;
}
Observable *reaC_op_map_finish(Observable *producer, void *context, reaC_op_map_func *transform)
{
    Observable *obsv = op_map_common(producer, context, transform);

    ((struct map_state*) obsv->userdata)->emit = reaC_emit_finish;
    obsv->finish = map_action;

    return obsv;
}
