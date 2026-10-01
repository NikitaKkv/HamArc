add_library(argparser argparser.cpp)
target_include_directories(argparser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(type type.cpp)  
target_include_directories(type PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})