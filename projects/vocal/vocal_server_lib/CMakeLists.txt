add_custom_command(
    OUTPUT storage.pb.cc
    COMMAND ${PROJECT_BINARY_DIR}/contrib/google/protobuf-compiler/protoc --proto_path=${PROJECT_SOURCE_DIR}/projects/vocal/vocal_server_lib --cpp_out=${PROJECT_BINARY_DIR}/projects/vocal/vocal_server_lib ${PROJECT_SOURCE_DIR}/projects/vocal/vocal_server_lib/storage.proto
    DEPENDS ${PROJECT_SOURCE_DIR}/projects/vocal/vocal_server_lib/storage.proto)

add_library (vocal_server_lib server.cpp storage.cpp storage.pb.cc storage.proto sync.cpp types.h)
target_link_libraries(vocal_server_lib utils
                             vocal_lib
                             udtlib
                             http_server
                             captcha
                             kwstorage
                             protobuf
                             ${Boost_LIBRARIES})
