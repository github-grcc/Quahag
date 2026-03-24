# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/Quahag_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/Quahag_autogen.dir/ParseCache.txt"
  "Quahag_autogen"
  )
endif()
