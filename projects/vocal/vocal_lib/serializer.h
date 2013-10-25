#pragma once

#include <string>

// adds size of the string at the begining
std::string Serialize(const std::string& data);

// extracts data to result, removes used data
// true - success
// false - require more data
// exception - size overflow
bool Deserialize(std::string& data, std::string& result, size_t maxSize = 1<<23);
