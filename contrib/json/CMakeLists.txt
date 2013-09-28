
add_library (json autolink.h config.h features.h forwards.h json.h reader.h value.h writer.h)
target_link_libraries(json libjson)
