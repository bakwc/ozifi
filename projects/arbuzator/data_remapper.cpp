#include <assert.h>
#include <iostream>

#include <utils/types.h>

#include "data_remapper.h"


template<typename addr_t>
void TDataRemapper<addr_t>::AddSection(const std::string& name, addr_t address, size_t length) {
    TRangeRef range = std::make_shared<TRange<addr_t> >();
    range->Address = address;
    range->Name = name;
    range->Length = length;

    Sections.insert(std::pair<addr_t, TRangeRef>(address, range));
    SectionsByName.insert(std::pair<std::string, TRangeRef>(name, range));
}

template<typename addr_t>
void TDataRemapper<addr_t>::AddNewSection(const std::string& name, addr_t address, size_t length) {
    assert(SectionsByName.find(name) != SectionsByName.end() && "section not found");
    TRangeRef range = SectionsByName[name];
    assert(range->Length == length && "sections sizes should be equal");
    range->NewAddress = address;
}

template<typename addr_t>
boost::optional<addr_t> TDataRemapper<addr_t>::GetNewAddress(addr_t oldAddress) const {
    auto it = Sections.lower_bound(oldAddress);
    if (it == Sections.end()) {
        if (!Sections.empty()) {
            --it;
        }
    } else {
        if (it->first > oldAddress) {
            if (it == Sections.begin()) {
                it = Sections.end();
            } else {
                --it;
            }
        }
    }
    if (it == Sections.end()) {
        return boost::optional<addr_t>();
    }
    const TRangeRef& range = it->second;
    if (range->Address <= oldAddress && range->Address + range->Length >= oldAddress) {
        std::cout << "remaping section " << range->Name << "\n";
        addr_t delta = oldAddress - range->Address;
        return range->NewAddress + delta;
    }
    return boost::optional<addr_t>();
}

template class TDataRemapper<ui32>;
template class TDataRemapper<ui64>;
