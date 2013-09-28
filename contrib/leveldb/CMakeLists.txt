include_directories(${PROJECT_SOURCE_DIR}/contrib/leveldb)

add_library (leveldb
    db/builder.cc
    db/db_impl.cc
    db/db_iter.cc
    db/filename.cc
    db/dbformat.cc
    db/log_reader.cc
    db/log_writer.cc
    db/memtable.cc
    db/repair.cc
    db/table_cache.cc
    db/version_edit.cc
    db/version_set.cc
    db/write_batch.cc
    table/block.cc
    table/block_builder.cc
    table/format.cc
    table/iterator.cc
    table/merger.cc
    table/table.cc
    table/table_builder.cc
    table/two_level_iterator.cc
    table/filter_block.cc
    util/env.cc
    util/env_boost.cc
    util/arena.cc
    util/cache.cc
    util/coding.cc
    util/comparator.cc
    util/crc32c.cc
    util/hash.cc
    util/histogram.cc
    util/logging.cc
    util/options.cc
    util/status.cc
    util/getpid.cc
    util/filter_policy.cc
    util/win_logger.cc
    port/port_win.cc
    port/mman.cc
    include/leveldb/filter_policy.h
    include/leveldb/env.h
    helpers/memenv/memenv.cc)

target_link_libraries(leveldb utils
                             ${Boost_LIBRARIES})
