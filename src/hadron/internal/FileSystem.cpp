#include "FileSystem.hpp"

#include "spdlog/spdlog.h"

#if (__APPLE__)
#include <mach-o/dyld.h>
#include <array>
#endif // __APPLE__

#include <cassert>

namespace hadron {

#if (__APPLE__)
fs::path findBinaryPath() {
    std::array<char, 4096> pathBuffer;
    uint32_t bufferSize = pathBuffer.size();
    auto ret = _NSGetExecutablePath(pathBuffer.data(), &bufferSize);
    if (ret >= 0) {
        return fs::canonical(fs::path(pathBuffer.data()).parent_path());
    }
    SPDLOG_ERROR("Failed to find path of executable!");
    return fs::path();
}
#endif // __APPLE__

fs::path findSCClassLibrary() {
    auto path = findBinaryPath();
    SPDLOG_INFO("Found binary path at {}", path.c_str());
    path += "/../../third_party/bootstrap/SCClassLibrary";
    path = fs::canonical(path);
    SPDLOG_INFO("Found Class Library path at {}", path.c_str());
    assert(fs::exists(path));
    assert(fs::is_directory(path));
    return path;
}

} // namespace hadron