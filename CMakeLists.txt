cmake_minimum_required(VERSION 3.14)
project(Lab3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.4.0
)
FetchContent_MakeAvailable(Catch2)


add_executable(lab3 main.cpp node.cpp parser.cpp)


add_executable(lab3_tests auto_test.cpp node.cpp parser.cpp)
target_link_libraries(lab3_tests PRIVATE Catch2::Catch2WithMain)

if(MSVC)
    target_compile_options(lab3 PRIVATE /W4)
    target_compile_options(lab3_tests PRIVATE /W4)
else()
    target_compile_options(lab3 PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(lab3_tests PRIVATE -Wall -Wextra -pedantic)
endif()