
#include <stdlib.h>
#include <stdio.h>

#include "reaCtive.h"

/* Demo observable, generates a finite stream of ints */

struct count_state {
    int max;
};

static void count_init(Observable *context)
{
    struct count_state *state = context->userdata;
    int i;
    printf("Start counting.\n", i);
    for(i = 0; i < state->max; i++) {
        printf("Emitting %d\n", i);
        reaC_emit_next(context, i, state->max);
    }
    printf("Done counting.\n", i);
    reaC_cancel(context);
};
static void count_dispose(Observable *context)
{
    struct count_state *state = context->userdata;
    printf("Disposing counter with max %d.\n", state->max);
};

static Observable *new_count_sequence(int max)
{
    void *obj = calloc(1, sizeof(Observable) + sizeof(struct count_state));

    Observable *obsv = obj;
    struct count_state *state = obj + sizeof(Observable);

    obsv->init = &count_init;
    obsv->dispose = &count_dispose;
    obsv->userdata = state;
    state->max = max;

    return obj;
}

/* Demo observable, prints data items as ints */

static void int_spy_next(Observable *context, uintptr_t a, uintptr_t b)
{
    printf("Spied values %d %d\n", a, b);
    /* pass along unchanged */
    reaC_emit_next(context, a, b);
}
static void int_spy_dispose(Observable *context)
{
    printf("Disposing int spy.\n");
};

static Observable *new_int_spy()
{
    Observable *obsv = calloc(1, sizeof(Observable));
    obsv->next = int_spy_next;
    obsv->dispose = int_spy_dispose;
    return obsv;
}

/* Main */

int main(int argc, char **argv)
{
    Observable *one = new_count_sequence(5);
    Observable *two = new_int_spy();
    Observable *three = new_int_spy();
    Observable *four = new_int_spy();
    reaC_subscribe(one, two, 0);
    reaC_subscribe(two, three, 0);
    reaC_subscribe(three, four, 0);
    reaC_start(four);
    return 0;
}
