# reaCtive - a reactive programming library for C

(work in progress, getting thoughts down)

## Observable lifecycle

Each Observable in the pipeline should behave as a state machine;
it receives input and updates its internal state appropriately.

The Observable structure contains a `void *userdata`, which is
entirely untouched by the library and may be pointed at a suitable
structure for internal state by the Observable's creator.

Any of the lifecycle functions may be `NULL` if not applicable.

#### Initialization

`void init(Observable *context)`

Called for each Observable in the chain, from the end to the
beginning, once the pipeline is completed.

Should allocate any resources this Observable needs; after it runs,
`next()`, `error()`, and `finish()` must be safe to call.

As the consumer is initialized before the producer, `init()`
is free to emit data items or even complete the sequence
as part of its operation.

#### Operation

`void next(Observable *context, uintptr_t a, uintptr_t b)`

Called for each data item in the subscribed sequence;
the actual data types of a & b are defined by the producer.

Two `uintptr_t`s are used instead of a single `void *`, because
many interesting data items can be directly expressed that way
instead of having to create a custom structure. For example, a
buffer address and count, or an fd and readiness flags.

`next()` may emit zero or more data items during its execution,
of a type defined by this Observable. An error or completion signal
may also be emitted to terminate the sequence; their types should
also be defined, but do not have to be the same type as the data
items.

If, upon emitting an item, the emission fails with a 

`void error(Observable *context, uintptr_t a, uintptr_t b)`

`void finish(Observable *context, uintptr_t a, uintptr_t b)`

The same interface as `next()`, except called upon termination of
the subscribed sequence with an error or completion signal,
respectively.

As such, they must emit an error or completion signal to properly
complete their sequence. 

`error()` and `finish()` may assume that `next()` will
not be called after they run.

#### Finalization

`void dispose(Observable *context)`

Called as the pipeline is torn down, after an error or completion
signal has been processed by all stages.

Should release any resources that were allocated by `init()`;
after `dispose()` runs, the Observable must be safe to `free()`
