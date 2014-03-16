#pragma once

#include <vector>

#include "import_remapper.h"
#include "data_remapper.h"

class TInstructionRemapper {
    struct TRule {
        std::string Opcode;
        size_t Offset;
    };
public:
    TInstructionRemapper(const TDataRemapper<ui32>& dataRemapper,
                         const TImportFunctionsMapper<ui32>& importRemapper,
                         const std::string& rulesFile);
    bool Remap(std::string& instruction);
    bool Remap(std::string& instruction, const std::vector<size_t>& offsets);
private:
    void LoadRules(const std::string& rulesFile);
private:
    const TDataRemapper<ui32>& DataRemapper;
    const TImportFunctionsMapper<ui32>& ImportRemapper;
    std::vector<TRule> Rules;
};
