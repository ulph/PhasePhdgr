cmake_minimum_required(VERSION 3.5)
project(pp_python)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(PYTHON_INC
    ${CMAKE_CURRENT_SOURCE_DIR}/phasephdgr.py
    ${CMAKE_CURRENT_SOURCE_DIR}/migration.py
)

add_custom_target(pp_python SOURCES ${PYTHON_INC})
set_target_properties(pp_python PROPERTIES FOLDER resources)
