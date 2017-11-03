set(JSON_INCLUDE_DIR "" CACHE FILEPATH "Install dir of https://nlohmann.github.io/json/")

include_directories(${JSON_INCLUDE_DIR})
if(NOT EXISTS "${JSON_INCLUDE_DIR}/nlohmann/json.hpp")
  message(FATAL_ERROR "Cannot find nlohmann/json.hpp")
endif()