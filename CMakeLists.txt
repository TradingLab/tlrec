project(tlrec)
cmake_minimum_required(VERSION 2.8)
project(tlrec)

set(wget_SRCS
  main.cpp
)

add_executable(tlrec ${wget_SRCS})

FIND_LIBRARY(pqxx_LIB
             NAMES pqxx
             PATHS /usr/lib64)
IF (pqxx_LIB)
# http://stackoverflow.com/questions/5395578/cmake-target-link-libraries-not-linking-my-library
    target_link_libraries(tlrec ${pqxx_LIB})
ENDIF (pqxx_LIB)

# http://stackoverflow.com/questions/11783932/how-to-add-linker-or-compile-flag-in-cmake-file
add_definitions(-fexceptions)

install(TARGETS tlrec RUNTIME DESTINATION bin)
