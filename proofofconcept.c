
#include <stdlib.h>
#include <stdio.h>

#include "reaCtive/core.h"
#include "reaCtive/operators.h"

/* Main */

static void map_printf(void *context, uintptr_t *a, uintptr_t *b)
{
    char *format = context;
    printf(format, *a, *b);
}
static void dispose_printf(void *context)
{
    char *str = context;
    printf("%s", str);
}
int main(int argc, char **argv)
{
    Observable *chain;
    chain = reaC_new_count();
    chain = reaC_op_map(chain, "Observe %d\n", map_printf);
    chain = reaC_op_map_finish(chain, "Finish %d\n", map_printf);
    chain = reaC_op_limit(chain, 3);
    chain = reaC_op_teardown(chain, "Disposing\n", dispose_printf);
    chain = reaC_op_map(chain, "Observe2 %d\n", map_printf);
    chain = reaC_op_map_finish(chain, "Finish2 %d\n", map_printf);
    reaC_start(chain);

    ReaC_Reader *source = reaC_new_count2();
    reaC_drain(source);

    return 0;
}
