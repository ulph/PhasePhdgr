set(SRC
    sinc.hpp
    sinc.cpp
)

add_library(dsp_obj OBJECT ${SRC})
set_property(TARGET dsp_obj PROPERTY FOLDER obj)
add_library(dsp ${SRC})

set_target_properties(dsp PROPERTIES COMPILE_FLAGS "-fPIC")

install(TARGETS dsp DESTINATION lib)
