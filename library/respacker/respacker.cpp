#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <inttypes.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <utils/string.h>

using namespace std;
using namespace boost::filesystem;

path relativePath( const path& base, const path &relativeTo )
{
    // create absolute paths
    path p = absolute(base);
    path r = absolute(relativeTo);

    // if root paths are different, return absolute path
    if( p.root_path() != r.root_path() )
        return p;

    // initialize relative path
    path result;

    // find out where the two paths diverge
    path::const_iterator itr_path = p.begin();
    path::const_iterator itr_relative_to = r.begin();
    while( *itr_path == *itr_relative_to && itr_path != p.end() && itr_relative_to != r.end() ) {
        ++itr_path;
        ++itr_relative_to;
    }

    // add "../" for each remaining token in relative_to
    if( itr_relative_to != r.end() ) {
        ++itr_relative_to;
        while( itr_relative_to != r.end() ) {
            result /= "..";
            ++itr_relative_to;
        }
    }

    // add remaining path
    while( itr_path != p.end() ) {
        result /= *itr_path;
        ++itr_path;
    }

    return result;
}

FILE* open_or_exit(const char* fname, const char* mode)
{
  FILE* f = fopen(fname, mode);
  if (f == NULL) {
    perror(fname);
    _exit(42);
  }
  return f;
}

void dump_data(const string& name, const string& data, FILE* out) {
    size_t linecount = 0;

    fprintf(out, "const unsigned char %s[] = {\n", name.c_str());
    for (size_t i = 0; i < data.size(); i++) {
        fprintf(out, "0x%02x, ", (unsigned char)data[i]);
        if (++linecount == 16) {
            fprintf(out, "\n");
            linecount = 0;
        }
    }
    fprintf(out, "};\n");
    fprintf(out, "const size_t %s_len = sizeof(%s);\n\n", name.c_str(), name.c_str());
}

struct TIndexRecord {
    string Name;
    size_t Position;
    size_t Size;
};

void pack(const vector<TIndexRecord>& index, const string& data) {
    FILE* out = open_or_exit("resources.h","w");
    fprintf(out, "#pragma once\n");
    fprintf(out, "#include <string>\n");
    fprintf(out, "bool GetResource(const std::string& key, std::string& data);\n");
    fclose(out);


    out = open_or_exit("resources.cpp","w");
    fprintf(out, "#include <stdlib.h>\n");
    fprintf(out, "#include <unordered_map>\n");
    fprintf(out, "#include \"resources.h\"\n");
    fprintf(out, "const unsigned char resource_data[] = {\n");

    size_t linecount = 0;
    for (size_t i = 0; i < data.size(); i++) {
        fprintf(out, "0x%02x, ", (unsigned char)data[i]);
        if (++linecount == 16) {
            fprintf(out, "\n");
            linecount = 0;
        }
    }
    fprintf(out, "};\n");
    fprintf(out,
            "struct TResourcesIndexLoader {\n"
            "    TResourcesIndexLoader() {\n");
    for (size_t i = 0; i < index.size(); i++) {
        fprintf(out,
            "        data[\"%s\"] = std::pair<size_t, size_t>(%u, %u);\n",
                index[i].Name.c_str(),
                index[i].Position,
                index[i].Size);
    }
    fprintf(out,
            "    }\n"
            "    std::unordered_map<std::string, std::pair<size_t, size_t> > data;\n"
            "} resources_index;\n\n"
            "bool GetResource(const std::string& key, std::string& data) {\n"
            "    std::unordered_map<std::string, std::pair<size_t, size_t> >::const_iterator it;\n"
            "    it = resources_index.data.find(key);\n"
            "    if (it == resources_index.data.end()) {\n"
            "        return false;\n"
            "    }\n"
            "    const std::pair<size_t, size_t>& record = it->second;\n"
            "    data = std::string((const char*)&resource_data[record.first], record.second);\n"
            "    return true;\n"
            "}\n"
            );
    fclose(out);
}

int main(int argc, const char** argv) {
    if (argc != 2) {
        cerr << "  Usage: " + string(argv[0]) + " directory_with_resources\n";
        return 42;
    }

    string serializedData;
    vector<TIndexRecord> index;
    string directory = argv[1];

    path base(directory);
    recursive_directory_iterator it(base);

    //cerr << "Base path: " << it
    recursive_directory_iterator end;
    while (it != end) {
        path p = it->path();
        if (is_regular_file(p) || is_symlink(p)) {
            try {
                string filename = p.normalize().string();
                string key = "/" + (relativePath(p, base).string());
                string data = LoadFile(filename);

                TIndexRecord record;
                record.Name = key;
                record.Position = serializedData.size();
                record.Size = data.size();
                index.push_back(record);
                serializedData += data;
            } catch (const std::exception& e) {
                // just skip file if loading failed
            }
        }
        it++;
    }

    pack(index, serializedData);

    return 0;
}
