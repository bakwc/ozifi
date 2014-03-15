#include <utils/string.h>
#include <utils/exception.h>

#include <contrib/json/reader.h>

#include "instruction_remapper.h"


using namespace std;

TInstructionRemapper::TInstructionRemapper(const TDataRemapper<ui32>& dataRemapper,
                                           const TImportFunctionsMapper<ui32>& importRemapper,
                                           const string& rulesFile)
    : DataRemapper(dataRemapper)
    , ImportRemapper(importRemapper)
{
    LoadRules(rulesFile);
}

// todo: use more efficient rule matching here
bool TInstructionRemapper::Remap(string& instruction) {
    for (size_t i = 0; i < Rules.size(); ++i) {
        const TRule& rule = Rules[i];
        if (instruction.find(rule.Opcode) != 0) {
            continue;
        }

        ui32* addr = reinterpret_cast<ui32*>(&instruction[rule.Offset]);
        auto newAddr = ImportRemapper.GetNewAddress(*addr);
        if (!newAddr.is_initialized()) {
            newAddr = DataRemapper.GetNewAddress(*addr);
        }
        if (newAddr.is_initialized()) {
            *addr = *newAddr;
            return true;
        }
    }
    return false;
}

string StrToOpcode(const string& opcodeStr) {
    if (opcodeStr.empty() || opcodeStr.size() % 2 != 0) {
        throw UException("failed to parse opcode string: " + opcodeStr);
    }
    string opcode;
    for (size_t i = 0; i < opcodeStr.size(); i += 2) {
        string currentOpcode = opcodeStr.substr(i, 2);
        istringstream hexStream(currentOpcode);
        unsigned int c;
        hexStream >> std::hex >> c;
        opcode.push_back(c);
    }
    return opcode;
}

void TInstructionRemapper::LoadRules(const string& rulesFile) {
    cout << "LOADING: " << rulesFile << "\n";
    string data = LoadFile(rulesFile);
    if (data.empty()) {
        throw UException("no data to parse");
    }
    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(data, root)) {
        throw UException("failed to parse json");
    }
    for (size_t i = 0; i < root.size(); ++i) {
        const Json::Value& ruleVal = root[i];
        TRule rule;
        rule.Opcode = StrToOpcode(ruleVal["opcode"].asString());
        rule.Offset = ruleVal["offset"].asUInt();
        Rules.push_back(rule);
    }
}
