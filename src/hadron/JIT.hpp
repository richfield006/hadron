#ifndef SRC_COMPILER_INCLUDE_HADRON_JIT_HPP_
#define SRC_COMPILER_INCLUDE_HADRON_JIT_HPP_

#include "hadron/Arch.hpp"

#include <cstddef>
#include <memory>

namespace hadron {

struct Function;

// TODO: refactor to represent a re-useable tool that produces some kind of build product, a callable block of code
// that is either "virtual" and so has no usable function pointers, or is rendered/realized and therefore has both a
// C entry pointer and a Hadron entry pointer. Then compiler context can produce build artifacts for consumption by
// the caller.

// Abstract base class for JIT compilation, allowing CodeGenerator to JIT to either virtual, testing or production
// backends.
// Steps to add a new instruction:
// 1) add a new abstract function here.
// 2) implement the new method in LightningJIT
// 3) implement the new method in VirtualJIT, including:
//    3a) add an enum value to Opcode
//    3b) implement the method in VirtualJIT
//    3c) add support for printing the opcode in VirtualJIT::toString()
// 4) add parsing for the opcode in main state machine in Assembler.rl, and a unit test in Assembler_unittests
// 5) add support for the opcode in MachineCodeRenderer::render()
class JIT {
public:
    JIT() = default;
    virtual ~JIT() = default;

    using Label = int32_t;
    using Reg = int32_t;
    using Address = int32_t;
 
    // We reserve GPR(0) and GPR(1) for the context and stack pointers, respectively.
    static constexpr JIT::Reg kContextPointerReg = -3;
    static constexpr JIT::Reg kFramePointerReg = -2;
    static constexpr JIT::Reg kStackPointerReg = -1;

    // ===== Serialization
    // Begin recording jit bytecode into the provided |buffer| with maximum |size|.
    virtual void begin(int8_t* buffer, size_t size) = 0;
    // Returns true if the bytecode didn't fit into the provided buffer size.
    virtual bool hasJITBufferOverflow() = 0;
    // Set jit pointer back to beginning of buffer.
    virtual void reset() = 0;
    // End recording jit bytecode into the buffer, writes the final size to sizeOut. Returns the jit address to begin().
    virtual Address end(size_t* sizeOut) = 0;

    // ===== Machine Properties
    virtual int getRegisterCount() const = 0;
    virtual int getFloatRegisterCount() const = 0;

    // ===== Instruction Set (directly modeled from GNU Lightning instruction set, added as needed)
    // suffixes _i means 32-bit integer, _l means 64-bit integer, _w will call one of _i or _l depending on
    // word size of host machine.

    // * arithmetic
    // %target = %a + %b
    virtual void addr(Reg target, Reg a, Reg b) = 0;
    // %target = %a + b
    virtual void addi(Reg target, Reg a, Word b) = 0;
    // %target = %a & b
    virtual void andi(Reg target, Reg a, UWord b) = 0;
    // %target = %a | b
    virtual void ori(Reg target, Reg a, UWord b) = 0;
    // %target = %a ^ %b
    virtual void xorr(Reg target, Reg a, Reg b) = 0;

    // * register setting
    // %target <- %value
    virtual void movr(Reg target, Reg value) = 0;
    // %target <- value
    virtual void movi(Reg target, Word value) = 0;
    // %target <- value (unsigned)
    virtual void movi_u(Reg target, UWord value) = 0;

    // * branches
    // if a >= b goto Label
    virtual Label bgei(Reg a, Word b) = 0;
    // if a == b goto Label
    virtual Label beqi(Reg a, Word b) = 0;
    // unconditionally jump to Label
    virtual Label jmp() = 0;
    // jump to register
    virtual void jmpr(Reg r) = 0;
    // jump to address
    virtual void jmpi(Address location) = 0;

    // * loads
    // %target = *address
    virtual void ldr_l(Reg target, Reg address) = 0;
    virtual void ldi_l(Reg target, void* address) = 0;
    // %target = *(%address + offset)
    virtual void ldxi_w(Reg target, Reg address, int offset) = 0;
    virtual void ldxi_i(Reg target, Reg address, int offset) = 0;
    virtual void ldxi_l(Reg target, Reg address, int offset) = 0;

    // * stores
    // *address = value
    virtual void str_i(Reg address, Reg value) = 0;
    virtual void str_l(Reg address, Reg value) = 0;
    // *(offset + address) = value  // note: immediate address with register offset not currently supported
    virtual void stxi_w(int offset, Reg address, Reg value) = 0;
    virtual void stxi_i(int offset, Reg address, Reg value) = 0;
    virtual void stxi_l(int offset, Reg address, Reg value) = 0;

    // * functions
    // return with no value
    virtual void ret() = 0;
    // retr %r  (return value of reg r)
    virtual void retr(Reg r) = 0;
    // reti value (return immediate value)
    virtual void reti(int value) = 0;

    // * labels - relocateable code addresses
    // Get the current address of the jitted code
    virtual Address address() = 0;
    // Makes |label| point to current position in JIT, for forward jumps.
    virtual void patchHere(Label label) = 0;
    // Makes |target| point to |location|, for backward jumps.
    virtual void patchThere(Label target, Address location) = 0;
};

} // namespace hadron

#endif // SRC_COMPILER_INCLUDE_HADRON_JIT_HPP_