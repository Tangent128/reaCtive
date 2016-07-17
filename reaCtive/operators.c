
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
