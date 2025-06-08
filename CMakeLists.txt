cmake_minimum_required(VERSION 3.28.3)
project(geoexport)

set(CMAKE_CXX_STANDARD 20)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)



add_executable(geoexport source/main.cpp
        header/height_map.h
        header/visibility.h
        header/test_deleted_data.h
        source/visibility.cpp
        source/height_map.cpp
        source/test_daleted_data.cpp
        source/coverage.cpp
        header/coverage.h
)

target_link_libraries(geoexport sfml-graphics sfml-window sfml-system)