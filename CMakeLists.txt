cmake_minimum_required(VERSION 3.10)
project(mu-tagg)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
# Remove find_package(Taglib REQUIRED)

# Try to link with 'tag' (the usual TagLib target)
find_library(TAGLIB_LIB tag)

add_executable(mu-tagg
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
)

target_link_libraries(mu-tagg Qt5::Widgets ${TAGLIB_LIB})
