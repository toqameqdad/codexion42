*This activity has been created as part of the 42 curriculum by tmeqdad.*

# Codexion

## Description

Codexion is a concurrency and resource-scheduling activity written in C using POSIX threads.

The simulation represents a group of coders working around a shared Quantum Compiler. Each coder is represented by a thread and alternates between three activities:

* **Compiling** — requires two USB dongles simultaneously.
* **Debugging**
* **Refactoring**

There is one dongle between each pair of coders. A coder must acquire both of their adjacent dongles before they can compile.

The main goal of the activity is to coordinate multiple threads competing for limited shared resources while preventing race conditions, deadlocks, starvation, and burnout.

The program supports two dongle scheduling policies:

* **FIFO (First In, First Out):** requests are served according to their arrival order.
* **EDF (Earliest Deadline First):** requests are prioritized according to the coder's burnout deadline.

The simulation ends when either:

1. A coder burns out because they did not start compiling before their deadline.
2. Every coder has completed the required number of compilations.

The project focuses on thread synchronization, resource management, scheduling algorithms, condition variables, timing, and concurrency in C.

## Instructions

### Compilation

The project requires a POSIX-compatible environment and `pthread`.

Compile the project using:

```bash
make
```

The Makefile uses:

```text
-Wall -Wextra -Werror -pthread
```

To remove object files:

```bash
make clean
```

To remove all generated files:

```bash
make fclean
```

To rebuild the project:

```bash
make re
```

### Execution

The program requires exactly eight arguments:

```text
./codexion number_of_coders time_to_burnout time_to_compile \
time_to_debug time_to_refactor number_of_compiles_required \
dongle_cooldown scheduler
```

Arguments:

| Argument                      | Description                                                                            |
| ----------------------------- | -------------------------------------------------------------------------------------- |
| `number_of_coders`            | Number of coder threads and dongles                                                    |
| `time_to_burnout`             | Maximum time, in milliseconds, before a coder burns out if they do not start compiling |
| `time_to_compile`             | Time spent compiling while holding two dongles                                         |
| `time_to_debug`               | Time spent debugging                                                                   |
| `time_to_refactor`            | Time spent refactoring                                                                 |
| `number_of_compiles_required` | Number of compilations required from every coder                                       |
| `dongle_cooldown`             | Time a released dongle remains unavailable                                             |
| `scheduler`                   | Scheduling policy: `fifo` or `edf`                                                     |

Example:

```bash
./codexion 7 900 200 200 200 3 100 fifo
```

Another example using EDF scheduling:

```bash
./codexion 7 900 200 200 200 3 100 edf
```

### Output

Each state change is printed with a timestamp and the coder number:

```text
0 1 has taken a dongle
1 1 has taken a dongle
1 1 is compiling
201 1 is debugging
401 1 is refactoring
```

A burnout is reported as:

```text
1204 3 burned out
```

Log messages are serialized so that messages from different threads cannot overlap or become corrupted.

## Scheduling

### FIFO

FIFO stands for **First In, First Out**.

When multiple coders are waiting for the same dongle, the request that entered the waiting queue first is served first.

This provides deterministic request ordering and prevents a continuously arriving request from simply jumping ahead of older requests.

### EDF

EDF stands for **Earliest Deadline First**.

Each coder has a deadline calculated from their last compile start:

```text
deadline = last_compile_start + time_to_burnout
```

When several coders are waiting, the coder with the earliest deadline receives priority.

The implementation uses a custom priority queue based on a heap rather than a standard-library priority queue.

## Blocking cases handled

### Deadlock prevention

A deadlock can occur when several coders hold one dongle while waiting indefinitely for another dongle.

The implementation avoids this situation by letting the scheduler reserve both adjacent dongles atomically. Dongle mutexes are always locked in ascending ID order, so a circular lock dependency cannot be formed.

The four Coffman conditions are:

1. Mutual exclusion
2. Hold and wait
3. No preemption
4. Circular wait

The fixed lock order breaks the circular-wait condition, while atomic pair reservation prevents a coder from keeping only one dongle while waiting for the other.

A coder only enters the compiling state after successfully obtaining both required dongles.

### Starvation prevention

Starvation happens when a coder continuously waits while other coders repeatedly receive the resources first.

The scheduler explicitly orders waiting requests:

* FIFO preserves request arrival order.
* EDF prioritizes the closest burnout deadline.

This prevents arbitrary resource acquisition and provides fair access to the dongles according to the selected scheduling policy.

Under EDF, the implementation is designed to maintain liveness when the provided parameters are feasible.

### Dongle cooldown

A dongle cannot immediately become available after being released.

After release, its cooldown period must expire before another coder can acquire it.

The cooldown state is protected by synchronization so that two threads cannot incorrectly treat the same dongle as available at the same time.

### Precise burnout detection

A separate monitor thread continuously checks coder deadlines.

A coder burns out when they fail to start a new compilation within:

```text
time_to_burnout
```

milliseconds from the beginning of their last compilation, or from the beginning of the simulation for their first compilation.

The monitor is synchronized with coder state and stops the simulation when burnout occurs.

The burnout message must be displayed within 10 ms of the actual burnout time, subject to normal operating-system scheduling limitations.

### Log serialization

Multiple coder threads can attempt to print at the same time.

Without synchronization, their output could interleave, for example:

```text
10 1 is comp20 2 is compiling
iling
```

A dedicated mutex protects logging so that one complete message is written before another thread can write its message.

This guarantees that individual state-change messages remain intact.

