
add_library (kwstorage kwstorage.h memory.cpp leveldb.cpp redis.cpp)
target_link_libraries(kwstorage utils
                            leveldb
                            ${Boost_LIBRARIES})
