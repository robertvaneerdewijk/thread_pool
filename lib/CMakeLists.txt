file(GLOB_RECURSE TEST_SOURCES "test/test_*.cpp")

foreach(TEST_CPP ${TEST_SOURCES})
    get_filename_component(TEST_TARGET ${TEST_CPP} NAME_WE)
    add_executable(${TEST_TARGET} ${TEST_CPP})
    target_link_libraries(${TEST_TARGET} gtest gtest_main ${TARGET})
    target_include_directories(${TEST_TARGET} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src)
    add_test(NAME ${TEST_TARGET} COMMAND ${TEST_TARGET} WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endforeach()