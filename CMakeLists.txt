cmake_minimum_required(VERSION 3.26)
project(airspacebookingutils VERSION 0.1.0 DESCRIPTION "Airspace Booking Utils" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)

include(FetchContent)
include(GNUInstallDirs)
include_guard(GLOBAL)

# Disallow in-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "In-source builds are not allowed. Please use a separate build directory.")
endif()

# Use ccache if available
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

# Enable optional linters
if(NOT MSVC)
    find_program(CLANGTIDY_EXE NAMES clang-tidy)
    if(CLANGTIDY_EXE)
        set(CMAKE_CXX_CLANG_TIDY ${CLANGTIDY_EXE})
    endif()
endif()

# Include source and header dirs
add_subdirectory(src)

# Create library target
add_library(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${ABU_SOURCES})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Enable PIC for static builds
if(NOT BUILD_SHARED_LIBS)
    set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

##############################################################
# Dependencies (via find_package with CMakeDeps from Conan)
##############################################################
find_package(Eigen3 REQUIRED CONFIG)
find_package(proj REQUIRED CONFIG)
find_package(GEOS REQUIRED CONFIG)
find_package(absl REQUIRED CONFIG)
find_package(OpenSSL REQUIRED CONFIG)
find_package(spdlog REQUIRED CONFIG)
find_package(h3 REQUIRED CONFIG)
find_package(s2 REQUIRED CONFIG)

# Link dependencies
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Eigen3::Eigen
        PROJ::proj
        GEOS::geos_c
        absl::base
        spdlog::spdlog
        h3::h3
        s2::s2
)

# Get Data dir in conan package
string(TOUPPER ${CMAKE_BUILD_TYPE} ${PROJECT_NAME}_UPPER_BUILD_TYPE)
set(PROJ_DATA "${proj_PACKAGE_FOLDER_${${PROJECT_NAME}_UPPER_BUILD_TYPE}}/res")

# check if PROJ_DATA is empty
if (NOT DEFINED PROJ_DATA OR PROJ_DATA STREQUAL "")
    # Hacky way to get the data dir from the conan package
    message(WARNING "Could not find PROJ data dir for build type ${CMAKE_BUILD_TYPE} using proj_PACKAGE_FOLDER_${${PROJECT_NAME}_UPPER_BUILD_TYPE} config target variable")
    set(PROJ_DATA "${PROJ_INCLUDE_DIR_${${PROJECT_NAME}_UPPER_BUILD_TYPE}}/../res/")
else ()
    message(STATUS "Found PROJ data dir: ${PROJ_DATA}")
endif ()
    # Check that proj.db actually exists in PROJ_DATA
    if (NOT EXISTS "${PROJ_DATA}/proj.db")
        message(FATAL_ERROR "Could not find proj.db in assumed PROJ_DATA dir: ${PROJ_DATA}")
    endif ()
add_custom_target(ABU_proj_data
        ${CMAKE_COMMAND} -E copy_directory ${PROJ_DATA} ${PROJECT_BINARY_DIR}/bin)
add_dependencies(${PROJECT_NAME} ABU_proj_data)
# @formatter:off
# Set this to a compile def that is used when PROJ contexts are created
target_compile_definitions(${PROJECT_NAME} PUBLIC PROJ_DATA_PATH="${PROJ_DATA}")
# @formatter:on

# # Hack to find PROJ data dir from the conan package
# if(NOT PROJ_DATA)
#     set(PROJ_DATA "${CMAKE_CURRENT_SOURCE_DIR}/data")
#     if(EXISTS "${PROJ_DATA}")
#         message(STATUS "Using PROJ data from ${PROJ_DATA}")
#     else()
#         message(FATAL_ERROR "PROJ data directory not found. Please set PROJ_DATA to the correct path.")
#     endif()
# endif()

# message(STATUS "Copying PROJ data from ${PROJ_DATA} to ${PROJECT_BINARY_DIR}/bin")
# add_custom_target(ABU_proj_data
#     ${CMAKE_COMMAND} copy_directory ${PROJ_DATA} ${PROJECT_BINARY_DIR}/bin
# )
# add_dependencies(${PROJECT_NAME} ABU_proj_data)
# target_compile_definitions(${PROJECT_NAME} PUBLIC PROJ_DATA_PATH="${PROJ_DATA}")

# Optional TBB support
find_package(TBB CONFIG)
if(TBB_FOUND)
    message(STATUS "Found TBB, enabling parallel STL")
    target_link_libraries(${PROJECT_NAME} PUBLIC TBB::tbb)
endif()

# Optional OpenMP
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC OpenMP::OpenMP_CXX)
endif()

# Windows-specific flags
if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PUBLIC NOMINMAX _USE_MATH_DEFINES)
    target_compile_options(${PROJECT_NAME} PRIVATE /permissive-)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -pedantic-errors)
endif()

###########################################################
# Python bindings and test setup
###########################################################
add_subdirectory(extern/pybind11)
add_subdirectory(python)

# Enable testing if requested
include(CTest)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()
