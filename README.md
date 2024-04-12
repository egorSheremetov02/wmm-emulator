# Emulator of weak memory models' behaviour

For now there is only support for sequentially consistent semantics, TSO-semantics (x86 architecture uses it) and PSO-semantics (partial store order).

In the future there is a plan to also add support for Release-Acquire semantics.

There are several modes you can run your emulations in: random walk mode, interactive mode and model checking mode.

### Random walk mode

Chooses next possible transition randomly. With uniform distribution.

### Interactive mode

Prints next available operations that can be potentially performed. Awaits for the user input do decide which one to choose.

### Model checking mode

Runs operations in all possible orders to discover all possible states of the main memory. Print number of discovered memory states.
