add_executable (protoc
                    code_generator.cc
                    command_line_interface.cc
                    importer.cc
                    main.cc
                    parser.cc
                    plugin.cc
                    plugin.pb.cc
                    subprocess.cc
                    zip_writer.cc
                    
                    cpp/cpp_enum.cc
                    cpp/cpp_enum_field.cc
                    cpp/cpp_extension.cc
                    cpp/cpp_field.cc
                    cpp/cpp_file.cc
                    cpp/cpp_generator.cc
                    cpp/cpp_helpers.cc
                    cpp/cpp_message.cc
                    cpp/cpp_message_field.cc
                    cpp/cpp_primitive_field.cc
                    cpp/cpp_service.cc
                    cpp/cpp_string_field.cc
                    
                    java/java_doc_comment.cc
                    java/java_enum.cc
                    java/java_enum_field.cc
                    java/java_extension.cc
                    java/java_field.cc
                    java/java_file.cc
                    java/java_generator.cc
                    java/java_helpers.cc
                    java/java_message.cc
                    java/java_message_field.cc
                    java/java_primitive_field.cc
                    java/java_service.cc
                    java/java_string_field.cc
                    
                    python/python_generator.cc)

target_link_libraries(protoc protobuf pthread)
