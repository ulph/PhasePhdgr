set(SRC
    sinc.hpp
    sinc.cpp
)

add_library(dsp_obj OBJECT ${SRC})
set_target_properties(dsp_obj PROPERTIES COMPILE_FLAGS "-fPIC")
