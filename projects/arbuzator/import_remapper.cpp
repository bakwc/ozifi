#include <iostream>

#include "import_remapper.h"

using namespace std;
using namespace pe_bliss;

template<typename addr_t>
TImportFunction<addr_t>::TImportFunction()
    : OriginalAddress(0)
    , NewAddress(0)
{
}

template<typename addr_t>
void TImportFunctionsMapper<addr_t>::Prepare(const pe_base& originalImage) {
    addr_t origBase;
    originalImage.get_image_base(origBase);
    imported_functions_list origFunctions = get_imported_functions(originalImage);

    for (size_t i = 0; i < origFunctions.size(); ++i) {
        import_library& lib = origFunctions[i];
        addr_t currentLibOffset = origBase + lib.get_rva_to_iat();
        const import_library::imported_list& currentLibFuncs = lib.get_imported_functions();
        for (size_t j = 0; j < currentLibFuncs.size(); ++j) {
            const imported_function& currentFunc = currentLibFuncs[j];
            TImportFunctionRef func = make_shared<TImporFunc>();
            func->Name = currentFunc.get_name();
            //func->OriginalAddress = currentLibOffset + currentFunc.get_hint();  // todo: check if hint is really func offset
            func->OriginalAddress = currentLibOffset + j * sizeof(addr_t);
            ImportFunctionsByName[func->Name] = func;
            cout << func->Name << " " << std::hex << func->OriginalAddress << "\n";
        }
    }
}

template<typename addr_t>
void TImportFunctionsMapper<addr_t>::Update(const pe_base& newImage) {
    addr_t newBase;
    newImage.get_image_base(newBase);
    imported_functions_list newFunctions = get_imported_functions(newImage);

    for (size_t i = 0; i < newFunctions.size(); ++i) {
        import_library& lib = newFunctions[i];
        addr_t currentLibOffset = newBase + lib.get_rva_to_iat();
        const import_library::imported_list& currentLibFuncs = lib.get_imported_functions();
        for (size_t j = 0; j < currentLibFuncs.size(); ++j) {
            const imported_function& currentFunc = currentLibFuncs[j];
            TImportFunctionRef func = ImportFunctionsByName[currentFunc.get_name()];
            func->NewAddress = currentLibOffset + j * sizeof(addr_t);
            assert(func->OriginalAddress && "original address shoul not be null");
            ImportFunctionsByOriginalAddress[func->OriginalAddress] = func;
        }
    }
}

template<typename addr_t>
boost::optional<addr_t> TImportFunctionsMapper<addr_t>::GetNewAddress(addr_t oldAddress) {
    auto it = ImportFunctionsByOriginalAddress.find(oldAddress);
    if (it != ImportFunctionsByOriginalAddress.end()) {
        cout << "Remaed func: " << it->second->Name << "\n";
        return it->second->NewAddress;
    }
    return boost::optional<addr_t>();
}

template class TImportFunctionsMapper<ui32>;
template class TImportFunctionsMapper<ui64>;
