// hashkw generates the Keywords.hpp header file for use in the Hadron library. This keywords header
// file contains computed hashes of keywords that it is useful at compile time to have hashes for.
#include "hadron/Hash.hpp"

#include "fmt/core.h"

#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {
    // TODO: standardize names of symbols across Hadron - some "unconventional" choices here..
    std::vector<std::array<std::string, 2>> hashNames = {
        { "kAddHash                 ", "+" },
        { "kArgHash                 ", "arg" },
        { "kAssignHash              ", "=" },
        { "kClassVarHash            ", "classvar" },
        { "kConstHash               ", "const" },
        { "kDivideHash              ", "/" },
        { "kEqualToHash             ", "==" },
        { "kExactlyEqualToHash      ", "===" },
        { "kFalseHash               ", "false" },
        { "kGreaterThanHash         ", ">" },
        { "kGreaterThanOrEqualToHash", ">=" },
        { "kIfHash                  ", "if" },
        { "kLeftArrowHash           ", "<-" },
        { "kLessThanHash            ", "<" },
        { "kLessThanOrEqualToHash   ", "<=" },
        { "kModuloHash              ", "%" },
        { "kMultiplyHash            ", "*" },
        { "kNewHash                 ", "new" },
        { "kNilHash                 ", "nil" },
        { "kNotEqualToHash          ", "!=" },
        { "kNotExactlyEqualToHash   ", "!==" },
        { "kObjectHash              ", "Object" },
        { "kPipeHash                ", "|" },
        { "kReadWriteHash           ", "<>" },
        { "kSubtractHash            ", "-" },
        { "kSuperHash               ", "super" },
        { "kThisHash                ", "this" },
        { "kTrueHash                ", "true" },
        { "kVarHash                 ", "var" },
        { "kWhileHash               ", "while" }
    };

    std::ofstream outFile("Keywords.hpp");
    if (!outFile) {
        std::cerr << "hashkw failed to open output Keywords.hpp file" << std::endl;
        return -1;
    }

    outFile << "#ifndef _BUILD_SRC_KEYWORDS_HPP_" << std::endl;
    outFile << "#define _BUILD_SRC_KEYWORDS_HPP_" << std::endl << std::endl;

    outFile << "// This file was automatically generated by the hashkw.cpp program." << std::endl;
    outFile << "// Please do not modify directly, modify that program instead." << std::endl;
    outFile << std::endl;

    outFile << "namespace hadron {" << std::endl << std::endl;

    for (const auto& pair : hashNames) {
        outFile << fmt::format("    static constexpr uint64_t {} = 0x{:016x};  // \'{}\'\n", pair[0],
            hadron::hash(pair[1]), pair[1]);
    }

    outFile << std::endl << "} // namespace hadron" << std::endl << std::endl;

    outFile << "#endif // _BUILD_SRC_KEYWORDS_HPP_" << std::endl;
    return 0;
}