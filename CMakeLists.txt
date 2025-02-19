cmake_minimum_required(VERSION 3.16)
project(DYanalysis VERSION 1.0 LANGUAGES CXX)

# Find ROOT and its necessary components.
find_package(ROOT REQUIRED COMPONENTS Core Tree RIO Hist)

# Include directories: ROOT's include paths and your project's headers.
include_directories(
    ${ROOT_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Set the install prefix
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/install)

# Build output directories in the build tree; this is the recommended approach.
# The executable will be built in <build_dir>/bin and the libraries in <build_dir>/lib.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Gather all .cc source files from the src folder.
file(GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc")

# Define the main source file separately.
set(MAIN_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/Main.cc")

# Create a library for each source file except for Main.cc.
set(LIB_LIST "")
foreach(src_file IN LISTS SRC_FILES)
    if(NOT src_file STREQUAL MAIN_SRC)
        # Get the library name based on the file name (without extension).
        get_filename_component(lib_name ${src_file} NAME_WE)
        add_library(${lib_name} STATIC ${src_file})
        target_link_libraries(${lib_name} PUBLIC ${ROOT_LIBRARIES})
        # Append the library to the list to link it to the final executable.
        list(APPEND LIB_LIST ${lib_name})
    endif()
endforeach()

# Create the executable using only Main.cc.
add_executable(DYanalysis ${MAIN_SRC})
    
# Link the executable with all libraries and ROOT.
target_link_libraries(DYanalysis PRIVATE ${LIB_LIST} ${ROOT_LIBRARIES})

# # For CentOS 7 (LCG_105)
# # Set the C++ standard to C++17.
# set_target_properties(DYanalysis PROPERTIES
#     CXX_STANDARD 17
#     CXX_STANDARD_REQUIRED YES
# )

# For Alma9 (LCG_107)
# Set the C++ standard to C++17.
set_target_properties(DYanalysis PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
)

## Install rules for executable and libraries
install(TARGETS DYanalysis
    RUNTIME DESTINATION .  # This will copy the executable directly into ${CMAKE_INSTALL_PREFIX}
)

install(TARGETS ${LIB_LIST}
    ARCHIVE DESTINATION lib  # Libraries will be installed into ${CMAKE_INSTALL_PREFIX}/lib
)