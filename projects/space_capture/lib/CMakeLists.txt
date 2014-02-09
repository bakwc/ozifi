add_custom_command(
    OUTPUT space.pb.cc
    COMMAND ${PROJECT_BINARY_DIR}/contrib/google/protobuf-compiler/protoc --proto_path=${PROJECT_SOURCE_DIR}/projects/space_capture/lib --cpp_out=${PROJECT_BINARY_DIR}/projects/space_capture/lib ${PROJECT_SOURCE_DIR}/projects/space_capture/lib/space.proto
    DEPENDS ${PROJECT_SOURCE_DIR}/projects/space_capture/lib/space.proto)


add_library(space_capture_lib space.proto
                              space.pb.cc
                              defines.h)

target_link_libraries(space_capture_lib
                             protobuf
                             vocal_lib)
