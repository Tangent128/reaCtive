
#include <stdlib.h>
#include <stdio.h>

#include "reaCtive/core.h"
#include "reaCtive/operators.h"

/* Main */

static const char testString[] = "Hello, Reactive World!";

static void map_print_cstr(void *context, uintptr_t *address, uintptr_t *count)
{
    (void)(context);
    printf("%.*s\n", (int) *count, (char *) *address);
}
int main(int argc, char **argv)
{
    (void)(argc + argv);
    ReaC_Reader *chain2 = reaC_constant_source(
        (uintptr_t) testString, sizeof(testString) - 1);
    chain2 = reaC_op_take2(chain2, 1);
    chain2 = reaC_op_map2(chain2, NULL, map_print_cstr);
    reaC_drain(chain2);

    return 0;
}