### Simulation termination

The simulation has two possible termination conditions:

* A coder burns out.
* Every coder reaches `number_of_compiles_required`.

Once termination is detected, shared simulation state is updated safely and the remaining threads are allowed to finish and join cleanly.

## Thread synchronization mechanisms

The implementation uses POSIX threading primitives together with a custom event mechanism to coordinate the coder threads, monitor, dongles, and scheduler.

### `pthread_mutex_t`

Mutexes protect shared data from concurrent access.

They are used for resources and states such as:

* Dongle ownership and availability
* Scheduler/queue state
* Simulation termination state
* Monitor-related state
* Logging/output

For example, when a coder wants to acquire a dongle, access to that dongle's state is synchronized. This prevents two coder threads from simultaneously believing that they own the same dongle.

The same principle is applied to shared simulation state. A thread reads or modifies the protected state while holding the appropriate mutex and releases the mutex afterwards.

### `pthread_cond_t`

Condition variables allow threads to wait efficiently until a shared condition changes.

Instead of continuously polling for a dongle, a waiting coder can sleep until the resource-management system signals that the state may have changed.

A typical synchronization sequence is:

```text
lock mutex
    check whether the requested resource can be granted
    if not:
        wait on the condition variable
    otherwise:
        reserve the resource
unlock mutex
```

When a dongle becomes available, the corresponding condition is signaled so waiting threads can re-check their eligibility.

The condition is always checked while holding the associated mutex. This prevents a race where a thread assumes that a resource is available based on stale information.

### Custom event implementation

The queue condition variable acts as the scheduler notification mechanism. A release broadcasts a queue-state change, and timed waits wake the scheduler when a dongle cooldown expires even if no other thread releases a resource at that moment.

### Coder and monitor communication

Coders update their timing/state information while holding the required synchronization lock.

The monitor reads this shared state under the same synchronization rules.

This prevents situations where:

1. A coder updates its last compilation timestamp.
2. The monitor reads the old timestamp at the same time.
3. The monitor incorrectly decides that the coder has burned out.

By synchronizing both sides, the monitor works with a consistent view of the coder's state.

### Example: preventing a dongle race condition

Without synchronization:

```text
Coder 1 checks dongle -> available
Coder 2 checks dongle -> available
Coder 1 takes dongle
Coder 2 takes same dongle
```

With the mutex:

```text
Coder 1 locks dongle state
Coder 1 checks and reserves dongle
Coder 1 unlocks dongle state

Coder 2 locks dongle state
Coder 2 sees that dongle is unavailable
Coder 2 waits
```

Only one thread can modify the dongle state at a time.

## Data structures

### Custom heap

A custom heap is used to implement the priority queue required by the project.

The heap supports the scheduling policies used by the dongle manager:

* FIFO ordering based on request arrival.
* EDF ordering based on burnout deadlines.

The project does not rely on a standard-library priority queue.

The heap allows the scheduler to efficiently select the next coder whose request should be served.

## Technical choices

* **Language:** C
* **Concurrency:** POSIX threads (`pthread`)
* **Synchronization:** `pthread_mutex_t`, `pthread_cond_t`
* **Scheduling:** FIFO and EDF
* **Priority queue:** Custom heap implementation
* **Timing:** `gettimeofday()` / time-based calculations
* **Memory management:** Explicit allocation and cleanup
* **Build system:** Makefile
* **Compiler flags:** `-Wall -Wextra -Werror -pthread`

No global variables are used.

## Resources

### POSIX Threads

* `pthread_create` — creates coder and monitor threads.
* `pthread_join` — waits for threads to finish.
* `pthread_mutex_*` — protects shared state.
* `pthread_cond_*` — coordinates threads waiting for shared conditions.

The official POSIX/Linux documentation is useful for understanding thread creation, mutexes, and condition variables.

### `gettimeofday`

Used for measuring elapsed time and generating timestamps for the simulation.

### Concurrency concepts

The project relies on classic concurrency concepts including:

* Race conditions
* Mutual exclusion
* Deadlocks
* Coffman's conditions
* Starvation
* Condition variables
* Resource scheduling
* Producer/consumer-style waiting
* Thread-safe communication

### Scheduling algorithms

The main scheduling concepts studied for this project were:

* First In, First Out (FIFO)
* Earliest Deadline First (EDF)
* Priority queues
* Binary heaps

### AI usage

AI was used as a learning and debugging assistant during the development of this activity.

It was used for:

* Explaining concurrency concepts such as mutexes, condition variables, deadlocks, race conditions, starvation, and scheduling.
* Helping reason about the interaction between coder threads and the monitor thread.
* Reviewing and debugging parts of the C implementation.
* Understanding compiler/linker errors and synchronization-related problems.
* Helping identify and fix code-structure and Norm-related issues.
* Reviewing test outputs and helping analyze unexpected execution behavior.

AI was **not treated as a replacement for understanding the implementation**. Generated explanations and suggestions were reviewed, tested, and adapted to the project's requirements. The final implementation was checked through compilation, testing, and peer evaluation.

## Project Requirements

This project follows the Codexion activity requirements, including:

* One thread per coder.
* Mutex-protected dongles.
* Dongle cooldown.
* FIFO and EDF scheduling.
* Custom heap-based priority queue.
* Separate burnout-monitoring thread.
* Serialized logging.
* Safe termination.
* Proper memory management.
* Compilation with `-Wall -Wextra -Werror -pthread`.

## License

This project was created as part of the 42 curriculum and is intended for educational purposes.
