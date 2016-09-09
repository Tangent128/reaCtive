
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "core.h"

void reaC_read(ReaC_Reader *context, reaC_err end, ReaC_Writer *callback, uintptr_t control)
{
    // auto-free simple filters/sources
    if(end && (context->flags & REAc_AUTOFREE)) {
        context->func(context, end, callback, control);
        free(context);
    } else {
        // on -O2, this should optimize to jump
        context->func(context, end, callback, control);
    }
}

void reaC_write(ReaC_Writer *context, reaC_err end, uintptr_t a, uintptr_t b)
{
    // on -O2, this should optimize to jump
    context->func(context, end, a, b);
}

struct drain_writer {
    ReaC_Writer writer;
    ReaC_Reader *source;
};
static void drain_write(ReaC_Writer *context, reaC_err end, uintptr_t a, uintptr_t b)
{
    (void)(a + b);
    struct drain_writer *state = (struct drain_writer *) context;

    if(end == REAc_OK) {
        // on -O2, this should optimize to jump
        reaC_read(state->source, end, context, 0);
    } else {
        // if stream's over, free reader & sink
        reaC_read(state->source, end, NULL, 0);
        free(context);
    }

}
void reaC_drain (ReaC_Reader *source)
{
    struct drain_writer *state = calloc(1, sizeof(struct drain_writer));

    state->writer.func = drain_write;
    state->source = source;

    reaC_read(source, 0, (ReaC_Writer *) state, 0);
}
