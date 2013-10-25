#include <utils/types.h>
#include <utils/exception.h>

#include "serializer.h"

using namespace std;

string Serialize(const string& data) {
    ui16 size = data.size();
    string result((char*)(&size), 2);
    result += data;
    return result;
}


bool Deserialize(string& data, string& result, size_t maxSize) {
    if (data.size() < 2) {
        return false;
    }
    const ui16* size = (const ui16*)(&data[0]);
    if (*size > maxSize) {
        throw UException("size overflow");
    }
    if (data.size() - 2 < *size) {
        return false;
    }
    result = data.substr(2, *size);
    data.erase(0, *size + 2);
    return true;
}
