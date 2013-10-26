add_custom_command(
    OUTPUT state.pb.cc
    COMMAND ${PROJECT_BINARY_DIR}/contrib/google/protobuf-compiler/protoc.exe --proto_path=${PROJECT_SOURCE_DIR}/projects/vocal/vocal_client_lib --cpp_out=${PROJECT_BINARY_DIR}/projects/vocal/vocal_client_lib ${PROJECT_SOURCE_DIR}/projects/vocal/vocal_client_lib/state.proto
    DEPENDS ${PROJECT_SOURCE_DIR}/projects/vocal/vocal_client_lib/state.proto)


add_library (vocal_client_lib client.cpp message.h friend.cpp conference.cpp callback.h state.proto state.pb.cc)
target_link_libraries(vocal_client_lib utils
                             udtlib
                             ${Boost_LIBRARIES})
