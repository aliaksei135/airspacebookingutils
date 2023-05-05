cmake_minimum_required(VERSION 3.26)
project(airspacebookingutils
        VERSION 0.1.0
        DESCRIPTION "Airspace Booking Utils"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)
set(PYTHON_VERSION 3)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
set(${PROJECT_NAME}_EXTERN_DIR ${PROJECT_SOURCE_DIR}/extern)
Include(FetchContent)
include_guard(GLOBAL)

# Verbose Builds
# set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON" FORCE)

### Require out-of-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if (EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles.")
endif ()

# Cache builds
find_program(CCACHE ccache)
if (CCACHE)
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
endif ()

###########################################################
# Static code analysis
###########################################################
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(ENABLE_CPPCHECK OFF)
else ()
    set(ENABLE_CPPCHECK OFF)
endif ()
if (MSVC)
    set(ENABLE_CLANG_TIDY OFF)
else ()
    set(ENABLE_CLANG_TIDY ON)
endif ()

if (ENABLE_CPPCHECK)
    find_program(CPPCHECK cppcheck)
    if (CPPCHECK)
        set(CMAKE_CXX_CPPCHECK
                ${CPPCHECK}
                --suppressions-list=${CMAKE_CURRENT_SOURCE_DIR}/.cppcheck-suppressions
                --suppress=missingIncludeSystem
                --suppress=unmatchedSuppression
                --cppcheck-build-dir=${CMAKE_BINARY_DIR}/cppcheck
                --enable=all
                --inconclusive)
    else ()
        message(WARNING "cppcheck requested but executable not found")
    endif ()
endif ()

if (ENABLE_CLANG_TIDY)
    find_program(CLANGTIDY clang-tidy)
    if (CLANGTIDY)
        set(CMAKE_CXX_CLANG_TIDY ${CLANGTIDY})
    else ()
        message(WARNING "clang-tidy requested but executable not found")
    endif ()
endif ()

###########################################################
# Build
###########################################################
set(ABU_SOURCES
        ${ABU_SOURCES})

add_subdirectory(src)

if (BUILD_SHARED_LIBS)
    add_library(${PROJECT_NAME} ${ABU_SOURCES})
    set_target_properties(${PROJECT_NAME} PROPERTIES
            WINDOWS_EXPORT_ALL_SYMBOLS ON)
else ()
    add_library(${PROJECT_NAME} STATIC ${ABU_SOURCES})
    set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif ()

target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>)

add_subdirectory(include)


##############################################################
# Dependencies
##############################################################
list(APPEND CMAKE_MODULE_PATH ${PROJECT_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${PROJECT_BINARY_DIR})

if (NOT EXISTS "${PROJECT_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0.18.1/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan.cmake"
            TLS_VERIFY ON)
endif ()
include(${CMAKE_BINARY_DIR}/conan.cmake)
conan_cmake_run(CONANFILE conanfile.py
        BASIC_SETUP
        BUILD missing)

# Don't build tests for deps
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

# Eigen required for lin alg
find_package(Eigen3 CONFIG REQUIRED)

find_package(proj CONFIG REQUIRED)
# Get Data dir in conan package
string(TOUPPER ${CMAKE_BUILD_TYPE} ${PROJECT_NAME}_UPPER_BUILD_TYPE)
set(PROJ_DATA ${PROJ_RES_DIRS_${${PROJECT_NAME}_UPPER_BUILD_TYPE}})

# check if PROJ_DATA is empty
if (NOT DEFINED PROJ_DATA OR PROJ_DATA STREQUAL "")
    # Hacky way to get the data dir from the conan package
    message(WARNING "Could not find PROJ data dir for build type ${CMAKE_BUILD_TYPE} using PROJ_RES_DIRS_${${PROJECT_NAME}_UPPER_BUILD_TYPE} config target variable")
    set(PROJ_DATA "${PROJ_INCLUDE_DIR_${${PROJECT_NAME}_UPPER_BUILD_TYPE}}/../res/")
    # Check that proj.db actually exists in PROJ_DATA
    if (NOT EXISTS "${PROJ_DATA}/proj.db")
        message(FATAL_ERROR "Could not find proj.db in assumed PROJ_DATA dir: ${PROJ_DATA}")
    endif ()
else ()
    message(STATUS "Found PROJ data dir: ${PROJ_DATA}")
endif ()
add_custom_target(ABU_proj_data
        ${CMAKE_COMMAND} -E copy_directory ${PROJ_DATA} ${PROJECT_BINARY_DIR}/bin)
add_dependencies(${PROJECT_NAME} ABU_proj_data)
# @formatter:off
# Set this to a compile def that is used when PROJ contexts are created
target_compile_definitions(${PROJECT_NAME} PUBLIC PROJ_DATA_PATH="${PROJ_DATA}")
# @formatter:on
target_link_libraries(${PROJECT_NAME} PUBLIC PROJ::PROJ)

find_package(GEOS CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC GEOS::GEOS)

# S2 Geometry needs building from source
find_package(absl CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC absl::base)
FETCHCONTENT_DECLARE(
        s2geometry
        GIT_REPOSITORY https://github.com/google/s2geometry.git
        GIT_TAG master
)
FETCHCONTENT_MAKEAVAILABLE(s2geometry)
target_link_libraries(${PROJECT_NAME} PUBLIC s2)

# Opportunistically use Intel Thread Building Blocks if available for parallel STL
find_package(TBB CONFIG)
if (TBB_FOUND)
    message(STATUS "Found TBB, using for parallel STL")
    target_link_libraries(${PROJECT_NAME} PUBLIC TBB::tbb)
endif ()

find_package(OpenMP)
target_link_directories(${PROJECT_NAME} PUBLIC OpenMP::OpenMP_CXX)
if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif ()

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC spdlog::spdlog)

find_package(h3 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC h3::h3)


if (MSVC)
    # Prevent Windows.h overwriting min and max with macros
    # Keep Math defines such as M_PI
    target_compile_definitions(${PROJECT_NAME} PUBLIC NOMINMAX _USE_MATH_DEFINES)
    target_compile_options(${PROJECT_NAME} PRIVATE /permissive-)
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE -pedantic-errors)
endif ()


###########################################################
# Python Bindings
###########################################################
add_subdirectory(extern/pybind11)
add_subdirectory(python)

###########################################################
# Testing
###########################################################
if (BUILD_TESTING OR DEFINED $ENV{CLION_IDE})
    enable_testing()
    add_subdirectory(test)
endif ()
