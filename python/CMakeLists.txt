if (SKBUILD)
    # Scikit-Build does not add your site-packages to the search path
    # automatically, so we need to add it _or_ the pybind11 specific directory
    # here.
    execute_process(
            COMMAND "${PYTHON_EXECUTABLE}" -c
            "import pybind11; print(pybind11.get_cmake_dir())"
            OUTPUT_VARIABLE _tmp_dir
            OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ECHO STDOUT)
    list(APPEND CMAKE_PREFIX_PATH "${_tmp_dir}")
endif ()

pybind11_add_module(_pyairspacebooking
        MODULE
        ${CMAKE_CURRENT_LIST_DIR}/Bindings.cpp
        )
set_target_properties(_pyairspacebooking PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
# @formatter:off
target_compile_definitions(_pyairspacebooking PRIVATE VERSION_INFO=${PROJECT_VERSION})
# @formatter:on
set_target_properties(airspacebookingutils PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

target_link_libraries(_pyairspacebooking PRIVATE airspacebookingutils)
install(TARGETS _pyairspacebooking DESTINATION .)
