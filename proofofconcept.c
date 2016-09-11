
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
    ReaC_Reader *chain = reaC_constant_source(
        (uintptr_t) testString, sizeof(testString) - 1);
    chain = reaC_take(chain, 1);
    chain = reaC_map(chain, NULL, map_print_cstr);
    reaC_drain(chain);

    return 0;
}
