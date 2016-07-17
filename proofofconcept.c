
#include <stdlib.h>
#include <stdio.h>

#include "reaCtive/core.h"
#include "reaCtive/operators.h"

/* Demo observable, prints data items as ints */

static void int_spy_next(Observable *context, uintptr_t a, uintptr_t b)
{
    printf("Spied values %d %d\n", a, b);
    /* pass along unchanged */
    reaC_emit_next(context, a, b);
}
static void int_spy_error(Observable *context, uintptr_t a, uintptr_t b)
{
    printf("Spied error %d %d\n", a, b);
    /* pass along unchanged */
    reaC_emit_error(context, a, b);
}
static void int_spy_finish(Observable *context, uintptr_t a, uintptr_t b)
{
    printf("Spied completion %d %d\n", a, b);
    /* pass along unchanged */
    reaC_emit_finish(context, a, b);
}
static void int_spy_dispose(Observable *context)
{
    printf("Disposing int spy.\n");
};

static Observable *new_int_spy(Observable *producer)
{
    Observable *obsv = calloc(1, sizeof(Observable));
    obsv->next = int_spy_next;
    obsv->error = int_spy_error;
    obsv->finish = int_spy_finish;
    obsv->dispose = int_spy_dispose;

    reaC_subscribe(producer, obsv, 0);

    return obsv;
}

/* Main */

int main(int argc, char **argv)
{
    Observable *counter = reaC_new_count();
    Observable *spy1 = new_int_spy(counter);
    Observable *limiter = reaC_op_limit(spy1, 3);
    Observable *spy2 = new_int_spy(limiter);
    reaC_start(spy2);
    return 0;
}
