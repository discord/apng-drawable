cmake_minimum_required(VERSION 3.14)
project(apng_frame_tests LANGUAGES C CXX)
enable_testing()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(native_dir ${CMAKE_CURRENT_SOURCE_DIR}/../../main/cpp)
add_executable(frame_storage_test
    FrameStorageTest.cpp
    ${native_dir}/apng-drawable/ApngFrame.cpp
    ${native_dir}/apng-drawable/ApngImage.cpp
    ${native_dir}/lz4/lz4.c)
target_include_directories(frame_storage_test PRIVATE
    ${native_dir}/apng-drawable ${native_dir}/lz4)
target_compile_options(frame_storage_test PRIVATE -UNDEBUG)
option(ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
if(ENABLE_SANITIZERS)
    target_compile_options(frame_storage_test PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(frame_storage_test PRIVATE -fsanitize=address,undefined)
endif()
add_test(NAME frame_storage COMMAND frame_storage_test)
