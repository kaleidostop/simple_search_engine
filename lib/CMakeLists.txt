add_library(indexer1 indexer.cpp)
add_library(search1 search.cpp)

add_executable(indexer indexer_main.cpp)
add_executable(search search_main.cpp)

target_link_libraries(indexer indexer1)
target_link_libraries(search indexer1 search1)