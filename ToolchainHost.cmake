set(CMAKE_CXX_FLAGS "-O0 -g -Wall --coverage")

if (WIN32)
    set(CMAKE_FIND_LIBRARY_PREFIXES "lib" "")
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".dll" ".dll.a" ".lib" ".a")
endif (WIN32)

function(add_mcu_executable NAME MCU)
    add_executable(${NAME}.elf apps/_empty.cpp)
endfunction(add_mcu_executable)

function(add_zoal_tests)
    find_package(GTest)
        
    set(TEST_CASES_FILES
            tests/static_assertion.cpp
            tests/stub_classes.cpp
            )
    add_executable(zoal_tests
            tests/utils/mcu_memory.hpp
            tests/utils/nop.hpp
            tests/utils/ms_counter.cpp
            ${TEST_CASES_FILES}
            )

    target_include_directories(zoal_tests PRIVATE ${GTEST_INCLUDE_DIRS})
    target_link_libraries(zoal_tests PRIVATE GTest::GTest GTest::Main)

    foreach (loop_var ${TEST_CASES_FILES})
        get_filename_component(FILE_NAME ${loop_var} NAME)
        set_property(
                DIRECTORY
                APPEND
                PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
                ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/zoal_tests.dir/tests/${FILE_NAME}.gcno
                ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/zoal_tests.dir/tests/${FILE_NAME}.gcda
        )
    endforeach ()

    set_property(
            DIRECTORY
            APPEND
            PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
            ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/zoal_tests.dir/tests/coverage.info
            ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/zoal_tests.dir/tests/coverage_full.info
    )
endfunction(add_zoal_tests)

function(add_host_executable NAME)
    add_executable(${NAME} ${ARGN})
endfunction(add_host_executable)
