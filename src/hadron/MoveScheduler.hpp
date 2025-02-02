#ifndef SRC_COMPILER_INCLUDE_HADRON_MOVE_SCHEDULER_HPP_
#define SRC_COMPILER_INCLUDE_HADRON_MOVE_SCHEDULER_HPP_

#include <unordered_map>

namespace hadron {
class JIT;
namespace hir {
struct MoveOperand;
} // namespace hir

// SSA resolution moves are assumed to happen all simultaneously. The MoveScheduler determines an order for all moves
// so that no value gets overwritten by another move before its use time. This class used by the Emitter during machine
// code generation.
class MoveScheduler {
public:
    MoveScheduler() = default;
    ~MoveScheduler() = default;

    // Emit JIT machine code to resolve all moves. Returns false if the moves cannot be scheduled because
    // they are ambiguous (>1 move has same destination).
    bool scheduleMoves(const std::unordered_map<int, int>& moves, JIT* jit);

private:
    void move(int destination, int origin, JIT* jit);
};

} // namespace hadron

#endif // SRC_COMPILER_INCLUDE_HADRON_MOVE_SCHEDULER_HPP_