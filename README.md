_This project has been created as part of the 42 curriculum by srosu._

# Codexion

## Description

Codexion is a concurrent system simulation written in C where multiple threads represent coders competing for limited shared resources called dongles. Each coder runs independently in parallel, and all coders must coordinate access to these resources without direct communication.. At the system level, this project models how an operating system manages multiple processes competing for shared hardware. Each coder is implemented as a thread (pthread), and each dongle is protected using synchronisation primitives such as mutexes and condition variables to ensure safe access. The program must prevent race conditions, deadlocks, starvation, and inconsistent shared state.

Each coder repeatedly performs a cycle of actions: acquiring two dongles, compiling code, releasing resources, debugging, and refactoring. However, each coder has a strict time constraint: if they fail to compile within a given deadline, they are considered to have burned out, and the simulation stops. A separate monitor thread continuously checks the state of all coders and enforces this rule with high precision timing.

The system also introduces a scheduling mechanism that controls how dongles are assigned when multiple coders request them at the same time. Depending on the selected policy (FIFO or EDF), the order of access changes dynamically based on request time or urgency. Additionally, a cooldown mechanism delays reuse of dongles after they are released, adding another layer of resource contention.

Overall, Codexion is a low-level concurrency problem that requires careful coordination between threads, precise timing control, and efficient synchronization to ensure fairness, liveness, and correctness under strict constraints.

### Core concepts

1. **Threads** (POSIX Threads)
   : a thread is a single flow of execution inside a process, in this project each coder is represented by a thread created using POSIX threads. Inside the operating system, the CPU does not run all threads at the same time. Instead it switches very fast between them using context switching. One thread runs for a short time, then it's paused, and another thread continues.

   All Threads share the same memory space. This means they can access the same data, such as shared counters and shared ressources (dongles). Because of this shared memory space, synchronisation is required to avoid conflicts such as a TOCTOU[^TOCTOU].

2. **Shared ressources** (Dongles)
   : Dongles are shared ressources used by coders to compile. Each coder needs two dongles at the same time on order to perform the compiling step.

   Since all coders share a limited number of dongles, multiple threads may try to access the same ressource simultaneously. Without control , this would lead to conflicts where two coders use the same dongle at the same time, which is not allowed.

3. **Mutex** (Mutual Exclusion)
   : A mutex is a locking mechanism used to protect shared ressources. Only one thread can lock a mutex at a time.

   When a coder locks a dongle, other coders trying to access it will be blocked by the operating system and place in a waiting state.

   The OS manages this waiting internally using kernel queues. The blocked threads does not consume cPU time until it is allowed to continue.

   Mutexes ensure that shared ressources are used safely without data corruption or simultaneous access.

## Technical choices

One of the core mechanisms of this project is scheduling.

When multiple coders try to acquire the same dongle at the same time, a scheduling
policy decides who gets it first. This project supports two scheduling policies,
FIFO[^FIFO] and EDF[^EDF], selected at launch through the `scheduler` argument.

The subject requires a heap-based priority queue for scheduling. We implemented
this requirement, but not for the reason one might expect, and want to explain why.

For a given dongle, there are structurally never more than two coders competing
for it at any given time (its two neighboring coders in the circular hub). This
holds true regardless of the scheduler in use. Given this fixed upper bound of
two candidates, arbitrating a single dongle never actually needs a heap: comparing
two values directly (arrival timestamp for FIFO, burnout deadline for EDF) is
always enough to decide who goes first, whether the metric is FIFO or EDF.

The real need for a heap comes from an entirely different part of the program:
the monitor thread. Unlike dongle arbitration, the monitor must track the closest
burnout deadline across _all_ coders at once, regardless of what each one is
currently doing (compiling, debugging, refactoring, or waiting on a dongle). That
is a genuine N-element problem, not a 2-element one, and that is where a min-heap
indexed by deadline earns its place.

