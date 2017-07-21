/** @page safemmio Memory Mapped I/O safety

@section mmioguar Guarantees

The MMIO API calls provide access to read and write to C pointers referencing Memory Mapped I/O devices.
These calls are implemented to prevent unsafe optimizations for these loads and stores.
They make some assumptions about how @ref mmuconfig are configured by the Operating System.

@section whatissafe What is "safe" MMIO

Memory mapped I/O (MMIO) operations are set apart from normal memory access operations by the associated "side-effects".
The lack side-effects is used to great effect by modern processors and compilers to optimize memory access.
However, these side-effects being the goal and entire reason for MMIO operations.

So MMIO operations are said to be "safe" in this context if these optimizations are disabled or inhibited
so that the only side-effects of MMIO operations are those encoded the user program.

So what optimizations are possible?

- Dead code elimination
 - Omitting writes to "memory" which is never read
- Reordering of "unrelated" read/writes
- Splitting and/or combining
- Caching

Where can these optimizations be performed?

- Compiler optimizer.
- CPU MMU

@note There are other actors in modern systems between the CPU and individual MMIO devices which could effect MMIO operations.
      However, this author hasn't had cause to study them, so they will not be discussed.

@section compileropt Disabling Compiler Optimizations

The first step towards safe MMIO is to ensure that the MMIO operations in C code are reflected in the instructures
to be executed by the CPU.

@subsection compileropt_volatile Volatile Qualifier

The C language specification has (circa C89) no standard constructs specifically to inhibit optimiztion of MMIO operations.
The closest is the 'volatile' qualifier, originally intended for synchronization with UNIX signal handlers.

The specifics of exactly what 'volatile' does are hard to pin down without diving into the formal specification language of sequencing points.  Wherein it is said that volatile objects are "stable" at all sequence points.  The clearest deliniation of sequence points is a semi-colon ';'.

For example.  Given the two examples

@code
volatile unsigned *X = ...;
unsigned A, B, b1, b2;
A = *X + *X;
b1 = *X;
b2 = *X;
B = b1 + b2;
@endcode

A complient compiler could optimize "A = *X + *X" into "2*(*X) with a single read,
but would be prevented from doing so when evaluating "B = b1 + b2".

https://gcc.gnu.org/onlinedocs/gcc/Volatiles.html#Volatiles

https://docs.microsoft.com/en-us/cpp/cpp/volatile-cpp

The default MMIO implemention included in devlib2 (common/os/default/epicsMMIODef.h)
uses volatile qualified pointer de-reference.

@note "volatile" access does __not__ imply a memory barrier.  Non-volatile accesses may be optimized around a volatile access.

@subsection compileropt_asm Inline Assembler Instructions

When 'volatile' isn't sufficient, or for cautious programmers, it is necessary to fall back to archetacture and OS specific mechanisms
to inject assembler instructions.

The "__asm__" statement is a GCC specific extension.

https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html#Extended-Asm

The general form of a MMIO read will be:

@code
unsigned int ret;
char *addr = ...;
__asm__ __volatile__("..... : "=r" (ret) : "m" (*addr) :"memory");
@endcode

The general form of a MMIO read will be:

@code
char *addr = ...;
unsigned int val = ...;
__asm__ __volatile__("..... :: "r"(val), "m" (*addr) :"memory");
@endcode

- The addition of "memory" causes these statements to act as a full barrier reordering.
- For the asm statement, the volatile qualifier prevents statements with outputs from being optimized out when the output isn't used.

@section mmuconfig MMU and Caching

For modern archetactures, the program instruction stream may be dynamically optimized by the CPU on execution.
Control over whether this is done is governed by target specific mechianisms.

@subsection mmuconfig_x86 x86

See "Architecture Software Developer's Manual" from Intel.  Section 11.3.

https://software.intel.com/en-us/articles/intel-sdm

Memory regions for MMIO should be configured as UC "Strong Uncacheable" to ensure that:
"All reads and writes appear on the system bus and are executed in program order without reordering."

For UC regions no barrier instructions are needed.

@subsection mmuconfig_ppc PowerPC

See "PowerPC Operating Environment Architecture Book III" chapter 4 (Storage Control).

https://www.google.se/search?q=PowerPC+Operating+Environment+Architecture+Book+III&btnG=Google+Search&gbv=1

Memory regions for MMIO should be configured as Guarded and Cache Inhibited.
This disables caching and splitting/combining.  Re-ordering is possible unless the "eieio" (aka "msync") barrier instruction is used.
See section 3.3.1.6.1

@subsection mmuconfig_arm ARM

See the appropriate ARM "Archetacture Reference Manual".
The following is correct for v7-A.

Memory regions for MMIO should be configured as Device or Strongly Ordered.
Device region writes are posted to the system bus.

No sychronization instructions are needed for Device or Strongly Ordered regions.

*/
