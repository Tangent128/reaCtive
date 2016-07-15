
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
};

static Observable *new_count_sequence(int max)
{
    void *obj = calloc(1, sizeof(Observable) + sizeof(struct count_state));

    Observable *obsv = obj;
    struct count_state *state = obj + sizeof(Observable);

    obsv->init = &count_init;
    obsv->userdata = state;
    state->max = max;

    return obj;
}

int main(int argc, char **argv)
{
    Observable *producer = new_count_sequence(5);
    reaC_start(producer);
    return 0;
}
