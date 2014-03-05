#include "import_remapper.h"

using namespace std;
using namespace pe_bliss;

TImportFunction::TImportFunction()
    : OriginalAddress(0)
    , NewAddress(0)
{
}

void TImportFunctionsMapper::Prepare(const pe_base& originalImage) {
    ui64 origBase;
    originalImage.get_image_base(origBase);
    imported_functions_list origFunctions = get_imported_functions(originalImage);

    for (size_t i = 0; i < origFunctions.size(); ++i) {
        import_library& lib = origFunctions[i];
        ui64 currentLibOffset = origBase + lib.get_rva_to_iat();
        const import_library::imported_list& currentLibFuncs = lib.get_imported_functions();
        for (size_t j = 0; j < currentLibFuncs.size(); ++j) {
            const imported_function& currentFunc = currentLibFuncs[j];
            TImportFunctionRef func = make_shared<TImportFunction>();
            func->Name = currentFunc.get_name();
            func->OriginalAddress = currentLibOffset + currentFunc.get_hint();  // todo: check if hint is really func offset
            ImportFunctionsByName[func->Name] = func;
        }
    }
}

void TImportFunctionsMapper::Update(const pe_base& newImage) {
    ui64 newBase;
    newImage.get_image_base(newBase);
    imported_functions_list newFunctions = get_imported_functions(newImage);

    for (size_t i = 0; i < newFunctions.size(); ++i) {
        import_library& lib = newFunctions[i];
        ui64 currentLibOffset = newBase + lib.get_rva_to_iat();
        const import_library::imported_list& currentLibFuncs = lib.get_imported_functions();
        for (size_t j = 0; j < currentLibFuncs.size(); ++j) {
            const imported_function& currentFunc = currentLibFuncs[j];
            TImportFunctionRef func = ImportFunctionsByName[currentFunc.get_name()];
            func->NewAddress = currentLibOffset + currentFunc.get_hint();
            assert(func->OriginalAddress && "original address shoul not be null");
            ImportFunctionsByOriginalAddress[func->OriginalAddress] = func;
        }
    }
}

boost::optional<ui64> TImportFunctionsMapper::GetNewAddress(ui64 oldAddress) {
    auto it = ImportFunctionsByOriginalAddress.find(oldAddress);
    if (it != ImportFunctionsByOriginalAddress.end()) {
        return it->second->NewAddress;
    }
    return boost::optional<ui64>();
}
