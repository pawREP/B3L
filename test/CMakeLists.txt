#############################################
# Unit tests

# enable CTest testing
enable_testing()

# Add a testing executable
file(GLOB TEST_FILES *.cpp )
add_executable(unit_tests ${TEST_FILES})
# Add an library for the example classes
set_target_properties(unit_tests PROPERTIES CXX_STANDARD 20)

target_link_libraries(unit_tests
    B3L
    GTest::GTest 
    GTest::Main
)

add_test(test_all unit_tests)