EDF scheduling does not require its own heap; it simply reads from this same
structure, since its priority metric (`last_compile_start + time_to_burnout`) is
identical to the deadline the monitor already tracks. Rather than maintaining a
second, redundant structure purely to satisfy the letter of the requirement, EDF
arbitration reuses the heap that exists for the monitor's sake. The heap's role
stays consistent throughout the program: it always answers "who is closest to
burning out," whether that question is asked by the monitor or by a dongle
running EDF arbitration.

## Blocking cases handled

### Deadlock prevention (Coffman's conditions)

A deadlock requires all four of Coffman's conditions to hold simultaneously:
mutual exclusion, hold-and-wait, no preemption, and circular wait. The first
three are inherent to the problem itself and cannot be removed without
changing what the subject actually asks for — a dongle must be exclusively
owned while in use, a coder legitimately holds one dongle while waiting for
the other, and no thread can forcibly strip a dongle from another. Codexion
instead breaks the fourth condition, circular wait, which is enough on its
own to make deadlock structurally impossible.

The break comes from `attribute_dongle`: coders alternate the order in which
they attempt to acquire their two dongles based on their position parity —
odd-positioned coders reach for their left dongle first, even-positioned
coders reach for their right dongle first. This prevents the classic
dining-philosophers cycle where every coder holds one dongle and waits
forever for the one held by its neighbor: with the acquisition order
alternated, no such closed cycle of mutual waiting can ever form, regardless
of coder count or timing.

### Starvation prevention

Both scheduling policies guarantee that a coder waiting on a dongle is
eventually served. Under FIFO, priority is given to whichever of the two
competing coders issued its request first, so a coder can never be
indefinitely passed over by a neighbor that keeps re-requesting. Under EDF,
priority goes to whichever coder is closer to burning out, which means the
coder under the most time pressure is always favored — a coder that keeps
losing arbitration necessarily has its deadline approaching, which in turn
increases its own priority on the next round. In both cases, priority is only
evaluated against a neighbor that is _actually_ currently waiting for that
same dongle (tracked via `waiting_for`), so an idle neighbor still in its
debug or refactor phase never blocks a genuinely waiting coder.

### Cooldown handling

After a dongle is released, it cannot be re-acquired until
`dongle_cooldown` milliseconds have passed. This is enforced inside
`dongle_acquire` via `dongle_cooldown()`, which computes the exact remaining
time from `last_release` and sleeps on the dongle's own condition variable
until that absolute deadline — rather than a fixed sleep — so the wait is
always accurate regardless of when the check happens to run.

### Precise burnout detection

A coder's burnout window runs from the start of its last compile until the
start of its next one, covering debug, refactor, and any dongle-acquisition
wait combined. The monitor thread tracks this via a single min-heap ordered
by deadline, always sleeping on `pthread_cond_timedwait` until exactly the
closest known deadline rather than polling. This keeps burnout detection
accurate to the timeout itself, satisfying the subject's 10ms precision
requirement, while avoiding any busy-loop over all coders.

### Log serialization

All state-change logs go through a single `output` function guarded by
`stdout_mutex`, so two coders' log lines can never interleave into a
corrupted line, regardless of how many threads are logging at once.

## Instructions

### Compilation

```bash
make            # builds the codexion binary
make clean      # removes object files
make fclean     # removes object files and the binary
make re         # fclean + all
```

### Usage

```bash
./codexion <number_of_coders> <time_to_burnout> <time_to_compile> \
    <time_to_debug> <time_to_refactor> <number_of_compiles_required> \
    <dongle_cooldown> <scheduler>
```

`scheduler` must be exactly `fifo` or `edf`. All arguments are mandatory
and must be positive integers.

### Usage examples

**1. A comfortable, guaranteed-success run**

```bash
./codexion 4 800 200 200 200 5 10 fifo
```

With `time_to_debug + time_to_refactor + time_to_compile` (600ms) well under
`time_to_burnout` (800ms), and a low cooldown (10ms), every coder has ample
slack even if it briefly waits for a dongle. No burnout should occur; the
simulation stops once every coder has compiled 5 times.

**2. The same run under EDF**

```bash
./codexion 4 800 200 200 200 5 10 edf
```

