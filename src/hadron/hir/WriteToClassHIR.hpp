#ifndef SRC_HADRON_HIR_WRITE_TO_CLASS_HIR_HPP_
#define SRC_HADRON_HIR_WRITE_TO_CLASS_HIR_HPP_

#include "hadron/hir/HIR.hpp"

namespace hadron {
namespace hir {

struct WriteToClassHIR : public HIR {
    WriteToClassHIR() = delete;
    WriteToClassHIR(hir::ID classArray, int32_t index, library::Symbol name, hir::ID v);
    virtual ~WriteToClassHIR() = default;

    hir::ID classVariableArray;
    int arrayIndex;
    library::Symbol valueName;
    hir::ID toWrite;

    ID proposeValue(ID proposedId) override;
    bool replaceInput(ID original, ID replacement) override;
    void lower(LinearFrame* linearFrame) const override;
};

} // namespace hir
} // namespace hadron

#endif // SRC_HADRON_HIR_WRITE_TO_CLASS_HIR_HPP_