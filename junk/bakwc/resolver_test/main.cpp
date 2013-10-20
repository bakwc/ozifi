#include <iostream>
#include <library/resolver/resolver.h>

using namespace std;

int main(int argc, char** argv) {
    vector<NResolver::TSrvRecord> records = NResolver::GetSrvRecords("_vocal._udp.pastexen.com");
    cout << "Records: " << records.size() << "\n";
    for (size_t i = 0; i < records.size(); ++i) {
        cout << records[i].Host << ":" << records[i].Port << "\n";
    }
    return 0;
}