Same timing, different arbitration policy. Both schedulers should succeed
here — this pair of examples is a good way to compare their log ordering
side by side without one masking the other's behavior.

**3. Deadlock-proof, even at scale**

```bash
./codexion 100 10000 66 24 87 10 10 fifo
```

A large coder count with tight per-action timings. Because dongle
acquisition order alternates by coder parity, no circular wait can form —
this case demonstrates that deadlock avoidance holds regardless of how many
coders are contending simultaneously, not just in small examples.

**4. Same parameters, burnout anyway — timing, not deadlock**

```bash
./codexion 3 1000 600 10 10 5 100 fifo
```

At first glance the numbers look sufficient: `time_to_burnout` (1000ms) is
larger than any single phase. But `time_to_compile` alone (600ms) already
consumes most of the burnout window, leaving little room for debug, refactor,
_and_ any dongle-acquisition wait caused by contention with a neighbor. A
coder unlucky enough to lose arbitration once can burn out — not because of a
design flaw, but because the parameters themselves are close to infeasible.
This is the "provided the parameters are feasible" caveat the subject warns
about: no scheduler can save a coder if the fixed-duration phases alone
already eat the entire burnout budget.

**5. Cooldown pushed to the point of failure**

```bash
./codexion 2 1000 100 100 100 5 2000 fifo
```

Only two coders, sharing two dongles, but `dongle_cooldown` (2000ms) is
double the entire burnout window (1000ms). Once a dongle is released, it
stays unavailable far longer than either coder can afford to wait — burnout
becomes essentially guaranteed here. Useful for demonstrating that cooldown
handling is real and enforced, not just present in name.

## Thread synchronization mechanisms

Codexion relies exclusively on `pthread_mutex_t` and `pthread_cond_t` — no
lock-free tricks, no busy-waiting. Every piece of shared state has exactly one
mutex responsible for protecting it, and every long wait uses a condition
variable rather than polling, so idle threads consume no CPU while waiting.

### Dongle synchronization (`t_dongle`)

Each dongle owns its own `mutex` and `cond`. `dongle_acquire` locks the
dongle's mutex, then loops on `pthread_cond_wait` while any of three
conditions hold: the dongle is currently `taken`, its cooldown has not
elapsed, or the calling coder does not currently have priority (FIFO arrival
order or EDF deadline, depending on the scheduler). Each condition is
re-checked on every wake-up rather than assumed after a single wait, which
protects against spurious wake-ups and against a wake-up that turns out to be
irrelevant to the calling coder. `dongle_release` reacquires the same mutex,
resets `taken` and `last_release`, and calls `pthread_cond_broadcast` so every
coder currently waiting on that specific dongle re-evaluates its condition.

Because `pthread_cond_wait` atomically unlocks the mutex for the duration of
the wait and relocks it on wake-up, a coder blocked waiting for a dongle never
holds its mutex idle — `dongle_release` and other coders' arbitration checks
can proceed freely in the meantime. This is what keeps waiting non-blocking
for everyone else, rather than causing contention on the mutex itself.

### Per-coder state (`t_coder`)

Each coder has its own `mutex` protecting fields read across threads:
`arrival_time`, `waiting_for`, `status`, and `finished`. Every dongle
arbitration decision reads a competing coder's `waiting_for` and
`arrival_time`/deadline through this mutex rather than reading the raw field
directly — an early version of this project read `waiting_for` in the caller
before the callee had a chance to lock the coder's mutex, which Helgrind
flagged as a genuine data race. The fix was to pass the `t_coder *` itself
into the checking function and let it perform the lock, read, and compare
entirely inside its own critical section, rather than trusting a value read
by the caller beforehand.

### Shared heap (`t_heap`)

The heap (deadline-ordered, shared by the monitor and by EDF arbitration) has
a single `mutex` covering both reads and writes — using two separate mutexes
for "read" and "write" access would not actually protect one against the
other, since a mutex only excludes threads contending for that same lock.
Every heap mutation (`update_deadline`, `remove_node`) captures the deadline
at the root before and after modifying the structure; if the root deadline
changed, it signals `monitor_cond` so the monitor recomputes its wait target
immediately rather than sleeping on a now-stale deadline.

