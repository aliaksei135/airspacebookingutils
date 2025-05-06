find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)
find_package(pybind11 CONFIG)

python_add_library(_pyairspacebooking MODULE
    ${CMAKE_CURRENT_LIST_DIR}/Bindings.cpp
    WITH_SOABI
)
target_link_libraries(_pyairspacebooking PRIVATE pybind11::headers)
# pybind11_add_module(_pyairspacebooking
#         MODULE
#         ${CMAKE_CURRENT_LIST_DIR}/Bindings.cpp
#         )
# set_target_properties(_pyairspacebooking PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
# @formatter:off
target_compile_definitions(_pyairspacebooking PRIVATE VERSION_INFO=${PROJECT_VERSION})
# @formatter:on
# set_target_properties(airspacebookingutils PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

target_link_libraries(_pyairspacebooking PRIVATE airspacebookingutils)
install(TARGETS _pyairspacebooking DESTINATION pyairspacebooking)
