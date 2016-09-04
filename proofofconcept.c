
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
static void end_printf(void *context, reaC_err end, uintptr_t a, uintptr_t b)
{
    char *format = context;
    printf(format, end, a, b);
}
static void cleanup_printf(void *context, reaC_err end)
{
    (void)(end);
    char *format = context;
    printf("%s", format);
}
int main(int argc, char **argv)
{
    (void)(argc + argv);
    ReaC_Reader *chain2 = reaC_new_count2();
    chain2 = reaC_op_map2(chain2, "Observe %lu %lu\n", map_printf);
    chain2 = reaC_op_cleanup2(chain2, "Canceled Source\n", cleanup_printf);
    chain2 = reaC_op_take2(chain2, 3);
    chain2 = reaC_op_map2(chain2, "Observe2 %lu %lu\n", map_printf);
    chain2 = reaC_op_on_end2(chain2, "End of Stream %d %lu %lu\n", end_printf);
    reaC_drain(chain2);

    return 0;
}
