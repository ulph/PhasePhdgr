set(THREADPOOL_INCLUDE_DIR "" CACHE FILEPATH "Install dir of https://github.com/progschj/ThreadPool")

include_directories(${THREADPOOL_INCLUDE_DIR})
if(NOT EXISTS "${THREADPOOL_INCLUDE_DIR}/ThreadPool.h")
  message(FATAL_ERROR "Cannot find ThreadPool.h")
endif()
