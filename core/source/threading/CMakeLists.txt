set(SRC
    locks.cpp
)

add_library(threading_obj OBJECT ${SRC})
set_target_properties(threading_obj PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(threading_obj PROPERTIES FOLDER core)
