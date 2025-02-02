#include "hadron/Materializer.hpp"

#include "hadron/Arch.hpp"
#include "hadron/BlockSerializer.hpp"
#include "hadron/Emitter.hpp"
#include "hadron/Frame.hpp"
#include "hadron/hir/BlockLiteralHIR.hpp"
#include "hadron/LifetimeAnalyzer.hpp"
#include "hadron/LinearFrame.hpp"
#include "hadron/RegisterAllocator.hpp"
#include "hadron/Resolver.hpp"
#include "hadron/ThreadContext.hpp"
#include "hadron/VirtualJIT.hpp"

namespace hadron {

// static
library::Int8Array Materializer::materialize(ThreadContext* context, Frame* frame) {
    // Compile any inner blocks first.
    for (const auto innerBlock : frame->innerBlocks) {
        auto functionDef = library::FunctionDef::alloc(context);
        auto innerByteCode = Materializer::materialize(context, innerBlock->frame.get());
        functionDef.setCode(innerByteCode);
        functionDef.setSelectors(innerBlock->frame->selectors);
        functionDef.setPrototypeFrame(innerBlock->frame->prototypeFrame);

        // TODO: argNames, varNames?

        innerBlock->functionDef = functionDef;
        frame->selectors = frame->selectors.typedAdd(context, functionDef);
    }

    BlockSerializer serializer;
    auto linearFrame = serializer.serialize(frame);

    LifetimeAnalyzer lifetimeAnalyzer;
    lifetimeAnalyzer.buildLifetimes(linearFrame.get());

    hadron::RegisterAllocator registerAllocator(hadron::kNumberOfPhysicalRegisters);
    registerAllocator.allocateRegisters(linearFrame.get());

    hadron::Resolver resolver;
    resolver.resolve(linearFrame.get());

    size_t bytecodeSize = linearFrame->instructions.size() * 16;
    auto bytecode = library::Int8Array::arrayAlloc(context, bytecodeSize);
    bytecodeSize = bytecode.capacity(context);

    hadron::VirtualJIT jit;
    jit.begin(bytecode.start(), bytecodeSize);

    hadron::Emitter emitter;
    emitter.emit(linearFrame.get(), &jit);

    size_t finalSize = 0;
    jit.end(&finalSize);
    assert(finalSize < bytecodeSize);

    return bytecode;
}

} // namespace hadron