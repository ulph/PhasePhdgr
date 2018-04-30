set(SRC
    sinc.hpp
    sinc.cpp
)

add_library(dsp_obj OBJECT ${SRC})
set_target_properties(dsp_obj PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(dsp_obj PROPERTIES FOLDER core)