### Monitor synchronization

The monitor sleeps on `pthread_cond_timedwait(&monitor_cond, &heap->mutex,
&ts)`, where `ts` is the absolute deadline of the coder currently at the root
of the heap. It has its own dedicated condition variable, separate from every
dongle's `cond` — sharing one would cause the monitor and coders to wake each
other on irrelevant signals. Two outcomes are possible on wake-up: `ETIMEDOUT`
without any prior deadline change means the coder at the root genuinely missed
its deadline, and the monitor logs the burnout and ends the simulation; a
wake-up before the timeout means some coder updated its deadline in a way
that changed the root, and the monitor loops back to recompute against the
new closest deadline.

### End-of-simulation propagation

`end_of_simulation` is protected by its own `end_mutex`, checked by every
coder inside a custom sleep function (`ft_usleep`) built on
`pthread_cond_timedwait` rather than the standard `usleep`, precisely because
`usleep` cannot be interrupted once called. `ft_usleep` sleeps until either
its requested duration elapses or `end_of_simulation` becomes true, whichever
comes first, so no coder is stuck finishing out a long compile/debug/refactor
phase after the simulation has already ended elsewhere.

### Logging

All output goes through a single `output` function guarded by `stdout_mutex`,
so two coders' log lines can never interleave into one corrupted line — the
timestamp is captured before acquiring the mutex so it reflects the true
moment of the event rather than however long the thread waited for the lock.


## Ressources

- ["The Dining Philosophers in C: Threads, Race Conditions and Deadlocks"](https://youtu.be/zOpzGHwJ3MU?si=puNrMJNSsaLrQPNF) — Oceano (YouTube)
- ["Codexion"](https://dev.to/yel-bakk/codexion-4fk8) — Yassir El bakkari (dev.to)
- ["Thread Synchronization"](https://academy.nordicsemi.com/courses/nrf-connect-sdk-fundamentals/lessons/lesson-8-thread-synchronization/topic/the-need-for-thread-synchronization/) — Nordic Developer Academy
- ["Multithreading in OS - Different Models"](https://www.geeksforgeeks.org/operating-systems/multithreading-in-operating-system/) — GeeksforGeeks
- ["Understanding the Fundamentals of Multi-Threading: A Beginner's Guide"](https://medium.com/@anton.baksheiev/understanding-the-fundamentals-of-multi-threading-a-beginners-guide-b6585844a538) — Anton Baksheiev (Medium)
- ["General Concepts"](https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap04.html) — The Open Group Base Specifications, Issue 7 (POSIX.1-2017)
- ["POSIX Threads (pthreads) — The Simplest Way to Understand Real Multithreading in C"](https://medium.com/@techdhaba.training/posix-threads-pthreads-the-simplest-way-to-understand-real-multithreading-in-c-c2f591ab7a03) — tech dhaba (Medium)
- ["Chapter 4: Threads & Concurrency"](https://www.andrew.cmu.edu/course/14-712-s20/applications/ln/14712-l5.pdf) — Carnegie Mellon University, 14-712 course notes
### AI usage

Claude was used strictly as a conceptual sounding board,
through a question-driven back-and-forth, never to write code.

It helped clarify threading primitives and heap mechanics, and served as a discussion partner for architecture decisions (heap scope, deadlock prevention, monitor wake-up strategy) and optimization trade-offs. It also assisted in drafting parts of this README, though most of it was written independently.

Every line of code was designed and written by hand, and all debugging was done independently aswell.

[^TOCTOU]:
    Time-Of-Check to Time-Of-Use — a race condition where a value is
    checked under one operation and used under another, allowing another thread
    to change it in between; avoided here by reading and comparing shared state
    within a single locked critical section.

[^FIFO]:
    First In, First Out — the coder whose dongle request arrived first
    is granted access first.

[^EDF]:
    Earliest Deadline First — the coder closest to its burnout deadline
    (`last_compile_start + time_to_burnout`) is granted access first.

