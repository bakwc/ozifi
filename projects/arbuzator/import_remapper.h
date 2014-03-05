#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <boost/optional.hpp>
#include <utils/types.h>

#include <pe_bliss/pe_bliss.h>

template<typename addr_t>
struct TImportFunction {
    TImportFunction();
    std::string Name;
    addr_t OriginalAddress;
    addr_t NewAddress;
};

template<typename addr_t>
class TImportFunctionsMapper {
    typedef TImportFunction<addr_t> TImporFunc;
    typedef std::shared_ptr<TImporFunc> TImportFunctionRef;
public:
    void Prepare(const pe_bliss::pe_base& originalImage); // first step - prepare
    void Update(const pe_bliss::pe_base& newImage);       // second step - update
    boost::optional<addr_t> GetNewAddress(addr_t oldAddress); // now you can use it to get new addresses
private:
    std::unordered_map<addr_t, TImportFunctionRef> ImportFunctionsByOriginalAddress;
    std::unordered_map<std::string, TImportFunctionRef> ImportFunctionsByName;
};
