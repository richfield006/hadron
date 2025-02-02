#include "hadron/Heap.hpp"

#include "spdlog/spdlog.h"

#include <cassert>

namespace hadron {

Heap::Heap(): m_stackPageOffset(0) {}

Heap::~Heap() { /* WRITEME */ }

void* Heap::allocateNew(size_t sizeInBytes) {
    return allocateSized(sizeInBytes, m_youngPages, false);
}

void* Heap::allocateJIT(size_t sizeInBytes, size_t& allocatedSize) {
    auto address = allocateSized(sizeInBytes, m_executablePages, true);
    if (address) {
        auto sizeClass = getSizeClass(sizeInBytes);
        allocatedSize = getSize(sizeClass);
    } else {
        allocatedSize = 0;
    }
    return address;
}

void* Heap::allocateStackSegment() {
    if (m_stackPageOffset == 0) {
        m_stackSegments.emplace_back(std::make_unique<Page>(kLargeObjectSize, kPageSize, false));
        if (!m_stackSegments.back()->map()) {
            SPDLOG_CRITICAL("Failed to map new stack segment.");
            assert(false);
            return nullptr;
        }
    }

    auto address = m_stackSegments.back()->startAddress();
    assert(address);
    m_pageEnds[reinterpret_cast<uintptr_t>(address) + kPageSize] = m_stackSegments.back().get();
    address += m_stackPageOffset;

    m_stackPageOffset = (m_stackPageOffset + kLargeObjectSize) % kPageSize;
    return address;
}

void Heap::freeTopStackSegment() {
    // TODO: consider some page recycling or other hysterisis here to prevent stack boundary oscillation from spamming
    // map/unmap syscalls.
    if (m_stackPageOffset == 0) {
        assert(m_stackSegments.size());
        m_stackSegments.pop_back();
        m_stackPageOffset = kPageSize - kLargeObjectSize;
    } else {
        assert(m_stackPageOffset >= kLargeObjectSize);
        m_stackPageOffset -= kLargeObjectSize;
    }
}

void Heap::addToRootSet(Slot object) {
    m_rootSet.emplace(object.getPointer());
}

void Heap::removeFromRootSet(Slot object) {
    m_rootSet.erase(object.getPointer());
}

size_t Heap::getAllocationSize(void* address) {
    Page* page = findPageContaining(address);
    if (!page) { assert(false); return 0; }
    return page->objectSize();
}

size_t Heap::getMaximumSize(size_t sizeInBytes) {
    return getSize(getSizeClass(sizeInBytes));
}

Heap::SizeClass Heap::getSizeClass(size_t sizeInBytes) {
    if (sizeInBytes <= kSmallObjectSize) {
        return SizeClass::kSmall;
    } else if (sizeInBytes <= kMediumObjectSize) {
        return SizeClass::kMedium;
    } else if (sizeInBytes <= kLargeObjectSize) {
        return SizeClass::kLarge;
    }
    return SizeClass::kOversize;
}

size_t Heap::getSize(SizeClass sizeClass) {
    switch(sizeClass) {
    case kSmall:
        return kSmallObjectSize;
    case kMedium:
        return kMediumObjectSize;
    case kLarge:
        return kLargeObjectSize;
    default:
        return 0;
    }
}

void* Heap::allocateSized(size_t sizeInBytes, SizedPages& sizedPages, bool isExecutable) {
    auto sizeClass = getSizeClass(sizeInBytes);
    if (sizeClass == kOversize) {
        assert(false);
        sizedPages[kOversize].emplace_back(std::make_unique<Page>(sizeInBytes, sizeInBytes, isExecutable));
        if (!sizedPages[kOversize].back()->map()) {
            SPDLOG_ERROR("Mapping failed for oversize object of {} bytes", sizeInBytes);
            return nullptr;
        }
        // We leave oversize pages out of page address map, and search the oversized pages separately.
        return sizedPages[kOversize].back()->allocate();
    }

    // Find existing capacity in already mapped pages.
    for (auto& page : sizedPages[sizeClass]) {
        if (page->capacity()) {
            return page->allocate();
        }
    }

    // HERE is where we would initiate a collection
    mark();
    sweep();

    sizedPages[sizeClass].emplace_back(std::make_unique<Page>(getSize(sizeClass), kPageSize, isExecutable));
    if (!sizedPages[sizeClass].back()->map()) {
        assert(false);
        return nullptr;
    }

    auto address = sizedPages[sizeClass].back()->startAddress();
    assert(address);
    m_pageEnds[reinterpret_cast<uintptr_t>(address) + kPageSize] = sizedPages[sizeClass].back().get();

    return sizedPages[sizeClass].back()->allocate();
}

void Heap::mark() {
    // TODO once we add the Stack should be enough to build a decent root set
}

void Heap::sweep() {
    // TODO once we have mark() going
}

Page* Heap::findPageContaining(void* address) {
    auto addressValue = reinterpret_cast<uintptr_t>(address);
    auto page = m_pageEnds.upper_bound(addressValue);
    if (page == m_pageEnds.end()) {
        return nullptr;
    }
    auto startAddress = reinterpret_cast<uintptr_t>(page->second->startAddress());
    assert(addressValue >= startAddress);
    if (addressValue - startAddress < page->second->totalSize()) {
        return page->second;
    }
    // TODO: search oversize
    return nullptr;
}

} // namespace hadron