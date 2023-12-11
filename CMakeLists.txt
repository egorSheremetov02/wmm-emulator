cmake_minimum_required(VERSION 3.14)
project(wmm_emulator)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
        tokenizer_test
        tests/tokenizer_ut.cpp
        parser/tokenizer.cpp
)
target_link_libraries(
        tokenizer_test
        GTest::gtest_main
)

add_executable(
        parser_test
        tests/parser_ut.cpp
        parser/tokenizer.cpp
        parser/parser.cpp
)
target_link_libraries(
        parser_test
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(tokenizer_test)
gtest_discover_tests(parser_test)

add_executable(
        wmm_emulator
        main.cpp
        parser/parser.cpp
        parser/tokenizer.cpp
        thread_local_storage.cpp
        memory_subsystem/memory_transition_labels.cpp
        thread_subsystem/thread_subsystem.cpp
        executors/controllable_executor.cpp
        executors/random_executor.cpp
        executors/interactive_executor.cpp
        executors/mc_executor.cpp
        executors/user_executor.cpp
        memory_subsystem/sc/sc_memory_subsystem.cpp
        memory_subsystem/tso/tso_memory_subsystem.cpp
        memory_subsystem/pso/pso_memory_subsystem.cpp
)
