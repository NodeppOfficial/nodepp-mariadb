cmake_minimum_required(VERSION 3.0.0)
project(nodepp-mariadb VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(NODEPP_MARIADB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Search for Nodepp Core
if (NOT TARGET nodepp)
    find_package(nodepp REQUIRED)
    if(NOT nodepp_FOUND)
        message(FATAL_ERROR "Nodepp not found. Please install nodepp.")
    endif()
endif()

# 1. Search for MariaDB/MySQL Headers
find_path(MARIADB_INCLUDE_DIR NAMES mariadb.h REQUIRED
    HINTS /usr/include /usr/local/include
    PATH_SUFFIXES mysql mariadb
)
if(NOT MARIADB_INCLUDE_DIR)
    message(FATAL_ERROR "MariaDB/MySQL header files (mariadb.h) not found.")
endif()

# Search for MariaDB/MySQL Library File
find_library(MARIADB_LIBRARY NAMES mariadbclient mariadb mysqlclient REQUIRED)
if(NOT MARIADB_LIBRARY)
    message(FATAL_ERROR "MariaDB/MySQL library file not found. Please install libmariadb-dev or equivalent files.")
endif()

# interface-target
add_library(nodepp-mariadb INTERFACE)

# Combine all necessary include directories
target_include_directories(nodepp-mariadb INTERFACE
    ${NODEPP_MARIADB_INCLUDE_DIR}
    ${MARIADB_INCLUDE_DIR}
)

# Link against all dependencies (using robust targets/variables)
target_link_libraries(nodepp-mariadb INTERFACE
    nodepp 
    ${MARIADB_LIBRARY}
)