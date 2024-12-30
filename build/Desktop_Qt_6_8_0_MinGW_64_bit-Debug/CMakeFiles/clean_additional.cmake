# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\tsena_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\tsena_autogen.dir\\ParseCache.txt"
  "tsena_autogen"
  )
endif()
