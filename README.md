# LSO2 VM Performance

## Summary

Repo to figure out why the LSO2 LSL VM is so slow, and maybe make it a bit faster.

Includes a `lscript_harness` binary that runs the `state_entry()` of an LSL script under the LSO2 VM
to allow for benchmarking performance improvements.

## Why?

Even though Mono has existed for over a decade, people continue to write scripts targeting
LSO2. So long as those scripts continue to be supported, sims will have to spend some of their
execution budget on evaluating them. Improving script engine performance isn't just important for scripts,
improving script engine performance also improves _sim_ performance.

## Benchmarks

Benchmark runtime comparison (in seconds):

| Benchmark  | Unmodified VM | Modified VM |
|------------|---------------|-------------|
| Mandelbrot | 0.31          | 0.08        |
| NSieve     | 0.17          | 0.11        |
| Recursion  | 0.008935      | 0.001847    |

## Obvious improvement possibilities

Focusing on improvements that don't involve bytecode or state serialization changes:

* ~~Stop leaving the interpreter loop to check for conditions that can't happen given the current instruction~~ (done)
* * ~~State changes, whatever else.~~ (done)
* ~~Only perform yield checks on backwards jumps, function calls, and returns, like Mono scripts.~~ (done)
* Move opcode handlers from functions to computed goto / switch statement.
* ~~Make VM registers native-endian, swapping to big-endian when (de)serializing VM state~~ (done)
* * Ideally all stack and heap data should be made native-endian, that's much more involved if we want to maintain compatibility with
    existing VM state serialization. Would need ability to map current IP to the type of the stack contents, walking
    up the call stack. Maybe not possible.
* Disable safety checks that only matter if untrusted bytecode is evaluated
* * Naturally, this is trickier since SL accepted arbitrary bytecode for a long time. Could only flag on those
    optimizations for scripts that proved `stored_bytecode == compile(stored_source)`

# License
This patchset is applied on top of Firestorm for demo purposes, Firestorm is licensed under the LGPL v2.1.

The diffs of any commits written by me in https://github.com/SaladDais/LSO2-VM-Performance/compare/ee368a40fc777d8ea7ea7ac66fa5bafd89579996~1...lscript_only
(excluding the LSL benchmarks / conformance tests) may be used under the terms of public domain, ISC, LGPL v2.1 or Apache v2 licenses.
Whatever your lawyers prefer.

Note that the code will probably become LGPL v2.1 when applied unless you're the original copyright holder for the SL viewer.

All documents and designs in these diffs authored by me have the same licensing terms as above and may be used without attribution.
