
add_subdirectory(src)

if (MY_ENABLE_TESTS)
    # add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/tests/test_common_lib")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)
endif()
