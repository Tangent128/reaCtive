
#include <stdlib.h>

#include "operators.h"

/* COUNT: generates an endless (until overflow) stream of ints */

struct count_state {
    ReaC_Reader reader;
    int i;
};
static void count_read(ReaC_Reader *context, reaC_err end, ReaC_Writer *callback, uintptr_t control) {
    (void)(control);
    struct count_state *counter = (struct count_state*) context;
    if(end != 0) {
        return;
    }
    reaC_write(callback, 0, counter->i++, 0);
}
ReaC_Reader *reaC_new_count2()
{
    struct count_state *counter = calloc(1, sizeof(struct count_state));

    counter->reader.func = count_read;
    counter->reader.flags |= REAc_AUTOFREE;

    return (ReaC_Reader *) counter;
}

/* TAKE: stops a sequence after a number of results */

struct take_writer {
    ReaC_Writer writer;

    ReaC_Writer *callback;
};
struct take_reader {
    ReaC_Reader reader;

    ReaC_Reader *source;
    struct take_writer writer;

    uintmax_t seen;
    uintmax_t max;
};
static void take_write(ReaC_Writer *context, reaC_err end, uintptr_t a, uintptr_t b) {
    struct take_writer *taker = (struct take_writer*) context;
    reaC_write(taker->callback, end, a, b);
}
static void take_read(ReaC_Reader *context, reaC_err end, ReaC_Writer *callback, uintptr_t control) {
    (void)(control);
    struct take_reader *taker = (struct take_reader*) context;
    if((end == REAc_OK) && (taker->seen++ >= taker->max)) {
        reaC_write(callback, REAc_DONE, 0, 0);
    } else {
        taker->writer.callback = callback;
        reaC_read(taker->source, end, (ReaC_Writer *) &taker->writer, 0);
    }
}
ReaC_Reader *reaC_op_take2(ReaC_Reader *source, uintmax_t max)
{
    struct take_reader *taker = calloc(1, sizeof(struct take_reader));

    taker->reader.func = take_read;
    taker->reader.flags |= REAc_AUTOFREE;

    taker->source = source;

    taker->writer.writer.func = take_write;
    taker->max = max;

    return (ReaC_Reader *) taker;
}

/* MAP: allows applying a transform function to each element in a
 * stream; a & b are passed by reference, to allow modifying them.
 * The transform function may be given a context pointer; be advised
 * that this context pointer is not auto-freed.
 */

struct map_writer {
    ReaC_Writer writer;

    ReaC_Writer *callback;

    void *context;
    reaC_op_map_func *transform;

    uintptr_t a;
    uintptr_t b;
};
struct map_reader {
    ReaC_Reader reader;

    ReaC_Reader *source;
    struct map_writer writer;
};
static void map_write(ReaC_Writer *context, reaC_err end, uintptr_t a, uintptr_t b) {
    struct map_writer *mapper = (struct map_writer*) context;

    if(end == REAc_OK) {
        /* Important: GCC won't tail call if any pointers to the stack
         * or arguments get passed to functions; "launder" a & b through
         * the context so the transform function can mutate them. */
        mapper->a = a;
        mapper->b = b;
        mapper->transform(mapper->context, &mapper->a, &mapper->b);
    }

    reaC_write(mapper->callback, end, mapper->a, mapper->b);
}
static void map_read(ReaC_Reader *context, reaC_err end, ReaC_Writer *callback, uintptr_t control) {
    (void)(control);
    struct map_reader *mapper = (struct map_reader*) context;
    mapper->writer.callback = callback;
    reaC_read(mapper->source, end, (ReaC_Writer *) &mapper->writer, 0);
}
ReaC_Reader *reaC_op_map2(ReaC_Reader *source, void *context, reaC_op_map_func *transform)
{
    struct map_reader *mapper = calloc(1, sizeof(struct map_reader));

    mapper->reader.func = map_read;
    mapper->reader.flags |= REAc_AUTOFREE;

    mapper->source = source;

    mapper->writer.writer.func = map_write;
    mapper->writer.context = context;
    mapper->writer.transform = transform;

    return (ReaC_Reader *) mapper;
}

/* ON_END: runs a function when a termination code comes from upstream.
 * Possibly useful for error handling, but does not run on cancel.
 * The handler function may be given a context pointer; be advised
 * that this context pointer is not auto-freed.
 */
struct on_end_writer {
    ReaC_Writer writer;

    ReaC_Writer *callback;

    void *context;
    reaC_op_on_end_func *handler;
};
struct on_end_reader {
    ReaC_Reader reader;

    ReaC_Reader *source;
    struct on_end_writer writer;
};
static void on_end_write(ReaC_Writer *context, reaC_err end, uintptr_t a, uintptr_t b) {
    struct on_end_writer *state = (struct on_end_writer*) context;

    if(end != REAc_OK) {
        state->handler(state->context, end, a, b);
    }

    reaC_write(state->callback, end, a, b);
}
static void on_end_read(ReaC_Reader *context, reaC_err end, ReaC_Writer *callback, uintptr_t control) {
    (void)(control);
    struct on_end_reader *mapper = (struct on_end_reader*) context;
    mapper->writer.callback = callback;
    reaC_read(mapper->source, end, (ReaC_Writer *) &mapper->writer, 0);
}
ReaC_Reader *reaC_op_on_end2(ReaC_Reader *source, void *context, reaC_op_on_end_func *handler)
{
    struct on_end_reader *state = calloc(1, sizeof(struct on_end_reader));

    state->reader.func = on_end_read;
    state->reader.flags |= REAc_AUTOFREE;

    state->source = source;

    state->writer.writer.func = on_end_write;
    state->writer.context = context;
    state->writer.handler = handler;

    return (ReaC_Reader *) state;
}

/* CLEANUP: runs a function upon being canceled. The function
 * receives a context pointer, which is not auto-freed. However,
 * a cleanup function is a suitable place to free context pointers
 * used, for example, by map transform functions.
 */
struct cleanup_writer {
    ReaC_Writer writer;

    ReaC_Writer *callback;
};
struct cleanup_reader {
    ReaC_Reader reader;

    ReaC_Reader *source;
    struct cleanup_writer writer;

    void *context;
    reaC_op_cleanup_func *handler;
};
static void cleanup_write(ReaC_Writer *context, reaC_err end, uintptr_t a, uintptr_t b) {
    struct cleanup_writer *state = (struct cleanup_writer*) context;
    reaC_write(state->callback, end, a, b);
}
static void cleanup_read(ReaC_Reader *context, reaC_err end, ReaC_Writer *callback, uintptr_t control) {
    (void)(control);
    struct cleanup_reader *state = (struct cleanup_reader*) context;

    if(end == REAc_OK) {
        state->writer.callback = callback;
        reaC_read(state->source, end, (ReaC_Writer *) &state->writer, 0);
    } else {
        state->handler(state->context, end);
        reaC_read(state->source, end, (ReaC_Writer *) &state->writer, 0);
    }
}
ReaC_Reader *reaC_op_cleanup2(ReaC_Reader *source, void *context, reaC_op_cleanup_func *handler)
{
    struct cleanup_reader *state = calloc(1, sizeof(struct cleanup_reader));

    state->reader.func = cleanup_read;
    state->reader.flags |= REAc_AUTOFREE;

    state->source = source;
    state->context = context;
    state->handler = handler;

    state->writer.writer.func = cleanup_write;

    return (ReaC_Reader *) state;
}
