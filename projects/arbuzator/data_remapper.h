#pragma once

#include <map>
#include <unordered_map>
#include <memory>

#include <boost/optional.hpp>

template<typename addr_t>
struct TRange {
    std::string Name;
    addr_t Address;
    addr_t NewAddress;
    size_t Length;
};

template<typename addr_t>
class TDataRemapper {
    typedef std::shared_ptr<TRange<addr_t> > TRangeRef;
public:
    void AddSection(const std::string& name, addr_t address, size_t length);
    void AddNewSection(const std::string& name, addr_t address, size_t length);
    boost::optional<addr_t> GetNewAddress(addr_t oldAddress) const;
private:
    std::map<addr_t, TRangeRef> Sections;
    std::unordered_map<std::string, TRangeRef> SectionsByName;
};
