add_library(archiver archiver.cpp)
target_include_directories(archiver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(archiver PRIVATE argparser type)  # если archiver использует их