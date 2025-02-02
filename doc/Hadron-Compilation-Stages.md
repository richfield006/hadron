# Hadron Compilation Stages

## Lexing produces a token stream

The source is lexed into tokens using a token analyzer state machine built using Ragel. Using `src/Lexer.rl` as input,
Ragel generates `src/Lexer.cpp` at build time, which contains the lexing state machine. We lex in a separate stage from
parsing to enable fast lexing of sclang input for situations like a syntax highlighter re-using just the lexer in
Hadron.

## Parsing produces a parse tree

The goal of parsing in Hadron is to produce a *parse tree* only, meaning a tree representation of the input tokens
identified by the lexer. This is different from LSC which lexes, parses, and does some optimizations to the parse tree
all in one pass. The parse tree is built by the Parser defined in `src/Parser.yy`, which Bison processes into
`src/Parser.cpp` at build time.

## SSABuilder lowers the parse tree to Hadron Intermediate Representation (HIR)

Hadron Intermediate Representation is defined in `src/include/hadron/HIR.hpp`. HIR is designed for conversion by the
`src/SSABuilder.cpp` object from the parse tree directly into [Static Single
Assignment](https://en.wikipedia.org/wiki/Static_single_assignment_form) (SSA) form. SSA form simplifies several of the
optimizations that Hadron can perform on input code, and also is very important to the type deduction system, because
SSA simplifies tracking modifications of variable values across branches in code, which may impact their type.

## Optimization steps using SSA form

Not yet implemented but dead code elimination, constant folding, and phi simplification/elimination can happen
iteratively multiple times, or not at all, depending on the compilation speed needs. Type deduction is also finalized here.

## Flatten Control Flow Graph to a single block and perform lifetime analysis

Hadron uses a Linear Scan algorithm on SSA form input. This algorithm requires the control flow graph to be in sorted
topological order (reverse postorder) with all loops in contiguous order. The other input to the Linear Scan algorithm
are lifetime intervals for each value, which are computed here.

## Register allocation using the Linear Scan algorithm

Processor register allocation is an NP-hard problem. Register allocation is also very important for compiled code speed,
as registers are by far the fastest form of storage on a computer. Some recent work in compiler research (see
Bibliography) explores performing register allocation from code in SSA form, instead of first translating code out of
SSA form and then performing the register allocation. This approach is also attractive to JIT compilers because it saves
a step in the compilation process, therefore saving on compilation time.

## Resolution from SSA form and machine code generation

To get out of SSA form once register allocation happens the compiler has to insert move instructions between blocks of
code where register allocations for values may differ. This happens during a final pass through the linear block of HIR,
while generating machine code at the same time.

To support both ARM and x86 processor architectures Hadron uses [Lightening](https://gitlab.com/wingo/lightening), a
fork of [GNU Lightning](https://www.gnu.org/software/lightning/). These libraries expose a rough "consensus" machine
code instruction set as a series of C functions that, when called, emit processor-specific machine bytecode to the JIT
buffer. This allows Hadron to write one machine code generation backend and cover both processor backends.
