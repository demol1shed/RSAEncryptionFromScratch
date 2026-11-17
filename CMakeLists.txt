cmake_minimum_required(VERSION 3.16)
project(RSAEncryptionFromScratch CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(bigint STATIC src/BigInt.cpp src/Prime.cpp src/Rsa.cpp)
target_include_directories(bigint PUBLIC include)
target_compile_options(bigint PRIVATE -Wall -Wextra -Wpedantic)

add_executable(rsa src/main.cpp)
target_link_libraries(rsa PRIVATE bigint)

enable_testing()
add_executable(tests tests/test_main.cpp tests/test_bigint.cpp tests/test_rsa.cpp)
target_link_libraries(tests PRIVATE bigint)
target_compile_options(tests PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME tests COMMAND tests)
