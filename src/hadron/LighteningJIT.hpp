#ifndef SRC_COMPILER_INCLUDE_HADRON_LIGHTENING_JIT_HPP_
#define SRC_COMPILER_INCLUDE_HADRON_LIGHTENING_JIT_HPP_

#include "hadron/JIT.hpp"

#include <vector>

// Lightening external declarations.
extern "C" {
struct jit_gpr;
typedef struct jit_gpr jit_gpr_t;
struct jit_state;
typedef struct jit_state jit_state_t;
struct jit_reloc;
typedef struct jit_reloc jit_reloc_t;
typedef void* jit_pointer_t;
}

namespace hadron {

class LighteningJIT : public JIT {
public:
    LighteningJIT();
    virtual ~LighteningJIT();

    static bool markThreadForJITCompilation();
    static void markThreadForJITExecution();

    // Save current state from the calling C-style stack frame, including all callee-save registers, and update the
    // C stack pointer (modulo alignment) to point just below this. Returns the number of bytes pushed on to the stack,
    // which should be passed back to leaveABI() as the stackSize argument to restore the stack to original state.
    size_t enterABI();
    // Load 2 pointer arguments from C calling code stack frame or registers, and move them into the supplied registers.
    void loadCArgs2(Reg arg1, Reg arg2);
    // Computes JIT_SP-2 and returns.
    Reg getCStackPointerRegister() const;
    void leaveABI(size_t stackSize);
    using FunctionPointer = void (*)();
    FunctionPointer addressToFunctionPointer(Address a);

    // ==== JIT overrides
    void begin(int8_t* buffer, size_t size) override;
    bool hasJITBufferOverflow() override;
    void reset() override;
    Address end(size_t* sizeOut) override;
    int getRegisterCount() const override;
    int getFloatRegisterCount() const override;

    void addr(Reg target, Reg a, Reg b) override;
    void addi(Reg target, Reg a, Word b) override;
    void andi(Reg target, Reg a, UWord b) override;
    void ori(Reg target, Reg a, UWord b) override;
    void xorr(Reg target, Reg a, Reg b) override;
    void movr(Reg target, Reg value) override;
    void movi(Reg target, Word value) override;
    void movi_u(Reg target, UWord value) override;
    Label bgei(Reg a, Word b) override;
    Label beqi(Reg a, Word b) override;
    Label jmp() override;
    void jmpr(Reg r) override;
    void jmpi(Address location) override;
    void ldr_l(Reg target, Reg address) override;
    void ldi_l(Reg target, void* address) override;
    void ldxi_w(Reg target, Reg address, int offset) override;
    void ldxi_i(Reg target, Reg address, int offset) override;
    void ldxi_l(Reg target, Reg address, int offset) override;
    void str_i(Reg address, Reg value) override;
    void str_l(Reg address, Reg value) override;
    void stxi_w(int offset, Reg address, Reg value) override;
    void stxi_i(int offset, Reg address, Reg value) override;
    void stxi_l(int offset, Reg address, Reg value) override;
    void ret() override;
    void retr(Reg r) override;
    void reti(int value) override;
    Address address() override;
    void patchHere(Label label) override;
    void patchThere(Label target, Address location) override;

    // Lightening requires a call to a global setup function before emitting any JIT bytecode. Repeated calls are
    // harmless.
    static void initJITGlobals();

    const int8_t* getAddress(Address a) const { return static_cast<const int8_t*>(m_addresses[a]); }

private:
    // Converts register number to the Lightening register type.
    jit_gpr_t reg(Reg r) const;
    jit_state_t* m_state;
    std::vector<jit_pointer_t> m_addresses;
    // Non-owning pointers to nodes within the jit_state struct, used for labels.
    std::vector<jit_reloc_t> m_labels;
};

}

#endif // SRC_COMPILER_INCLUDE_HADRON_LIGHTENING_JIT_HPP_