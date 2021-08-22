#include "hadron/Resolver.hpp"

#include "hadron/BlockSerializer.hpp"
#include "hadron/HIR.hpp"
#include "hadron/SSABuilder.hpp"

#include <cassert>

/*
Pseudocode taken from [RA5] in the Bibliography, "Linear Scan Register Allocation on SSA Form." by C. Wimmer and M.
Franz.

RESOLVE
for each control flow edge from predecessor to successor do
    for each interval it live at begin of successor do
        if it starts at begin of successor then
            phi = phi function defining it
            opd = phi.inputOf(predecessor)
            if opd is a constant then
                moveFrom = opd
            else
                moveFrom = location of intervals[opd] at end of predecessor
        else
            moveFrom = location of it at end of predecessor
        moveTo = location of it at begin of successor
        if moveFrom ≠ moveTo then
            mapping.add(moveFrom, moveTo)

    mapping.orderAndInsertMoves()
*/

namespace hadron {

void Resolver::resolve(LinearBlock* linearBlock) {
    // for each control flow edge from predecessor to successor do
    for (auto blockNumber : linearBlock->blockOrder) {
        auto blockRange = linearBlock->blockRanges[blockNumber];
        assert(linearBlock->instructions[blockRange.first]->opcode == hir::kLabel);
        const auto blockLabel = reinterpret_cast<const hir::LabelHIR*>(
                linearBlock->instructions[blockRange.first].get());
        for (auto successorNumber : blockLabel->successors) {
            std::vector<std::pair<hir::MoveOperand, hir::MoveOperand>> moves;

            // for each interval it live at begin of successor do
            auto successorRange = linearBlock->blockRanges[successorNumber];
            assert(linearBlock->instructions[successorRange.first]->opcode == hir::kLabel);
            auto successorLabel = reinterpret_cast<hir::LabelHIR*>(
                linearBlock->instructions[successorRange.first].get());
            size_t blockIndex = 0;
            for (; blockIndex < successorLabel->predecessors.size(); ++blockIndex) {
                if (successorLabel->predecessors[blockIndex] == blockNumber) {
                    break;
                }
            }
            assert(blockIndex < successorLabel->predecessors.size());
            for (auto live : successorLabel->liveIns) {
                hir::MoveOperand moveFrom, moveTo;

                // if it starts at begin of successor then
                const hir::PhiHIR* livePhi = nullptr;
                for (const auto& phi : successorLabel->phis) {
                    if (phi->value.number == live) {
                        livePhi = phi.get();
                        break;
                    }
                }
                if (livePhi) {
                    // phi = phi function defining it
                    // opd = phi.inputOf(predecessor)
                    size_t opd = livePhi->inputs[blockIndex].number;
                    // if opd is a constant then TODO
                    //   moveFrom = opd
                    // else
                    //  moveFrom = location of intervals[opd] at end of predecessor
                    bool found = findAt(opd, linearBlock, blockRange.second - 1, moveFrom);
                    assert(found);
                } else {
                    // else
                    // moveFrom = location of it at end of predecessor
                    bool found = findAt(live, linearBlock, blockRange.second - 1, moveFrom);
                    assert(found);
                }
                // moveTo = location of it at begin of successor
                bool found = findAt(live, linearBlock, blockRange.first, moveTo);
                assert(found);
                if (moveFrom.number != moveTo.number || moveFrom.isSpill != moveTo.isSpill) {
                    moves.emplace_back(std::make_pair(moveFrom, moveTo));
                }
            }
            if (moves.size()) {
                // If the block has only one successor, or this is the last successor, we can simply prepend the move
                // instructions to the outbound branch.
                if (blockLabel->successors.size() == 1 || successorNumber == blockLabel->successors.back()) {
                    // Last instruction should always be a branch.
                    assert(linearBlock->instructions[blockRange.second - 1]->opcode == hir::kBranch);
                    auto branch = reinterpret_cast<hir::BranchHIR*>(
                            linearBlock->instructions[blockRange.second -1].get());
                    branch->moves.insert(branch->moves.begin(), moves.begin(), moves.end());
                } else if (successorLabel->predecessors.size() == 1) {
                    // If the successor has only the predecessor we can prepend the move instructions in the successor
                    // label.
                    successorLabel->moves.insert(successorLabel->moves.begin(), moves.begin(), moves.end());
                } else {
                    // TODO: probably need to insert an additional block with the move instructions before successor,
                    // and have everything else jump over that.
                    assert(false);
                }
            }
        }
    }
}

bool Resolver::findAt(size_t valueNumber, LinearBlock* linearBlock, size_t line, hir::MoveOperand& operand) {
    for (const auto& lifetime : linearBlock->valueLifetimes[valueNumber]) {
        if (lifetime.start() <= line && line < lifetime.end()) {
            if (!lifetime.isSpill) {
                operand.isSpill = false;
                operand.number = lifetime.registerNumber;
            } else {
                operand.isSpill = true;
                operand.number = lifetime.spillSlot;
            }
            return true;
        }
    }
    return false;
}

} // namespace hadron