# reaCtive - a reactive programming library for C

(work in progress, getting thoughts down;
check the proofofconcept.c file for an incompleted example.
Look into RxJS and friends for a decent explaination of reactive programming.)

# Observable Pipelines

The producer and consumer roles are combined into one `Observable`
type in order to keep the implementation simple. A pipeline is
linear, implemented as a simple double-linked list starting with
a producer, ending with a consumer, and having any number of
filters in between.

Each Observable in the pipeline should behave as a state machine;
it receives input in the form of `next()`, `error()`, and
`finish()` events, updates its internal state appropriately,
and in turn emits its own `next()`, `error()`, and `finish()`
events to the next entry in the chain.

The Observable structure contains a `void *userdata`, which is
entirely untouched by the library and may be pointed at a suitable
structure for internal state by the Observable's creator.

## Observable Lifecycle

You can create a valid Observable by `calloc`ing a space of suitable size, potentially with extra space for userdata (see below). It will be automatically `free`d by the library when the pipeline ends, unless you set the `REAc_PINNED` flag on the structure. It becomes
your job to free it after disposal if you set that flag.

An `Observable` defines `init()`, `next()`, `error()`, `finish()`,
and `dispose()` functions on its struct; these handle the
associated events.

Within these event handlers, you can call `reaC_emit_next()`,
`reaC_emit_error()`, and `reaC_emit_finish()` to pass items down
the chain. `reaC_cancel()` can also be called to shutdown the
pipeline without raising an error or finish signal. 

Any of the lifecycle functions may be `NULL` if not applicable.

### Initialization

```
void init(Observable *context)
```

Called for each Observable in the chain, from the end to the
beginning, once the pipeline is completed.

Should allocate any resources this Observable needs; after it runs,
`next()`, `error()`, and `finish()` must be safe to call.

As the consumer is initialized before the producer, `init()`
is free to emit data items or even complete the sequence
as part of its operation.

### Operation

```
void next(Observable *context, uintptr_t a, uintptr_t b)
```

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
`REAc_ECANCELLED` error, it's probably good to short-circuit any
processing, since nobody is listening anymore.

```
void error(Observable *context, uintptr_t a, uintptr_t b)
void finish(Observable *context, uintptr_t a, uintptr_t b)
```

The same interface as `next()`, except called upon termination of
the subscribed sequence with an error or completion signal,
respectively.

As such, they **must** emit an error or completion signal to properly
complete the sequence.

`error()` and `finish()` may assume that `next()` will
not be called after they run.

### Finalization

```
void dispose(Observable *context)
```

Called as the pipeline is torn down, after one of three events:
* an error signal
* a completion signal
* a cancellation of the pipeline

Should release any resources that were allocated by `init()`;
after `dispose()` runs, the Observable will be `free()`ed
automatically, unless its `REAc_PINNED` flag is set.

Pipeline stages are `dispose()`d in order from the consumer end to the producer end.

# Concurrency

Not thought out much yet, and definitely not implemented yet.

So short answer, not thread-safe yet.

Long answer:

Library itself has no global state, so independent pipelines
should be safe to access from multiple threads. Accessing one
pipeline from multiple threads probably will be bad.

Of course, having a scheduler to dispatch, say, IO events to
relevant pipelines, using multiple working threads, would be nice.
Which can be a problem if pipelines need to manipulate each other,
such as a merge or flatmap operator would need to do, since they may
end up scheduled to parallel threads.

What seems sensible would be to assemble Observable chains into
some sort of Pipeline object, which could be the subject of a
lock that all functions to externally manipulate a pipeline
would respect.

External-manipulation functions I can think of are the current
`cancel` function (which would take a Pipeline, not an Observable,
then, since Observables should finish or error, not self-cancel),
and future functions to inject next, error, & finish items into
the start of the pipeline.

Pipelines should probably be ref-counted then.

The producer's `init()` function should probably be passed the
nascent Pipeline object; since many interesting producers would
want to register the pipeline with an asynchronous scheduler,
they should have the ability to install `lock()` and `unlock()`
functions onto the Pipeline.

Problem: many interesting operators (like delay, and flatmap)
would want to schedule calls into the rest of the pipeline, not
just its head. Either give every `init()` method the ability to
split the pipeline into sub-pipelines, or have some way to inject
into pipeline stages with the Observable pointer, using the
Pipeline object as a guard/lock.
