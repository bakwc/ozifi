add_custom_command(
    OUTPUT vocal.pb.cc
    COMMAND ${PROJECT_BINARY_DIR}/contrib/google/protobuf-compiler/protoc --proto_path=${PROJECT_SOURCE_DIR}/projects/vocal/vocal_lib --cpp_out=${PROJECT_BINARY_DIR}/projects/vocal/vocal_lib ${PROJECT_SOURCE_DIR}/projects/vocal/vocal_lib/vocal.proto
    DEPENDS ${PROJECT_SOURCE_DIR}/projects/vocal/vocal_lib/vocal.proto)

add_library (vocal_lib resolver.cpp nat_pmp.cpp
             crypto.cpp defines.h serializer.cpp
             compress.cpp vocal.pb.cc vocal.proto
             utils.cpp opus.cpp)

target_link_libraries(vocal_lib utils
                             snappy
                             cryptopp
                             resolver
                             libnatpmp
                             opus
                             ${Boost_LIBRARIES})
