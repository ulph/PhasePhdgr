set(SRC
    sinc.hpp
    sinc.cpp
)

add_library(dsp_obj OBJECT ${SRC})
add_library(dsp $<TARGET_OBJECTS:dsp_obj>)
set_property(TARGET dsp PROPERTY FOLDER lib)

set_target_properties(dsp PROPERTIES COMPILE_FLAGS "-fPIC")

install(TARGETS dsp DESTINATION lib)
