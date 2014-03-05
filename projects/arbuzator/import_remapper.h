#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <boost/optional.hpp>
#include <utils/types.h>

#include <pe_bliss/pe_bliss.h>

struct TImportFunction {
    TImportFunction();
    std::string Name;
    ui64 OriginalAddress;
    ui64 NewAddress;
};
typedef std::shared_ptr<TImportFunction> TImportFunctionRef;

class TImportFunctionsMapper {
public:
    void Prepare(const pe_bliss::pe_base& originalImage); // first step - prepare
    void Update(const pe_bliss::pe_base& newImage);       // second step - update
    boost::optional<ui64> GetNewAddress(ui64 oldAddress); // now you can use it to get new addresses
private:
    std::unordered_map<ui64, TImportFunctionRef> ImportFunctionsByOriginalAddress;
    std::unordered_map<std::string, TImportFunctionRef> ImportFunctionsByName;
};
