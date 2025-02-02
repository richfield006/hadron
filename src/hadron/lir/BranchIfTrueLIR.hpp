#ifndef SRC_HADRON_LIR_BRANCH_IF_TRUE_LIR_HPP_
#define SRC_HADRON_LIR_BRANCH_IF_TRUE_LIR_HPP_

#include "hadron/lir/LIR.hpp"

namespace hadron {
namespace lir {

struct BranchIfTrueLIR : public LIR {
    BranchIfTrueLIR() = delete;
    explicit BranchIfTrueLIR(VReg cond, LabelID label):
            LIR(kBranchIfTrue, TypeFlags::kNoFlags),
            condition(cond),
            labelId(label) { read(condition); }
    virtual ~BranchIfTrueLIR() = default;

    VReg condition;
    LabelID labelId;

    void emit(JIT* jit, std::vector<std::pair<JIT::Label, LabelID>>& patchNeeded) const override {
        emitBase(jit);
        patchNeeded.emplace_back(std::make_pair(jit->beqi(locate(condition), 1), labelId));
    }
};

} // namespace lir
} // namespace hadron

#endif