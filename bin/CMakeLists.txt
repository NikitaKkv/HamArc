add_executable(hamarc main.cpp)

target_compile_features(hamarc PRIVATE cxx_std_20)

target_link_libraries(hamarc
    PRIVATE
        archiver      
        argparser       
        type          
)