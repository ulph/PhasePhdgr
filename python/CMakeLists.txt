project(phasephckr_python)
cmake_minimum_required(VERSION 3.0)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(PYTHON_INC
    ${CMAKE_CURRENT_SOURCE_DIR}/phasephckr.py
)

add_custom_target(phasephckr_python SOURCES ${PYTHON_INC})
set_target_properties(phasephckr_python PROPERTIES FOLDER python)
