##################################
# GoogleTest
# Test framework
# License: BSD-3
##################################
if (NOT TARGET gtest)
	FetchContent_Declare(
			googletest
			GIT_REPOSITORY https://github.com/google/googletest.git
			GIT_TAG v1.13.0
	)
	# For Windows: Prevent overriding the parent project's compiler/linker settings
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
	FetchContent_MakeAvailable(googletest)
endif ()
include(GoogleTest)

macro(ab_add_test TESTNAME)
    # create an exectuable in which the tests will be stored
    add_executable(${TESTNAME} ${ARGN})
    # link the Google test infrastructure, mocking library, and a default main fuction to
    # the test executable.  Remove g_test_main if writing your own main function.
    target_link_libraries(${TESTNAME} PUBLIC gmock_main gtest_main gtest gmock ${PROJECT_NAME})
    target_include_directories(${TESTNAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
    if(NOT DEFINED ENV{CI})
        # Set defines to run only in local environments, not in CI
    endif()
    # gtest_discover_tests replaces gtest_add_tests,
    # see https://cmake.org/cmake/help/v3.10/module/GoogleTest.html for more options to pass to it
    gtest_discover_tests(
            ${TESTNAME}
            # set a working directory so your project root so that you can find test data via paths relative to the project root
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    )
    set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
endmacro()

ab_add_test(ConflictTests ConflictTests.cpp)
ab_add_test(H3IndexTests H3IndexTests.cpp)

