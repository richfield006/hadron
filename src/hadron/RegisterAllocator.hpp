#ifndef SRC_COMPILER_INCLUDE_HADRON_REGISTER_ALLOCATOR_HPP_
#define SRC_COMPILER_INCLUDE_HADRON_REGISTER_ALLOCATOR_HPP_

#include "hadron/LifetimeInterval.hpp"

#include <list>
#include <vector>

namespace hadron {

struct LinearFrame;

// The RegisterAllocator takes a LinearFrame in SSA form with lifetime ranges and outputs a register allocation
// schedule for each value.
//
// This class implements the Linear Scan algorithm detailed in [RA4] in the bibliography, "Optimized Interval Splitting
// in a Linear Scan Register Allocator", by C. Wimmer and H. Mössenböck, including the modifications to the algorithm to
// accomodate SSA form suggested in [RA5], "Linear Scan Register Allocation on SSA Form", by C. Wimmer and M. Franz.
class RegisterAllocator {
public:
    RegisterAllocator() = delete;
    RegisterAllocator(size_t numberOfRegisters);
    ~RegisterAllocator() = default;

    void allocateRegisters(LinearFrame* linearFrame);

private:
    bool tryAllocateFreeReg();
    void allocateBlockedReg(LinearFrame* linearFrame);
    void spill(LtIRef interval, LinearFrame* linearFrame);
    void handled(LtIRef interval, LinearFrame* linearFrame);

    LtIRef m_current;
    std::vector<LtIRef> m_unhandled;
    std::vector<LtIRef> m_active;
    std::vector<std::list<LtIRef>> m_inactive;
    std::vector<LtIRef> m_activeSpills;
    size_t m_numberOfRegisters;
};

} // namespace hadron

#endif // SRC_COMPILER_INCLUDE_HADRON_REGISTER_ALLOCATOR_HPP_