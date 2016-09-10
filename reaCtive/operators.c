
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

/* CONSTANT: generates an endless stream of a specific value */

struct constant_state {
    ReaC_Reader reader;
    uintptr_t a;
    uintptr_t b;
};
static void constant_read(ReaC_Reader *context, reaC_err end, ReaC_Writer *callback, uintptr_t control) {
    (void)(control);
    struct constant_state *state = (struct constant_state*) context;
    if(end) {
        return;
    }
    reaC_write(callback, 0, state->a, state->b);
}
ReaC_Reader *reaC_constant_source(uintptr_t a, uintptr_t b)
{
    struct constant_state *state = calloc(1, sizeof(struct constant_state));

    state->reader.func = constant_read;
    state->reader.flags |= REAc_AUTOFREE;

    state->a = a;
    state->b = b;

    return (ReaC_Reader *) state;
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

struct map_filter {
    ReaC_Filter filter;

    void *context;
    reaC_op_map_func *transform;

    uintptr_t a;
    uintptr_t b;
};
static void map_write(ReaC_Writer *context, reaC_err end, uintptr_t a, uintptr_t b) {
    struct map_filter *mapper = (struct map_filter*) context;

    if(end == REAc_OK) {
        /* Important: GCC won't tail call if any pointers to the stack
         * or arguments get passed to functions; "launder" a & b through
         * the context so the transform function can mutate them. */
        mapper->a = a;
        mapper->b = b;
        mapper->transform(mapper->context, &mapper->a, &mapper->b);
    }

    reaC_write(mapper->filter.output, end, mapper->a, mapper->b);
}
ReaC_Reader *reaC_op_map2(ReaC_Reader *source, void *context, reaC_op_map_func *transform)
{
    ReaC_Filter_Reader *reader = reaC_new_filter(
        source, map_write, sizeof(struct map_filter), NULL);

    struct map_filter *mapper = (struct map_filter *) reader->filter;
    mapper->context = context;
    mapper->transform = transform;

    return (ReaC_Reader *) reader;
}

/* ON_END: runs a function when a termination code comes from upstream.
 * Possibly useful for error handling, but does not run on cancel.
 * The handler function may be given a context pointer; be advised
 * that this context pointer is not auto-freed.
 */
struct on_end_filter {
    ReaC_Filter filter;

    void *context;
    reaC_op_on_end_func *handler;
};
static void on_end_write(ReaC_Writer *context, reaC_err end, uintptr_t a, uintptr_t b) {
    struct on_end_filter *state = (struct on_end_filter*) context;

    if(end != REAc_OK) {
        state->handler(state->context, end, a, b);
    }

    reaC_write(state->filter.output, end, a, b);
}
ReaC_Reader *reaC_op_on_end2(ReaC_Reader *source, void *context, reaC_op_on_end_func *handler)
{
    ReaC_Filter_Reader *reader = reaC_new_filter(
        source, on_end_write, sizeof(struct on_end_filter), NULL);

    struct on_end_filter *mapper = (struct on_end_filter *) reader->filter;
    mapper->context = context;
    mapper->handler = handler;

    return (ReaC_Reader *) reader;
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
        reaC_read(state->source, end, (ReaC_Writer *) &state->writer, 0);
        state->handler(state->context, end);
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
