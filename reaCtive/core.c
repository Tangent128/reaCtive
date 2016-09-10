
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

static void reaC_filter_read (ReaC_Reader *context, reaC_err end, ReaC_Writer *callback, uintptr_t control)
{
    (void)(control);
    struct ReaC_Filter_Reader *reader = (ReaC_Filter_Reader *) context;
    if(end) {
        // shutting down, free source then teardown filter
        reaC_read(reader->input, end, NULL, 0);
        if(reader->filter->destroy != NULL) {
            reader->filter->destroy(reader->filter);
        }
    } else {
        // normal operation, register filter for next item
        reader->filter->output = callback;
        reaC_read(reader->input, end, (ReaC_Writer *) reader->filter, 0);
    }
}

ReaC_Filter_Reader *reaC_new_filter (ReaC_Reader *source, ReaC_Writer_Func *filter, size_t size, ReaC_Filter_Destroy_Func *destroy)
{
    struct ReaC_Filter_Reader *reader = calloc(1, sizeof(struct ReaC_Filter_Reader) + size);

    reader->reader.func = reaC_filter_read;
    reader->reader.flags |= REAc_AUTOFREE;

    reader->input = source;

    reader->filter->writer.func = filter;
    reader->filter->destroy = destroy;

    return (ReaC_Filter_Reader *) reader;
}
